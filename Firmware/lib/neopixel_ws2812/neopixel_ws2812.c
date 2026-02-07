#include "neopixel_ws2812.h"

#include "hardware/clocks.h"
#include "pico/time.h"
#include "ws2812.pio.h"

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  // WS2812 expects GRB byte order
  return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

static inline void neopixel_ws2812_put_pixel(neopixel_ws2812_t *np,
                                             uint32_t pixel_grb) {
  // The WS2812 program shifts out MSB-first, with data in bits 23:0.
  // Shift left 8 so the top 24 bits of the FIFO word are used.
  pio_sm_put_blocking(np->pio, np->sm, pixel_grb << 8u);
}

static void ws2812_sm_init(PIO pio, uint sm, uint offset, uint pin,
                           float freq_hz, bool rgbw) {
  pio_sm_config c = ws2812_program_get_default_config(offset);

  sm_config_set_sideset_pins(&c, pin);
  sm_config_set_out_shift(&c, false, true, rgbw ? 32 : 24);
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

  int cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
  float div = (float)clock_get_hz(clk_sys) / (freq_hz * (float)cycles_per_bit);
  sm_config_set_clkdiv(&c, div);

  pio_gpio_init(pio, pin);
  pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);
  pio_sm_init(pio, sm, offset, &c);
  pio_sm_set_enabled(pio, sm, true);
}

void neopixel_ws2812_init(neopixel_ws2812_t *np, PIO pio, uint pin,
                          float freq_hz, bool is_rgbw) {
  if (!np) {
    return;
  }

  np->pio = pio;
  np->pin = pin;
  np->is_rgbw = is_rgbw;

  uint offset = pio_add_program(pio, &ws2812_program);
  np->sm = pio_claim_unused_sm(pio, true);

  ws2812_sm_init(pio, np->sm, offset, pin, freq_hz, is_rgbw);

  // Start "off"
  neopixel_ws2812_put_pixel(np, urgb_u32(0, 0, 0));
  sleep_ms(10);
}

void neopixel_ws2812_put_rgb(neopixel_ws2812_t *np, uint8_t r, uint8_t g,
                             uint8_t b) {
  if (!np) {
    return;
  }
  neopixel_ws2812_put_pixel(np, urgb_u32(r, g, b));
}

void neopixel_ws2812_put_grb_u32(neopixel_ws2812_t *np, uint32_t grb) {
  if (!np) {
    return;
  }
  neopixel_ws2812_put_pixel(np, grb);
}

