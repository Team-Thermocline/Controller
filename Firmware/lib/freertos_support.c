#include "FreeRTOS.h"
#include "task.h"

#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pindefs.h"

// Some Pico SDK builds keep the vector table entries as `isr_*` symbols rather
// than CMSIS names. Provide minimal wrappers that branch straight into the
// FreeRTOS port handlers.
//
// Note: These wrappers are "indirect routing", so FreeRTOS'
// configCHECK_HANDLER_INSTALLATION must be 0.
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

void __attribute__((naked)) isr_svcall(void) {
  __asm volatile(
      "ldr r0, =SVC_Handler\n"
      "bx  r0\n");
}
void __attribute__((naked)) isr_pendsv(void) {
  __asm volatile(
      "ldr r0, =PendSV_Handler\n"
      "bx  r0\n");
}
void __attribute__((naked)) isr_systick(void) {
  __asm volatile(
      "ldr r0, =SysTick_Handler\n"
      "bx  r0\n");
}

static void fatal_blink(uint32_t on_ms, uint32_t off_ms) {
  // Make sure the fault LED is initialized even if we crash early.
  gpio_init(FAULT_LED_PIN);
  gpio_set_dir(FAULT_LED_PIN, GPIO_OUT);

  while (true) {
    gpio_put(FAULT_LED_PIN, 1);
    busy_wait_ms(on_ms);
    gpio_put(FAULT_LED_PIN, 0);
    busy_wait_ms(off_ms);
  }
}

void vAssertCalled(const char *file, int line) {
  (void)file;
  (void)line;

  // Fast blink indicates an assertion.
  fatal_blink(100, 100);
}

void vApplicationMallocFailedHook(void) {
  // Two short blinks + pause.
  gpio_init(FAULT_LED_PIN);
  gpio_set_dir(FAULT_LED_PIN, GPIO_OUT);
  while (true) {
    gpio_put(FAULT_LED_PIN, 1);
    busy_wait_ms(80);
    gpio_put(FAULT_LED_PIN, 0);
    busy_wait_ms(80);
    gpio_put(FAULT_LED_PIN, 1);
    busy_wait_ms(80);
    gpio_put(FAULT_LED_PIN, 0);
    busy_wait_ms(600);
  }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  (void)pcTaskName;

  // Long blink indicates stack overflow.
  fatal_blink(600, 200);
}

