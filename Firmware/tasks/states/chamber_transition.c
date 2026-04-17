#include "chamber_transition.h"

#include "analog_task.h"
#include "constants.h"
#include "globals.h"

#include <math.h>

float chamber_air_temp_c(void) { return sht35_temperature_c; }

chamber_state_t chamber_transition(chamber_state_t cur, float chamber, float sp,
                                   float h, bool cool_en,
                                   bool inhibit_cooling_entry, TickType_t now,
                                   TickType_t state_entered_at) {
  if (cur == CHAMBER_STANDBY || cur == CHAMBER_FAULT)
    return cur;

  const float cool_out = sp - T_DEADBAND_C; // idle: 3 °C below setpoint
  const float cool_in = sp + T_DEADBAND_C; // idle→cool only above deadband (symmetric to cool_out)
  const float cool_fast_in = cool_in + THERMO_COOL_FAST_ABOVE_SP_C;
  const float fast_to_slow = sp - THERMO_COOL_FAST_TO_SLOW_BELOW_SP_C;
  const bool amb_ok = tdr_temperature_c_valid(tdr3_temperature_c);
  const float amb = tdr3_temperature_c;
  const bool sub_ambient_sp = amb_ok && sp < amb;
  const bool sp_near_ambient =
      amb_ok && fabsf(sp - amb) <= THERMO_COOL_EXCLUDE_WITHIN_AMBIENT_C;
  const bool cool_allowed = cool_en && !sp_near_ambient;

  if (cur == CHAMBER_DEFROST) {
    if (!cool_allowed)
      return CHAMBER_IDLE;
    const bool evap_warm_enough =
        tdr_temperature_c_valid(EVAPORATOR_TEMP) &&
        EVAPORATOR_TEMP > THERMO_DEFROST_EXIT_EVAP_ABOVE_C;
    const bool defrost_timeout =
        (TickType_t)(now - state_entered_at) >=
        pdMS_TO_TICKS(THERMO_DEFROST_MAX_MS);
    if (evap_warm_enough || defrost_timeout)
      return CHAMBER_COOL_FAST;
    return CHAMBER_DEFROST;
  }

  // If we're currently cooling..
  if (cur == CHAMBER_COOL_FAST || cur == CHAMBER_COOL_SLOW) {

    // ..chamber at or below setpoint − 3 °C, cooling disabled, or setpoint ~ambient → idle
    if (chamber <= cool_out || !cool_allowed)
      return CHAMBER_IDLE;

    // If we're currently cooling fast..
    if (cur == CHAMBER_COOL_FAST) {
      // If the Evaporator - Chamber differential is greater than the defrost threshold, move to defrost
      if (tdr_temperature_c_valid(EVAPORATOR_TEMP) &&
          fabsf(EVAPORATOR_TEMP - chamber) > THERMO_DEFROST_EVAP_AIR_DELTA_C)
        return CHAMBER_DEFROST;
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

  if (cool_allowed && !inhibit_cooling_entry && chamber >= cool_fast_in)
    return CHAMBER_COOL_FAST;
  if (cool_allowed && !inhibit_cooling_entry && chamber >= cool_in)
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
