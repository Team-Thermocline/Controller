#include "tcode_protocol.h"

#include <string.h>

// Helper function to convert a hex character to a nibble
static int hex_nibble(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return 10 + (c - 'a');
  if (c >= 'A' && c <= 'F')
    return 10 + (c - 'A');
  return -1;
}

// XOR checksum of a null-terminated string.
uint8_t tcode_checksum_xor(const char *s) {
  uint8_t x = 0;
  if (!s)
    return 0;
  for (const char *p = s; *p; ++p) {
    x ^= (uint8_t)(unsigned char)(*p);
  }
  return x;
}

// parse two hex chars into a byte. Returns true on success.
bool tcode_parse_hex_u8(const char *hex2, uint8_t *out) {
  if (!hex2 || !out)
    return false;
  int hi = hex_nibble(hex2[0]);
  int lo = hex_nibble(hex2[1]);
  if (hi < 0 || lo < 0)
    return false;
  *out = (uint8_t)((hi << 4) | lo);
  return true;
}

// readable string for status. You could directly send this back over serial to a client
const char *tcode_status_str(tcode_status_t st) {
  switch (st) {
  case TCODE_OK:
    return "OK";
  case TCODE_ERR_EMPTY:
    return "EMPTY";
  case TCODE_ERR_TOO_MANY_TOKENS:
    return "TOO_MANY_TOKENS";
  case TCODE_ERR_CHECKSUM_FORMAT:
    return "CHECKSUM_FORMAT";
  case TCODE_ERR_CHECKSUM_MISMATCH:
    return "CHECKSUM_MISMATCH";
  default:
    return "UNKNOWN";
  }
}

// remove trailing whitespace from a string
static void rstrip(char *s) {
  if (!s)
    return;
  size_t n = strlen(s);
  while (n > 0) {
    char c = s[n - 1];
    if (c == '\r' || c == '\n' || c == ' ' || c == '\t')
      s[--n] = '\0';
    else
      break;
  }
}

// parse a line in-place
// @param line - the line to parse
// @param out - the parsed line
// @return the status of the parse (tcode_status_t)
tcode_status_t tcode_parse_inplace(char *line, tcode_parsed_line_t *out) {
  if (!out)
    // Need to have somewhere to put the line into.
    return TCODE_ERR_EMPTY;
  memset(out, 0, sizeof(*out));

  if (!line)
    // Have to have a line to process
    return TCODE_ERR_EMPTY;

  rstrip(line); // Strip the line
  if (!*line)
    // If the line is empty after strip
    return TCODE_ERR_EMPTY;

  // Perform checksum
  char *star = strrchr(line, '*');
  // Find the * checksum delim
  if (star) {
    if (!star[1] || !star[2])
      return TCODE_ERR_CHECKSUM_FORMAT;
    uint8_t given = 0;
    if (!tcode_parse_hex_u8(star + 1, &given))
      return TCODE_ERR_CHECKSUM_FORMAT;

    *star = '\0'; // strip checksum from string before calculating
    uint8_t calc = tcode_checksum_xor(line);

    out->has_checksum = true;
    out->given_checksum = given;
    out->calculated_checksum = calc;

    if (calc != given)
      // Obviously this is bad
      return TCODE_ERR_CHECKSUM_MISMATCH;
  }

  // Tokenize on spaces (collapse runs).
  char *p = line;
  while (*p) {
    while (*p == ' ' || *p == '\t')
      ++p;
    if (!*p)
      break;
    if (out->token_count >= TCODE_MAX_TOKENS)
      return TCODE_ERR_TOO_MANY_TOKENS;
    out->tokens[out->token_count++] = p;
    while (*p && *p != ' ' && *p != '\t')
      ++p;
    if (*p)
      *p++ = '\0';
  }

  return out->token_count ? TCODE_OK : TCODE_ERR_EMPTY;
}

