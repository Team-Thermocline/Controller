#include "chamber_transition.h"

#include "analog_task.h"
#include "constants.h"
#include "globals.h"

#include <math.h>

float chamber_air_temp_c(void) { return sht35_temperature_c; }

chamber_state_t chamber_transition(chamber_state_t cur, float chamber, float sp,
                                   float h, bool cool_en) {
  if (cur == CHAMBER_STANDBY || cur == CHAMBER_FAULT)
    return cur;

  const float cool_out = sp - T_DEADBAND_C; // idle: 3 °C below setpoint
  const float cool_fast_in = sp + THERMO_COOL_FAST_ABOVE_SP_C;
  const float fast_to_slow = sp - THERMO_COOL_FAST_TO_SLOW_BELOW_SP_C;
  const bool sub_ambient_sp = tdr_temperature_c_valid(tdr3_temperature_c) &&
                              sp < tdr3_temperature_c;
  const bool near_ambient =
      tdr_temperature_c_valid(tdr3_temperature_c) &&
      fabsf(chamber - tdr3_temperature_c) <= THERMO_COOL_EXCLUDE_WITHIN_AMBIENT_C;
  const bool cool_allowed = cool_en && !near_ambient;

  // If we're currently cooling..
  if (cur == CHAMBER_COOL_FAST || cur == CHAMBER_COOL_SLOW) {

    // ..chamber at or below setpoint − 3 °C, cooling disabled, or near ambient → idle
    if (chamber <= cool_out || !cool_allowed)
      return CHAMBER_IDLE;

    // If we're currently cooling fast..
    if (cur == CHAMBER_COOL_FAST) {
      if (chamber <= fast_to_slow)
        return CHAMBER_COOL_SLOW;
      // Otherwise, stay in fast cool
      return CHAMBER_COOL_FAST;
    }

    // If the chamber is above the cool fast in threshold, return to fast cool
    if (chamber >= cool_fast_in)
      return CHAMBER_COOL_FAST;
    return CHAMBER_COOL_SLOW;
  }

  // If we're currently heating..
  if (cur == CHAMBER_HEATING) {
    // ..and the chamber is above the setpoint, return to idle
    if (chamber >= sp)
      return CHAMBER_IDLE;
    return CHAMBER_HEATING;
  }

  if (cool_allowed && chamber >= cool_fast_in)
    return CHAMBER_COOL_FAST;
  if (cool_allowed && chamber >= sp)
    return CHAMBER_COOL_SLOW;

  if (sub_ambient_sp) {
    if (chamber < sp - THERMO_HEAT_SUBAMBIENT_MIN_BELOW_SP_C)
      return CHAMBER_HEATING;
  } else if (chamber <= sp - h) {
    return CHAMBER_HEATING;
  }
  // Otherwise, return to idle
  return CHAMBER_IDLE;
}
