#pragma once
#include <cstddef>
#include <cstdint>

namespace hex {

static char signs[] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

inline size_t hex_length_from_binary(size_t length) {
  return length * 2;
}

static void binary_to_hex(const char *input, size_t size, char *output) {
  for (int i = 0; i < size; ++i) {
    uint8_t byte = input[i];
    uint8_t upper = (byte & 0xF0) >> 4;
    uint8_t lower = byte & 0xF;

    size_t j = i * 2;
    output[j] = signs[upper];
    output[j + 1] = signs[lower];
  }
}

} // namespace hex
