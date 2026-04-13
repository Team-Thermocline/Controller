#include "safety_task.h"

#include "globals.h"
#include "constants.h"
#include "hardware/gpio.h"
#include "pindefs.h"

static void safety_task(void *pvParameters) {
  (void)pvParameters;

  TickType_t last = xTaskGetTickCount();
  uint32_t stat_phase = 0;

  while (true) {
    vTaskDelayUntil(&last, pdMS_TO_TICKS(SAFETY_POLL_MS));

    // Failsafe load chop on fault
    if (FAULT != FAULT_CODE_NONE) {
      gpio_put(LOAD_PIN_1, 0);
      gpio_put(LOAD_PIN_2, 0);
      gpio_put(LOAD_PIN_3, 0);
      gpio_put(LOAD_PIN_4, 0);
      gpio_put(LOAD_PIN_5, 0);
      gpio_put(LOAD_PIN_6, 0);
    }

    // Door is open when non-zero
    door_open = !gpio_get(SWITCH_PIN_1);

    // LED Phase
    stat_phase++;

    bool stat_on = (stat_phase % 8) < 4; // Slow

    if (chamber_fsm_state != CHAMBER_STANDBY) {
      stat_on = (stat_phase % 4) < 2; // Fast
    }

    gpio_put(FAULT_LED_PIN, FAULT != FAULT_CODE_NONE);
    gpio_put(STAT_LED_PIN, stat_on);
  }
}

BaseType_t safety_task_create(UBaseType_t priority, TaskHandle_t *out_handle) {
  return xTaskCreate(safety_task, "safety", 256, NULL, priority, out_handle);
}
