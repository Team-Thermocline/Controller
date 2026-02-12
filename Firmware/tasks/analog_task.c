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
        // Select Current Transformer Channel TODO: Loop through all current transformers ADG_CH_CT0, ADG_CH_CT1, ADG_CH_CT2 and ADG_CH_CT3
        if (!adg728_select_channel(i2c, addr, ADG_CH_CT0)) {
            FAULT = FAULT_CODE_I2C_COMMUNICATION_ERROR;
            vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));
            continue;
        }

        // Allow for mux to settle
        vTaskDelay(pdMS_TO_TICKS(MUX_SETTLE_MS));

        sample_ac_rms(samples, NUM_SAMPLES, &mean, &rms_adc);
        ct0_amps = analog_rms_adc_to_primary_amps(rms_adc);
        // printf("CT0: mean=%.1f rms=%.1f (ADC) ~ %.2f A\n", mean, rms_adc, ct0_amps);

        vTaskDelay(pdMS_TO_TICKS(POLL_INTERVAL_MS));

        // Read temperature sensor channels TDR0 and TDR1 in a loop
        const uint8_t tdr_channels[2] = {ADG_CH_TDR0, ADG_CH_TDR1};
        float *tdr_temperatures[2] = {&tdr0_temperature_c, &tdr1_temperature_c};

        for (int i = 0; i < 2; i++) {
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

            // Calculate temperature using T = (V - 1.25) / 0.005
            // Formula assumes sensor outputs 1.25V at 0°C, 0.005V/°C slope
            *(tdr_temperatures[i]) = (tdr_voltage - 1.265f) / 0.005f;

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
