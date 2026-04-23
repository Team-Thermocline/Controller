#include "interior_led_task.h"

#include "constants.h"
#include "globals.h"
#include "hardware/pio.h"
#include "neopixel_ws2812.h"
#include "pindefs.h"
#include <math.h>
#include <stdint.h>

// NeoPixel (WS2812) config for interior strip
static const float NEOPIXEL_FREQ_HZ = 800000.0f;
static const uint NEOPIXEL_NUM_PIXELS = 11;
static neopixel_ws2812_t g_interior_strip;

// Layout (11 pixels total):
// - Even indices 0,2,4,6,8,10 : vanity (white) lights
// - Odd  indices 1,3,5,7,9    : status lights

typedef enum {
  INTERIOR_ANIM_IDLE = 0,   // Idle: all white
  INTERIOR_ANIM_DOOR = 1,   // Smooth yellow breathing
  INTERIOR_ANIM_FAULT = 2,  // Flashing red
} interior_anim_mode_t;
static inline uint32_t pack_grb(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;
}

static inline float clampf(float x, float lo, float hi) {
  if (x < lo)
    return lo;
  if (x > hi)
    return hi;
  return x;
}

static inline uint8_t lerp_u8(uint8_t a, uint8_t b, float t) {
  t = clampf(t, 0.0f, 1.0f);
  const float v = (float)a + ((float)b - (float)a) * t;
  if (v <= 0.0f)
    return 0u;
  if (v >= 255.0f)
    return 255u;
  return (uint8_t)(v + 0.5f);
}

static uint32_t apply_temp_tint_grb(uint8_t base_r, uint8_t base_g,
                                   uint8_t base_b, float tint_strength,
                                   bool cold_side) {
  const float t = clampf(tint_strength, 0.0f, 1.0f);

  const uint8_t tr =
      cold_side ? (uint8_t)LED_TINT_COLD_R : (uint8_t)LED_TINT_HOT_R;
  const uint8_t tg =
      cold_side ? (uint8_t)LED_TINT_COLD_G : (uint8_t)LED_TINT_HOT_G;
  const uint8_t tb =
      cold_side ? (uint8_t)LED_TINT_COLD_B : (uint8_t)LED_TINT_HOT_B;

  const uint8_t out_r = lerp_u8(base_r, tr, t);
  const uint8_t out_g = lerp_u8(base_g, tg, t);
  const uint8_t out_b = lerp_u8(base_b, tb, t);
  return pack_grb(out_r, out_g, out_b);
}

static void update_interior_strip(interior_anim_mode_t mode,
                                  bool fault_on,
                                  float phase,
                                  float tint_strength,
                                  bool cold_side) {
  // Vanity lights: steady, soft white.
  const uint8_t vanity_level = 128;
  for (uint i = 0; i < 6; ++i) {
    uint index = (uint)(i * 2); // 0,2,4,6,8,10
    g_interior_strip.pixel_buf[index] =
        pack_grb(vanity_level, vanity_level, vanity_level);
  }

  for (uint i = 0; i < 5; ++i) {
    uint index = (uint)(i * 2 + 1); // 1,3,5,7,9
    if (mode == INTERIOR_ANIM_FAULT) {
      // Fault: fast red flash on status pixels (highest priority).
      uint8_t level = fault_on ? 255 : 0;
      g_interior_strip.pixel_buf[index] = pack_grb(level, 0, 0);
    } else if (mode == INTERIOR_ANIM_DOOR) {
      // Door-open status: smooth yellow <-> black using sinusoid.
      float s = 0.5f * (sinf(phase) + 1.0f); // 0..1
      if (s < 0.0f) s = 0.0f;
      if (s > 1.0f) s = 1.0f;
      uint8_t status_level = (uint8_t)(s * 255.0f);
      g_interior_strip.pixel_buf[index] =
          pack_grb(status_level, status_level, 0);
    } else {
      // No status to display: keep these as steady white like vanity lights.
      if (mode == INTERIOR_ANIM_IDLE) {
        g_interior_strip.pixel_buf[index] = apply_temp_tint_grb(
            vanity_level, vanity_level, vanity_level, tint_strength, cold_side);
      } else {
        g_interior_strip.pixel_buf[index] =
            pack_grb(vanity_level, vanity_level, vanity_level);
      }
    }
  }

  neopixel_ws2812_flush(&g_interior_strip);
}

static void interior_led_task(void *pvParameters) {
  (void)pvParameters;

  // Initialize the NeoPixel interior strip once the scheduler is running.
  neopixel_ws2812_init(&g_interior_strip, pio0, NEOPIXEL_PIN, NEOPIXEL_FREQ_HZ,
                       false, NEOPIXEL_NUM_PIXELS);
  neopixel_ws2812_put_rgb(&g_interior_strip, 0, 0, 0);

  float phase = 0.0f;
  uint32_t fault_tick = 0;
  float tint_strength = 0.0f;

  while (true) {
    // Use global door_open. Alert when door closed and not standby.
    bool should_alert_door =
        !door_open && chamber_fsm_state != CHAMBER_STANDBY;

    // Advance internal phase
    phase += 0.12f;
    if (phase > 6.2831853f) {
      phase -= 6.2831853f;
    }

    // Select animation
    interior_anim_mode_t mode = INTERIOR_ANIM_IDLE;
    if (FAULT != FAULT_CODE_NONE) {
      mode = INTERIOR_ANIM_FAULT;
    } else if (should_alert_door) {
      mode = INTERIOR_ANIM_DOOR;
    }

    // ================
    // Fault animation
    // ================
    bool fault_on = false;
    if (mode == INTERIOR_ANIM_FAULT) {
      fault_tick++;
      // Toggle on/off every 10 ticks (~250 ms with 25 ms delay).
      fault_on = ((fault_tick / 10u) & 0x1u) != 0;
    }

    /* Temperature tint */
    const float temp_c = (float)sht35_temperature_c;
    const bool cold_side = temp_c < LED_TINT_NEUTRAL_C;
    float target_strength = 0.0f;
    if (temp_c < LED_TINT_NEUTRAL_C) {
      const float denom = (LED_TINT_NEUTRAL_C - LED_TINT_COLDEST_C);
      const float x = (denom > 0.001f) ? ((LED_TINT_NEUTRAL_C - temp_c) / denom) : 0.0f;
      target_strength = clampf(x, 0.0f, 1.0f) * LED_TINT_MAX_STRENGTH;
    } else if (temp_c > LED_TINT_NEUTRAL_C) {
      const float denom = (LED_TINT_HOTTEST_C - LED_TINT_NEUTRAL_C);
      const float x = (denom > 0.001f) ? ((temp_c - LED_TINT_NEUTRAL_C) / denom) : 0.0f;
      target_strength = clampf(x, 0.0f, 1.0f) * LED_TINT_MAX_STRENGTH;
    }

    /* Slew tint slowly for smoothness (runs every 25 ms). */
    const float dt = 0.025f;
    const float max_step = LED_TINT_SLEW_PER_SEC * dt;
    if (tint_strength < target_strength) {
      tint_strength = fminf(tint_strength + max_step, target_strength);
    } else if (tint_strength > target_strength) {
      tint_strength = fmaxf(tint_strength - max_step, target_strength);
    }

    // Update the strip
    update_interior_strip(mode, fault_on, phase, tint_strength, cold_side);

    // Delay
    vTaskDelay(pdMS_TO_TICKS(25));
  }
}

BaseType_t interior_led_task_create(UBaseType_t priority,
                                    TaskHandle_t *out_handle) {
  return xTaskCreate(interior_led_task, "interior", 256, NULL, priority,
                     out_handle);
}
