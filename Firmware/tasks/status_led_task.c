#include "status_led_task.h"

#include "hardware/gpio.h"
#include "pico/stdio_usb.h"
#include "pindefs.h"

static void status_led_task(void *pvParameters) {
  (void)pvParameters;

  // Loop Start
  while (true) {
    // Default Status Blink, always running if system is alive.
    gpio_put(STAT_LED_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_put(STAT_LED_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    
    // User Connected Status, Only blinks if user is connected
    if (stdio_usb_connected()) {
      gpio_put(STAT_LED_PIN, 1);
      vTaskDelay(pdMS_TO_TICKS(100));
      gpio_put(STAT_LED_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(100));
    }else{
      gpio_put(STAT_LED_PIN, 0);
      vTaskDelay(pdMS_TO_TICKS(200));
    }

    // Work Status, Blinks if work is in progress
    vTaskDelay(pdMS_TO_TICKS(200));

    // TODO: Add checks for Error and other statuses
  }
}

BaseType_t status_led_task_create(UBaseType_t priority, TaskHandle_t *out_handle) {
  return xTaskCreate(status_led_task, "status", 256, NULL, priority, out_handle);
}

