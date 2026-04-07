# harness

## Purpose
The Harness is the deterministic orchestrator for the V1 pipeline. It enforces fixed gate ordering, bounded attempts, deterministic validation behavior, deterministic freeze behavior, and deterministic ledger emission around a non-deterministic generation gate.

## Inputs
- Validated contract.
- Validated global ceilings.
- Validated exemption manifest (applicable subset for run identity and budgeting).
- Toolchain identity/hash inputs.
- Run configuration including `generator_timeout_ms`, `max_attempts`, and `generator_interface_spec_version`.
- Generator interface request/response exchange per attempt.

## Outputs
- Deterministic run terminal status: `RUN_SUCCESS` or `RUN_ABORTED`.
- Attempt-level status: `ATTEMPT_PASSED` or `ATTEMPT_FAILED`.
- Ordered ledger event stream conforming to ledger grammar, including contract/budget/attempt/freeze/terminal events.

## Invariants
- Fixed state machine order: `CONTRACT_VALIDATION -> BUDGET_DERIVATION -> ATTEMPT_LOOP -> FREEZE -> DONE`.
- Deterministic gates are trusted-core and do not retry unchanged inputs.
- `GENERATION_ATTEMPT` is external and may be non-deterministic.
- Attempts start at 1, increase by exactly 1, never skip, never repeat, and one generator invocation equals one attempt.
- Verification executes exactly once per candidate artifact.
- Execution model is single-threaded, serial, and synchronous; no parallel attempts.
- Exactly one terminal event exists per run.
- If `ARTIFACT_FROZEN` is emitted, `RUN_SUCCESS` is emitted immediately after.

## Failure Modes
- Contract gate failure leads to terminal abort (`CONTRACT_E_*` path).
- Budget gate failure leads to terminal abort (`BUDGET_E_*` path).
- Generation protocol/timeout/other failure produces `GENERATION_FAILED`, `ATTEMPT_FAILED`, and may lead to terminal abort at attempt limit.
- Verification failure (`STATIC_ANALYSIS_FAILED` or `TESTS_FAILED`) produces `ATTEMPT_FAILED` and may lead to terminal abort at attempt limit.
- Freeze failure produces `FREEZE_FAILED` and terminal `RUN_ABORTED`.
- Harness-native errors:
  - `HARNESS_E_INVALID_STATE`
  - `HARNESS_E_UNEXPECTED_INPUT`
  - `HARNESS_E_MAX_ATTEMPTS_EXCEEDED`
  - `HARNESS_E_GATE_FAILURE` (with wrapped subsystem cause)

## 6. Implementation Boundary
- MAY implement the fixed deterministic run state machine, gate ordering, bounded-attempt control, and terminal conditions defined by `RUN_MODEL.md` and `HARNESS.md`.
- MAY implement deterministic invocation coordination for trusted-core gates and external generation attempts.
- MAY implement deterministic ledger event emission sequencing for run, attempt, freeze, and terminal events.
- MUST NOT implement subsystem-internal logic for budget derivation, static metric computation, normalization algorithms, or freeze hashing algorithms.
- Responsibility ends at orchestration, classification, and canonical event emission for one run lifecycle.

## 7. Forbidden Responsibilities
- MUST NEVER assume generator determinism or infer generator internal state.
- MUST NEVER retry deterministic gates on unchanged canonical inputs.
- MUST NEVER reorder, rewrite, or delete ledger events once emitted.
- MUST NEVER alter run identity inputs within a run or reinterpret `run_id` hash domain.
- MUST NEVER violate attempt monotonicity, terminal-event uniqueness, or freeze terminal ordering invariants.

## 8. External Dependencies
- MAY depend on `budget_compiler`, `static_analysis`, `normalization`, `generator_interface`, `freeze`, and `ledger` through declared gate interfaces only.
- MAY read contract, ceilings, applicable exemptions, and toolchain identity as run-fixed read-only inputs.
- MAY write events to `ledger` as write-only append operations.
- MUST NOT depend on agent/planner internals or nondeterministic side channels outside `generator_interface`.
- MUST NOT permit nondeterministic components to write into deterministic harness-controlled paths.

## 9. State & Mutability Rules
- MAY hold run-local orchestration state only (`run_id`, gate state, attempt index, terminal status, and run-fixed hashes/configuration).
- Run configuration inputs and `run_id` MUST be immutable within a run.
- Attempt index MUST be strictly monotonic and immutable for completed attempts.
- MUST NOT persist hidden mutable state that alters deterministic gate behavior for equivalent normalized inputs.
