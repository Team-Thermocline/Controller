#include "chamber_states.h"

#include "chamber_outputs.h"

void chamber_state_dehumidify_entry(chamber_context_t *ctx) { (void)ctx; }

void chamber_state_dehumidify_run(chamber_context_t *ctx) {
  chamber_outputs_apply_dehumidify(ctx->now);
}

void chamber_state_dehumidify_exit(chamber_context_t *ctx) { (void)ctx; }
