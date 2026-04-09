#include "chamber_states.h"

#include "chamber_outputs.h"

void chamber_state_heating_entry(chamber_context_t *ctx) { (void)ctx; }

void chamber_state_heating_run(chamber_context_t *ctx) {
  chamber_outputs_apply_heating(ctx->cfg, ctx->air_sp, ctx->now);
}

void chamber_state_heating_exit(chamber_context_t *ctx) { (void)ctx; }
