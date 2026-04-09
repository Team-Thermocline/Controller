#include "chamber_states.h"

#include "chamber_outputs.h"

void chamber_state_cool_fast_entry(chamber_context_t *ctx) { (void)ctx; }

void chamber_state_cool_fast_run(chamber_context_t *ctx) {
  chamber_outputs_apply_cool_fast(ctx->now);
}

void chamber_state_cool_fast_exit(chamber_context_t *ctx) { (void)ctx; }
