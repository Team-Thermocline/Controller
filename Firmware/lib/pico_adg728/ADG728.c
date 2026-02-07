/**
 * Minimal Pico SDK driver for ADG728 I2C 1x8 multiplexer switch.
 * Inspired by Rob Tillaart's Arduino library: https://github.com/RobTillaart/ADG728
 */

#include "ADG728.h"
#include "hardware/i2c.h"

bool adg728_probe(i2c_inst_t *i2c, uint8_t addr)
{
    uint8_t zero = 0;
    return i2c_write_blocking(i2c, addr, &zero, 1, false) == 1;
}

bool adg728_init(i2c_inst_t *i2c, uint8_t addr)
{
    return adg728_probe(i2c, addr);
}

bool adg728_select_channel(i2c_inst_t *i2c, uint8_t addr, uint8_t channel)
{
    uint8_t mask = (channel <= 7) ? (1u << channel) : 0;
    return i2c_write_blocking(i2c, addr, &mask, 1, false) == 1;
}
