#pragma once

// Power accounting constants
// Recorded by Joe on NEVER
#define STANDBY_WATTS 9.0f

// Time Limits
#define MIN_COMPRESSOR_ON_TIME_MS 60000 // 1 minute
#define MIN_COMPRESSOR_OFF_TIME_MS 180000 // 3 minutes
#define MAX_HEATER_ON_TIME_MS 30000 // The max amount of time the heater can be on for
#define MIN_RELAY_ON_TIME_MS 2000 // For any load that dosn't have its own min time

// State Machine Constants
#define THERMO_COOL_ENTRY_ABOVE_SP_C 5.0f // Require this much above air setpoint before cooling
#define THERMO_COOL_FAST_EXTRA_C 5.0f // Above cool-entry threshold to run internal fan (fast cool)
#define THERMO_COOL_FAST_DOWNSHIFT_C 1.0f // Hysteresis when dropping from fast -> slow cool
#define HEATER_TC_OFFSET_ABOVE_AIR_SP_C 70.0f // Heater TC (TDR0) target = air setpoint + this

