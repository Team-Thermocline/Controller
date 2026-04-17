#include "chamber_outputs.h"

#include "analog_task.h"
#include "constants.h"
#include "globals.h"
#include "hardware/gpio.h"
#include "hysteresis.h"
#include "pindefs.h"

// Just checks at compile
_Static_assert(CONDENSER_HOT_HEADROOM_HYST_C < CONDENSER_HOT_HEADROOM_C,
               "condenser headroom hysteresis must be < headroom");

static TickType_t compressor_on_time = 0;
static TickType_t compressor_off_time = 0;
static bool compressor_state = false;
static bool cooling_requested = false;
static hyst_below_t heater_tc_hyst;
static hyst_above_t condenser_headroom_hyst;

/**
 * Idle/heating: condenser fan vs (compressor TC − ambient) with hysteresis.
 */
static bool condenser_hot_headroom_wanted(void) {
  const bool v = tdr_temperature_c_valid(COMPRESSOR_TEMP) &&
                 tdr_temperature_c_valid(AMBIENT_TEMP);
  const float delta = COMPRESSOR_TEMP - AMBIENT_TEMP;
  const float off_below =
      CONDENSER_HOT_HEADROOM_C - CONDENSER_HOT_HEADROOM_HYST_C;
  return hyst_above_update(&condenser_headroom_hyst, v, delta,
                           CONDENSER_HOT_HEADROOM_C, off_below);
}

/**
 * Compressor contactor FSM
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
                                 bool condenser_hot_headroom, TickType_t now) {
  if (heater_request && compressor_want)
    heater_request = false;

  cooling_requested = cooling_demand;
  fsm_compressor(compressor_want, now);

  const bool heat_gpio = heater_request && !compressor_state;
  const bool condenser_gpio =
      cooling_requested || compressor_state ||
      (condenser_hot_headroom && condenser_hot_headroom_wanted());

  gpio_put(THERMO_HEAT_PIN, heat_gpio ? 1u : 0u);
  gpio_put(THERMO_COMPRESSOR_PIN, compressor_state ? 1u : 0u);
  gpio_put(THERMO_CONDENSER_FAN_PIN, condenser_gpio ? 1u : 0u);
  gpio_put(THERMO_INTERNAL_FAN_PIN, internal_fan ? 1u : 0u);

  heater_on = heat_gpio;
  compressor_on = compressor_state;
}

static bool heater_bangbang(float air_sp, float t_heater_tc, float tc_hyst) {
  const float target = air_sp + HEATER_TC_OFFSET_ABOVE_AIR_SP_C;
  return hyst_below_update(&heater_tc_hyst, tdr_temperature_c_valid(t_heater_tc),
                           t_heater_tc, target - tc_hyst, target + tc_hyst);
}

void chamber_outputs_apply_idle(TickType_t now) {
  hyst_below_reset(&heater_tc_hyst);
  commit_chamber_loads(false, false, false, true, true, now);
}

void chamber_outputs_apply_heating(const thermo_control_config_t *cfg,
                                   float air_sp, TickType_t now) {
  bool heat =
      heater_bangbang(air_sp, tdr0_temperature_c, cfg->heater_tc_hysteresis_c);
  commit_chamber_loads(heat, false, false, true, true, now);
}

void chamber_outputs_apply_cool_slow(TickType_t now) {
  hyst_below_reset(&heater_tc_hyst);
  /* Compressor off: coast on cold evap; internal fan on to pull chamber heat to evap. */
  commit_chamber_loads(false, false, true, true, false, now);
}

void chamber_outputs_apply_cool_fast(TickType_t now) {
  hyst_below_reset(&heater_tc_hyst);
  commit_chamber_loads(false, true, true, true, false, now);
}

void chamber_outputs_apply_all_off(TickType_t now) {
  hyst_below_reset(&heater_tc_hyst);
  hyst_above_reset(&condenser_headroom_hyst);
  commit_chamber_loads(false, false, false, false, false, now);
}

bool chamber_outputs_compressor_is_on(void) { return compressor_state; }

TickType_t chamber_outputs_compressor_on_time(void) {
  return compressor_on_time;
}

TickType_t chamber_outputs_compressor_off_time(void) {
  return compressor_off_time;
}
