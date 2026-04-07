# budget_compiler

## Purpose
The Budget Compiler is a deterministic trusted-core subsystem that validates a contract and derives a local budget from `(contract, global_ceilings, exemption_manifest/applicable_exemptions)`.

## Inputs
- Validated Contract object (including cross-field constraints and declarations).
- Validated Global Ceilings object.
- Exemption Manifest entries applicable to `contract.artifact_id`.

## Outputs
- Local budget object with fields:
  - `artifact_id`
  - `max_lines_per_function`
  - `max_nesting_depth`
  - `max_cyclomatic_complexity`
  - `max_fan_out`
  - `max_file_size`
  - `max_public_surface`
  - `max_states`
  - `max_transitions_per_state`
- Deterministic compatibility decision: budget derived or rejected.

## Invariants
- Pure function of declared inputs; no randomness, clocks, environment access, or model calls.
- Same normalized inputs produce byte-identical output.
- Canonical field ordering is used for output.
- Numeric budget fields are non-negative and do not exceed global ceilings.
- `artifact_id == contract.artifact_id`.
- `max_public_surface <= contract.public_surface_target`.
- `max_states <= contract.declared_state_count`.
- If `is_stateful=false` then `max_states=0` and `max_transitions_per_state=0`.
- If `artifact_kind=pure_function` then `max_states=0` and `max_transitions_per_state=0`.
- Exemptions may widen only `max_lines_per_function` and `max_file_size`; exemptions do not widen `max_public_surface`, `max_states`, `max_transitions_per_state`, or `max_fan_out`.
- Only applicable exemptions participate in derivation and manifest hash used for run identity.

## Failure Modes
- Rejects invalid schemas with:
  - `BUDGET_E_INVALID_CONTRACT_SCHEMA`
  - `BUDGET_E_INVALID_GLOBAL_CEILINGS_SCHEMA`
- Rejects contract field conflicts with `BUDGET_E_CONTRACT_FIELD_CONFLICT`.
- Rejects incompatibility with global limits and invalid overrides with:
  - `BUDGET_E_CONTRACT_INCOMPATIBLE_WITH_GLOBAL_LIMITS`
  - `BUDGET_E_FORBIDDEN_OVERRIDE`
  - `BUDGET_E_OVERRIDE_EXCEEDS_CEILING`
- Rejects invalid internal rule table with `BUDGET_E_INTERNAL_RULE_TABLE_INVALID`.

## 6. Implementation Boundary
- MAY implement deterministic contract/ceiling/exemption validation required for budget derivation.
- MAY implement deterministic local-budget derivation and compatibility rejection for declared inputs only.
- MUST NOT implement harness orchestration, attempt lifecycle control, ledger emission policy, generator invocation, static analysis, test execution, or freeze hashing.
- Responsibility ends at emitting deterministic budget output or deterministic budget rejection.

## 7. Forbidden Responsibilities
- MUST NEVER interpret generator behavior, candidate artifact semantics, or test outcomes.
- MUST NEVER relax constraints outside V1 exemption scope (`max_lines_per_function`, `max_file_size` only).
- MUST NEVER define or reinterpret `run_id`, attempt numbering, or ledger grammar.
- MUST NEVER mutate contract, global ceilings, exemption manifest membership, or prior gate artifacts.

## 8. External Dependencies
- MAY depend on validated contract, validated global ceilings, and applicable exemptions as read-only inputs.
- MAY depend on canonical normalization/hashing rules as read-only rules for deterministic input identity.
- MUST NOT depend on `generator_interface`, `static_analysis`, `freeze`, or `ledger` for derivation decisions.
- Interaction with `harness` is input/output only: read-only inputs from harness, write-only budget result back to harness.

## 9. State & Mutability Rules
- MUST operate as a pure, stateless function per invocation.
- MUST treat all input objects as immutable.
- MUST NOT persist caches, learned heuristics, environment snapshots, or hidden derivation state.
- Output budget is immutable after emission within the run.
