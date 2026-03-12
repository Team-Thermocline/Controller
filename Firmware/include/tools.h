#pragma once

#include <stddef.h>

// This was really handy to have in testing so i just decided to bake it into the program properly.
// This should build a short I2C device summary string (up to out_len-1 chars, and NUL-terminated).
// Does some funny one-off things to check for devices that dont scan super easily but, handy
// for the Q1 I2C_SCAN command
void build_i2c_scan_string(char *out, size_t out_len);

