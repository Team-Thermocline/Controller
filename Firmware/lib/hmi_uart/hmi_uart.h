#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** PIO UART on HMI_TX_PIN / HMI_RX_PIN (pindefs.h), 8N1 @ 115200. Uses PIO1. */
void hmi_uart_init(void);
/** Enqueues bytes for TX (non-blocking); PIO IRQ drains to the TX FIFO. */
void hmi_uart_write(const uint8_t *data, size_t len);
/** @return 0–255, or -1 if no byte available (software RX ring). */
int hmi_uart_try_getc(void);
/** True if the HMI RX ring overflowed (host sent faster than firmware drained). */
bool hmi_uart_rx_overflowed(void);
void hmi_uart_clear_rx_overflow(void);
