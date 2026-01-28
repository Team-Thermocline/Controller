#ifndef FAULT_LED_H
#define FAULT_LED_H

#include <Arduino.h>

/**
 * @brief Initialize fault LED pin
 *
 * Call this in setup() to configure the fault LED pin as output.
 */
void faultLedInit();

/**
 * @brief Set fault LED on
 */
void faultLedSet();

/**
 * @brief Clear/reset fault LED off
 */
void faultLedReset();

/**
 * @brief Flash fault LED with specified timing
 *
 * @param onTimeMs Time in milliseconds LED stays on
 * @param offTimeMs Time in milliseconds LED stays off
 * @param count Number of flash cycles (0 = infinite)
 */
void faultLedFlash(uint32_t onTimeMs, uint32_t offTimeMs, uint32_t count = 0);

#endif // FAULT_LED_H
