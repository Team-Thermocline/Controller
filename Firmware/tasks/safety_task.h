#pragma once

#include "FreeRTOS.h"
#include "task.h"

/** Create the safety task. */
BaseType_t safety_task_create(UBaseType_t priority, TaskHandle_t *out_handle);
