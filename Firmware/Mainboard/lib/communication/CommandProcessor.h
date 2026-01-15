#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "Command.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <Arduino.h>

/**
 * @brief Parameters structure for commandProcessorTask
 *
 * We pass this structure to the commandProcessorTask
 */
struct CommandProcessorParams {
  QueueHandle_t commandQueue;
  HardwareSerial *usbSerial; // Serial port for USB_SERIAL interface
  HardwareSerial
      *hmiSerial; // Serial port for HMI_SERIAL interface (can be nullptr)
};

/**
 * @brief Task processor for commands
 *
 * One at a time, process commands. Basically a big lookup for whatever
 * functions or settings we need to do.
 *
 * @param pvParameters Pointer to a CommandProcessorParams structure
 */
void commandProcessorTask(void *pvParameters);

#endif // COMMAND_PROCESSOR_H
