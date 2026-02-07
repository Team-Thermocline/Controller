#pragma once

#include "FreeRTOS.h"
#include "task.h"

// Possible fault codes
typedef enum fault_code {
  FAULT_CODE_NONE = 0,
  FAULT_CODE_I2C_COMMUNICATION_ERROR = 1,
} fault_code_t;

// Global fault state
extern fault_code_t FAULT;

// Create a task that mirrors USB connected state on STAT_LED_PIN.
BaseType_t status_led_task_create(UBaseType_t priority, TaskHandle_t *out_handle);

