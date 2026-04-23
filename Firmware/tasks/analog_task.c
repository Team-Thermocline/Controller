#include "analog_task.h"
#include "ADG728.h"
#include "calibration.h"
#include "chamber_outputs.h"
#include "globals.h"
#include "constants.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "pindefs.h"
#include "sht35.h"
#include "task.h"
#include <math.h>

/* Math for 60hz AC sampling */
#define AC_HZ              60
#define SAMPLE_RATE_HZ      2000
#define NUM_PERIODS         3
#define NUM_SAMPLES         ((SAMPLE_RATE_HZ * NUM_PERIODS) / AC_HZ)
#define SAMPLE_INTERVAL_US  (1000000 / SAMPLE_RATE_HZ)

// Mux interval and poll settings
#define MUX_SETTLE_MS    2
#define POLL_INTERVAL_MS 45

/* 1000:1 CT current estimate: ADC ref and burden */
#define ADC_REF_V       3.3f
#define ADC_MAX_COUNTS  4096.f
#define CT_RATIO        1000
#define BURDEN_OHMS     68.f

/* Thermocouple open/broken: amp reports ~ -245°C */
#define TDR_OPEN_THERMOCOUPLE_THRESHOLD_C  (-200.0f)

/* In-range TDR for control (distinct from open-TC fault threshold). */
#define TDR_VALID_MIN_C  (-80.0f)
#define TDR_VALID_MAX_C  (600.0f)

bool tdr_temperature_c_valid(float t_c) {
    return t_c > TDR_VALID_MIN_C && t_c < TDR_VALID_MAX_C && t_c != 0.0f;
}

/**
 * Capture ADC0 at SAMPLE_RATE_HZ for an integer number
 * of 60 Hz line periods (see NUM_SAMPLES).
 *
 * Idea Credit: Professor Monk
 */
static void adc_sample_line_sync_window(uint16_t *buf, int num_samples) {
    adc_select_input(0); /* ADC0 = GPIO26 (ADC_TMUX_PIN) */
    for (int i = 0; i < num_samples; i++) {
        buf[i] = adc_read();
        busy_wait_us(SAMPLE_INTERVAL_US);
    }
}

static float adc_buffer_mean_counts(const uint16_t *buf, int num_samples) {
    uint32_t sum = 0;
    for (int i = 0; i < num_samples; i++)
        sum += buf[i];
    return (float)sum / (float)num_samples;
}

static float adc_buffer_ac_rms_counts(const uint16_t *buf, int num_samples,
                                        float mean) {
    float sum_sq = 0.f;
    for (int i = 0; i < num_samples; i++) {
        float d = (float)buf[i] - mean;
        sum_sq += d * d;
    }
    return sqrtf(sum_sq / (float)num_samples);
}

/**
 * Line-sync capture plus DC mean and AC RMS (ADC counts). For CT channels.
 */
static void sample_ac_rms(uint16_t *buf, int num_samples, float *out_mean,
                          float *out_rms) {
    adc_sample_line_sync_window(buf, num_samples);
    *out_mean = adc_buffer_mean_counts(buf, num_samples);
    *out_rms = adc_buffer_ac_rms_counts(buf, num_samples, *out_mean);
}

float analog_rms_adc_to_primary_amps(float rms_adc) {
    float v_rms = (rms_adc / ADC_MAX_COUNTS) * ADC_REF_V;
    float i_secondary_rms = v_rms / BURDEN_OHMS;
    return i_secondary_rms * (float)CT_RATIO;
}

