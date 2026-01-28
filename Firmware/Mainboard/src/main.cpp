#include "FreeRTOS.h"
#include "pindefs.h"
#include "queue.h"
#include "task.h"
#include <Arduino.h>
#include <CommTask.h>
#include <Command.h>
#include <CommandProcessor.h>
#include <fault_led.h>

static void blinkTask(void *pvParameters);

// Command queue for processing incoming commands
QueueHandle_t commandQueue = nullptr;

void setup() {
  // USB Serial (for debugging/PC communication)
  Serial.begin(115200);
  Serial.setTimeout(50);
  Serial.ignoreFlowControl(true);

  // HMI UART on pins 26 (TX) and 27 (RX)
  Serial1.setTX(PIN_HMI_UART_TX);
  Serial1.setRX(PIN_HMI_UART_RX);
  Serial1.begin(115200);
  Serial1.setTimeout(50);

  pinMode(PIN_STAT, OUTPUT);

  // Initialize fault LED
  faultLedInit();

  // Create command queue (can hold 3 commands)
  commandQueue = xQueueCreate(3, sizeof(Command));
  if (commandQueue == nullptr) {
    // Failed to create queue - critical error, flash fault LED
    faultLedFlash(100, 100, 0); // Flash forever (blocks)
  }

  xTaskCreate(blinkTask, "blink", configMINIMAL_STACK_SIZE + 128, nullptr,
              tskIDLE_PRIORITY + 1, nullptr);

  // Create USB Serial comm task
  static CommTaskParams usbSerialParams = {Interface::USB_SERIAL, &Serial,
                                           commandQueue};
  xTaskCreate(commTask, "comm_usb", configMINIMAL_STACK_SIZE + 128,
              &usbSerialParams, tskIDLE_PRIORITY + 2, nullptr);

  // Create HMI Serial comm task
  static CommTaskParams hmiSerialParams = {Interface::HMI_SERIAL, &Serial1,
                                           commandQueue};
  xTaskCreate(commTask, "comm_hmi", configMINIMAL_STACK_SIZE + 128,
              &hmiSerialParams, tskIDLE_PRIORITY + 2, nullptr);

  // Create command processor task
  // static CommandProcessorParams processorParams = {commandQueue, &Serial,
  //                                                  &Serial1};
  // xTaskCreate(commandProcessorTask, "cmd_proc", configMINIMAL_STACK_SIZE +
  // 128,
  //             &processorParams, tskIDLE_PRIORITY + 3, nullptr);
}

void loop() {
  // Loop is running the vTaskScheduler in the background.
  vTaskDelay(pdMS_TO_TICKS(1000));
}

static void blinkTask(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    digitalWrite(PIN_STAT, HIGH);
    vTaskDelay(pdMS_TO_TICKS(250));
    digitalWrite(PIN_STAT, LOW);
    vTaskDelay(pdMS_TO_TICKS(250));
  }
}

extern "C" {

void vApplicationIdleHook(void) {
  // Idle hook runs when no other tasks are ready.
  __asm volatile("wfi");
}

void vApplicationMallocFailedHook(void) {
  taskDISABLE_INTERRUPTS();
  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name) {
  (void)task;
  (void)name;

  taskDISABLE_INTERRUPTS();
  for (;;) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(25);
    digitalWrite(LED_BUILTIN, LOW);
    delay(25);
  }
}

} // extern "C"
