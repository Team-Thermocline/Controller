#include "chamber_states.h"

#include "chamber_outputs.h"
#include "globals.h"
#include "hardware/gpio.h"
#include "pindefs.h"

void chamber_state_standby_entry(chamber_context_t *ctx) { (void)ctx; }

void chamber_state_standby_run(chamber_context_t *ctx) {
  (void)ctx;
  /* Thermo loads: single write path so compressor FSM stays consistent */
  chamber_outputs_apply_all_off(ctx->now);
  gpio_put(LOAD_PIN_5, 0);
  gpio_put(LOAD_PIN_6, 0);
  if (FAULT != FAULT_CODE_THERMOCOUPLE_OPEN &&
      FAULT != FAULT_CODE_ENV_SENSOR &&
      FAULT != FAULT_CODE_COMPRESSOR_OVERCURRENT)
    fault_raise(FAULT_CODE_NONE);
}

void chamber_state_standby_exit(chamber_context_t *ctx) { (void)ctx; }
