#include "hmi_uart.h"

#include <stdbool.h>

#include "hmi_uart.pio.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/sync.h"
#include "pindefs.h"

#define HMI_UART_BAUD 115200u

/* Monotonic write / read indices; difference bounded by ring capacity. */
#define HMI_TX_RING_CAP 512u
#define HMI_RX_RING_CAP 512u

static PIO const hmi_pio = pio1;
static uint hmi_sm_tx;
static uint hmi_sm_rx;
static bool hmi_inited;

static uint8_t tx_rb[HMI_TX_RING_CAP];
static uint8_t rx_rb[HMI_RX_RING_CAP];
static uint32_t tx_w;
static uint32_t tx_r;
static uint32_t rx_w;
static uint32_t rx_r;

static volatile bool hmi_rx_overflow;

static pio_interrupt_source_t hmi_rx_irq_src(void) {
  return (pio_interrupt_source_t)(pis_sm0_rx_fifo_not_empty + hmi_sm_rx);
}

static pio_interrupt_source_t hmi_tx_irq_src(void) {
  return (pio_interrupt_source_t)(pis_sm0_tx_fifo_not_full + hmi_sm_tx);
}

static void hmi_uart_isr_pump_rx(void) {
  while (!pio_sm_is_rx_fifo_empty(hmi_pio, hmi_sm_rx)) {
    uint32_t w = pio_sm_get(hmi_pio, hmi_sm_rx);
    int byte = (int)((w >> 24) & 0xFFu);

    if (rx_w - rx_r >= HMI_RX_RING_CAP) {
      hmi_rx_overflow = true;
      continue;
    }
    rx_rb[rx_w % HMI_RX_RING_CAP] = (uint8_t)byte;
    rx_w++;
  }
}

static void hmi_uart_isr_pump_tx(void) {
  while (!pio_sm_is_tx_fifo_full(hmi_pio, hmi_sm_tx)) {
    if (tx_w == tx_r)
      break;
    pio_sm_put(hmi_pio, hmi_sm_tx, (uint32_t)tx_rb[tx_r % HMI_TX_RING_CAP]);
    tx_r++;
  }
  if (tx_w == tx_r)
    pio_set_irq0_source_enabled(hmi_pio, hmi_tx_irq_src(), false);
}

static void hmi_uart_irq_handler(void) {
  hmi_uart_isr_pump_rx();
  hmi_uart_isr_pump_tx();
}

void hmi_uart_init(void) {
  if (hmi_inited)
    return;

  tx_w = tx_r = 0;
  rx_w = rx_r = 0;
  hmi_rx_overflow = false;

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

  irq_set_exclusive_handler(PIO1_IRQ_0, hmi_uart_irq_handler);
  irq_set_enabled(PIO1_IRQ_0, true);

  pio_set_irq0_source_enabled(hmi_pio, hmi_rx_irq_src(), true);
  pio_set_irq0_source_enabled(hmi_pio, hmi_tx_irq_src(), false);

  hmi_inited = true;
}

void hmi_uart_write(const uint8_t *data, size_t len) {
  if (!hmi_inited || !data || len == 0)
    return;

  uint32_t ints = save_and_disable_interrupts();
  for (size_t i = 0; i < len; i++) {
    if (tx_w - tx_r >= HMI_TX_RING_CAP)
      break;
    tx_rb[tx_w % HMI_TX_RING_CAP] = data[i];
    tx_w++;
  }
  restore_interrupts(ints);

  if (tx_w != tx_r)
    pio_set_irq0_source_enabled(hmi_pio, hmi_tx_irq_src(), true);
}

int hmi_uart_try_getc(void) {
  if (!hmi_inited)
    return -1;

  uint32_t ints = save_and_disable_interrupts();
  if (rx_w == rx_r) {
    restore_interrupts(ints);
    return -1;
  }
  uint8_t c = rx_rb[rx_r % HMI_RX_RING_CAP];
  rx_r++;
  restore_interrupts(ints);
  return (int)c;
}

bool hmi_uart_rx_overflowed(void) {
  return hmi_rx_overflow;
}

void hmi_uart_clear_rx_overflow(void) {
  hmi_rx_overflow = false;
}
