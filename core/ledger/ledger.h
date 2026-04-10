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

typedef struct ledger_payload_ref_t {
  const uint8_t *opaque_payload;
  ledger_u64_t length;
} ledger_payload_ref_t;

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
 *
 * docs/LEDGER.md section 6 rules:
 * * Fields MUST appear in the exact order listed above.
 * * Nulls MUST be explicit.
 * * Hashes MUST be canonical digests of normalized inputs.
 * * No timestamps, UUIDs, or nondeterministic metadata.
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
  ledger_payload_ref_t payload;
} ledger_event_t;

/*
 * Deterministic construction inputs for the event envelope.
 * Nullable members use explicit has_value flags; no defaulting is permitted.
 */
typedef struct ledger_event_envelope_inputs_t {
  ledger_string_t event_type;
  ledger_string_t run_id;
  ledger_optional_u64_t attempt;
  ledger_optional_string_t artifact_id;
  ledger_optional_string_t contract_hash;
  ledger_optional_string_t global_ceilings_hash;
  ledger_optional_string_t exemption_manifest_hash;
  ledger_optional_string_t toolchain_hash;
  ledger_payload_ref_t payload;
} ledger_event_envelope_inputs_t;

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

enum {
  LEDGER_SHA256_DIGEST_SIZE = 32U,
  LEDGER_SHA256_HEX_LENGTH = 64U,
  LEDGER_SHA256_HEX_STORAGE_LENGTH = 65U
};

typedef struct ledger_sha256_digest_t {
  uint8_t bytes[LEDGER_SHA256_DIGEST_SIZE];
} ledger_sha256_digest_t;

/*
 * Normalized JSON hash inputs used to populate the ledger event envelope
 * `*_hash` fields in canonical schema order.
 */
typedef struct ledger_event_hash_inputs_t {
  ledger_json_t contract;
  ledger_json_t global_ceilings;
  ledger_json_t exemption_manifest;
  ledger_json_t toolchain;
} ledger_event_hash_inputs_t;

/*
 * Caller-owned storage for 64-char lowercase hex SHA-256 digest strings.
 * The ledger event envelope points into this storage after population.
 */
typedef struct ledger_event_hash_storage_t {
  char contract_hash[LEDGER_SHA256_HEX_STORAGE_LENGTH];
  char global_ceilings_hash[LEDGER_SHA256_HEX_STORAGE_LENGTH];
  char exemption_manifest_hash[LEDGER_SHA256_HEX_STORAGE_LENGTH];
  char toolchain_hash[LEDGER_SHA256_HEX_STORAGE_LENGTH];
} ledger_event_hash_storage_t;

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

/*
 * docs/LEDGER.md section 6.1:
 * * Hash input domain is the exact normalized JSON UTF-8 byte sequence.
 * * Output encoding is lowercase hex with exactly 64 characters.
 * * Envelope hash fields are set explicitly and treated as immutable after
 *   this construction step.
 */
void ledger_event_populate_envelope_hashes(
    ledger_event_t *event,
    const ledger_event_hash_inputs_t *hash_inputs,
    ledger_event_hash_storage_t *hash_storage);

/*
 * Deterministically constructs the canonical event envelope from explicit inputs.
 * By convention, the resulting envelope is immutable after construction.
 */
void ledger_event_construct_envelope(
    ledger_event_t *out_event,
    const ledger_event_envelope_inputs_t *inputs);

/*
 * docs/LEDGER.md section 8.1 canonical JSON rules:
 * * keys in exact section 6 schema order
 * * no extra whitespace
 * * UTF-8 bytes
 * * no trailing commas
 * * explicit nulls
 * * strings escaped only as required by JSON
 * * numbers emitted unquoted
 * * hash fields emitted as lowercase hex strings or null
 */
void ledger_event_serialize_json(const ledger_event_t *envelope,
                                 uint8_t *out_bytes,
                                 ledger_u64_t *in_out_length);

/*
 * docs/LEDGER.md section 2 + section 8.2 + section 10:
 * * Append-only writes
 * * Events appended in exact emitted order (no reordering)
 * * Single-writer serial emission, no parallel writes, no async buffering
 *
 * Returns:
 * * 0 on success
 * * LEDGER_E_SERIALIZATION for zero/invalid input length or invalid input
 * * LEDGER_E_APPEND_FAILURE on append syscall failure
 */
ledger_error_code_t ledger_append_bytes(int fd,
                                        const uint8_t *bytes,
                                        ledger_u64_t length);

/*
 * Mechanical deterministic emission only:
 * event -> canonical envelope -> canonical JSON bytes -> append-only write.
 * This function does not interpret event semantics, grammar, or policy.
 */
ledger_error_code_t ledger_emit_event(int fd, const ledger_event_t *event);

#endif /* CORE_LEDGER_LEDGER_H */
