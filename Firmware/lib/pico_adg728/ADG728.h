#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h"

/**
 * ADG728 I2C address range: 0x4C .. 0x4F (set by hardware pins)
 */
#define ADG728_ADDR_MIN  0x4C
#define ADG728_ADDR_MAX  0x4F

/**
 * Probes the I2C bus for an ADG728 at the given address.
 * Returns true if the device ACKs.
 */
bool adg728_probe(i2c_inst_t *i2c, uint8_t addr);

/**
 * Probes and initializes the ADG728 (all channels off).
 * Returns true if the device is present and init succeeded.
 */
bool adg728_init(i2c_inst_t *i2c, uint8_t addr);

/**
 * Select a single channel 0..7 (all others off).
 * Returns true on success.
 */
bool adg728_select_channel(i2c_inst_t *i2c, uint8_t addr, uint8_t channel);
