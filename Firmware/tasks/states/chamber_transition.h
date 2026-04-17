#pragma once

#include "chamber_context.h"

float chamber_air_temp_c(void);

/*
 * Cooling: idle→COOL_SLOW when chamber >= sp + T_DEADBAND_C; idle→COOL_FAST when >= that + FAST_ABOVE.
 * inhibit_cooling_entry: suppress idle→cool only (e.g. post-heat lockout). FAST→SLOW / SLOW→FAST unchanged.
 * Exit cooling to idle at sp − T_DEADBAND_C. COOL_SLOW: compressor off, internal fan on (evap bleed).
 * No cooling when |setpoint − ambient| <= THERMO_COOL_EXCLUDE_WITHIN_AMBIENT_C (valid TDR3).
 * Heating from idle: chamber <= sp − h; if sp is sub-ambient (sp < valid ambient T) then chamber < sp − THERMO_HEAT_SUBAMBIENT_MIN_BELOW_SP_C.
 */
chamber_state_t chamber_transition(chamber_state_t cur, float chamber, float sp,
                                   float h, bool cool_en,
                                   bool inhibit_cooling_entry);
