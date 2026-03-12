#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

struct i2c_inst;

/*
 * ADG728 mux channel mapping (channel index 0..7).
 * CT0-3: S4, S3, S2, S1 in order.
 * TDR0-3: S5, S6, S7, S8 in order.
 */
#define ADG_CH_CT0  3u  /* S4 */
#define ADG_CH_CT1  1u  /* S2 */
#define ADG_CH_CT2  2u  /* S3 */
#define ADG_CH_CT3  0u  /* S1 */
#define ADG_CH_TDR0 4u  /* S5 */
#define ADG_CH_TDR1 5u  /* S6 */
#define ADG_CH_TDR2 6u  /* S7 */
#define ADG_CH_TDR3 7u  /* S8 */

typedef struct analog_task_config {
    struct i2c_inst *i2c;
    uint8_t adg728_addr;
} analog_task_config_t;

BaseType_t analog_task_create(const analog_task_config_t *cfg,
                              UBaseType_t priority,
                              TaskHandle_t *out_handle);

/**
 * Estimate primary RMS current (A) from CT secondary RMS in ADC counts.
 * Assumes 1000:1 CT and circuit scaling (burden, ref) – tune BURDEN_OHMS in .c to match hardware.
 */
float analog_rms_adc_to_primary_amps(float rms_adc);
