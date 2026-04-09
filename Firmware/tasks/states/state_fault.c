#include "chamber_states.h"

#include "chamber_outputs.h"
#include "hardware/gpio.h"
#include "pindefs.h"

#define THERMO_INTERNAL_FAN_PIN LOAD_PIN_4

void chamber_state_fault_entry(chamber_context_t *ctx) { (void)ctx; }

void chamber_state_fault_run(chamber_context_t *ctx) {
  chamber_outputs_apply_all_off(ctx->now);
  gpio_put(THERMO_INTERNAL_FAN_PIN, 0);
}

void chamber_state_fault_exit(chamber_context_t *ctx) { (void)ctx; }
