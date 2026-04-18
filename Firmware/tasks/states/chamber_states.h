#pragma once

/* entry/run/exit per state; run() every thermo tick. entry/exit for timeouts,
 * checks, teardown. */

#include "chamber_context.h"

void chamber_state_standby_entry(chamber_context_t *ctx);
void chamber_state_standby_run(chamber_context_t *ctx);
void chamber_state_standby_exit(chamber_context_t *ctx);

void chamber_state_fault_entry(chamber_context_t *ctx);
void chamber_state_fault_run(chamber_context_t *ctx);
void chamber_state_fault_exit(chamber_context_t *ctx);

void chamber_state_idle_entry(chamber_context_t *ctx);
void chamber_state_idle_run(chamber_context_t *ctx);
void chamber_state_idle_exit(chamber_context_t *ctx);

void chamber_state_heating_entry(chamber_context_t *ctx);
void chamber_state_heating_run(chamber_context_t *ctx);
void chamber_state_heating_exit(chamber_context_t *ctx);

void chamber_state_cool_slow_entry(chamber_context_t *ctx);
void chamber_state_cool_slow_run(chamber_context_t *ctx);
void chamber_state_cool_slow_exit(chamber_context_t *ctx);

void chamber_state_cool_fast_entry(chamber_context_t *ctx);
void chamber_state_cool_fast_run(chamber_context_t *ctx);
void chamber_state_cool_fast_exit(chamber_context_t *ctx);

void chamber_state_dehumidify_entry(chamber_context_t *ctx);
void chamber_state_dehumidify_run(chamber_context_t *ctx);
void chamber_state_dehumidify_exit(chamber_context_t *ctx);

void chamber_dispatch(chamber_state_t *cur, chamber_state_t next,
                      chamber_context_t *ctx);
void chamber_state_run_current(chamber_state_t cur, chamber_context_t *ctx);
