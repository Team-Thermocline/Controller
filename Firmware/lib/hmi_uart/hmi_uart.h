#pragma once

#include <stddef.h>
#include <stdint.h>

/** PIO UART on HMI_TX_PIN / HMI_RX_PIN (pindefs.h), 8N1 @ 115200. Uses PIO1. */
void hmi_uart_init(void);
void hmi_uart_write(const uint8_t *data, size_t len);
/** @return 0–255, or -1 if RX FIFO empty */
int hmi_uart_try_getc(void);
