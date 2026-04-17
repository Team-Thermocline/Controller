#pragma once

#include "chamber_context.h"

float chamber_air_temp_c(void);

/*
 * Cooling: idle→COOL_SLOW when chamber >= sp; idle→COOL_FAST when >= sp + THERMO_COOL_FAST_ABOVE_SP_C.
 * FAST→SLOW when chamber <= sp − THERMO_COOL_FAST_TO_SLOW_BELOW_SP_C. SLOW→FAST at sp + FAST_ABOVE.
 * Exit cooling to idle at sp − T_DEADBAND_C. COOL_SLOW: compressor off, internal fan on (evap bleed).
 * No cooling when |chamber − ambient| <= THERMO_COOL_EXCLUDE_WITHIN_AMBIENT_C (valid ambient TDR3).
 * Heating from idle: chamber <= sp − h; if sp is sub-ambient (sp < valid ambient T) then chamber < sp − THERMO_HEAT_SUBAMBIENT_MIN_BELOW_SP_C.
 */
chamber_state_t chamber_transition(chamber_state_t cur, float chamber, float sp,
                                 float h, bool cool_en);
