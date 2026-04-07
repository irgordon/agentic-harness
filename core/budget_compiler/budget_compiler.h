#ifndef CORE_BUDGET_COMPILER_BUDGET_COMPILER_H
#define CORE_BUDGET_COMPILER_BUDGET_COMPILER_H

#include <stdbool.h>
#include <stdint.h>

/* Canonical scalar aliases for deterministic, non-negative count/limit fields. */
typedef uint64_t budget_u64_t;

/* Canonical UTF-8 string view (no implicit defaults, no ownership semantics). */
typedef struct budget_string_t {
  const char *bytes;
  budget_u64_t length;
} budget_string_t;

/* Canonical nullable integer representation (explicit optionality via has_value). */
typedef struct budget_optional_u64_t {
  bool has_value;
  budget_u64_t value;
} budget_optional_u64_t;

/* artifact_kind values from docs/ARCHITECTURE.md section 4 contract schema. */
typedef enum budget_artifact_kind_t {
  BUDGET_ARTIFACT_KIND_PURE_FUNCTION = 0,
  BUDGET_ARTIFACT_KIND_STATEFUL_MODULE = 1,
  BUDGET_ARTIFACT_KIND_ORCHESTRATOR = 2,
  BUDGET_ARTIFACT_KIND_ADAPTER = 3,
  BUDGET_ARTIFACT_KIND_IO_BOUNDARY = 4
} budget_artifact_kind_t;

/* test_obligation_class values from docs/ARCHITECTURE.md section 4 contract schema. */
typedef enum budget_test_obligation_class_t {
  BUDGET_TEST_OBLIGATION_CLASS_LIGHT = 0,
  BUDGET_TEST_OBLIGATION_CLASS_NORMAL = 1,
  BUDGET_TEST_OBLIGATION_CLASS_HEAVY = 2
} budget_test_obligation_class_t;

/* Exemption scope values from docs/EXEMPTION_MANIFEST.md section 5 entry schema. */
typedef enum budget_exemption_scope_t {
  BUDGET_EXEMPTION_SCOPE_FUNCTION = 0,
  BUDGET_EXEMPTION_SCOPE_MODULE = 1
} budget_exemption_scope_t;

/*
 * Deterministic error registry (wire-level identifiers).
 *
 * docs/ARCHITECTURE.md section 5.10:
 * Error codes:
 * * `BUDGET_E_INVALID_CONTRACT_SCHEMA`
 * * `BUDGET_E_INVALID_GLOBAL_CEILINGS_SCHEMA`
 * * `BUDGET_E_CONTRACT_FIELD_CONFLICT`
 * * `BUDGET_E_CONTRACT_INCOMPATIBLE_WITH_GLOBAL_LIMITS`
 * * `BUDGET_E_INTERNAL_RULE_TABLE_INVALID`
 *
 * docs/EXEMPTION_MANIFEST.md section 10:
 * Budget Compiler MUST emit:
 * * `BUDGET_E_CONTRACT_INCOMPATIBLE_WITH_GLOBAL_LIMITS`
 * * `BUDGET_E_FORBIDDEN_OVERRIDE`
 * * `BUDGET_E_OVERRIDE_EXCEEDS_CEILING`
 *
 * docs/EXEMPTION_MANIFEST.md section 10:
 * Manifest parser MUST emit:
 * * `EXEMPTION_E_INVALID_SCHEMA`
 * * `EXEMPTION_E_DUPLICATE_ENTRY`
 * * `EXEMPTION_E_INVALID_SCOPE`
 * * `EXEMPTION_E_INVALID_TARGET`
 * * `EXEMPTION_E_EMPTY_OVERRIDE` (both override fields null)
 * * `EXEMPTION_E_INTERNAL_ERROR`
 */
typedef enum budget_error_code_t {
  BUDGET_E_INVALID_CONTRACT_SCHEMA = 1000,
  BUDGET_E_INVALID_GLOBAL_CEILINGS_SCHEMA = 1001,
  BUDGET_E_CONTRACT_FIELD_CONFLICT = 1002,
  BUDGET_E_CONTRACT_INCOMPATIBLE_WITH_GLOBAL_LIMITS = 1003,
  BUDGET_E_INTERNAL_RULE_TABLE_INVALID = 1004,
  BUDGET_E_FORBIDDEN_OVERRIDE = 1005,
  BUDGET_E_OVERRIDE_EXCEEDS_CEILING = 1006,
  EXEMPTION_E_INVALID_SCHEMA = 1007,
  EXEMPTION_E_DUPLICATE_ENTRY = 1008,
  EXEMPTION_E_INVALID_SCOPE = 1009,
  EXEMPTION_E_INVALID_TARGET = 1010,
  EXEMPTION_E_EMPTY_OVERRIDE = 1011,
  EXEMPTION_E_INTERNAL_ERROR = 1012
} budget_error_code_t;

/*
 * Contract (consumed)
 * Immutable within a run; any change requires a new run configuration.
 * Field order is canonical and MUST match docs/ARCHITECTURE.md section 4.
 */
typedef struct budget_contract_t {
  budget_string_t artifact_id;
  budget_artifact_kind_t artifact_kind;
  budget_u64_t public_surface_target;
  bool is_stateful;
  bool has_io;
  bool has_concurrency;
  budget_u64_t declared_state_count;
  budget_u64_t declared_transition_count;
  budget_u64_t declared_invariant_count;
  budget_test_obligation_class_t test_obligation_class;
} budget_contract_t;

/*
 * GlobalCeilings (consumed)
 * Immutable within a run; all numeric fields are non-negative.
 * Field order is canonical and MUST match docs/ARCHITECTURE.md section 5.5.
 */
typedef struct budget_global_ceilings_t {
  budget_u64_t max_lines_per_function;
  budget_u64_t max_nesting_depth;
  budget_u64_t max_cyclomatic_complexity;
  budget_u64_t max_fan_out;
  budget_u64_t max_file_size;
  budget_u64_t max_public_surface;
  budget_u64_t max_states;
  budget_u64_t max_transitions_per_state;
} budget_global_ceilings_t;

/*
 * ExemptionEntry (consumed)
 * At least one override field MUST be non-null in authoritative input.
 * Field order is canonical and MUST match docs/EXEMPTION_MANIFEST.md section 5.
 */
typedef struct budget_exemption_entry_t {
  budget_string_t exemption_id;
  budget_string_t artifact_id;
  budget_exemption_scope_t scope;
  budget_string_t target;
  budget_string_t reason;
  budget_optional_u64_t max_lines_per_function_override;
  budget_optional_u64_t max_file_size_override;
} budget_exemption_entry_t;

/*
 * ExemptionManifest (consumed collection)
 * Entries are immutable for derivation and canonicalized in deterministic order.
 */
typedef struct budget_exemption_manifest_t {
  const budget_exemption_entry_t *entries;
  budget_u64_t entry_count;
} budget_exemption_manifest_t;

/*
 * LocalBudget (produced)
 * Immutable after emission within a run.
 * Field order is canonical and MUST match docs/ARCHITECTURE.md section 5.5.
 */
typedef struct budget_local_budget_t {
  budget_string_t artifact_id;
  budget_u64_t max_lines_per_function;
  budget_u64_t max_nesting_depth;
  budget_u64_t max_cyclomatic_complexity;
  budget_u64_t max_fan_out;
  budget_u64_t max_file_size;
  budget_u64_t max_public_surface;
  budget_u64_t max_states;
  budget_u64_t max_transitions_per_state;
} budget_local_budget_t;

#endif /* CORE_BUDGET_COMPILER_BUDGET_COMPILER_H */
