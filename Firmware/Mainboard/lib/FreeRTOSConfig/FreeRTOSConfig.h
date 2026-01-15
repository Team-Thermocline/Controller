#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#define configUSE_PREEMPTION 1
#define configUSE_IDLE_HOOK 0
#define configUSE_PASSIVE_IDLE_HOOK 1
#define configUSE_TICK_HOOK 0
#define configRUN_MULTIPLE_PRIORITIES 1
#define configCPU_CLOCK_HZ (125000000UL)
#define configTICK_RATE_HZ ((uint32_t)1000)
#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE ((unsigned short)128)
#define configMAX_TASK_NAME_LEN 16
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 1
#define configUSE_TASK_NOTIFICATIONS 1
#define configUSE_MUTEXES 1
#define configUSE_RECURSIVE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_QUEUE_SETS 0
#define configQUEUE_REGISTRY_SIZE 8
#define configUSE_TIME_SLICING 1
#define configUSE_NEWLIB_REENTRANT 1
#define configENABLE_BACKWARD_COMPATIBILITY 0

#define configSUPPORT_STATIC_ALLOCATION 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configTOTAL_HEAP_SIZE ((size_t)(32 * 1024))

#define configUSE_CO_ROUTINES 0
#define configMAX_CO_ROUTINE_PRIORITIES 2

#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH 10
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2)

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

#define configCHECK_FOR_STACK_OVERFLOW 2
#define configUSE_MALLOC_FAILED_HOOK 1
#define configUSE_TRACE_FACILITY 1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#define configUSE_TASK_PREEMPTION_DISABLE 1
#define configNUMBER_OF_CORES 2
#define configUSE_CORE_AFFINITY 1

#define configPRIO_BITS 2
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 3
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 1

#define configKERNEL_INTERRUPT_PRIORITY                                        \
  (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY                                   \
  (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define configASSERT(x)                                                        \
  if ((x) == 0) {                                                              \
    __asm volatile("cpsid i");                                                 \
    for (;;) {                                                                 \
    }                                                                          \
  }

// RP2040 SMP spinlock definitions (required for multi-core FreeRTOS)
#ifndef configSMP_SPINLOCK_0
#define configSMP_SPINLOCK_0    8  // PICO_SPINLOCK_ID_OS1
#endif
#ifndef configSMP_SPINLOCK_1
#define configSMP_SPINLOCK_1    9  // PICO_SPINLOCK_ID_OS2
#endif

// Include API functions
#ifndef INCLUDE_xQueueGetMutexHolder
#define INCLUDE_xQueueGetMutexHolder 1
#endif
#ifndef INCLUDE_vTaskDelay
#define INCLUDE_vTaskDelay 1
#endif

#endif /* FREERTOS_CONFIG_H */
