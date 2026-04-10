#include "safety_task.h"

#include "globals.h"
#include "hardware/gpio.h"
#include "pindefs.h"

#define SAFETY_POLL_MS 50

static void safety_task(void *pvParameters) {
  (void)pvParameters;

  TickType_t last = xTaskGetTickCount();
  uint32_t stat_phase = 0;

  while (true) {
    vTaskDelayUntil(&last, pdMS_TO_TICKS(SAFETY_POLL_MS));

    /* Failsafe load chop on fault (50 ms vs thermo period). */
    if (FAULT != FAULT_CODE_NONE) {
      gpio_put(LOAD_PIN_1, 0);
      gpio_put(LOAD_PIN_6, 0);
    }

    door_open = gpio_get(SWITCH_PIN_1);

    stat_phase++;

    bool stat_on = (stat_phase % 4) < 2;

    if (chamber_fsm_state != CHAMBER_STANDBY) {
      stat_on = (stat_phase % 2) == 0;
    }

    gpio_put(FAULT_LED_PIN, FAULT != FAULT_CODE_NONE);
    gpio_put(STAT_LED_PIN, stat_on);
  }
}

BaseType_t safety_task_create(UBaseType_t priority, TaskHandle_t *out_handle) {
  return xTaskCreate(safety_task, "safety", 256, NULL, priority, out_handle);
}
