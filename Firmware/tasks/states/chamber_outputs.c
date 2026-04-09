#include "chamber_outputs.h"

#include "constants.h"
#include "globals.h"
#include "hardware/gpio.h"
#include "pindefs.h"

#define THERMO_COMPRESSOR_PIN    LOAD_PIN_2
#define THERMO_HEAT_PIN          LOAD_PIN_1
#define THERMO_CONDENSER_FAN_PIN LOAD_PIN_3
#define THERMO_INTERNAL_FAN_PIN  LOAD_PIN_4

static TickType_t compressor_on_time = 0;
static TickType_t compressor_off_time = 0;
static bool compressor_state = false;
static bool cooling_requested = false;
static bool heater_relay_latch = false;

/**
 * Compressor contactor FSM only (no GPIO). Updates compressor_state and timers.
 */
static void fsm_compressor(bool want_on, TickType_t now) {
  if (want_on) {
    if (!compressor_state) {
      if (compressor_off_time == 0) {
        compressor_state = true;
        compressor_on_time = now;
        compressor_off_time = 0;
      } else {
        TickType_t off_duration = now - compressor_off_time;
        if (off_duration >= pdMS_TO_TICKS(MIN_COMPRESSOR_OFF_TIME_MS)) {
          compressor_state = true;
          compressor_on_time = now;
          compressor_off_time = 0;
        }
      }
    }
  } else {
    if (compressor_state) {
      if (compressor_on_time == 0) {
        compressor_state = false;
        compressor_off_time = now;
        compressor_on_time = 0;
      } else {
        TickType_t on_duration = now - compressor_on_time;
        if (on_duration >= pdMS_TO_TICKS(MIN_COMPRESSOR_ON_TIME_MS)) {
          compressor_state = false;
          compressor_off_time = now;
          compressor_on_time = 0;
        }
      }
    }
  }
}

/**
 * Single write point for chamber thermo loads. Enforces: heater and compressor
 * contactor cannot both be on (power / interlock). If both are requested,
 * cooling wins and heat is suppressed.
 */
static void commit_chamber_loads(bool heater_request, bool compressor_want,
                                 bool cooling_demand, bool internal_fan,
                                 TickType_t now) {
  if (heater_request && compressor_want)
    heater_request = false;

  cooling_requested = cooling_demand;
  fsm_compressor(compressor_want, now);

  const bool heat_gpio = heater_request && !compressor_state;
  const bool condenser_gpio = cooling_requested || compressor_state;

  gpio_put(THERMO_HEAT_PIN, heat_gpio ? 1u : 0u);
  gpio_put(THERMO_COMPRESSOR_PIN, compressor_state ? 1u : 0u);
  gpio_put(THERMO_CONDENSER_FAN_PIN, condenser_gpio ? 1u : 0u);
  gpio_put(THERMO_INTERNAL_FAN_PIN, internal_fan ? 1u : 0u);

  heater_on = heat_gpio;
  compressor_on = compressor_state;
}

static bool heater_tc_valid(float t) {
  return t > -80.0f && t < 600.0f && t != 0.0f;
}

static bool heater_bangbang(float air_sp, float t_heater_tc, float tc_hyst) {
  const float target = air_sp + HEATER_TC_OFFSET_ABOVE_AIR_SP_C;
  if (!heater_tc_valid(t_heater_tc)) {
    heater_relay_latch = false;
    return false;
  }
  if (t_heater_tc < target - tc_hyst)
    heater_relay_latch = true;
  else if (t_heater_tc > target + tc_hyst)
    heater_relay_latch = false;
  return heater_relay_latch;
}

void chamber_outputs_apply_idle(TickType_t now) {
  heater_relay_latch = false;
  commit_chamber_loads(false, false, false, true, now);
}

void chamber_outputs_apply_heating(const thermo_control_config_t *cfg,
                                   float air_sp, TickType_t now) {
  bool heat = heater_bangbang(air_sp, tdr0_temperature_c,
                              cfg->heater_tc_hysteresis_c);
  commit_chamber_loads(heat, false, false, true, now);
}

void chamber_outputs_apply_cool_slow(TickType_t now) {
  heater_relay_latch = false;
  commit_chamber_loads(false, true, true, false, now);
}

void chamber_outputs_apply_cool_fast(TickType_t now) {
  heater_relay_latch = false;
  commit_chamber_loads(false, true, true, true, now);
}

void chamber_outputs_apply_all_off(TickType_t now) {
  heater_relay_latch = false;
  commit_chamber_loads(false, false, false, false, now);
}

bool chamber_outputs_compressor_is_on(void) { return compressor_state; }

TickType_t chamber_outputs_compressor_on_time(void) {
  return compressor_on_time;
}

TickType_t chamber_outputs_compressor_off_time(void) {
  return compressor_off_time;
}
