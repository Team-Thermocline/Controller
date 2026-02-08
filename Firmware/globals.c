#include "globals.h"

/* -----------------------------------------------------------------------------
 * Fault / alarm strings (must match fault_code_t enum order)
 * ----------------------------------------------------------------------------- */
static const char *const fault_strings[] = {
    [FAULT_CODE_NONE] = "NONE",
    [FAULT_CODE_I2C_COMMUNICATION_ERROR] = "I2C_COMMUNICATION_ERROR",
};

const char *fault_code_string(fault_code_t code) {
  if ((unsigned)code >= sizeof(fault_strings) / sizeof(fault_strings[0]))
    return "UNKNOWN";
  const char *s = fault_strings[code];
  return s ? s : "UNKNOWN";
}

/* -----------------------------------------------------------------------------
 * Run state strings
 * ----------------------------------------------------------------------------- */
static const char *const run_state_strings[] = {
    [RUN_STATE_IDLE] = "IDLE",
    [RUN_STATE_RUN] = "RUN",
    [RUN_STATE_STOP] = "STOP",
    [RUN_STATE_FAULT] = "FAULT",
};

const char *run_state_string(run_state_t state) {
  if ((unsigned)state >= sizeof(run_state_strings) / sizeof(run_state_strings[0]))
    return "UNKNOWN";
  const char *s = run_state_strings[state];
  return s ? s : "UNKNOWN";
}

/* -----------------------------------------------------------------------------
 * Global definitions
 * ----------------------------------------------------------------------------- */
fault_code_t FAULT = FAULT_CODE_NONE;

float current_temperature_setpoint = 20.0f;
float current_humidity_setpoint = 100.0f;
float current_temperature = 0.0f;
float current_humidity = 0.0f;
bool heater_on = false;
bool compressor_on = false;
run_state_t current_state = RUN_STATE_IDLE;
