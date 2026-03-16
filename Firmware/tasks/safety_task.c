#include "safety_task.h"

#include "fault.h"
#include "globals.h"
#include "hardware/gpio.h"
#include "pico/stdio_usb.h"
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

  TickType_t last = xTaskGetTickCount();
  uint32_t stat_phase = 0;

  while (true) {
    vTaskDelayUntil(&last, pdMS_TO_TICKS(SAFETY_POLL_MS));

    // Drain fault queue: set FAULT and run on_fault (safe GPIO).
    fault_process();

    // Door sensor: SWITCH_PIN_1 is pull-up; high = door open.
    door_open = gpio_get(SWITCH_PIN_1);

    // =========================
    // Status LED (STAT/FAULT)
    // =========================
    stat_phase++;

    // Default heartbeat: 100ms on / 100ms off (period 200ms).
    // With SAFETY_POLL_MS=50, 4 ticks = 200ms.
    bool stat_on = (stat_phase % 4) < 2;

    // If USB connected, overlay faster blink (100ms on / 100ms off).
    if (stdio_usb_connected()) {
      bool fast_on = (stat_phase % 2) == 0;
      stat_on = fast_on;
    }

    gpio_put(STAT_LED_PIN, stat_on);

    // Fault LED: solid on for any non-zero fault.
    gpio_put(FAULT_LED_PIN, FAULT != FAULT_CODE_NONE);
  }
}

BaseType_t safety_task_create(UBaseType_t priority, TaskHandle_t *out_handle) {
  fault_register_on_fault(safety_on_fault);
  return xTaskCreate(safety_task, "safety", 256, NULL, priority, out_handle);
}
