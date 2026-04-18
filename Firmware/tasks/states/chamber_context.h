#pragma once

#include "FreeRTOS.h"
#include "thermo_control_task.h"
#include <stdbool.h>

typedef enum chamber_state {
  CHAMBER_STANDBY = 0,
  CHAMBER_FAULT,
  CHAMBER_IDLE,
  CHAMBER_HEATING,
  CHAMBER_COOL_SLOW,
  CHAMBER_COOL_FAST,
  CHAMBER_DEHUMIDIFY,
} chamber_state_t;

typedef struct chamber_context {
  const thermo_control_config_t *cfg;
  TickType_t now;
  float air_sp;
  float chamber;
  /** Set on each state entry in chamber_dispatch() (for entry dwell in run()). */
  TickType_t state_entered_at;
} chamber_context_t;
