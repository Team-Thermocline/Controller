#include "thermo_control_task.h"

#include "chamber_context.h"
#include "chamber_outputs.h"
#include "chamber_states.h"
#include "chamber_transition.h"
#include "globals.h"

// Main task function for thermo control
static void thermo_control_task(void *pvParameters) {
  const thermo_control_config_t *cfg =
      (const thermo_control_config_t *)pvParameters;
  if (!cfg || cfg->update_period_ticks == 0) {
    vTaskDelete(NULL);
    return;
  }

  TickType_t last = xTaskGetTickCount();
  const float h = cfg->temp_hysteresis_c;

  while (true) {
    vTaskDelayUntil(&last, cfg->update_period_ticks);

    TickType_t now = xTaskGetTickCount();
    const float sp = current_temperature_setpoint;
    const float chamber = chamber_air_temp_c();

    chamber_context_t ctx = {
        .cfg = cfg, .now = now, .air_sp = sp, .chamber = chamber};

    const bool post_standby = chamber_post_standby;
    chamber_post_standby = false;
    const bool post_arm_idle = chamber_post_arm_idle;
    chamber_post_arm_idle = false;

    chamber_state_t state = (chamber_state_t)chamber_fsm_state;

    if (post_standby)
      chamber_dispatch(&state, CHAMBER_STANDBY, &ctx);
    else if (post_arm_idle && state == CHAMBER_STANDBY)
      chamber_dispatch(&state, CHAMBER_IDLE, &ctx);

    if (FAULT != FAULT_CODE_NONE && state != CHAMBER_FAULT &&
        state != CHAMBER_STANDBY)
      chamber_dispatch(&state, CHAMBER_FAULT, &ctx);

    if (state == CHAMBER_STANDBY) {
      chamber_state_run_current(state, &ctx);
    } else if (state == CHAMBER_FAULT) {
      chamber_state_run_current(state, &ctx);
      if (FAULT == FAULT_CODE_NONE) {
        chamber_dispatch(&state, CHAMBER_STANDBY, &ctx); // Return to standby upon clearing a fault
        chamber_state_run_current(state, &ctx);
      }
    } else if (sp == 0.0f) {
      chamber_dispatch(&state, CHAMBER_IDLE, &ctx);
      chamber_state_run_current(state, &ctx);
    } else {
      chamber_state_t next = chamber_transition(
          state, chamber, sp, h, cfg->enable_active_cooling);
      chamber_dispatch(&state, next, &ctx);
      chamber_state_run_current(state, &ctx);
    }

    chamber_fsm_state = state;
  }
}

BaseType_t thermo_control_task_create(const thermo_control_config_t *cfg,
                                      UBaseType_t priority,
                                      TaskHandle_t *out_handle) {
  return xTaskCreate(thermo_control_task, "thermo_control", 512, (void *)cfg,
                     priority, out_handle);
}

float thermo_control_get_compressor_on_time(void) {
  TickType_t now = xTaskGetTickCount();
  TickType_t on_time_tick = chamber_outputs_compressor_on_time();
  if (now < on_time_tick) {
    return 0.0f;
  }
  // Seconds
  return (float)(now - on_time_tick) / (float)configTICK_RATE_HZ;
}
  
float thermo_control_get_compressor_off_time(void) {
  TickType_t now = xTaskGetTickCount();
  TickType_t off_time_tick = chamber_outputs_compressor_off_time();
  if (now < off_time_tick) {
    return 0.0f;
  }
  
  // Seconds
  return (float)(now - off_time_tick) / (float)configTICK_RATE_HZ;
}
