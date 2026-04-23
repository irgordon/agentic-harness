#include "static_analysis.h"

#include <stddef.h>

static bool sae_match_at(const uint8_t *s, sae_u64_t len, sae_u64_t i,
                         const char *tok) {
  sae_u64_t j = 0U;
  while (tok[j] != '\0') {
    if (i + j >= len || s[i + j] != (uint8_t)tok[j]) {
      return false;
    }
    ++j;
  }
  return true;
}

sae_error_code_t sae_evaluate_candidate(
    const sae_normalized_candidate_artifact_t *artifact,
    const sae_local_budget_t *budget,
    const sae_contract_declared_counts_t *declared_counts,
    sae_structural_metrics_t *out_metrics,
    sae_failure_payload_t *out_failure) {
  sae_u64_t i;
  sae_u64_t line_count = 0U;
  sae_u64_t depth = 0U;
  sae_u64_t max_depth = 0U;
  sae_u64_t cyclomatic = 1U;
  sae_u64_t fan_out = 0U;
  if (artifact == NULL || budget == NULL || declared_counts == NULL ||
      out_metrics == NULL) {
    return SAE_E_INTERNAL_ERROR;
  }
  if (artifact->normalized_bytes.bytes == NULL) {
    return SAE_E_PARSE_ERROR;
  }

  for (i = 0U; i < artifact->normalized_bytes.length; ++i) {
    const uint8_t ch = artifact->normalized_bytes.bytes[i];
    if (ch == (uint8_t)'\n') {
      line_count += 1U;
    }
    if (ch == (uint8_t)'{') {
      depth += 1U;
      if (depth > max_depth) {
        max_depth = depth;
      }
    } else if (ch == (uint8_t)'}') {
      if (depth > 0U) {
        depth -= 1U;
      }
    }
    if (sae_match_at(artifact->normalized_bytes.bytes,
                     artifact->normalized_bytes.length, i, "if(" ) ||
        sae_match_at(artifact->normalized_bytes.bytes,
                     artifact->normalized_bytes.length, i, "for(") ||
        sae_match_at(artifact->normalized_bytes.bytes,
                     artifact->normalized_bytes.length, i, "while(") ||
        sae_match_at(artifact->normalized_bytes.bytes,
                     artifact->normalized_bytes.length, i, "case ") ||
        sae_match_at(artifact->normalized_bytes.bytes,
                     artifact->normalized_bytes.length, i, "&&") ||
        sae_match_at(artifact->normalized_bytes.bytes,
                     artifact->normalized_bytes.length, i, "||")) {
      cyclomatic += 1U;
    }
    if (sae_match_at(artifact->normalized_bytes.bytes,
                     artifact->normalized_bytes.length, i, "#include") ||
        sae_match_at(artifact->normalized_bytes.bytes,
                     artifact->normalized_bytes.length, i, "import ")) {
      fan_out += 1U;
    }
  }

  out_metrics->artifact_id = budget->artifact_id;
  out_metrics->lines_per_function = line_count;
  out_metrics->nesting_depth = max_depth;
  out_metrics->cyclomatic_complexity = cyclomatic;
  out_metrics->fan_out = fan_out;
  out_metrics->file_size = line_count;
  out_metrics->public_surface = 0U;
  out_metrics->state_count = 0U;
  out_metrics->transition_count = 0U;

  if (line_count > budget->max_file_size) {
    if (out_failure != NULL) {
      out_failure->error_code = SAE_E_FILE_SIZE;
    }
    return SAE_E_FILE_SIZE;
  }
  if (max_depth > budget->max_nesting_depth) {
    if (out_failure != NULL) {
      out_failure->error_code = SAE_E_NESTING_DEPTH;
    }
    return SAE_E_NESTING_DEPTH;
  }
  if (cyclomatic > budget->max_cyclomatic_complexity) {
    if (out_failure != NULL) {
      out_failure->error_code = SAE_E_CYCLOMATIC_COMPLEXITY;
    }
    return SAE_E_CYCLOMATIC_COMPLEXITY;
  }
  if (declared_counts->declared_state_count != 0U ||
      declared_counts->declared_transition_count != 0U) {
    if (out_failure != NULL) {
      out_failure->error_code = SAE_E_STATE_COUNT;
    }
    return SAE_E_STATE_COUNT;
  }

  return (sae_error_code_t)0;
}
