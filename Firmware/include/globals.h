#pragma once

#include <stdbool.h>
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * Fault / alarm codes and string lookup
 * ----------------------------------------------------------------------------- */
typedef enum fault_code {
  FAULT_CODE_NONE = 0,
  FAULT_CODE_I2C_COMMUNICATION_ERROR = 1,
  /* Add further fault codes here and in fault_code_string() in globals.c */
} fault_code_t;

/** Human-readable string for a fault code (for logging, display, Q0). */
const char *fault_code_string(fault_code_t code);

/* -----------------------------------------------------------------------------
 * Run state (chamber controller state machine)
 * ----------------------------------------------------------------------------- */
typedef enum run_state {
  RUN_STATE_IDLE = 0,
  RUN_STATE_RUN = 1,
  RUN_STATE_STOP = 2,
  RUN_STATE_FAULT = 3,
} run_state_t;

/** Human-readable string for run state. */
const char *run_state_string(run_state_t state);

/* -----------------------------------------------------------------------------
 * Global state (defined in globals.c)
 * ----------------------------------------------------------------------------- */
extern fault_code_t FAULT;

extern float current_temperature_setpoint;
extern float current_humidity_setpoint;
extern float current_temperature;
extern float current_humidity;
extern bool heater_on;
extern bool compressor_on;
extern run_state_t current_state;
