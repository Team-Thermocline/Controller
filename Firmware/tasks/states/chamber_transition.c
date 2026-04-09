#include "chamber_transition.h"

#include "constants.h"
#include "globals.h"

float chamber_air_temp_c(void) { return sht35_temperature_c; }

chamber_state_t chamber_transition(chamber_state_t cur, float chamber, float sp,
                                   float h, bool cool_en) {
  if (cur == CHAMBER_STANDBY || cur == CHAMBER_FAULT)
    return cur;

  const float cool_in = sp + THERMO_COOL_ENTRY_ABOVE_SP_C;
  const float cool_fast_in = cool_in + THERMO_COOL_FAST_EXTRA_C;
  const float fast_down = cool_fast_in - THERMO_COOL_FAST_DOWNSHIFT_C;

  // If we're currently cooling..
  if (cur == CHAMBER_COOL_FAST || cur == CHAMBER_COOL_SLOW) {

    // ..and the chamber is below the setpoint + hysteresis, or cooling is disabled, return to idle
    if (chamber <= sp + h || !cool_en)
      return CHAMBER_IDLE;

    // If we're currently cooling fast..
    if (cur == CHAMBER_COOL_FAST) {
      // ..then if the chamber is below the fast down threshold, return to slow cool
      if (chamber <= fast_down)
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

  // If cooling is enabled..
  if (cool_en && chamber >= cool_in) {
    // ..and the chamber is above the cool fast in threshold, return to fast cool
    if (chamber >= cool_fast_in)
      return CHAMBER_COOL_FAST;
    // Otherwise, return to slow cool
    return CHAMBER_COOL_SLOW;
  }

  // If the chamber is below the setpoint - hysteresis, return to heating
  if (chamber <= sp - h)
    return CHAMBER_HEATING;
  // Otherwise, return to idle
  return CHAMBER_IDLE;
}
