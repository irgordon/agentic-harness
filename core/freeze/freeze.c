#include "freeze.h"

#include <stddef.h>

static uint64_t freeze_fnv1a_update(uint64_t hash, const uint8_t *bytes,
                                    freeze_u64_t length) {
  freeze_u64_t i;
  for (i = 0U; i < length; ++i) {
    hash ^= (uint64_t)bytes[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}

static char freeze_hex_digit(uint8_t nibble) {
  static const char digits[] = "0123456789abcdef";
  return digits[nibble & 0x0FU];
}

freeze_u64_t freeze_compute_hash_hex(const freeze_inputs_t *inputs,
                                     char out_hex[FREEZE_HASH_HEX_LEN + 1U]) {
  uint64_t hash = 1469598103934665603ULL;
  int i;
  if (inputs == NULL || out_hex == NULL) {
    return 0U;
  }

  hash = freeze_fnv1a_update(hash, inputs->candidate_artifact.bytes,
                             inputs->candidate_artifact.length);
  hash = freeze_fnv1a_update(hash, inputs->contract.normalized_bytes.bytes,
                             inputs->contract.normalized_bytes.length);
  hash = freeze_fnv1a_update(hash, inputs->global_ceilings.normalized_bytes.bytes,
                             inputs->global_ceilings.normalized_bytes.length);
  hash = freeze_fnv1a_update(hash, inputs->exemption_manifest.normalized_bytes.bytes,
                             inputs->exemption_manifest.normalized_bytes.length);
  hash = freeze_fnv1a_update(hash, (const uint8_t *)inputs->toolchain_version.bytes,
                             inputs->toolchain_version.length);

  for (i = 0; i < 8; ++i) {
    const uint8_t byte = (uint8_t)((hash >> (56 - 8 * i)) & 0xFFU);
    out_hex[i * 2] = freeze_hex_digit((uint8_t)(byte >> 4));
    out_hex[i * 2 + 1] = freeze_hex_digit(byte & 0x0FU);
  }
  out_hex[FREEZE_HASH_HEX_LEN] = '\0';
  return FREEZE_HASH_HEX_LEN;
}
