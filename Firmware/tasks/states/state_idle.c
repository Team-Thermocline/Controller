#include "chamber_states.h"

#include "chamber_outputs.h"

void chamber_state_idle_entry(chamber_context_t *ctx) { (void)ctx; }

void chamber_state_idle_run(chamber_context_t *ctx) {
  chamber_outputs_apply_idle(ctx->now);
}

void chamber_state_idle_exit(chamber_context_t *ctx) { (void)ctx; }
