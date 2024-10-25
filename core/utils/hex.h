#pragma once
#include <cstddef>
#include <cstdint>

namespace hex {

static char signs[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

inline size_t hex_length_from_binary(size_t length) { return length * 2; }

static bool binary_to_hex(const char *input, size_t size, char *output) {
  for (int i = 0; i < size; ++i) {
    uint8_t byte = input[i];
    uint8_t upper = (byte & 0xF0) >> 4;
    uint8_t lower = byte & 0xF;

    size_t j = i * 2;
    output[j] = signs[upper];
    output[j + 1] = signs[lower];
  }

  return true;
}

static char byte_map[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  -1, -1, -1, -1, -1, -1, // '0'-'9'
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 'A'-'F'
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 'a'-'f'
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

inline size_t binary_length_from_hex(size_t length) { return length / 2; }

// not tested btw:
static bool hex_to_binary(const char *input, size_t size, char *output) {
  if (size % 2 != 0) {
    return false;
  }

  for (int i = 0; i < size; i += 2) {
    uint8_t upper = input[i];
    uint8_t lower = input[i + 1];

    if (byte_map[upper] == -1 || byte_map[lower] == -1) {
      return false;
    }

    uint8_t upper_byte = byte_map[upper] << 4;
    uint8_t lower_byte = byte_map[lower];

    output[i / 2] = upper_byte | lower_byte;
  }

  return true;
}

} // namespace hex
