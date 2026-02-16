#pragma once

#include "FreeRTOS.h"
#include "neopixel_ws2812.h"
#include "task.h"
#include <stdbool.h>

typedef struct thermo_control_config {
  float temp_hysteresis_c;  // hysteresis around setpoint
  bool enable_active_cooling;
  
  // Optional: if set, task will set a solid status color.
  neopixel_ws2812_t *status_pixel;
  uint8_t color_idle[3];
  uint8_t color_heat[3];
  uint8_t color_cool[3];

  TickType_t update_period_ticks;
} thermo_control_config_t;

// Creates the thermo control task.
// The task updates:
// - heater_on / compressor_on (load outputs)
// - current_state (STANDBY -> IDLE when setpoint set, then IDLE/RUN based on mode)
// - FAULT (cleared to NONE when running)
//
// It uses tdr0_temperature_c as temperature input and current_temperature_setpoint.
BaseType_t thermo_control_task_create(const thermo_control_config_t *cfg,
                                      UBaseType_t priority,
                                      TaskHandle_t *out_handle);

// Get compressor timing state (for debugging/monitoring)
TickType_t thermo_control_get_compressor_on_time(void);
TickType_t thermo_control_get_compressor_off_time(void);
