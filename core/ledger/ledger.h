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

/*
 * Deterministic error registry (wire-level identifiers).
 *
 * docs/LEDGER.md section 9:
 * Ledger errors MUST be limited to:
 * * `LEDGER_E_SERIALIZATION`
 * * `LEDGER_E_APPEND_FAILURE`
 * * `LEDGER_E_INVALID_EVENT_SCHEMA`
 */
typedef enum ledger_error_code_t {
  LEDGER_E_SERIALIZATION = 5000,
  LEDGER_E_APPEND_FAILURE = 5001,
  LEDGER_E_INVALID_EVENT_SCHEMA = 5002
} ledger_error_code_t;

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
  ledger_string_t error_code;
  ledger_string_t artifact_id;
  ledger_string_t message;
  ledger_static_analysis_failure_details_t details;
} ledger_static_analysis_failure_payload_t;

enum { LEDGER_SHA256_DIGEST_SIZE = 32U };

typedef struct ledger_sha256_digest_t {
  uint8_t bytes[LEDGER_SHA256_DIGEST_SIZE];
} ledger_sha256_digest_t;

/*
 * docs/LEDGER.md section 6.1:
 * * **Algorithm:** SHA-256
 * * **Input domain:** the exact UTF-8 byte sequence of the authoritative normalized JSON
 *   for the referenced object (no BOM, no implicit newline, no trailing whitespace unless
 *   present in the normalized form)
 */
void ledger_sha256_digest(const uint8_t *input_bytes,
                          ledger_u64_t input_length,
                          ledger_sha256_digest_t *out_digest);

#endif /* CORE_LEDGER_LEDGER_H */
