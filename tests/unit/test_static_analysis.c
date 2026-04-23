#include "../../core/static_analysis/static_analysis.h"

#include <stdio.h>

int main(void) {
  const char *src = "int main(){\nif(1){return 0;}\n}\n";
  sae_normalized_candidate_artifact_t artifact = {
      {(const uint8_t *)src, (sae_u64_t)34}};
  sae_local_budget_t budget = {{"a", 1}, 100, 5, 10, 10, 100, 1, 0, 0};
  sae_contract_declared_counts_t counts = {0, 0};
  sae_structural_metrics_t metrics;
  sae_failure_payload_t fail;

  if (sae_evaluate_candidate(&artifact, &budget, &counts, &metrics, &fail) != 0) {
    fprintf(stderr, "expected success\n");
    return 1;
  }

  budget.max_nesting_depth = 0;
  if (sae_evaluate_candidate(&artifact, &budget, &counts, &metrics, &fail) !=
      SAE_E_NESTING_DEPTH) {
    fprintf(stderr, "expected nesting depth error\n");
    return 1;
  }
  return 0;
}
