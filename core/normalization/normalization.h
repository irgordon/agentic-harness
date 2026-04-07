#ifndef CORE_NORMALIZATION_NORMALIZATION_H
#define CORE_NORMALIZATION_NORMALIZATION_H

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t normalization_u64_t;

typedef struct normalization_string_t {
  const char *bytes;
  normalization_u64_t length;
} normalization_string_t;

typedef struct normalization_bytes_t {
  const uint8_t *bytes;
  normalization_u64_t length;
} normalization_bytes_t;

typedef struct normalization_json_t {
  normalization_bytes_t normalized_bytes;
} normalization_json_t;

typedef struct normalization_optional_json_t {
  bool has_value;
  normalization_json_t value;
} normalization_optional_json_t;

typedef struct normalization_optional_u64_t {
  bool has_value;
  normalization_u64_t value;
} normalization_optional_u64_t;

typedef enum normalization_exemption_scope_t {
  NORMALIZATION_EXEMPTION_SCOPE_FUNCTION = 0,
  NORMALIZATION_EXEMPTION_SCOPE_MODULE = 1
} normalization_exemption_scope_t;

enum { NORMALIZATION_MIN_ATTEMPT_NUMBER = 1U };

/*
 * GeneratorRequest_without_id
 * One normalized request payload per attempt.
 * Field order is canonical and matches docs/NORMALIZATION_SPEC.md section 7.1.
 */
typedef struct normalization_generator_request_without_id_t {
  normalization_string_t run_id;
  normalization_u64_t attempt;
  normalization_json_t contract;
  normalization_json_t local_budget;
  normalization_optional_json_t toolchain_capabilities;
} normalization_generator_request_without_id_t;

/*
 * Applicable Exemption Entry
 * Entry key order is canonical; manifest membership is append-only.
 */
typedef struct normalization_applicable_exemption_entry_t {
  normalization_string_t exemption_id;
  normalization_string_t artifact_id;
  normalization_exemption_scope_t scope;
  normalization_string_t target;
  normalization_string_t reason;
  normalization_optional_u64_t max_lines_per_function_override;
  normalization_optional_u64_t max_file_size_override;
} normalization_applicable_exemption_entry_t;

/*
 * Applicable Exemption Sequence
 * Entries are ordered deterministically before hashing.
 */
typedef struct normalization_applicable_exemptions_t {
  const normalization_applicable_exemption_entry_t *entries;
  normalization_u64_t entry_count;
} normalization_applicable_exemptions_t;

/*
 * Canonical artifact representation (produced)
 * Immutable once emitted for verification/freeze.
 */
typedef struct normalization_canonical_artifact_t {
  normalization_bytes_t normalized_bytes;
} normalization_canonical_artifact_t;

/*
 * Canonical JSON representation (produced)
 * Immutable bytes for a given logical value.
 */
typedef struct normalization_canonical_json_t {
  normalization_bytes_t normalized_bytes;
} normalization_canonical_json_t;

#endif /* CORE_NORMALIZATION_NORMALIZATION_H */
