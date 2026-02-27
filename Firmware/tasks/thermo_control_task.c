#include "thermo_control_task.h"

#include "globals.h"
#include "hardware/gpio.h"
#include "pindefs.h"
#include <stdbool.h>

// Compressor time constraints
#define COMPRESSOR_MIN_ON_TIME_MS  10000  // 10 seconds minimum on-time
#define COMPRESSOR_MIN_OFF_TIME_MS 30000  // 30 seconds minimum off-time

// Enum for control modes
typedef enum thermo_mode {
  THERMO_MODE_IDLE = 0,
  THERMO_MODE_HEAT = 1,
  THERMO_MODE_COOL = 2,
} thermo_mode_t;

// State tracking for compressor timing
static TickType_t compressor_on_time = 0;   // When compressor was turned on (0 = off)
static TickType_t compressor_off_time = 0;   // When compressor was turned off (0 = never off or currently on)
static bool compressor_state = false;        // Actual GPIO state

// Sets the status color based on the current mode
static void set_status_color(const thermo_control_config_t *cfg,
                             thermo_mode_t mode) {
  if (!cfg->status_pixel)
    return;
  const uint8_t *rgb = cfg->color_idle;
  if (mode == THERMO_MODE_HEAT)
    rgb = cfg->color_heat;
  else if (mode == THERMO_MODE_COOL)
    rgb = cfg->color_cool;
  neopixel_ws2812_put_rgb(cfg->status_pixel, rgb[0], rgb[1], rgb[2]);
}

// Updates compressor GPIO state respecting minimum on/off times
// Returns true if compressor is actually on, false if off (may differ from want_on due to timing)
static bool update_compressor_state(bool want_on, TickType_t now) {
  if (want_on) {
    // Desire compressor on
    if (!compressor_state) {
      // Currently off - check if we've waited long enough
      if (compressor_off_time == 0) {
        // Never been off before, or was just initialized - allow on
        compressor_state = true;
        compressor_on_time = now;
        compressor_off_time = 0;
        gpio_put(LOAD_PIN_6, 1);
      } else {
        TickType_t off_duration = now - compressor_off_time;
        if (off_duration >= pdMS_TO_TICKS(COMPRESSOR_MIN_OFF_TIME_MS)) {
          // Minimum off-time satisfied
          compressor_state = true;
          compressor_on_time = now;
          compressor_off_time = 0;
          gpio_put(LOAD_PIN_6, 1);
        }
        // else: still waiting for minimum off-time, stay off
      }
    } else {
      // Already on - check minimum on-time if we just turned it on
      if (compressor_on_time != 0) {
        TickType_t on_duration = now - compressor_on_time;
        if (on_duration < pdMS_TO_TICKS(COMPRESSOR_MIN_ON_TIME_MS)) {
          // Still in minimum on-time, keep it on even if logic wants off
          // (This case shouldn't happen if want_on is true, but handle it)
        }
      }
    }
  } else {
    // Want compressor off
    if (compressor_state) {
      // Currently on - check minimum on-time
      if (compressor_on_time == 0) {
        // Shouldn't happen, but handle it
        compressor_state = false;
        compressor_off_time = now;
        compressor_on_time = 0;
        gpio_put(LOAD_PIN_6, 0);
      } else {
        TickType_t on_duration = now - compressor_on_time;
        if (on_duration >= pdMS_TO_TICKS(COMPRESSOR_MIN_ON_TIME_MS)) {
          // Minimum on-time satisfied, can turn off
          compressor_state = false;
          compressor_off_time = now;
          compressor_on_time = 0;
          gpio_put(LOAD_PIN_6, 0);
        }
        // else: still in minimum on-time, keep it on
      }
    }
  }
  
  return compressor_state;
}

// Command heat: safety check (turn off cooling first), then turn on heater
static void command_heat(TickType_t now) {
  // Safety: turn off cooling before heating
  if (compressor_state) {
    // Force compressor off (will respect minimum on-time)
    update_compressor_state(false, now);
  }
  
  // Turn on heater
  heater_on = true;
  gpio_put(LOAD_PIN_1, 1);
}

// Command cool: safety check (turn off heating first), then handle compressor timing
static void command_cool(TickType_t now) {
  // Safety: turn off heating before cooling
  if (heater_on) {
    heater_on = false;
    gpio_put(LOAD_PIN_1, 0);
  }
  
  // Update compressor state (respects minimum on/off times)
  compressor_on = update_compressor_state(true, now);
}

// Command idle: turn off both heat and cool
static void command_idle(TickType_t now) {
  heater_on = false;
  gpio_put(LOAD_PIN_1, 0);
  
  compressor_on = update_compressor_state(false, now);
}

// Main task function for thermo control
static void thermo_control_task(void *pvParameters) {
  const thermo_control_config_t *cfg =
      (const thermo_control_config_t *)pvParameters;
  if (!cfg || cfg->update_period_ticks == 0) {
    vTaskDelete(NULL);
    return;
  }

  thermo_mode_t mode = THERMO_MODE_IDLE;
  TickType_t last = xTaskGetTickCount();
  float h = cfg->temp_hysteresis_c;

  // Main loop
  while (true) {
    vTaskDelayUntil(&last, cfg->update_period_ticks);

    TickType_t now = xTaskGetTickCount();

    // In STANDBY, outputs stay off (transition to IDLE handled by serial_task when setpoint is set)
    if (current_state == RUN_STATE_STANDBY) {
      command_idle(now);
      if (FAULT != FAULT_CODE_THERMOCOUPLE_OPEN)
        FAULT = FAULT_CODE_NONE;
      continue;
    }

    // Only control if we're in IDLE or RUN state (not STANDBY, STOP, FAULT)
    if (current_state != RUN_STATE_IDLE && current_state != RUN_STATE_RUN) {
      command_idle(now);
      continue;
    }

    float sp = current_temperature_setpoint;
    float t = tdr0_temperature_c;

    // Don't run control if setpoint is not set (0.0) or temperature reading is invalid
    if (sp == 0.0f || t == 0.0f) {
      mode = THERMO_MODE_IDLE;
      command_idle(now);
      current_state = RUN_STATE_IDLE;
      continue;
    }

    // Simple control: if colder than setpoint - hysteresis, heat; if hotter than setpoint + hysteresis, cool
    if (t <= sp - h) {
      mode = THERMO_MODE_HEAT;
      command_heat(now);
    } else if (cfg->enable_active_cooling && t >= sp + h) {
      mode = THERMO_MODE_COOL;
      command_cool(now);
    } else {
      mode = THERMO_MODE_IDLE;
      command_idle(now);
    }

    set_status_color(cfg, mode);

    // Update state: IDLE if both are off, RUN if either is on
    current_state = (mode == THERMO_MODE_IDLE && !compressor_state && !heater_on) 
                    ? RUN_STATE_IDLE : RUN_STATE_RUN;
    if (FAULT != FAULT_CODE_THERMOCOUPLE_OPEN)
      FAULT = FAULT_CODE_NONE;
  }
}

BaseType_t thermo_control_task_create(const thermo_control_config_t *cfg,
                                     UBaseType_t priority,
                                     TaskHandle_t *out_handle) {
  return xTaskCreate(thermo_control_task, "thermo_control", 512, (void *)cfg,
                     priority, out_handle);
}
/* ========================================================
 * For Debugging/monitoring
 * ======================================================== */
TickType_t thermo_control_get_compressor_on_time(void) {
  return compressor_on_time;
}

TickType_t thermo_control_get_compressor_off_time(void) {
  return compressor_off_time;
}
