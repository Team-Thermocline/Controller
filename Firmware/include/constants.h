#pragma once

// Power accounting constants
// Recorded by Joe on NEVER
#define STANDBY_WATTS 9.0f

// Time Limits
#define MIN_COMPRESSOR_ON_TIME_MS 60000 // 1 minute
#define MIN_COMPRESSOR_OFF_TIME_MS 180000 // 3 minutes
#define MAX_HEATER_ON_TIME_MS 30000 // The max amount of time the heater can be on for
#define MIN_RELAY_ON_TIME_MS 2000 // For any load that dosn't have its own min time

// Locked Rotor Constants
#define COMPRESSOR_STARTUP_TIME_MS 500 // Milliseconds to wait before interrogating compressure load for stall-check
#define LOCKED_ROTOR_THRESHOLD_A 15.0f // Amps to consider the compressor to be locked rotor

// State Machine Constants
#define THERMO_COOL_ENTRY_ABOVE_SP_C 5.0f // Require this much above air setpoint before cooling
#define THERMO_COOL_FAST_EXTRA_C 5.0f // Above cool-entry threshold to run internal fan (fast cool)
#define THERMO_COOL_FAST_DOWNSHIFT_C 1.0f // Hysteresis when dropping from fast -> slow cool
#define HEATER_TC_OFFSET_ABOVE_AIR_SP_C 70.0f // Heater TC (TDR0) target = air setpoint + this
#define CONDENSER_HOT_HEADROOM_C 20.0f // Idle/heating: condenser on when (comp−amb) >= this
#define CONDENSER_HOT_HEADROOM_HYST_C 10.0f // Drop condenser when (comp−amb) <= HEADROOM − this

// Connected Load Functions
#define THERMO_COMPRESSOR_PIN    LOAD_PIN_2
#define THERMO_HEAT_PIN          LOAD_PIN_1
#define THERMO_CONDENSER_FAN_PIN LOAD_PIN_3
#define THERMO_INTERNAL_FAN_PIN  LOAD_PIN_4

// Connected Current Monitoring functions
#define COMPRESSOR_LOAD (ct2_amps)
#define HEATER_LOAD (ct0_amps)
#define CONDENSER_FAN_LOAD (ct3_amps)
#define INTERNAL_FAN_LOAD (ct1_amps)

// Named Thermal Channels
#define COMPRESSOR_TEMP (tdr2_temperature_c)
#define HEATER_TEMP (tdr0_temperature_c)
#define EVAPORATOR_TEMP (tdr1_temperature_c)
#define AMBIENT_TEMP (tdr3_temperature_c)

// SHT35 sensor tolerance
#define ENV_SENSOR_FAIL_STREAK 3 // Number of missed updates in a row
#define ENV_SENSOR_RETRY_DELAY_MS 5 // Time to delay to retry