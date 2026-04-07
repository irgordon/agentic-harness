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
