#pragma once

// TCODE Parser
// Written by Joe
// For capstone 2025-2026
// at Southern New Hampshire University

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TCODE_MAX_TOKENS
#define TCODE_MAX_TOKENS 32
#endif

typedef enum tcode_status {
  TCODE_OK = 0,
  TCODE_ERR_EMPTY = 1,
  TCODE_ERR_TOO_MANY_TOKENS = 2,
  TCODE_ERR_CHECKSUM_FORMAT = 3,
  TCODE_ERR_CHECKSUM_MISMATCH = 4,
} tcode_status_t;

typedef struct tcode_parsed_line {
  bool has_checksum;
  uint8_t given_checksum;
  uint8_t calculated_checksum;

  uint8_t token_count;
  char *tokens[TCODE_MAX_TOKENS]; // pointers into the caller's buffer
} tcode_parsed_line_t;

// Parse a line in-place
// @param line - the line to parse
// @param out - the parsed line
// @return the status of the parse (tcode_status_t)
//
// The input buffer will be modified (spaces and '*' replaced with '\0').
tcode_status_t tcode_parse_inplace(char *line, tcode_parsed_line_t *out);

// XOR checksum of a null-terminated string.
uint8_t tcode_checksum_xor(const char *s);

// parse two hex chars into a byte. Returns true on success.
bool tcode_parse_hex_u8(const char *hex2, uint8_t *out);

// readable string for status.
const char *tcode_status_str(tcode_status_t st);

#ifdef __cplusplus
} // extern "C"
#endif

