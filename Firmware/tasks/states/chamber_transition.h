#pragma once

#include "chamber_context.h"

float chamber_air_temp_c(void);

/*
 * Cooling: from idle, enter COOL_FAST when chamber >= sp + T_DEADBAND_C; COOL_SLOW only after
 * FAST when chamber <= that same threshold. Upshift slow→fast at sp + T_DEADBAND + THERMO_COOL_FAST_EXTRA_C.
 * Exit cooling at sp − T_DEADBAND_C. Heating: chamber <= sp − h.
 */
chamber_state_t chamber_transition(chamber_state_t cur, float chamber, float sp,
                                 float h, bool cool_en);
