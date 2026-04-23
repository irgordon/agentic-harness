#include "budget_compiler.h"

#include <stddef.h>

static bool budget_string_equals(budget_string_t a, budget_string_t b) {
  budget_u64_t i;
  if (a.length != b.length || a.bytes == NULL || b.bytes == NULL) {
    return false;
  }
  for (i = 0U; i < a.length; ++i) {
    if (a.bytes[i] != b.bytes[i]) {
      return false;
    }
  }
  return true;
}

static budget_u64_t budget_min_u64(budget_u64_t a, budget_u64_t b) {
  return (a < b) ? a : b;
}

budget_error_code_t budget_compile_local_budget(
    const budget_contract_t *contract,
    const budget_global_ceilings_t *ceilings,
    const budget_exemption_manifest_t *manifest,
    budget_local_budget_t *out_budget) {
  budget_u64_t i;
  if (contract == NULL || ceilings == NULL || out_budget == NULL) {
    return BUDGET_E_INVALID_CONTRACT_SCHEMA;
  }

  if (contract->artifact_id.bytes == NULL || contract->artifact_id.length == 0U) {
    return BUDGET_E_INVALID_CONTRACT_SCHEMA;
  }

  if (!contract->is_stateful && contract->declared_state_count > 0U) {
    return BUDGET_E_CONTRACT_FIELD_CONFLICT;
  }
  if (contract->declared_state_count == 0U &&
      contract->declared_transition_count > 0U) {
    return BUDGET_E_CONTRACT_FIELD_CONFLICT;
  }

  out_budget->artifact_id = contract->artifact_id;
  out_budget->max_lines_per_function =
      budget_min_u64(120U, ceilings->max_lines_per_function);
  out_budget->max_nesting_depth = budget_min_u64(6U, ceilings->max_nesting_depth);
  out_budget->max_cyclomatic_complexity =
      budget_min_u64(12U, ceilings->max_cyclomatic_complexity);
  out_budget->max_fan_out = budget_min_u64(16U, ceilings->max_fan_out);
  out_budget->max_file_size = budget_min_u64(1200U, ceilings->max_file_size);
  out_budget->max_public_surface =
      budget_min_u64(contract->public_surface_target, ceilings->max_public_surface);

  if (contract->is_stateful) {
    out_budget->max_states =
        budget_min_u64(contract->declared_state_count, ceilings->max_states);
    out_budget->max_transitions_per_state =
        budget_min_u64(ceilings->max_transitions_per_state,
                       (contract->declared_state_count == 0U)
                           ? 0U
                           : contract->declared_transition_count /
                                 contract->declared_state_count +
                                 1U);
  } else {
    out_budget->max_states = 0U;
    out_budget->max_transitions_per_state = 0U;
  }

  if (manifest != NULL && manifest->entries != NULL) {
    for (i = 0U; i < manifest->entry_count; ++i) {
      const budget_exemption_entry_t *entry = &manifest->entries[i];
      if (!budget_string_equals(entry->artifact_id, contract->artifact_id)) {
        continue;
      }
      if (!entry->max_lines_per_function_override.has_value &&
          !entry->max_file_size_override.has_value) {
        return EXEMPTION_E_EMPTY_OVERRIDE;
      }
      if (entry->max_lines_per_function_override.has_value) {
        if (entry->max_lines_per_function_override.value >
            ceilings->max_lines_per_function) {
          return BUDGET_E_OVERRIDE_EXCEEDS_CEILING;
        }
        out_budget->max_lines_per_function =
            entry->max_lines_per_function_override.value;
      }
      if (entry->max_file_size_override.has_value) {
        if (entry->max_file_size_override.value > ceilings->max_file_size) {
          return BUDGET_E_OVERRIDE_EXCEEDS_CEILING;
        }
        out_budget->max_file_size = entry->max_file_size_override.value;
      }
    }
  }

  if (out_budget->max_public_surface > ceilings->max_public_surface ||
      out_budget->max_states > ceilings->max_states ||
      out_budget->max_transitions_per_state > ceilings->max_transitions_per_state) {
    return BUDGET_E_CONTRACT_INCOMPATIBLE_WITH_GLOBAL_LIMITS;
  }

  return (budget_error_code_t)0;
}
