#ifndef CORE_HARNESS_HARNESS_H
#define CORE_HARNESS_HARNESS_H

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t harness_u64_t;

typedef struct harness_string_t {
  const char *bytes;
  harness_u64_t length;
} harness_string_t;

typedef struct harness_bytes_t {
  const uint8_t *bytes;
  harness_u64_t length;
} harness_bytes_t;

typedef struct harness_json_t {
  harness_bytes_t normalized_bytes;
} harness_json_t;

typedef struct harness_optional_json_t {
  bool has_value;
  harness_json_t value;
} harness_optional_json_t;

typedef struct harness_optional_string_t {
  bool has_value;
  harness_string_t value;
} harness_optional_string_t;

typedef struct harness_optional_u64_t {
  bool has_value;
  harness_u64_t value;
} harness_optional_u64_t;

enum { HARNESS_MIN_ATTEMPT_NUMBER = 1U };

/* Response status values from docs/GENERATOR_INTERFACE.md section 4. */
typedef enum harness_generator_status_t {
  HARNESS_GENERATOR_STATUS_SUCCESS = 0,
  HARNESS_GENERATOR_STATUS_FAILURE = 1
} harness_generator_status_t;

/*
 * Deterministic error registry (wire-level identifiers).
 *
 * docs/HARNESS.md section 11:
 * * `HARNESS_E_INVALID_STATE`
 * * `HARNESS_E_UNEXPECTED_INPUT`
 * * `HARNESS_E_MAX_ATTEMPTS_EXCEEDED`
 * Wrapped subsystem errors use `HARNESS_E_GATE_FAILURE`.
 */
typedef enum harness_error_code_t {
  HARNESS_E_INVALID_STATE = 3000,
  HARNESS_E_UNEXPECTED_INPUT = 3001,
  HARNESS_E_MAX_ATTEMPTS_EXCEEDED = 3002,
  HARNESS_E_GATE_FAILURE = 3003
} harness_error_code_t;

/*
 * RunConfiguration (consumed)
 * These fields define run_id and are immutable for the entire run.
 * Field order is canonical and matches docs/RUN_MODEL.md section 2.
 */
typedef struct harness_run_configuration_t {
  harness_string_t contract_hash;
  harness_string_t global_ceilings_hash;
  harness_string_t exemption_manifest_hash;
  harness_string_t toolchain_hash;
  harness_u64_t generator_timeout_ms;
  harness_u64_t max_attempts;
  harness_string_t generator_interface_spec_version;
} harness_run_configuration_t;

/*
 * GeneratorRequest_without_id (identity domain)
 * request_id = hash(normalized(GeneratorRequest_without_id)).
 */
typedef struct harness_generator_request_without_id_t {
  harness_string_t run_id;
  harness_u64_t attempt;
  harness_json_t contract;
  harness_json_t local_budget;
  harness_optional_json_t toolchain_capabilities;
} harness_generator_request_without_id_t;

/*
 * GeneratorRequest (emitted)
 * request_id, run_id, and attempt are immutable once emitted.
 */
typedef struct harness_generator_request_t {
  harness_string_t request_id;
  harness_string_t run_id;
  harness_u64_t attempt;
  harness_json_t contract;
  harness_json_t local_budget;
  harness_optional_json_t toolchain_capabilities;
} harness_generator_request_t;

/*
 * GeneratorResponse (consumed)
 * candidate_artifact is present only for success.
 * error_code is required for failure; error_details is nullable.
 */
typedef struct harness_generator_response_t {
  harness_string_t request_id;
  harness_generator_status_t status;
  harness_optional_json_t candidate_artifact;
  harness_optional_string_t error_code;
  harness_optional_json_t error_details;
} harness_generator_response_t;

/*
 * LedgerEvent envelope (emitted)
 * Field order is canonical and MUST match docs/LEDGER.md section 6.
 * Events are immutable and append-only after emission.
 */
typedef struct harness_ledger_event_t {
  harness_string_t event_type;
  harness_string_t run_id;
  harness_optional_u64_t attempt;
  harness_optional_string_t artifact_id;
  harness_optional_string_t contract_hash;
  harness_optional_string_t global_ceilings_hash;
  harness_optional_string_t exemption_manifest_hash;
  harness_optional_string_t toolchain_hash;
  harness_json_t payload;
} harness_ledger_event_t;

/*
 * Wrapped gate failure payload shape from docs/HARNESS.md section 11.2.
 */
typedef struct harness_gate_failure_cause_t {
  harness_string_t subsystem_error_code;
  harness_json_t details;
} harness_gate_failure_cause_t;

typedef struct harness_gate_failure_t {
  harness_string_t error_code;
  harness_string_t gate;
  harness_gate_failure_cause_t cause;
} harness_gate_failure_t;

#endif /* CORE_HARNESS_HARNESS_H */
