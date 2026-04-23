#ifndef CORE_FREEZE_FREEZE_H
#define CORE_FREEZE_FREEZE_H

#include <stdint.h>

typedef uint64_t freeze_u64_t;

typedef struct freeze_string_t {
  const char *bytes;
  freeze_u64_t length;
} freeze_string_t;

typedef struct freeze_bytes_t {
  const uint8_t *bytes;
  freeze_u64_t length;
} freeze_bytes_t;

typedef struct freeze_json_t {
  freeze_bytes_t normalized_bytes;
} freeze_json_t;

/*
 * Freeze input bundle (consumed)
 * Freeze executes only after ATTEMPT_PASSED and only on normalized artifact bytes.
 * Inputs are immutable for the freeze invocation.
 */
typedef struct freeze_inputs_t {
  freeze_bytes_t candidate_artifact;
  freeze_json_t contract;
  freeze_json_t global_ceilings;
  freeze_json_t exemption_manifest;
  freeze_string_t toolchain_version;
} freeze_inputs_t;

/*
 * ARTIFACT_FROZEN payload (produced)
 * Immutable after emission; RUN_SUCCESS must immediately follow.
 */
typedef struct freeze_artifact_frozen_payload_t {
  freeze_string_t freeze_hash;
} freeze_artifact_frozen_payload_t;

/*
 * FREEZE_FAILED payload (produced)
 * Immutable after emission; RUN_ABORTED must follow as terminal event.
 */
typedef struct freeze_failure_payload_t {
  freeze_string_t error_code;
} freeze_failure_payload_t;


enum { FREEZE_HASH_HEX_LEN = 16U };

freeze_u64_t freeze_compute_hash_hex(const freeze_inputs_t *inputs,
                                     char out_hex[FREEZE_HASH_HEX_LEN + 1U]);

#endif /* CORE_FREEZE_FREEZE_H */
