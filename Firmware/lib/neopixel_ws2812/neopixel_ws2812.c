#include "neopixel_ws2812.h"

#include "hardware/clocks.h"
#include "pico/time.h"
#include "ws2812.pio.h"

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
  // WS2812 expects GRB byte order
  return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

static inline void ws2812_put_word(neopixel_ws2812_t *np, uint32_t pixel_grb) {
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
                          float freq_hz, bool is_rgbw, uint num_pixels) {
  if (!np || num_pixels == 0 || num_pixels > NEOPIXEL_WS2812_MAX_PIXELS) {
    return;
  }

  np->pio = pio;
  np->pin = pin;
  np->is_rgbw = is_rgbw;
  np->num_pixels = num_pixels;

  for (uint i = 0; i < num_pixels; i++)
    np->pixel_buf[i] = 0;

  uint offset = pio_add_program(pio, &ws2812_program);
  np->sm = pio_claim_unused_sm(pio, true);

  ws2812_sm_init(pio, np->sm, offset, pin, freq_hz, is_rgbw);

  for (uint i = 0; i < num_pixels; i++)
    ws2812_put_word(np, 0);
  sleep_ms(10);
}

void neopixel_ws2812_flush(neopixel_ws2812_t *np) {
  if (!np) return;
  for (uint i = 0; i < np->num_pixels; i++)
    ws2812_put_word(np, np->pixel_buf[i]);
}

void neopixel_ws2812_put_rgb(neopixel_ws2812_t *np, uint8_t r, uint8_t g,
                             uint8_t b) {
  if (!np) return;
  uint32_t grb = urgb_u32(r, g, b);
  for (uint i = 0; i < np->num_pixels; i++)
    np->pixel_buf[i] = grb;
  neopixel_ws2812_flush(np);
}

void neopixel_ws2812_set_pixel_rgb(neopixel_ws2812_t *np, uint index,
                                  uint8_t r, uint8_t g, uint8_t b) {
  if (!np || index >= np->num_pixels) return;
  np->pixel_buf[index] = urgb_u32(r, g, b);
  neopixel_ws2812_flush(np);
}

void neopixel_ws2812_put_grb_u32(neopixel_ws2812_t *np, uint32_t grb) {
  if (!np) return;
  for (uint i = 0; i < np->num_pixels; i++)
    np->pixel_buf[i] = grb;
  neopixel_ws2812_flush(np);
}

