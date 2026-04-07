#ifndef CORE_STATIC_ANALYSIS_STATIC_ANALYSIS_H
#define CORE_STATIC_ANALYSIS_STATIC_ANALYSIS_H

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t sae_u64_t;

typedef struct sae_string_t {
  const char *bytes;
  sae_u64_t length;
} sae_string_t;

typedef struct sae_bytes_t {
  const uint8_t *bytes;
  sae_u64_t length;
} sae_bytes_t;

/*
 * LocalBudget (consumed)
 * Fixed for all attempts in a run and immutable during verification.
 * Field order is canonical and matches docs/STATIC_ANALYSIS_ENGINE.md section 3.2.
 */
typedef struct sae_local_budget_t {
  sae_string_t artifact_id;
  sae_u64_t max_lines_per_function;
  sae_u64_t max_nesting_depth;
  sae_u64_t max_cyclomatic_complexity;
  sae_u64_t max_fan_out;
  sae_u64_t max_file_size;
  sae_u64_t max_public_surface;
  sae_u64_t max_states;
  sae_u64_t max_transitions_per_state;
} sae_local_budget_t;

/*
 * Contract declared-count view (consumed)
 * Immutable within a run; any contract change requires a new run.
 */
typedef struct sae_contract_declared_counts_t {
  sae_u64_t declared_state_count;
  sae_u64_t declared_transition_count;
} sae_contract_declared_counts_t;

/*
 * Normalized candidate artifact (consumed)
 * Produced by normalization and verified exactly once per attempt.
 */
typedef struct sae_normalized_candidate_artifact_t {
  sae_bytes_t normalized_bytes;
} sae_normalized_candidate_artifact_t;

/*
 * Structural metrics (computed)
 * Field order follows metric definitions in docs/STATIC_ANALYSIS_ENGINE.md section 5.
 */
typedef struct sae_structural_metrics_t {
  sae_string_t artifact_id;
  sae_u64_t lines_per_function;
  sae_u64_t nesting_depth;
  sae_u64_t cyclomatic_complexity;
  sae_u64_t fan_out;
  sae_u64_t file_size;
  sae_u64_t public_surface;
  sae_u64_t state_count;
  sae_u64_t transition_count;
} sae_structural_metrics_t;

/*
 * Deterministic error registry (wire-level identifiers).
 *
 * docs/STATIC_ANALYSIS_ENGINE.md section 4.3:
 * Required Error Codes
 * * `SAE_E_PARSE_ERROR`
 * * `SAE_E_LINES_PER_FUNCTION`
 * * `SAE_E_NESTING_DEPTH`
 * * `SAE_E_CYCLOMATIC_COMPLEXITY`
 * * `SAE_E_FAN_OUT`
 * * `SAE_E_FILE_SIZE`
 * * `SAE_E_PUBLIC_SURFACE`
 * * `SAE_E_STATE_COUNT`
 * * `SAE_E_TRANSITION_COUNT`
 * * `SAE_E_INTERNAL_ERROR`
 */
typedef enum sae_error_code_t {
  SAE_E_PARSE_ERROR = 2000,
  SAE_E_LINES_PER_FUNCTION = 2001,
  SAE_E_NESTING_DEPTH = 2002,
  SAE_E_CYCLOMATIC_COMPLEXITY = 2003,
  SAE_E_FAN_OUT = 2004,
  SAE_E_FILE_SIZE = 2005,
  SAE_E_PUBLIC_SURFACE = 2006,
  SAE_E_STATE_COUNT = 2007,
  SAE_E_TRANSITION_COUNT = 2008,
  SAE_E_INTERNAL_ERROR = 2009
} sae_error_code_t;

/*
 * StaticAnalysisFailure.details
 * location is explicitly nullable via has_location.
 */
typedef struct sae_failure_details_t {
  sae_string_t metric;
  sae_bytes_t value;
  sae_bytes_t limit;
  bool has_location;
  sae_string_t location;
} sae_failure_details_t;

/*
 * StaticAnalysisFailure payload (produced)
 * Immutable after emission for the attempt.
 * Field order is canonical and matches docs/STATIC_ANALYSIS_ENGINE.md section 4.2.
 */
typedef struct sae_failure_payload_t {
  sae_error_code_t error_code;
  sae_string_t artifact_id;
  sae_string_t message;
  sae_failure_details_t details;
} sae_failure_payload_t;

#endif /* CORE_STATIC_ANALYSIS_STATIC_ANALYSIS_H */
