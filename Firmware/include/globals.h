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
  RUN_STATE_STANDBY = 0, // Totally standby, all systems off and no automatic logic
  RUN_STATE_IDLE = 1, // Automatic logic is ready, choose to be idle
  RUN_STATE_RUN = 2, // We're running, either heating or cooling
  RUN_STATE_STOP = 3, // We're instructed to stop (move to standby quickly)
  RUN_STATE_FAULT = 4, // Theres a fault, stay here, then when cleared move to standby
} run_state_t;

/** Human-readable string for run state. */
const char *run_state_string(run_state_t state);

/* -----------------------------------------------------------------------------
 * Global state (defined in globals.c)
 * ----------------------------------------------------------------------------- */
extern fault_code_t FAULT;

// Setpoints
extern float current_temperature_setpoint;
extern float current_humidity_setpoint;
extern float current_temperature;
extern float current_humidity;

// Outputs
extern bool heater_on;
extern bool compressor_on;
extern run_state_t current_state;

// Debug/Monitoring
extern float ct0_amps;
extern float ct1_amps;
extern float ct2_amps;
extern float ct3_amps;
extern float tdr0_temperature_c;
extern float tdr1_temperature_c;
extern float tdr2_temperature_c;
extern float tdr3_temperature_c;
