#pragma once

#include "globals.h"

/**
 * Fault reporting via RTOS queue. Tasks raise faults with fault_raise(); the
 * safety task drains the queue, sets the
 * global FAULT, and runs the registered on_fault handler.
 *
 * Call fault_init() once before starting the scheduler.
 * Call fault_process() from the safety task each loop.
 */

void fault_init(void);

/** Send a fault to the safety handler. Use FAULT_CODE_NONE to clear. */
void fault_raise(fault_code_t code);

/** Process one fault from the queue: set FAULT and call on_fault handler. Call from safety task. */
void fault_process(void);

/** Register handler run when a fault is applied (set loads to safe state, etc.). */
void fault_register_on_fault(void (*handler)(fault_code_t));
