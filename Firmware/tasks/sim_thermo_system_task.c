#include "sim_thermo_system_task.h"

#include "globals.h"
#include "hardware/gpio.h"
#include "pindefs.h"
#include <stdbool.h>

// Enum for the different simulation modes
typedef enum sim_mode {
  SIM_MODE_IDLE = 0,
  SIM_MODE_HEAT = 1,
  SIM_MODE_COOL = 2,
} sim_mode_t;

// Checks if a tick has reached a target
static bool tick_reached(TickType_t now, TickType_t target) {
  // wrap-safe check for "now >= target" on unsigned tick counters
  return (TickType_t)(now - target) < (TickType_t)0x80000000u;
}

// Determines the desired mode based on the current temperature, setpoint, and hysteresis
static sim_mode_t desired_mode(const sim_thermo_system_config_t *cfg, float t,
                               float sp, sim_mode_t current) {
  float h = cfg->temp_hysteresis_c;
  if (t <= sp - h)
    return SIM_MODE_HEAT;
  if (cfg->enable_active_cooling && t >= sp + h)
    return SIM_MODE_COOL;
  // Inside band (sp-h < t < sp+h): don't heat when already at/above setpoint, don't cool when at/below
  if (t > sp - h && t < sp + h) {
    if (current == SIM_MODE_HEAT && t >= sp)
      return SIM_MODE_IDLE;
    if (current == SIM_MODE_COOL && t <= sp)
      return SIM_MODE_IDLE;
    return current;
  }
  return SIM_MODE_IDLE;
}


// Calculates the delay before a transition can occur
static TickType_t transition_delay_ticks(const sim_thermo_system_config_t *cfg,
                                        sim_mode_t from, sim_mode_t to) {
  if (from == to)
    return 0;
  if (to == SIM_MODE_HEAT)
    return cfg->heat_on_delay_ticks;
  if (to == SIM_MODE_COOL)
    return cfg->cool_on_delay_ticks;
  // to IDLE
  if (from == SIM_MODE_HEAT)
    return cfg->heat_off_delay_ticks;
  if (from == SIM_MODE_COOL)
    return cfg->cool_off_delay_ticks;
  return 0;
}

// Sets the status color based on the current mode
static void set_status_color(const sim_thermo_system_config_t *cfg,
                             sim_mode_t mode) {
  if (!cfg->status_pixel)
    return;
  const uint8_t *rgb = cfg->color_idle;
  if (mode == SIM_MODE_HEAT)
    rgb = cfg->color_heat;
  else if (mode == SIM_MODE_COOL)
    rgb = cfg->color_cool;
  neopixel_ws2812_put_rgb(cfg->status_pixel, rgb[0], rgb[1], rgb[2]);
}

// Main task function for the simulator thermo system
static void sim_thermo_system_task(void *pvParameters) {
  const sim_thermo_system_config_t *cfg =
      (const sim_thermo_system_config_t *)pvParameters;
  if (!cfg || cfg->update_period_ticks == 0) {
    vTaskDelete(NULL);
    return;
  }

  sim_mode_t mode = SIM_MODE_IDLE;
  if (heater_on)
    mode = SIM_MODE_HEAT;
  if (compressor_on)
    mode = SIM_MODE_COOL;

  bool pending = false;
  sim_mode_t pending_mode = SIM_MODE_IDLE;
  TickType_t pending_until = 0;

  // When cooling undershoots to sp - h/2, we stop and rest until drift to sp + h/2
  bool cooling_rest = false;

  TickType_t last = xTaskGetTickCount();
  float h = cfg->temp_hysteresis_c;

  // Main loop
  while (true) {
    vTaskDelayUntil(&last, cfg->update_period_ticks);

    float sp = current_temperature_setpoint;
    float t = tdr1_temperature_c;
    TickType_t now = xTaskGetTickCount();

    // Cooling undershoot: when cooling drops temp to sp - h/2, stop and rest
    // until we passively drift up to sp + h/2
    if (mode == SIM_MODE_COOL && t <= sp - h / 2.0f)
      cooling_rest = true;
    if (cooling_rest && t >= sp + h / 2.0f)
      cooling_rest = false;
    // Clear cooling_rest if temp falls below heating threshold (allow heating)
    if (cooling_rest && t <= sp - h)
      cooling_rest = false;

    sim_mode_t want_raw = desired_mode(cfg, t, sp, mode);
    // cooling_rest only blocks cooling, not heating
    sim_mode_t want = (cooling_rest && want_raw == SIM_MODE_COOL) ? SIM_MODE_IDLE : want_raw;

    if (!pending && want != mode) {
      pending = true;
      pending_mode = want;
      pending_until = now + transition_delay_ticks(cfg, mode, want);
    }
    if (pending && tick_reached(now, pending_until)) {
      mode = pending_mode;
      pending = false;
    }

    heater_on = (mode == SIM_MODE_HEAT);
    compressor_on = (mode == SIM_MODE_COOL);
    gpio_put(LOAD_PIN_1, heater_on ? 1 : 0);
    gpio_put(LOAD_PIN_6, compressor_on ? 1 : 0);
    set_status_color(cfg, mode);

    current_state = (mode == SIM_MODE_IDLE) ? RUN_STATE_IDLE : RUN_STATE_RUN;
    FAULT = FAULT_CODE_NONE;
  }
}

BaseType_t sim_thermo_system_task_create(const sim_thermo_system_config_t *cfg,
                                        UBaseType_t priority,
                                        TaskHandle_t *out_handle) {
  return xTaskCreate(sim_thermo_system_task, "sim_thermo", 512, (void *)cfg,
                     priority, out_handle);
}

