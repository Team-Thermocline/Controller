#include "chamber_transition.h"

#include "constants.h"
#include "globals.h"

float chamber_air_temp_c(void) { return sht35_temperature_c; }

chamber_state_t chamber_transition(chamber_state_t cur, float chamber, float sp,
                                   float h, bool cool_en) {
  if (cur == CHAMBER_STANDBY || cur == CHAMBER_FAULT)
    return cur;

  const float cool_in = sp + T_DEADBAND_C;
  const float cool_out = sp - T_DEADBAND_C;
  const float cool_fast_in = cool_in + THERMO_COOL_FAST_EXTRA_C;
  // In FAST: stay fast while chamber is still above the cool-in line (sp+deadband);
  const float fast_down = cool_in;

  // If we're currently cooling..
  if (cur == CHAMBER_COOL_FAST || cur == CHAMBER_COOL_SLOW) {

    // ..chamber at or below setpoint − deadband, or cooling disabled → idle
    if (chamber <= cool_out || !cool_en)
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

  // If cooling is enabled and we're above the cool-in band, enter fast cool.
  // (Slow cool is reached by downshifting from fast near cool_in, not from idle here.)
  if (cool_en && chamber >= cool_in)
    return CHAMBER_COOL_FAST;

  // If the chamber is below the setpoint - hysteresis, return to heating
  if (chamber <= sp - h)
    return CHAMBER_HEATING;
  // Otherwise, return to idle
  return CHAMBER_IDLE;
}
