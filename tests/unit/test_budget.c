#include "../../core/budget_compiler/budget_compiler.h"

#include <stdio.h>

int main(void) {
  budget_contract_t contract = {
      {"a", 1},
      BUDGET_ARTIFACT_KIND_ADAPTER,
      2,
      false,
      false,
      false,
      0,
      0,
      0,
      BUDGET_TEST_OBLIGATION_CLASS_LIGHT};
  budget_global_ceilings_t ceilings = {120, 6, 12, 16, 1200, 10, 0, 0};
  budget_local_budget_t out = {0};
  budget_error_code_t err =
      budget_compile_local_budget(&contract, &ceilings, NULL, &out);
  if (err != 0) {
    fprintf(stderr, "unexpected error %d\n", (int)err);
    return 1;
  }
  if (out.max_public_surface != 2 || out.max_states != 0) {
    fprintf(stderr, "unexpected budget output\n");
    return 1;
  }
  contract.declared_transition_count = 1;
  err = budget_compile_local_budget(&contract, &ceilings, NULL, &out);
  if (err != BUDGET_E_CONTRACT_FIELD_CONFLICT) {
    fprintf(stderr, "expected field conflict\n");
    return 1;
  }
  return 0;
}
