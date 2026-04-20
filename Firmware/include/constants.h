#pragma once

// Power accounting constants
// Recorded by Joe on NEVER
#define STANDBY_WATTS 9.0f

// Derrived Requirement Constants
#define T_DEADBAND_C 3.0f // Allowable deadband for temperature control

// Time Limits
#define MIN_COMPRESSOR_ON_TIME_MS 30000 // 30 seconds
#define MIN_COMPRESSOR_OFF_TIME_MS 120000 // 2 minutes
#define MAX_HEATER_ON_TIME_MS 30000 // The max amount of time the heater can be on for
#define MIN_RELAY_ON_TIME_MS 2000 // For any load that dosn't have its own min time

// Locked Rotor Constants
#define COMPRESSOR_STARTUP_TIME_MS 500 // Milliseconds to wait before interrogating compressure load for stall-check
#define LOCKED_ROTOR_THRESHOLD_A 15.0f // Amps to consider the compressor to be locked rotor

/* -----------------------
 * State Machine Constants
 * ----------------------- */
// Cooling
#define THERMO_COOL_FAST_ABOVE_SP_C 1.0f // COOL_SLOW -> COOL_FAST when chamber >= setpoint + this
#define THERMO_COOL_FAST_TO_SLOW_BELOW_SP_C 2.0f // COOL_FAST -> COOL_SLOW when chamber <= setpoint − this
#define THERMO_COOL_EXCLUDE_WITHIN_AMBIENT_C 5.0f // no cooling if |setpoint − ambient| <= this
#define THERMO_COOL_POST_HEAT_LOCKOUT_MS 10000u // block IDLE -> COOLING after leaving HEATING

// Dehumidify
#define MAX_HUMIDITY_BEFORE_ENTERING_FREEZE 60.0f // %RH: COOL_FAST->dehumidify when above this (with air > 0 C)
#define THERMO_DEHUMIDIFY_ENTRY_EVAP_MAX_C 8.0f // TDR1 evap must be <= this to enter dehumidify
#define THERMO_DEHUMIDIFY_TARGET_EVAP_C 2.0f // aim for evap (TDR1) near this (°C)
#define THERMO_DEHUMIDIFY_EVAP_BAND_C 0.75f // exit when |evap − target| <= this (after min dwell)
#define THERMO_DEHUMIDIFY_MIN_MS (10u * 60u * 1000u) // at least this long in dehumidify (ms)
#define THERMO_DEHUMIDIFY_MAX_MS (30u * 60u * 1000u) // safety cap if evap never reaches band (ms)

// Heating
#define THERMO_HEAT_SUBAMBIENT_MIN_BELOW_SP_C 5.0f // sp below ambient: idle->heat when chamber <= sp − this
#define HEATER_TC_OFFSET_ABOVE_AIR_SP_C 70.0f // Heater TC (TDR0) target = air setpoint + this
#define CONDENSER_HOT_HEADROOM_C 20.0f // Idle/heating: condenser on when (comp−amb) >= this
#define CONDENSER_HOT_HEADROOM_HYST_C 10.0f // Drop condenser when (comp−amb) <= HEADROOM − this

/* -----------------
 * Pins and Hardware
 * ----------------- */
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

// Safety Constants
#define SAFETY_POLL_MS 50 // ms between safety checks

/* --------------
 * Misc and Color
 * -------------- */
#define LED_TINT_NEUTRAL_C 20.0f
#define LED_TINT_COLDEST_C -20.0f
#define LED_TINT_HOTTEST_C 80.0f

/* Target colors at full tint (0..255 each). */
#define LED_TINT_COLD_R 0u
#define LED_TINT_COLD_G 10u
#define LED_TINT_COLD_B 255u
#define LED_TINT_HOT_R 255u
#define LED_TINT_HOT_G 10u
#define LED_TINT_HOT_B 0u

/* Max tint strength (0..1) and slew rate. */
#define LED_TINT_MAX_STRENGTH 0.90f
#define LED_TINT_SLEW_PER_SEC 0.60f
