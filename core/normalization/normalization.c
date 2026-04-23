#include "normalization.h"

#include <stddef.h>

normalization_u64_t normalization_canonicalize_text(
    normalization_bytes_t input,
    uint8_t *out_bytes,
    normalization_u64_t out_capacity) {
  normalization_u64_t i;
  normalization_u64_t out_len = 0U;
  for (i = 0U; i < input.length; ++i) {
    const uint8_t ch = input.bytes[i];
    if (ch == (uint8_t)'\r') {
      continue;
    }
    if (out_bytes != NULL && out_len < out_capacity) {
      out_bytes[out_len] = ch;
    }
    out_len += 1U;
  }
  return out_len;
}
