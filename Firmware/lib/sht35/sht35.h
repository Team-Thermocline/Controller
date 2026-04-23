#pragma once

#include "hardware/i2c.h"
#include <stdbool.h>
#include <stdint.h>

// SHT35 (SHT3x): ADDR low = 0x44, ADDR high (VDD) = 0x45.
#define SHT35_I2C_ADDR_LOW  0x44u
#define SHT35_I2C_ADDR_HIGH 0x45u
#define SHT35_DEFAULT_ADDR  SHT35_I2C_ADDR_HIGH

typedef struct {
  i2c_inst_t *i2c;
  uint8_t addr; // 7-bit I2C address
} sht35_t;

// Initialize an SHT35 instance
// Returns true if a soft-reset command was acknowledged.
bool sht35_init(sht35_t *dev, i2c_inst_t *i2c, uint8_t addr);

// Probe the sensor by issuing a no-op soft reset.
// Returns true if the device ACKs on the bus.
bool sht35_probe(const sht35_t *dev);

// Perform a single-shot measurement
// On success, writes temperature in °C and relative humidity in %RH.
// Returns true on success, false on I2C error or CRC failure.
bool sht35_read_single_shot(const sht35_t *dev, float *out_temp_c,
                            float *out_rh);

