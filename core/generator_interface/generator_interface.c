#include "generator_interface.h"

#include <stddef.h>

static uint64_t gi_fnv1a_update(uint64_t hash, const uint8_t *bytes,
                                gi_u64_t length) {
  gi_u64_t i;
  for (i = 0U; i < length; ++i) {
    hash ^= (uint64_t)bytes[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}

static char gi_hex_digit(uint8_t nibble) {
  static const char digits[] = "0123456789abcdef";
  return digits[nibble & 0x0FU];
}

gi_error_code_t gi_build_request_id(
    const gi_generator_request_without_id_t *request_without_id,
    char out_request_id[GI_REQUEST_ID_HEX_LEN + 1U]) {
  uint64_t hash = 1469598103934665603ULL;
  int i;
  if (request_without_id == NULL || out_request_id == NULL) {
    return GEN_E_INTERNAL;
  }
  hash = gi_fnv1a_update(hash, (const uint8_t *)request_without_id->run_id.bytes,
                         request_without_id->run_id.length);
  hash = gi_fnv1a_update(hash,
                         (const uint8_t *)&request_without_id->attempt,
                         sizeof(request_without_id->attempt));
  hash = gi_fnv1a_update(hash, request_without_id->contract.normalized_bytes.bytes,
                         request_without_id->contract.normalized_bytes.length);
  hash = gi_fnv1a_update(hash,
                         request_without_id->local_budget.normalized_bytes.bytes,
                         request_without_id->local_budget.normalized_bytes.length);

  for (i = 0; i < 8; ++i) {
    const uint8_t byte = (uint8_t)((hash >> (56 - 8 * i)) & 0xFFU);
    out_request_id[i * 2] = gi_hex_digit((uint8_t)(byte >> 4));
    out_request_id[i * 2 + 1] = gi_hex_digit(byte & 0x0FU);
  }
  out_request_id[GI_REQUEST_ID_HEX_LEN] = '\0';
  return (gi_error_code_t)0;
}

gi_error_code_t gi_validate_response(const gi_generator_response_t *response) {
  if (response == NULL || response->request_id.bytes == NULL ||
      response->request_id.length == 0U) {
    return GEN_E_PROTOCOL;
  }

  if (response->status == GI_GENERATOR_STATUS_SUCCESS) {
    if (!response->candidate_artifact.has_value || response->error_code.has_value) {
      return GEN_E_PROTOCOL;
    }
  } else {
    if (response->candidate_artifact.has_value || !response->error_code.has_value) {
      return GEN_E_PROTOCOL;
    }
  }

  return (gi_error_code_t)0;
}


gi_error_code_t gi_validate_response_for_request(
    const gi_generator_response_t *response,
    gi_string_t expected_request_id) {
  gi_u64_t i;
  gi_error_code_t base = gi_validate_response(response);
  if (base != 0) {
    return base;
  }
  if (expected_request_id.bytes == NULL ||
      response->request_id.length != expected_request_id.length) {
    return GEN_E_PROTOCOL;
  }
  for (i = 0U; i < expected_request_id.length; ++i) {
    if (response->request_id.bytes[i] != expected_request_id.bytes[i]) {
      return GEN_E_PROTOCOL;
    }
  }
  return (gi_error_code_t)0;
}
