#include "chamber_states.h"

#include "chamber_outputs.h"

void chamber_state_cool_slow_entry(chamber_context_t *ctx) { (void)ctx; }

void chamber_state_cool_slow_run(chamber_context_t *ctx) {
  chamber_outputs_apply_cool_slow(ctx->now);
}

void chamber_state_cool_slow_exit(chamber_context_t *ctx) { (void)ctx; }
