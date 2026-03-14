#include "safety_task.h"

#include "globals.h"
#include "hardware/gpio.h"
#include "pindefs.h"

#define SAFETY_POLL_MS 50

static void safety_task(void *pvParameters) {
  (void)pvParameters;

  while (true) {
    // Door sensor: SWITCH_PIN_1 is pull-up; high = door open.
    door_open = gpio_get(SWITCH_PIN_1);

    vTaskDelay(pdMS_TO_TICKS(SAFETY_POLL_MS));
  }
}

BaseType_t safety_task_create(UBaseType_t priority, TaskHandle_t *out_handle) {
  return xTaskCreate(safety_task, "safety", 128, NULL, priority, out_handle);
}
