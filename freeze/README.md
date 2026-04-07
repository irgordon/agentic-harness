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

## 10. Deterministic Data Models

### Freeze Input Bundle (consumed)
- Name: `FreezeInputs`
- Field list (ordered):
  1. `candidate_artifact` — normalized byte representation — non-null
  2. `contract` — object — non-null
  3. `global_ceilings` — object — non-null
  4. `exemption_manifest` — object/array (applicable subset) — non-null
  5. `toolchain_version` — string — non-null
- Canonical ordering rules: `candidate_artifact` is canonicalized before freeze; JSON/object inputs use their canonical ordering rules from their owning specs.
- Hash participation rules: freeze hash is computed over normalized artifact representation with run-fixed inputs and toolchain.
- Immutability guarantees: all inputs are immutable for the freeze invocation.
- Lifecycle constraints: freeze executes only after `ATTEMPT_PASSED`; inputs remain run-fixed.

### Artifact Frozen Payload (produced)
- Name: `ArtifactFrozen`
- Field list (ordered):
  1. `freeze_hash` — string digest — non-null
- Canonical ordering rules: payload fields serialize deterministically in schema order.
- Hash participation rules: `freeze_hash` is the canonical hash output of the freeze phase and is part of the audit trail.
- Immutability guarantees: immutable once emitted.
- Lifecycle constraints: emitted exactly in the freeze phase and immediately followed by `RUN_SUCCESS`.

### Freeze Failure Payload (produced)
- Name: `FreezeFailure`
- Field list (ordered):
  1. `error_code` — string (`FREEZE_E_*`) — non-null
- Canonical ordering rules: deterministic field ordering for serialized payload.
- Hash participation rules: participates in canonical ledger event serialization for failure traceability.
- Immutability guarantees: immutable once emitted.
- Lifecycle constraints: emitted only on freeze failure and followed by terminal `RUN_ABORTED`.

## 11. Error codes and ownership

### 1) Owned error code prefixes
- Owned prefix: `FREEZE_E_*`.
- This subsystem MUST NOT emit codes outside `FREEZE_E_*`.

### 2) Emitted error codes
- Concrete `FREEZE_E_*` codes are not enumerated in authoritative documents.
- Authoritative emission rule: freeze failure uses `FREEZE_E_*` in the harness freeze failure path (`RUN_ABORTED` with `FREEZE_E_*`).
- Terminal vs non-terminal: terminal at run level on freeze failure (`RUN_ABORTED`).
- Surface location: run-abort freeze failure path carrying `FREEZE_E_*`; freeze failure is also represented by `FREEZE_FAILED` before `RUN_ABORTED` in run-model flow.

### 3) Forbidden error behavior
- MUST NOT reinterpret errors from other subsystems.
- MUST NOT mint new error codes.
- MUST NOT collapse distinct failures into one code unless specified.

### 4) Cross-subsystem propagation rules
- No cross-subsystem pass-through rule is specified for this subsystem.
