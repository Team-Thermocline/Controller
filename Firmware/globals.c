#include "globals.h"

/* -----------------------------------------------------------------------------
 * Fault / alarm strings (must match fault_code_t enum order)
 * ----------------------------------------------------------------------------- */
static const char *const fault_strings[] = {
    [FAULT_CODE_NONE] = "NONE",
    [FAULT_CODE_I2C_COMMUNICATION_ERROR] = "I2C_COMMUNICATION_ERROR",
    [FAULT_CODE_THERMOCOUPLE_OPEN] = "THERMOCOUPLE_OPEN",
    [FAULT_CODE_OVERCURRENT] = "OVERCURRENT",
    [FAULT_CODE_ENV_SENSOR] = "ENV_SENSOR",
    [FAULT_CODE_COMPRESSOR_OVERCURRENT] = "COMPRESSOR_OVERCURRENT",
};

const char *fault_code_string(fault_code_t code) {
  if ((unsigned)code >= sizeof(fault_strings) / sizeof(fault_strings[0]))
    return "UNKNOWN";
  const char *s = fault_strings[code];
  return s ? s : "UNKNOWN";
}

/* -----------------------------------------------------------------------------
 * Chamber FSM strings
 * ----------------------------------------------------------------------------- */
static const char *const chamber_state_strings[] = {
    [CHAMBER_STANDBY] = "STANDBY",
    [CHAMBER_FAULT] = "FAULT",
    [CHAMBER_IDLE] = "IDLE",
    [CHAMBER_HEATING] = "HEATING",
    [CHAMBER_COOL_SLOW] = "COOL_SLOW",
    [CHAMBER_COOL_FAST] = "COOL_FAST",
    [CHAMBER_DEFROST] = "DEFROST",
};

const char *chamber_state_string(chamber_state_t state) {
  if ((unsigned)state >=
      sizeof(chamber_state_strings) / sizeof(chamber_state_strings[0]))
    return "UNKNOWN";
  const char *s = chamber_state_strings[state];
  return s ? s : "UNKNOWN";
}

/* -----------------------------------------------------------------------------
 * Global definitions
 * ----------------------------------------------------------------------------- */
fault_code_t FAULT = FAULT_CODE_NONE;

void fault_raise(fault_code_t code) { FAULT = code; }

// Setpoints
float current_temperature_setpoint = 20.0f;
float current_humidity_setpoint = 100.0f;
float current_temperature = 0.0f;
float current_humidity = 0.0f;

// Outputs
bool heater_on = false;
bool compressor_on = false;

volatile chamber_state_t chamber_fsm_state = CHAMBER_STANDBY;

volatile bool chamber_post_standby = false;
volatile bool chamber_post_arm_idle = false;

void chamber_request_standby(void) { chamber_post_standby = true; }

void chamber_request_arm_idle(void) { chamber_post_arm_idle = true; }

// Global Sensor States
volatile float ct0_amps = 0.0f;
volatile float ct1_amps = 0.0f;
volatile float ct2_amps = 0.0f;
volatile float ct3_amps = 0.0f;
volatile float current_power = 0.0f;
volatile float tdr0_temperature_c = 0.0f;
volatile float tdr1_temperature_c = 0.0f;
volatile float tdr2_temperature_c = 0.0f;
volatile float tdr3_temperature_c = 0.0f;
volatile float sht35_temperature_c = 0.0f;
volatile float sht35_humidity = 0.0f;

// Door state, true when shut.
volatile bool door_open = false;