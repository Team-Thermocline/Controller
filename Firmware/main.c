#include "ADG728.h"
#include "FreeRTOS.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "neopixel_ws2812.h"
#include "pico/error.h"
#include "pico/stdio.h"
#include "pico/stdio_usb.h"
#include "analog_task.h"
#include "pindefs.h"
#include "serial_task.h"
#include "sim_thermo_system_task.h"
#include "status_led_task.h"
#include "task.h"
#include <stdio.h>

bool ENABLE_ECHO = false;

// NeoPixel (WS2812) config
static const float NEOPIXEL_FREQ_HZ = 800000.0f;

static neopixel_ws2812_t g_neopixel;

// Global, overall setpoint and currentvariables
float current_temperature_setpoint = 20.0f; // Current temperature setpoint in Celsius
float current_humidity_setpoint = 100.0f; // Current humidity setpoint in %
float current_temperature; // Current temperature in Celsius
float current_humidity; // Current humidity in %
bool heater_on; // Heater on/off
bool compressor_on; // Compressor on/off (active cooling)
int current_state; // Current state (0=IDLE, 1=RUN, 2=STOP, 3=FAULT) TODO: use an enum
int alarm_state; // Alarm state (0=OK, 1=ERROR) TODO: use an enum


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

  neopixel_ws2812_init(&g_neopixel, pio0, NEOPIXEL_PIN, NEOPIXEL_FREQ_HZ,
                       false);
  neopixel_ws2812_put_rgb(&g_neopixel, 2, 2, 2); // Dim white light on startup.

  fflush(stdout);

  // =============================
  // Initialize I2C devices
  // =============================
  i2c_init(i2c0, 400000);
  gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

  // Initialize ADG728
  adg728_init(i2c0, ADG728_ADDR_MIN);
  if (!adg728_init(i2c0, ADG728_ADDR_MIN)) {
    FAULT = FAULT_CODE_I2C_COMMUNICATION_ERROR; // Set global fault
  }

  gpio_put(FAULT_LED_PIN, FAULT == FAULT_CODE_NONE);

  // ===========
  // Begin Tasks
  // ===========

  static const serial_task_config_t serial_cfg = {
      .enable_echo = &ENABLE_ECHO,
  };
  static const sim_thermo_system_config_t thermo_cfg = {
      .ambient_temp_c = 22.0f,
      .ambient_rh = 45.0f,
      .heat_ramp_c_per_s = 0.30f,
      .passive_ramp_c_per_s = 0.05f,
      .cool_ramp_c_per_s = 0.40f,
      .heat_on_delay_ticks = pdMS_TO_TICKS(500),
      .heat_off_delay_ticks = pdMS_TO_TICKS(500),
      .cool_on_delay_ticks = pdMS_TO_TICKS(500),
      .cool_off_delay_ticks = pdMS_TO_TICKS(500),
      .enable_active_cooling = true,
      .temp_hysteresis_c = 3.0f,
      .min_temp_c = -40.0f,
      .max_temp_c = 90.0f,
      .status_pixel = &g_neopixel,
      .color_idle = {2, 2, 2},
      .color_heat = {16, 2, 0},
      .color_cool = {0, 2, 16},
      .update_period_ticks = pdMS_TO_TICKS(100),
  };

  if (serial_task_create(&serial_cfg, 2, NULL) != pdPASS)
    vApplicationMallocFailedHook();
  if (status_led_task_create(1, NULL) != pdPASS)
    vApplicationMallocFailedHook();

  static const analog_task_config_t analog_cfg = {
      .i2c = i2c0,
      .adg728_addr = ADG728_ADDR_MIN,
  };
  if (analog_task_create(&analog_cfg, 1, NULL) != pdPASS)
    vApplicationMallocFailedHook();

  if (sim_thermo_system_task_create(&thermo_cfg, 1, NULL) != pdPASS)
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
