#pragma once

#include "chamber_context.h"

float chamber_air_temp_c(void);

/*
 * Transition to the next state based on the current state, the chamber temperature, the setpoint, the hysteresis, and whether cooling is enabled.
 * Returns the next state.
 * cur: The current state
 * chamber: The chamber temperature
 * sp: The setpoint
 * h: The hysteresis
 * cool_en: Whether cooling is enabled
 */
chamber_state_t chamber_transition(chamber_state_t cur, float chamber, float sp,
                                 float h, bool cool_en);
