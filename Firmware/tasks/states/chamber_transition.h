#pragma once

#include "chamber_context.h"

float chamber_air_temp_c(void);
float chamber_air_rh_pct(void);

/*
 * Cooling: idle->COOL_SLOW when chamber >= sp + T_DEADBAND_C; idle->COOL_FAST when >= that + FAST_ABOVE.
 * COOL_SLOW->COOL_FAST when chamber >= sp + THERMO_COOL_FAST_ABOVE_SP_C.
 * inhibit_cooling_entry: suppress idle->cool only (e.g. post-heat lockout). FAST->SLOW / SLOW->FAST unchanged.
 * Exit cooling to idle at sp − T_DEADBAND_C. COOL_SLOW: compressor off, internal fan on (evap bleed).
 * No cooling when |setpoint − ambient| <= THERMO_COOL_EXCLUDE_WITHIN_AMBIENT_C (valid TDR3).
 * Heating from idle: chamber <= sp − h; if sp is sub-ambient (sp < valid ambient T) then chamber < sp − THERMO_HEAT_SUBAMBIENT_MIN_BELOW_SP_C.
 * DEHUMIDIFY: from COOL_FAST when chamber air > 0 C, RH > MAX_HUMIDITY_BEFORE_ENTERING_FREEZE, and TDR1 evap
 * <= THERMO_DEHUMIDIFY_ENTRY_EVAP_MAX_C (cold coil). Dwell: internal fan,
 * compressor off, until THERMO_DEHUMIDIFY_MIN_MS and |evap − THERMO_DEHUMIDIFY_TARGET_EVAP_C| <= THERMO_DEHUMIDIFY_EVAP_BAND_C,
 * or THERMO_DEHUMIDIFY_MAX_MS; then COOL_FAST.
 * state_entered_at must be the tick when the current state was entered (persist across thermo ticks).
 */
chamber_state_t chamber_transition(chamber_state_t cur, float chamber, float sp,
                                   float h, bool cool_en,
                                   bool inhibit_cooling_entry, TickType_t now,
                                   TickType_t state_entered_at);
