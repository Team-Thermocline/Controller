#include "thermo_control_task.h"

#include "globals.h"
#include "hardware/gpio.h"
#include "pindefs.h"
#include <stdbool.h>

// Enum for control modes
typedef enum thermo_mode {
  THERMO_MODE_IDLE = 0,
  THERMO_MODE_HEAT = 1,
  THERMO_MODE_COOL = 2,
} thermo_mode_t;

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

    // In STANDBY, outputs stay off (transition to IDLE handled by serial_task when setpoint is set)
    if (current_state == RUN_STATE_STANDBY) {
      heater_on = false;
      compressor_on = false;
      gpio_put(LOAD_PIN_1, 0);
      gpio_put(LOAD_PIN_6, 0);
      FAULT = FAULT_CODE_NONE;
      continue;
    }

    // Only control if we're in IDLE or RUN state (not STANDBY, STOP, FAULT)
    if (current_state != RUN_STATE_IDLE && current_state != RUN_STATE_RUN) {
      heater_on = false;
      compressor_on = false;
      gpio_put(LOAD_PIN_1, 0);
      gpio_put(LOAD_PIN_6, 0);
      continue;
    }

    float sp = current_temperature_setpoint;
    float t = tdr0_temperature_c;

    // Don't run control if setpoint is not set (0.0) or temperature reading is invalid
    if (sp == 0.0f || t == 0.0f) {
      mode = THERMO_MODE_IDLE;
      heater_on = false;
      compressor_on = false;
      gpio_put(LOAD_PIN_1, 0);
      gpio_put(LOAD_PIN_6, 0);
      current_state = RUN_STATE_IDLE;
      continue;
    }

    // Simple control: if colder than setpoint - hysteresis, heat; if hotter than setpoint + hysteresis, cool
    if (t <= sp - h) {
      mode = THERMO_MODE_HEAT;
    } else if (cfg->enable_active_cooling && t >= sp + h) {
      mode = THERMO_MODE_COOL;
    } else {
      mode = THERMO_MODE_IDLE;
    }

    heater_on = (mode == THERMO_MODE_HEAT);
    compressor_on = (mode == THERMO_MODE_COOL);
    gpio_put(LOAD_PIN_1, heater_on ? 1 : 0);
    gpio_put(LOAD_PIN_6, compressor_on ? 1 : 0);
    set_status_color(cfg, mode);

    current_state = (mode == THERMO_MODE_IDLE) ? RUN_STATE_IDLE : RUN_STATE_RUN;
    FAULT = FAULT_CODE_NONE;
  }
}

BaseType_t thermo_control_task_create(const thermo_control_config_t *cfg,
                                     UBaseType_t priority,
                                     TaskHandle_t *out_handle) {
  return xTaskCreate(thermo_control_task, "thermo_control", 512, (void *)cfg,
                     priority, out_handle);
}
