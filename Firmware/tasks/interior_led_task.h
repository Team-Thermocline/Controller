#pragma once

#include "FreeRTOS.h"
#include "task.h"

// Creates the interior LED task that drives the WS2812 strip
BaseType_t interior_led_task_create(UBaseType_t priority,
                                    TaskHandle_t *out_handle);
