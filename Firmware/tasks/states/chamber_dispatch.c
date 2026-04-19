#include "chamber_states.h"

// Take the state we're in now and transition to the next state
void chamber_dispatch(chamber_state_t *cur, chamber_state_t next,
                      chamber_context_t *ctx) {
  
  // If the current state is the same as the next state, return
  if (*cur == next)
    return;

  // Exit the current state
  switch (*cur) {
  case CHAMBER_STANDBY:
    chamber_state_standby_exit(ctx);
    break;
  case CHAMBER_FAULT:
    chamber_state_fault_exit(ctx);
    break;
  case CHAMBER_IDLE:
    chamber_state_idle_exit(ctx);
    break;
  case CHAMBER_HEATING:
    chamber_state_heating_exit(ctx);
    break;
  case CHAMBER_COOL_SLOW:
    chamber_state_cool_slow_exit(ctx);
    break;
  case CHAMBER_COOL_FAST:
    chamber_state_cool_fast_exit(ctx);
    break;
  case CHAMBER_DEHUMIDIFY:
    chamber_state_dehumidify_exit(ctx);
    break;
  }

  // Set the current state to the next state
  *cur = next;
  ctx->state_entered_at = ctx->now;

  // Enter the new state
  switch (*cur) {
  case CHAMBER_STANDBY:
    chamber_state_standby_entry(ctx);
    break;
  case CHAMBER_FAULT:
    chamber_state_fault_entry(ctx);
    break;
  case CHAMBER_IDLE:
    chamber_state_idle_entry(ctx);
    break;
  case CHAMBER_HEATING:
    chamber_state_heating_entry(ctx);
    break;
  case CHAMBER_COOL_SLOW:
    chamber_state_cool_slow_entry(ctx);
    break;
  case CHAMBER_COOL_FAST:
    chamber_state_cool_fast_entry(ctx);
    break;
  case CHAMBER_DEHUMIDIFY:
    chamber_state_dehumidify_entry(ctx);
    break;
  }
}

// Run the current state
void chamber_state_run_current(chamber_state_t cur, chamber_context_t *ctx) {
  switch (cur) {
  case CHAMBER_STANDBY:
    chamber_state_standby_run(ctx);
    break;
  case CHAMBER_FAULT:
    chamber_state_fault_run(ctx);
    break;
  case CHAMBER_IDLE:
    chamber_state_idle_run(ctx);
    break;
  case CHAMBER_HEATING:
    chamber_state_heating_run(ctx);
    break;
  case CHAMBER_COOL_SLOW:
    chamber_state_cool_slow_run(ctx);
    break;
  case CHAMBER_COOL_FAST:
    chamber_state_cool_fast_run(ctx);
    break;
  case CHAMBER_DEHUMIDIFY:
    chamber_state_dehumidify_run(ctx);
    break;
  }
}
