#pragma once

#include "FreeRTOS.h"
#include "task.h"

// Create a task that mirrors USB connected state on STAT_LED_PIN.
BaseType_t status_led_task_create(UBaseType_t priority, TaskHandle_t *out_handle);

