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
    // The way the ADG728 works is kinda cool
    // you can select multiple channels if you mask more than one bit
    // we just need one though so we'll set 1u (1) in the bit we want shifted by the channel number
    uint8_t mask = (channel <= 7) ? (1u << channel) : 0;
    return i2c_write_blocking(i2c, addr, &mask, 1, false) == 1;
}
