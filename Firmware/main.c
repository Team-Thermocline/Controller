#include "ADG728.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdio.h"
#include "analog_task.h"
#include "fault.h"
#include "globals.h"
#include "pindefs.h"
#include <stdio.h>

// Tasks
#include "serial_task.h"
#include "thermo_control_task.h"
#include "status_led_task.h"
#include "safety_task.h"
#include "interior_led_task.h"
#include "task.h"


bool ENABLE_ECHO = false;

static void heartbeat_task(void *pvParameters) {
  (void)pvParameters;
  while (true) {
    printf(".\n");
    fflush(stdout);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

int main() {
  // =============
  // Program Begin
  // =============

  // Configure Status LEDs (GPIO25 on Pico)
  gpio_init(STAT_LED_PIN);
  gpio_set_dir(STAT_LED_PIN, GPIO_OUT);
  gpio_init(FAULT_LED_PIN);
  gpio_set_dir(FAULT_LED_PIN, GPIO_OUT);
  gpio_put(STAT_LED_PIN, 0);
  gpio_put(FAULT_LED_PIN, 1);

  // =============================
  // Initialize stdio (USB serial)
  // =============================
  stdio_init_all();
  setvbuf(stdout, NULL, _IONBF, 0); // Disable buffering for stdout

  // Load outputs (relays and SSRs)
  gpio_init(LOAD_PIN_1);
  gpio_set_dir(LOAD_PIN_1, GPIO_OUT);
  gpio_put(LOAD_PIN_1, 0);
  gpio_init(LOAD_PIN_6);
  gpio_set_dir(LOAD_PIN_6, GPIO_OUT);
  gpio_put(LOAD_PIN_6, 0);

  fflush(stdout);

  // =============================
  // Initialize I2C devices
  // =============================
  i2c_init(i2c0, 400000);
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

  // Initalize fault queue
  fault_init();

  // Initialize ADG728
  adg728_init(i2c0, ADG728_ADDR_MIN);
  if (!adg728_init(i2c0, ADG728_ADDR_MIN)) {
    fault_raise(FAULT_CODE_I2C_COMMUNICATION_ERROR);
  }

  // Intalize Misc
  gpio_init(SWITCH_PIN_1);
  gpio_set_dir(SWITCH_PIN_1, GPIO_IN);
  gpio_pull_up(SWITCH_PIN_1);

  // Initialize Fault LED
  gpio_put(FAULT_LED_PIN, FAULT == FAULT_CODE_NONE);

  // Process any faults that were raised before the scheduler started
  fault_process();

  // ===========
  // Begin Tasks
  // ===========

  static const serial_task_config_t serial_cfg = {
      .enable_echo = &ENABLE_ECHO,
  };
  static const thermo_control_config_t thermo_cfg = {
      .temp_hysteresis_c = 3.0f,
      .enable_active_cooling = true,
      .update_period_ticks = pdMS_TO_TICKS(100),
  };

  if (serial_task_create(&serial_cfg, 2, NULL) != pdPASS)
    vApplicationMallocFailedHook();
  if (status_led_task_create(1, NULL) != pdPASS)
    vApplicationMallocFailedHook();
  if (safety_task_create(2, NULL) != pdPASS)
    vApplicationMallocFailedHook();
  if (interior_led_task_create(1, NULL) != pdPASS)
    vApplicationMallocFailedHook();

  static const analog_task_config_t analog_cfg = {
      .i2c = i2c0,
      .adg728_addr = ADG728_ADDR_MIN,
  };
  if (analog_task_create(&analog_cfg, 1, NULL) != pdPASS)
    vApplicationMallocFailedHook();

  if (thermo_control_task_create(&thermo_cfg, 1, NULL) != pdPASS)
    vApplicationMallocFailedHook();
  if (xTaskCreate(heartbeat_task, "heartbeat", 512, NULL, 1, NULL) != pdPASS)
    vApplicationMallocFailedHook();

  vTaskStartScheduler();

  // If we get here, the scheduler couldn't start (usually heap too small).
  vApplicationMallocFailedHook();

  // Should never reach here.
  while (true) {
  }

  return 0;
}
