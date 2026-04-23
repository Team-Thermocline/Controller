#pragma once

#include <stdint.h>

/**
 * Approximate non-idle CPU use over the whole run so far: 100 minus the idle
 * task's share of total measured run time (FreeRTOS run-time stats).
 *
 * Range 0–100. ISR time is mostly attributed to whichever task was running when
 * the ISR preempted it. The first moments after boot may read 0 before enough
 * time has accumulated for a stable percentage.
 */
uint32_t freertos_cpu_load_pct(void);
