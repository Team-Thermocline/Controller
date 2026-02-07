#pragma once

#include "FreeRTOS.h"
#include "neopixel_ws2812.h"
#include "task.h"
#include <stdbool.h>

typedef struct sim_thermo_system_config {
  float ambient_temp_c;
  float ambient_rh;

  float heat_ramp_c_per_s; // temperature rise rate when heater is on
  float passive_ramp_c_per_s; // drift rate toward ambient when idle
  float cool_ramp_c_per_s; // temperature fall rate when compressor is on

  TickType_t heat_on_delay_ticks;   // delay before heater turns on after request
  TickType_t heat_off_delay_ticks;  // delay before heater turns off after request

  TickType_t cool_on_delay_ticks;  // delay before compressor turns on
  TickType_t cool_off_delay_ticks; // delay before compressor turns off
  bool enable_active_cooling;

  float temp_hysteresis_c; // bang-bang hysteresis around setpoint

  float min_temp_c;
  float max_temp_c;

  // Optional: if set, task will set a solid status color.
  neopixel_ws2812_t *status_pixel;
  uint8_t color_idle[3];
  uint8_t color_heat[3];
  uint8_t color_cool[3];

  TickType_t update_period_ticks;
} sim_thermo_system_config_t;

// Creates the simulator thermo system task.
// The task updates:
// - current_temperature / current_humidity
// - heater_on (simulated output)
// - compressor_on (simulated output)
// - current_state (0=IDLE, 1=RUN)
//
// It uses current_temperature_setpoint / current_humidity_setpoint as inputs.
BaseType_t sim_thermo_system_task_create(const sim_thermo_system_config_t *cfg,
                                        UBaseType_t priority,
                                        TaskHandle_t *out_handle);

