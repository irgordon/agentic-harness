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

typedef enum sae_error_code_t {
  SAE_ERROR_PARSE_ERROR = 0,
  SAE_ERROR_LINES_PER_FUNCTION = 1,
  SAE_ERROR_NESTING_DEPTH = 2,
  SAE_ERROR_CYCLOMATIC_COMPLEXITY = 3,
  SAE_ERROR_FAN_OUT = 4,
  SAE_ERROR_FILE_SIZE = 5,
  SAE_ERROR_PUBLIC_SURFACE = 6,
  SAE_ERROR_STATE_COUNT = 7,
  SAE_ERROR_TRANSITION_COUNT = 8,
  SAE_ERROR_INTERNAL_ERROR = 9
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
