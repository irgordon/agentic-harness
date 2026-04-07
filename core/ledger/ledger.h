#ifndef CORE_LEDGER_LEDGER_H
#define CORE_LEDGER_LEDGER_H

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t ledger_u64_t;

typedef struct ledger_string_t {
  const char *bytes;
  ledger_u64_t length;
} ledger_string_t;

typedef struct ledger_bytes_t {
  const uint8_t *bytes;
  ledger_u64_t length;
} ledger_bytes_t;

typedef struct ledger_json_t {
  ledger_bytes_t normalized_bytes;
} ledger_json_t;

typedef struct ledger_optional_u64_t {
  bool has_value;
  ledger_u64_t value;
} ledger_optional_u64_t;

typedef struct ledger_optional_string_t {
  bool has_value;
  ledger_string_t value;
} ledger_optional_string_t;

typedef enum ledger_sae_error_code_t {
  LEDGER_SAE_ERROR_PARSE_ERROR = 0,
  LEDGER_SAE_ERROR_LINES_PER_FUNCTION = 1,
  LEDGER_SAE_ERROR_NESTING_DEPTH = 2,
  LEDGER_SAE_ERROR_CYCLOMATIC_COMPLEXITY = 3,
  LEDGER_SAE_ERROR_FAN_OUT = 4,
  LEDGER_SAE_ERROR_FILE_SIZE = 5,
  LEDGER_SAE_ERROR_PUBLIC_SURFACE = 6,
  LEDGER_SAE_ERROR_STATE_COUNT = 7,
  LEDGER_SAE_ERROR_TRANSITION_COUNT = 8,
  LEDGER_SAE_ERROR_INTERNAL_ERROR = 9
} ledger_sae_error_code_t;

/*
 * Ledger event envelope (consumed and persisted)
 * Field order is canonical and MUST match docs/LEDGER.md section 6.
 * Envelope values are immutable after append.
 */
typedef struct ledger_event_t {
  ledger_string_t event_type;
  ledger_string_t run_id;
  ledger_optional_u64_t attempt;
  ledger_optional_string_t artifact_id;
  ledger_optional_string_t contract_hash;
  ledger_optional_string_t global_ceilings_hash;
  ledger_optional_string_t exemption_manifest_hash;
  ledger_optional_string_t toolchain_hash;
  ledger_json_t payload;
} ledger_event_t;

/*
 * ExemptionApplied payload (payload object for EXEMPTION_APPLIED).
 */
typedef struct ledger_exemption_applied_payload_t {
  ledger_string_t exemption_id;
  ledger_string_t exemption_hash;
} ledger_exemption_applied_payload_t;

/*
 * StaticAnalysisFailure.details nested payload.
 * location is explicitly nullable via has_location.
 */
typedef struct ledger_static_analysis_failure_details_t {
  ledger_string_t metric;
  ledger_json_t value;
  ledger_json_t limit;
  bool has_location;
  ledger_string_t location;
} ledger_static_analysis_failure_details_t;

/*
 * StaticAnalysisFailure payload for STATIC_ANALYSIS_FAILED.
 */
typedef struct ledger_static_analysis_failure_payload_t {
  ledger_sae_error_code_t error_code;
  ledger_string_t artifact_id;
  ledger_string_t message;
  ledger_static_analysis_failure_details_t details;
} ledger_static_analysis_failure_payload_t;

#endif /* CORE_LEDGER_LEDGER_H */
