#ifndef COMM_TASK_H
#define COMM_TASK_H

#include "Command.h"
#include "FreeRTOS.h"
#include "queue.h"
#include <Arduino.h>

/**
 * @brief Parameters structure for commTask
 *
 * This structure is passed to the commTask FreeRTOS task to configure
 * which interface, Serial port, and command queue it should use.
 */
struct CommTaskParams {
  Interface interface;
  HardwareSerial *serialPort;
  QueueHandle_t commandQueue; // Queue to send parsed commands to
};

/**
 * @brief FreeRTOS task function for handling communication
 *
 * This task listens for incoming commands on a specified serial interface.
 * Commands are parsed and should be placed in a queue for processing.
 * This controller only replies to instructions - it never sends unprompted.
 *
 * @param pvParameters Pointer to a CommTaskParams structure
 */
void commTask(void *pvParameters);

#endif // COMM_TASK_H
