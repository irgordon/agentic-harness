#ifndef CORE_GENERATOR_INTERFACE_GENERATOR_INTERFACE_H
#define CORE_GENERATOR_INTERFACE_GENERATOR_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t gi_u64_t;

typedef struct gi_string_t {
  const char *bytes;
  gi_u64_t length;
} gi_string_t;

typedef struct gi_bytes_t {
  const uint8_t *bytes;
  gi_u64_t length;
} gi_bytes_t;

typedef struct gi_json_t {
  gi_bytes_t normalized_bytes;
} gi_json_t;

typedef struct gi_optional_json_t {
  bool has_value;
  gi_json_t value;
} gi_optional_json_t;

typedef struct gi_optional_string_t {
  bool has_value;
  gi_string_t value;
} gi_optional_string_t;

enum { GI_MIN_ATTEMPT_NUMBER = 1U };

/* Response status values from docs/GENERATOR_INTERFACE.md section 4. */
typedef enum gi_generator_status_t {
  GI_GENERATOR_STATUS_SUCCESS = 0,
  GI_GENERATOR_STATUS_FAILURE = 1
} gi_generator_status_t;

/*
 * GeneratorRequest_without_id (consumed for identity derivation)
 * Field order is canonical and matches docs/NORMALIZATION_SPEC.md section 7.1.
 */
typedef struct gi_generator_request_without_id_t {
  gi_string_t run_id;
  gi_u64_t attempt;
  gi_json_t contract;
  gi_json_t local_budget;
  gi_optional_json_t toolchain_capabilities;
} gi_generator_request_without_id_t;

/*
 * GeneratorRequest (emitted)
 * request_id = hash(normalized(GeneratorRequest_without_id)).
 */
typedef struct gi_generator_request_t {
  gi_string_t request_id;
  gi_string_t run_id;
  gi_u64_t attempt;
  gi_json_t contract;
  gi_json_t local_budget;
  gi_optional_json_t toolchain_capabilities;
} gi_generator_request_t;

/*
 * GeneratorResponse (consumed)
 * Success/failure nullability constraints are explicit in optional fields.
 * Field order is canonical and matches docs/GENERATOR_INTERFACE.md section 4.
 */
typedef struct gi_generator_response_t {
  gi_string_t request_id;
  gi_generator_status_t status;
  gi_optional_json_t candidate_artifact;
  gi_optional_string_t error_code;
  gi_optional_json_t error_details;
} gi_generator_response_t;

/*
 * Normalized candidate artifact (emitted to verification path on success).
 */
typedef struct gi_normalized_candidate_artifact_t {
  gi_bytes_t normalized_bytes;
} gi_normalized_candidate_artifact_t;

#endif /* CORE_GENERATOR_INTERFACE_GENERATOR_INTERFACE_H */
