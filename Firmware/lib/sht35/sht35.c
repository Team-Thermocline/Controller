#include "sht35.h"

#include "hardware/timer.h"

// SHT35 commands (MSB first)
#define SHT35_CMD_SOFT_RESET 0x30A2u
// Single-shot
#define SHT35_CMD_SINGLE_SHOT_HIGH 0x2400u

// CRC-8 polynomial for SHT3x (x^8 + x^5 + x^4 + 1 = 0x31), initial 0xFF
static uint8_t sht35_crc8(const uint8_t *data, int len) {
  uint8_t crc = 0xFF;
  for (int i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int b = 0; b < 8; ++b) {
      if (crc & 0x80)
        crc = (uint8_t)((crc << 1) ^ 0x31u);
      else
        crc <<= 1;
    }
  }
  return crc;
}

static bool sht35_write_cmd(const sht35_t *dev, uint16_t cmd) {
  uint8_t buf[2];
  buf[0] = (uint8_t)((cmd >> 8) & 0xFF);
  buf[1] = (uint8_t)(cmd & 0xFF);
  int rc = i2c_write_blocking(dev->i2c, dev->addr, buf, 2, false);
  return rc == 2;
}

bool sht35_init(sht35_t *dev, i2c_inst_t *i2c, uint8_t addr) {
  if (!dev || !i2c)
    return false;
  dev->i2c = i2c;
  dev->addr = addr;

  // Soft reset;
  (void)sht35_write_cmd(dev, SHT35_CMD_SOFT_RESET);
  sleep_ms(10);
  return true;
}

bool sht35_probe(const sht35_t *dev) {
  if (!dev || !dev->i2c)
    return false;

  // Just check soft-reset to see if its alive/there
  return sht35_write_cmd(dev, SHT35_CMD_SOFT_RESET);
}

bool sht35_read_single_shot(const sht35_t *dev, float *out_temp_c,
                            float *out_rh) {
  if (!dev || !dev->i2c || !out_temp_c || !out_rh)
    return false;

  if (!sht35_write_cmd(dev, SHT35_CMD_SINGLE_SHOT_HIGH))
    return false;

  // Max conversion time for high repeatability ~15 ms; wait a bit longer.
  sleep_ms(20);

  uint8_t rx[6];
  int rc = i2c_read_blocking(dev->i2c, dev->addr, rx, 6, false);
  if (rc != 6)
    return false;

  // CRC check temperature
  if (sht35_crc8(&rx[0], 2) != rx[2])
    return false;
  // CRC check humidity
  if (sht35_crc8(&rx[3], 2) != rx[5])
    return false;

  uint16_t raw_t = ((uint16_t)rx[0] << 8) | rx[1];
  uint16_t raw_rh = ((uint16_t)rx[3] << 8) | rx[4];

  // Conversion formulas from SHT3x datasheet:
  // T = -45 + 175 * raw / 65535
  // RH = 100 * raw / 65535
  float t = -45.0f + (175.0f * (float)raw_t) / 65535.0f;
  float rh = (100.0f * (float)raw_rh) / 65535.0f;

  if (rh < 0.0f)
    rh = 0.0f;
  if (rh > 100.0f)
    rh = 100.0f;

  *out_temp_c = t;
  *out_rh = rh;
  return true;
}

