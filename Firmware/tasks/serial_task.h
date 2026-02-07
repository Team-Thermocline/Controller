#pragma once

#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

typedef struct serial_task_config {
  bool *enable_echo; // optional; if NULL, echo is disabled
} serial_task_config_t;

// Create the serial task. `cfg` must remain valid for the lifetime of the task.
BaseType_t serial_task_create(const serial_task_config_t *cfg,
                              UBaseType_t priority, TaskHandle_t *out_handle);

