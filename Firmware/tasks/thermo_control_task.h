#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

typedef struct thermo_control_config {
  float temp_hysteresis_c; // Band for heat entry (chamber <= sp - h) and cool exit (chamber <= sp + h)
  bool enable_active_cooling;

  /** Bang-bang half-band on heater block TC (TDR0) around (air_sp + HEATER_TC_OFFSET). */
  float heater_tc_hysteresis_c;

  TickType_t update_period_ticks;
} thermo_control_config_t;

// Creates the thermo control task.
BaseType_t thermo_control_task_create(const thermo_control_config_t *cfg,
                                      UBaseType_t priority,
                                      TaskHandle_t *out_handle);

// Get compressor timing state (for debugging/monitoring)
TickType_t thermo_control_get_compressor_on_time(void);
TickType_t thermo_control_get_compressor_off_time(void);
