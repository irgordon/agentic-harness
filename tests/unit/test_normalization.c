#include "../../core/normalization/normalization.h"

#include <stdio.h>

int main(void) {
  const uint8_t input[] = {'a', '\r', '\n', 'b', '\r', '\n'};
  uint8_t out[16];
  normalization_u64_t len = normalization_canonicalize_text(
      (normalization_bytes_t){input, 6}, out, 16);
  if (len != 4) {
    fprintf(stderr, "unexpected normalized length %llu\n",
            (unsigned long long)len);
    return 1;
  }
  if (out[0] != 'a' || out[1] != '\n' || out[2] != 'b' || out[3] != '\n') {
    fprintf(stderr, "unexpected normalized bytes\n");
    return 1;
  }
  return 0;
}
