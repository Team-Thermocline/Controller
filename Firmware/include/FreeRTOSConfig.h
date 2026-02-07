#pragma once

// FreeRTOS configuration for Raspberry Pi Pico (RP2040, Cortex-M0+).
//
// Notes:
// - RP2040 has no MPU -> configENABLE_MPU must be 0 (required by this port).
// - This project uses the SysTick-based tick from the ARM_CM0 GCC port.

#include <stddef.h>
#include <stdint.h>

// Provided by the application (see `lib/freertos_support.c`)
void vAssertCalled(const char *file, int line);

// -----------------------------
// Scheduler / core configuration
// -----------------------------

#define configUSE_PREEMPTION 1
#define configUSE_TIME_SLICING 1
#define configUSE_TICKLESS_IDLE 0

// RP2040 default clk_sys is 125 MHz unless you change it.
#define configCPU_CLOCK_HZ ( ( unsigned long ) 125000000UL )
#define configTICK_RATE_HZ ( ( TickType_t ) 1000 )

#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS

#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE ( ( unsigned short ) 256 ) // words, not bytes
#define configMAX_TASK_NAME_LEN 16

// -----------------------------
// Memory allocation
// -----------------------------

#define configSUPPORT_STATIC_ALLOCATION 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1

// heap_4.c uses this as the heap size (bytes).
// Tune this as you add tasks/queues/etc.
#define configTOTAL_HEAP_SIZE ( ( size_t ) ( 64 * 1024 ) )

// -----------------------------
// Optional features
// -----------------------------

#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1

#define configUSE_TIMERS 1
#define configTIMER_QUEUE_LENGTH 10
#define configTIMER_TASK_PRIORITY ( configMAX_PRIORITIES - 1 )
#define configTIMER_TASK_STACK_DEPTH ( configMINIMAL_STACK_SIZE )

#define configUSE_QUEUE_SETS 0
#define configQUEUE_REGISTRY_SIZE 0

#define configUSE_TASK_NOTIFICATIONS 1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 1

// -----------------------------
// Hooks / debugging
// -----------------------------

#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configUSE_MALLOC_FAILED_HOOK 1
#define configCHECK_FOR_STACK_OVERFLOW 2

#define configUSE_TRACE_FACILITY 0
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_STATS_FORMATTING_FUNCTIONS 0

#define configASSERT( x )                                                        \
  if ( ( x ) == 0 ) {                                                            \
    portDISABLE_INTERRUPTS();                                                    \
    vAssertCalled(__FILE__, __LINE__);                                           \
  }

// -----------------------------
// RP2040 / Cortex-M0+ specifics
// -----------------------------

#define configENABLE_MPU 0

// Pico SDK uses the standard exception names via CMSIS renaming by default, so
// Some Pico SDK builds route exceptions via `isr_*` symbols, so we provide
// wrapper handlers (indirect routing). When indirect routing is used, this must
// be disabled.
#define configCHECK_HANDLER_INSTALLATION 0

// -----------------------------
// API inclusion (keep minimal)
// -----------------------------

#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_xTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_vTaskCleanUpResources 0
#define INCLUDE_xTaskGetIdleTaskHandle 0
#define INCLUDE_eTaskGetState 0
#define INCLUDE_uxTaskGetStackHighWaterMark 0
#define INCLUDE_xTaskAbortDelay 0
#define INCLUDE_xTaskGetHandle 0

