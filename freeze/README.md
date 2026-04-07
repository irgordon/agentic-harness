# freeze

## Purpose
The Freeze subsystem is a deterministic trusted-core subsystem that finalizes a verified candidate artifact by producing the canonical freeze hash over normalized artifact representation and establishing terminal success semantics.

## Inputs
- Candidate artifact that passed verification.
- Contract input.
- Global ceilings input.
- Exemption manifest input.
- Toolchain version/hash inputs.
- Canonical normalization rules used for artifact representation and hashing.

## Outputs
- Canonical freeze hash for the artifact (`ARTIFACT_FROZEN` payload includes `freeze_hash`).
- Terminal run transition on success: `ARTIFACT_FROZEN` immediately followed by `RUN_SUCCESS`.
- On freeze failure: `FREEZE_FAILED` then terminal `RUN_ABORTED`.

## Invariants
- Deterministic gate behavior: identical normalized inputs produce the same freeze hash or the same failure.
- Freeze operates only on normalized artifact representation.
- Freeze occurs only after an attempt has passed verification.
- No mutation of prior gate artifacts.
- Freeze result participates in reproducibility and auditability guarantees.

## Failure Modes
- Freeze failure emits `FREEZE_FAILED` with `error_code: FREEZE_E_*` and then terminal `RUN_ABORTED`.
- Harness wrapper may expose gate failure through `HARNESS_E_GATE_FAILURE` with freeze cause details.

## 6. Implementation Boundary
- MAY implement deterministic freeze hashing and integrity validation over normalized artifact representation.
- MAY implement freeze gate success/failure output for harness terminal transition handling.
- MUST NOT implement generation, budget derivation, static analysis, test execution, or ledger schema interpretation.
- Responsibility ends at deterministic freeze result (`freeze_hash` or `FREEZE_E_*` failure cause) for the provided normalized artifact and run inputs.

## 7. Forbidden Responsibilities
- MUST NEVER operate on unnormalized artifact representation.
- MUST NEVER mutate candidate artifacts, prior gate outputs, run configuration inputs, or ledger history.
- MUST NEVER redefine terminal event ordering (`ARTIFACT_FROZEN` then `RUN_SUCCESS`; freeze failure then `RUN_ABORTED`).
- MUST NEVER define or reinterpret `run_id`, attempt numbering, or generator protocol semantics.

## 8. External Dependencies
- MAY depend on normalized artifact representation and run-fixed inputs (contract, ceilings, applicable exemptions, toolchain identity) as read-only inputs.
- MAY depend on normalization rules/subsystem output as read-only canonical input.
- MUST NOT depend on `generator_interface` behavior, agent internals, or test-execution internals.
- Interaction with `harness` is input/output only: read-only freeze inputs from harness, write-only freeze outcome back to harness.

## 9. State & Mutability Rules
- MUST be deterministic and stateless per freeze invocation.
- MUST treat all freeze inputs as immutable within the run.
- MUST NOT persist mutable freeze working state across runs.
- Emitted freeze result is immutable after gate completion.
