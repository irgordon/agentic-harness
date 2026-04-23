# Ledger Grammar (Spec Freeze v1)

This file freezes the minimum deterministic ledger grammar used by `harness_cli run`.

## Canonical success sequence

1. `CONTRACT_ACCEPTED`
2. `BUDGET_DERIVED`
3. `GENERATION_ATTEMPTED` (attempt=1)
4. `STATIC_ANALYSIS_PASSED` (attempt=1)
5. `ARTIFACT_FROZEN` (`payload.freeze_hash` required)
6. `RUN_SUCCESS`

## Envelope constraints

- `event_type` MUST be present for every event.
- attempt-scoped events MUST include `attempt`.
- `ARTIFACT_FROZEN` MUST include `payload.freeze_hash`.
- `RUN_SUCCESS` MUST be terminal.

## Drift policy

Any change to this grammar requires a version bump under `spec/`.