static void analog_task(void *pvParameters) {
    const analog_task_config_t *cfg = (const analog_task_config_t *)pvParameters;
    i2c_inst_t *i2c = cfg->i2c;
    uint8_t addr = cfg->adg728_addr;

    /* Init SHT35 once before ADC; read at end of loop after mux (see below). */
    sht35_t sht;
    sht35_init(&sht, i2c, SHT35_DEFAULT_ADDR);

    adc_init();
    adc_gpio_init(ADC_TMUX_PIN);

    uint16_t samples[NUM_SAMPLES];
    float mean, rms_adc;
    uint8_t env_sensor_fail_streak = 0;

    // Check ADG728
    if (!adg728_probe(i2c, addr)) {
        fault_raise(FAULT_CODE_I2C_COMMUNICATION_ERROR);
        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
    }

    while (true) {
        // Scan all current transformer channels CT0–CT3
        const uint8_t ct_channels[4] = {ADG_CH_CT0, ADG_CH_CT1, ADG_CH_CT2, ADG_CH_CT3};
        volatile float *ct_amps[4] = {&ct0_amps, &ct1_amps, &ct2_amps, &ct3_amps};

        for (int i = 0; i < 4; i++) {
            if (!adg728_select_channel(i2c, addr, ct_channels[i])) {
                fault_raise(FAULT_CODE_I2C_COMMUNICATION_ERROR);
                vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
                continue;
            }
            vTaskDelay(pdMS_TO_TICKS(MUX_SETTLE_MS));

            // Fill buf with AC samples
            sample_ac_rms(samples, NUM_SAMPLES, &mean, &rms_adc);

            // Convert ADC counts to current (A)
            float a = analog_rms_adc_to_primary_amps(rms_adc);

            /* Below minimum resolvable load, treat as zero */
            *ct_amps[i] = (a < MINIMUM_POSSIBLE_LOAD_A) ? 0.0f : a;

            // Wait for next poll
            vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
        }

        {
            TickType_t now_ticks = xTaskGetTickCount(); // Get the current time
            if (compressor_on) { // If the compressor is on
                TickType_t t_on = chamber_outputs_compressor_on_time(); // Get the time the compressor was turned on
                if (t_on != 0 && (now_ticks - t_on) >= // If the compressor has been on for longer than the startup time
                                    pdMS_TO_TICKS(COMPRESSOR_STARTUP_TIME_MS)) {

                    // If the compressor load is greater than the locked rotor threshold, raise a fault
                    if (COMPRESSOR_LOAD > LOCKED_ROTOR_THRESHOLD_A)
                        fault_raise(FAULT_CODE_COMPRESSOR_OVERCURRENT);
                }
            } else if (FAULT == FAULT_CODE_COMPRESSOR_OVERCURRENT) {
                // Only clear the fault if the startup time was MIN_COMPRESSOR_OFF_TIME_MS ago
                TickType_t t_off = chamber_outputs_compressor_off_time();
                if (t_off != 0 && (now_ticks - t_off) >= pdMS_TO_TICKS(MIN_COMPRESSOR_OFF_TIME_MS)) {
                    fault_raise(FAULT_CODE_NONE);
                }
            } // Compressor is not on so do nothing
    
        }

        // Compute total power in watts
        current_power =
            (ct0_amps + ct1_amps + ct2_amps + ct3_amps) * 120.0f + STANDBY_WATTS;

        /* TDR0 only for now (mux channels 4–7 available in tdr_channels). */
        const uint8_t tdr_channels[4] = {ADG_CH_TDR0, ADG_CH_TDR1, ADG_CH_TDR2,
                                         ADG_CH_TDR3};
        static const float tdr_offset_c[4] = {
            TDR0_OFFSET_C, TDR1_OFFSET_C, TDR2_OFFSET_C, TDR3_OFFSET_C};
        volatile float *tdr_temperatures[4] = {&tdr0_temperature_c, &tdr1_temperature_c, &tdr2_temperature_c, &tdr3_temperature_c};
        bool any_open = false;

        for (int i = 0; i < 4; i++) {
            if (!adg728_select_channel(i2c, addr, tdr_channels[i])) {
                fault_raise(FAULT_CODE_I2C_COMMUNICATION_ERROR);
                vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
                continue;
            }

            // Allow for mux to settle
            vTaskDelay(pdMS_TO_TICKS(MUX_SETTLE_MS));

            /* Average over line cycles so 60 Hz pickup tends to zero; DC is TC voltage */
            adc_sample_line_sync_window(samples, NUM_SAMPLES);
            float tdr_mean = adc_buffer_mean_counts(samples, NUM_SAMPLES);

            float tdr_voltage = (tdr_mean / ADC_MAX_COUNTS) * ADC_REF_V;

            // Calculate temperature (5 mV/°C slope; open thermocouple ~ -245°C)
            float t_c = (tdr_voltage - 1.265f) / 0.005f;
            float t_cal = t_c - tdr_offset_c[i];
            *(tdr_temperatures[i]) = t_cal;

            if (t_cal < TDR_OPEN_THERMOCOUPLE_THRESHOLD_C) {
                any_open = true;
            }

            vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
        }

        if (any_open) {
            fault_raise(FAULT_CODE_THERMOCOUPLE_OPEN);
        } else if (FAULT == FAULT_CODE_THERMOCOUPLE_OPEN) {
            fault_raise(FAULT_CODE_NONE);
        }

        float sht_t = 0.0f;
        float sht_rh = 0.0f;
        bool sht_ok = sht35_read_single_shot(&sht, &sht_t, &sht_rh);
        if (!sht_ok) {
            vTaskDelay(pdMS_TO_TICKS(ENV_SENSOR_RETRY_DELAY_MS));
            sht_ok = sht35_read_single_shot(&sht, &sht_t, &sht_rh);
        }
        if (sht_ok) {
            env_sensor_fail_streak = 0;
            sht35_temperature_c = sht_t;
            sht35_humidity = sht_rh;
            if (FAULT == FAULT_CODE_ENV_SENSOR)
                fault_raise(FAULT_CODE_NONE);
        } else {
            const bool env_fault_context =
                (FAULT == FAULT_CODE_NONE || FAULT == FAULT_CODE_ENV_SENSOR);
            if (env_fault_context) {
                env_sensor_fail_streak++;
                if (env_sensor_fail_streak >= ENV_SENSOR_FAIL_STREAK)
                    fault_raise(FAULT_CODE_ENV_SENSOR);
            }
            vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
        }
    }
}

BaseType_t analog_task_create(const analog_task_config_t *cfg,
                               UBaseType_t priority,
                               TaskHandle_t *out_handle) {
    return xTaskCreate(analog_task, "analog", 384, (void *)cfg, priority,
                       out_handle);
}
