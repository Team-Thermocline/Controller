#include "hmi_uart.h"

#include "hmi_uart.pio.h"
#include "hardware/pio.h"
#include "pindefs.h"

#define HMI_UART_BAUD 115200u

static PIO const hmi_pio = pio1;
static uint hmi_sm_tx;
static uint hmi_sm_rx;
static bool hmi_inited;

void hmi_uart_init(void) {
  if (hmi_inited)
    return;

  uint offset_tx = pio_add_program(hmi_pio, &uart_tx_program);
  uint offset_rx = pio_add_program(hmi_pio, &uart_rx_mini_program);
  hmi_sm_tx = pio_claim_unused_sm(hmi_pio, true);
  hmi_sm_rx = pio_claim_unused_sm(hmi_pio, true);

  hmi_uart_tx_program_init(hmi_pio, hmi_sm_tx, offset_tx, HMI_TX_PIN,
                           HMI_UART_BAUD);
  hmi_uart_rx_mini_program_init(hmi_pio, hmi_sm_rx, offset_rx, HMI_RX_PIN,
                                HMI_UART_BAUD);
  pio_sm_clear_fifos(hmi_pio, hmi_sm_tx);
  pio_sm_clear_fifos(hmi_pio, hmi_sm_rx);
  hmi_inited = true;
}

void hmi_uart_write(const uint8_t *data, size_t len) {
  if (!hmi_inited || !data)
    return;
  for (size_t i = 0; i < len; i++)
    pio_sm_put_blocking(hmi_pio, hmi_sm_tx, (uint32_t)data[i]);
}

int hmi_uart_try_getc(void) {
  if (!hmi_inited)
    return -1;
  if (pio_sm_is_rx_fifo_empty(hmi_pio, hmi_sm_rx))
    return -1;
  /* 32-bit read pops one FIFO entry. uart_rx_mini autopush (shift right,
   * threshold 8) leaves the character in bits 31:24 (see pico-examples uart_rx). */
  uint32_t w = pio_sm_get(hmi_pio, hmi_sm_rx);
  return (int)((w >> 24) & 0xFFu);
}
