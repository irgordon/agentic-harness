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

## 10. Deterministic Data Models

### Contract (consumed)
- Name: `Contract`
- Field list (ordered):
  1. `artifact_id` — string — non-null
  2. `artifact_kind` — enum (`pure_function | stateful_module | orchestrator | adapter | io_boundary`) — non-null
  3. `public_surface_target` — integer (`>= 0`) — non-null
  4. `is_stateful` — boolean — non-null
  5. `has_io` — boolean — non-null
  6. `has_concurrency` — boolean — non-null
  7. `declared_state_count` — integer (`>= 0`) — non-null
  8. `declared_transition_count` — integer (`>= 0`) — non-null
  9. `declared_invariant_count` — integer (`>= 0`) — non-null
  10. `test_obligation_class` — enum (`light | normal | heavy`) — non-null
- Canonical ordering rules: keys follow the contract schema order shown above.
- Hash participation rules: normalized `Contract` bytes define `contract_hash`, which participates in `run_id`.
- Immutability guarantees: treated as immutable input by Budget Compiler.
- Lifecycle constraints: any change requires a new run configuration.

### Global Ceilings (consumed)
- Name: `GlobalCeilings`
- Field list (ordered):
  1. `max_lines_per_function` — integer (`>= 0`) — non-null
  2. `max_nesting_depth` — integer (`>= 0`) — non-null
  3. `max_cyclomatic_complexity` — integer (`>= 0`) — non-null
  4. `max_fan_out` — integer (`>= 0`) — non-null
  5. `max_file_size` — integer (`>= 0`) — non-null
  6. `max_public_surface` — integer (`>= 0`) — non-null
  7. `max_states` — integer (`>= 0`) — non-null
  8. `max_transitions_per_state` — integer (`>= 0`) — non-null
- Canonical ordering rules: keys are serialized in schema order.
- Hash participation rules: normalized `GlobalCeilings` bytes define `global_ceilings_hash`, which participates in `run_id`.
- Immutability guarantees: treated as immutable input by Budget Compiler.
- Lifecycle constraints: any change requires a new run configuration.

### Applicable Exemption Entry (consumed)
- Name: `ExemptionEntry`
- Field list (ordered):
  1. `exemption_id` — string — non-null
  2. `artifact_id` — string — non-null
  3. `scope` — enum (`function | module`) — non-null
  4. `target` — string — non-null
  5. `reason` — string — non-null
  6. `max_lines_per_function_override` — integer — nullable (explicit `null` allowed)
  7. `max_file_size_override` — integer — nullable (explicit `null` allowed)
- Canonical ordering rules: entry keys follow schema order; applicable entries are ordered by `(artifact_id, scope, target, exemption_id)` before hashing and derivation.
- Hash participation rules: normalized applicable entry sequence defines `exemption_manifest_hash`, which participates in `run_id`; each applicable entry also participates in per-entry exemption hashing for `EXEMPTION_APPLIED`.
- Immutability guarantees: entries are immutable inputs; manifest membership is append-only.
- Lifecycle constraints: only applicable entries for the artifact participate; any change to applicable entries requires a new run.

### Local Budget (produced)
- Name: `LocalBudget`
- Field list (ordered):
  1. `artifact_id` — string — non-null
  2. `max_lines_per_function` — integer (`>= 0`) — non-null
  3. `max_nesting_depth` — integer (`>= 0`) — non-null
  4. `max_cyclomatic_complexity` — integer (`>= 0`) — non-null
  5. `max_fan_out` — integer (`>= 0`) — non-null
  6. `max_file_size` — integer (`>= 0`) — non-null
  7. `max_public_surface` — integer (`>= 0`) — non-null
  8. `max_states` — integer (`>= 0`) — non-null
  9. `max_transitions_per_state` — integer (`>= 0`) — non-null
- Canonical ordering rules: keys follow the emitted budget schema order shown above.
- Hash participation rules: when included in `GeneratorRequest_without_id`, normalized `LocalBudget` bytes participate in `request_id` computation.
- Immutability guarantees: immutable after deterministic emission within a run.
- Lifecycle constraints: emitted only after `BUDGET_DERIVATION`; consumed by generation and verification phases in the same run.

## 11. Error codes and ownership

### 1) Owned error code prefixes
- Owned prefix: `BUDGET_E_*`.
- This subsystem MUST NOT emit codes outside `BUDGET_E_*`.

### 2) Emitted error codes
- `BUDGET_E_INVALID_CONTRACT_SCHEMA`
  - Emission condition: contract schema is invalid.
  - Terminal vs non-terminal: terminal at run level when `BUDGET_DERIVATION` fails and `RUN_ABORTED` is emitted.
  - Surface location: budget-derivation failure surfaced by harness in run-abort failure path (`BUDGET_E_*`) and recorded in ledger failure payload.
- `BUDGET_E_INVALID_GLOBAL_CEILINGS_SCHEMA`
  - Emission condition: global ceilings schema is invalid.
  - Terminal vs non-terminal: terminal at run level when `BUDGET_DERIVATION` fails and `RUN_ABORTED` is emitted.
  - Surface location: budget-derivation failure surfaced by harness in run-abort failure path (`BUDGET_E_*`) and recorded in ledger failure payload.
- `BUDGET_E_CONTRACT_FIELD_CONFLICT`
  - Emission condition: contract cross-field validity rules fail.
  - Terminal vs non-terminal: terminal at run level when `BUDGET_DERIVATION` fails and `RUN_ABORTED` is emitted.
  - Surface location: budget-derivation failure surfaced by harness in run-abort failure path (`BUDGET_E_*`) and recorded in ledger failure payload.
- `BUDGET_E_CONTRACT_INCOMPATIBLE_WITH_GLOBAL_LIMITS`
  - Emission condition: contract is incompatible with global limits.
  - Terminal vs non-terminal: terminal at run level when `BUDGET_DERIVATION` fails and `RUN_ABORTED` is emitted.
  - Surface location: budget-derivation failure surfaced by harness in run-abort failure path (`BUDGET_E_*`) and recorded in ledger failure payload.
- `BUDGET_E_INTERNAL_RULE_TABLE_INVALID`
  - Emission condition: internal rule table is invalid.
  - Terminal vs non-terminal: terminal at run level when `BUDGET_DERIVATION` fails and `RUN_ABORTED` is emitted.
  - Surface location: budget-derivation failure surfaced by harness in run-abort failure path (`BUDGET_E_*`) and recorded in ledger failure payload.
- `BUDGET_E_FORBIDDEN_OVERRIDE`
  - Emission condition: exemption override violates allowed override scope.
  - Terminal vs non-terminal: terminal at run level when `BUDGET_DERIVATION` fails and `RUN_ABORTED` is emitted.
  - Surface location: budget-derivation failure surfaced by harness in run-abort failure path (`BUDGET_E_*`) and recorded in ledger failure payload.
- `BUDGET_E_OVERRIDE_EXCEEDS_CEILING`
  - Emission condition: exemption override exceeds global ceiling.
  - Terminal vs non-terminal: terminal at run level when `BUDGET_DERIVATION` fails and `RUN_ABORTED` is emitted.
  - Surface location: budget-derivation failure surfaced by harness in run-abort failure path (`BUDGET_E_*`) and recorded in ledger failure payload.

### 3) Forbidden error behavior
- MUST NOT reinterpret errors from other subsystems.
- MUST NOT mint new error codes.
- MUST NOT collapse distinct failures into one code unless specified.

### 4) Cross-subsystem propagation rules
- No cross-subsystem pass-through rule is specified for this subsystem.
