#pragma once

#include "chamber_context.h"
#include <stdbool.h>
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * Fault / alarm codes and string lookup
 * ----------------------------------------------------------------------------- */
typedef enum fault_code {
  FAULT_CODE_NONE = 0,
  FAULT_CODE_I2C_COMMUNICATION_ERROR = 1,
  FAULT_CODE_THERMOCOUPLE_OPEN = 2,
  FAULT_CODE_OVERCURRENT = 3,
  FAULT_CODE_ENV_SENSOR = 4,
} fault_code_t;

/** Human-readable string for a fault code (for logging, display, Q0). */
const char *fault_code_string(fault_code_t code);

/** Set global FAULT (NONE clears). Thermo task drives CHAMBER_FAULT from this. */
void fault_raise(fault_code_t code);

/** Human-readable string for chamber FSM state (Q0, logging). */
const char *chamber_state_string(chamber_state_t state);

/** Posted to thermo task (M0, fault hook, T setpoint from standby). */
void chamber_request_standby(void);
void chamber_request_arm_idle(void);

/* -----------------------------------------------------------------------------
 * Global state (defined in globals.c)
 * ----------------------------------------------------------------------------- */
extern fault_code_t FAULT;

// Setpoints
extern float current_temperature_setpoint;
extern float current_humidity_setpoint;
extern float current_temperature; // TODO: make this explicit/use this by setting or aggregating sensors
extern float current_humidity; // TODO: make this explicit/use this by setting or aggregating sensors

// Outputs
extern bool heater_on;
extern bool compressor_on;

/** Chamber FSM; written only by thermo_control_task, readable anywhere */
extern volatile chamber_state_t chamber_fsm_state;

// Global Sensor States
extern volatile float ct0_amps;
extern volatile float ct1_amps;
extern volatile float ct2_amps;
extern volatile float ct3_amps;
extern volatile float current_power;
extern volatile float tdr0_temperature_c;
extern volatile float tdr1_temperature_c;
extern volatile float tdr2_temperature_c;
extern volatile float tdr3_temperature_c;
extern volatile float sht35_temperature_c;
extern volatile float sht35_humidity;

// Door state, true when shut.
extern volatile bool door_open;
