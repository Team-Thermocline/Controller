#include "analog_task.h"
#include "ADG728.h"
#include "globals.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "pico/stdio.h"
#include "pindefs.h"
#include "task.h"
#include <math.h>
#include <stdio.h>

/* Math for 60hz AC sampling */
#define AC_HZ              60
#define SAMPLE_RATE_HZ      2000
#define NUM_PERIODS         3
#define NUM_SAMPLES         ((SAMPLE_RATE_HZ * NUM_PERIODS) / AC_HZ)
#define SAMPLE_INTERVAL_US  (1000000 / SAMPLE_RATE_HZ)

// Mux interval and poll settings
#define MUX_SETTLE_MS    2
#define POLL_INTERVAL_MS 500

/* 1000:1 CT current estimate: ADC ref and burden */
#define ADC_REF_V       3.3f
#define ADC_MAX_COUNTS  4096.f
#define CT_RATIO        1000
#define BURDEN_OHMS     68.f

/* Theres some slight nearby coupling to the sensors, this sets the zero noise floor */
#define CT_ZERO_THRESHOLD_A  0.05f

/* Thermocouple open/broken: amp reports ~ -245°C */
#define TDR_OPEN_THERMOCOUPLE_THRESHOLD_C  (-200.0f)

/**
 * Sample ADC over the configured AC window and compute DC mean and AC RMS.
 * Caller must have already selected the mux channel and waited for settle.
 * Fills buf[0..num_samples-1] and writes mean and rms (ADC counts) to *out_mean, *out_rms.
 */
static void sample_ac_rms(uint16_t *buf, int num_samples,
                          float *out_mean, float *out_rms) {
    adc_select_input(0); /* ADC0 = GPIO26 (ADC_TMUX_PIN) */
    for (int i = 0; i < num_samples; i++) {
        buf[i] = adc_read();
        busy_wait_us(SAMPLE_INTERVAL_US);
    }
    uint32_t sum = 0;
    for (int i = 0; i < num_samples; i++)
        sum += buf[i];
    float mean = (float)sum / (float)num_samples;
    float sum_sq = 0.f;
    for (int i = 0; i < num_samples; i++) {
        float d = (float)buf[i] - mean;
        sum_sq += d * d;
    }
    *out_mean = mean;
    *out_rms = sqrtf(sum_sq / (float)num_samples);
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

    adc_init();
    adc_gpio_init(ADC_TMUX_PIN);

    uint16_t samples[NUM_SAMPLES];
    float mean, rms_adc;

    // Check ADG728
    if (!adg728_probe(i2c, addr)) {
        FAULT = FAULT_CODE_I2C_COMMUNICATION_ERROR;
        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
    }

    while (true) {
        // Scan all current transformer channels CT0–CT3
        const uint8_t ct_channels[4] = {ADG_CH_CT0, ADG_CH_CT1, ADG_CH_CT2, ADG_CH_CT3};
        float *ct_amps[4] = {&ct0_amps, &ct1_amps, &ct2_amps, &ct3_amps};

        for (int i = 0; i < 4; i++) {
            if (!adg728_select_channel(i2c, addr, ct_channels[i])) {
                FAULT = FAULT_CODE_I2C_COMMUNICATION_ERROR;
                vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
                continue;
            }
            vTaskDelay(pdMS_TO_TICKS(MUX_SETTLE_MS));

            // Fill buf with AC samples
            sample_ac_rms(samples, NUM_SAMPLES, &mean, &rms_adc);

            // Convert ADC counts to current (A)
            float a = analog_rms_adc_to_primary_amps(rms_adc);

            // Account for noise floor
            *ct_amps[i] = (a < CT_ZERO_THRESHOLD_A) ? 0.0f : a;

            // Wait for next poll
            vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
        }

        // Read temperature sensor channels TDR0-3 in a loop
        const uint8_t tdr_channels[4] = {ADG_CH_TDR0, ADG_CH_TDR1, ADG_CH_TDR2, ADG_CH_TDR3};
        float *tdr_temperatures[4] = {&tdr0_temperature_c, &tdr1_temperature_c, &tdr2_temperature_c, &tdr3_temperature_c};
        bool any_open = false;

        for (int i = 0; i < 4; i++) {
            if (!adg728_select_channel(i2c, addr, tdr_channels[i])) {
                FAULT = FAULT_CODE_I2C_COMMUNICATION_ERROR;
                vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
                continue;
            }

            // Allow for mux to settle
            vTaskDelay(pdMS_TO_TICKS(MUX_SETTLE_MS));

            // Read temperature sensor ADC value
            uint16_t tdr_adc = adc_read();

            // Convert ADC value to voltage
            float tdr_voltage = ((float)tdr_adc / ADC_MAX_COUNTS) * ADC_REF_V;

            // Calculate temperature (5 mV/°C slope; open thermocouple ~ -245°C)
            float t_c = (tdr_voltage - 1.265f) / 0.005f;
            *(tdr_temperatures[i]) = t_c;

            if (t_c < TDR_OPEN_THERMOCOUPLE_THRESHOLD_C) {
                any_open = true;
            }

            vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
        }

        if (any_open) {
            FAULT = FAULT_CODE_THERMOCOUPLE_OPEN;
        } else if (FAULT == FAULT_CODE_THERMOCOUPLE_OPEN) {
            FAULT = FAULT_CODE_NONE;
        }
    }
}

BaseType_t analog_task_create(const analog_task_config_t *cfg,
                               UBaseType_t priority,
                               TaskHandle_t *out_handle) {
    return xTaskCreate(analog_task, "analog", 384, (void *)cfg, priority,
                       out_handle);
}
