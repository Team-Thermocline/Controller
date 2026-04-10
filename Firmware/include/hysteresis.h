#pragma once

#include <stdbool.h>

/**
 * Two-point hysteresis helpers for float sensor values (relay / latch style).
 *
 * Each holds persistent `on` state between calls. On invalid input, output is
 * forced false and the latch clears — match typical "safe off" behavior.
 */

/** "Call when cold": on when value < low_on; off when value > high_off; else hold. */
typedef struct {
  bool on;
} hyst_below_t;

/** "Call when hot": on when value >= on_above; off when value <= off_below; else hold. */
typedef struct {
  bool on;
} hyst_above_t;

static inline void hyst_below_reset(hyst_below_t *s) { s->on = false; }
static inline void hyst_above_reset(hyst_above_t *s) { s->on = false; }

/**
 * Heater-style band around a target: require low_on < high_off (usually target−h, target+h).
 */
static inline bool hyst_below_update(hyst_below_t *s, bool valid, float value,
                                     float low_on_below, float high_off_above) {
  if (!valid) {
    s->on = false;
    return false;
  }
  if (value < low_on_below)
    s->on = true;
  else if (value > high_off_above)
    s->on = false;
  return s->on;
}

/**
 * High-side Schmitt: require off_below < on_above (e.g. off at delta 15, on at delta 20).
 */
static inline bool hyst_above_update(hyst_above_t *s, bool valid, float value,
                                     float on_at_or_above, float off_at_or_below) {
  if (!valid) {
    s->on = false;
    return false;
  }
  if (!s->on) {
    if (value >= on_at_or_above)
      s->on = true;
  } else {
    if (value <= off_at_or_below)
      s->on = false;
  }
  return s->on;
}
