#include "tools.h"

#include "ADG728.h"
#include "hardware/i2c.h"
#include "sht35.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void build_i2c_scan_string(char *out, size_t out_len) {
  if (!out || out_len == 0)
    return;
  out[0] = '\0';

  bool first = true;

  // Probe ADG728 range 0x4C–0x4F using its probe helper.
  for (uint8_t a = ADG728_ADDR_MIN; a <= ADG728_ADDR_MAX; ++a) {
    if (adg728_probe(i2c0, a)) {
      snprintf(out + strlen(out), out_len - strlen(out),
               "%sADG728@0x%02X", first ? "" : ",", a);
      first = false;
    }
  }

  // I didn't have a great way to probe SHT35 so i just used its the soft-reset
  // the same way i check for its health on init in analog_task.c
  sht35_t dev;
  for (uint8_t addr = 0x44u; addr <= 0x45u; ++addr) {
    sht35_init(&dev, i2c0, addr);
    if (sht35_probe(&dev)) {
      snprintf(out + strlen(out), out_len - strlen(out),
               "%sSHT35@0x%02X", first ? "" : ",", addr);
      first = false;
    }
  }

  if (first) {
    // Nothing found
    snprintf(out, out_len, "none");
  }
}

