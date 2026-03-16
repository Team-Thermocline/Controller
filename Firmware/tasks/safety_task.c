#include "safety_task.h"

#include "fault.h"
#include "globals.h"
#include "hardware/gpio.h"
#include "pindefs.h"

#define SAFETY_POLL_MS 50

static void safety_on_fault(fault_code_t code) {
  // On fault (not on clear), drive loads to safe state (off).
  if (code != FAULT_CODE_NONE) {
    gpio_put(LOAD_PIN_1, 0);
    gpio_put(LOAD_PIN_6, 0);
  }
}

static void safety_task(void *pvParameters) {
  (void)pvParameters;

  while (true) {
    // Drain fault queue: set FAULT and run on_fault (safe GPIO).
    fault_process();

    // Door sensor: SWITCH_PIN_1 is pull-up; high = door open.
    door_open = gpio_get(SWITCH_PIN_1);

    vTaskDelay(pdMS_TO_TICKS(SAFETY_POLL_MS));
  }
}

BaseType_t safety_task_create(UBaseType_t priority, TaskHandle_t *out_handle) {
  fault_register_on_fault(safety_on_fault);
  return xTaskCreate(safety_task, "safety", 128, NULL, priority, out_handle);
}
