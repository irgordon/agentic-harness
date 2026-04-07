# ledger

## Purpose
The Ledger is the authoritative append-only, deterministic event log for a harness run. It records canonical gate outcomes, attempt outcomes, exemption applications, freeze outcomes, and terminal run status.

## Inputs
- Explicit event emissions from the Harness state machine.
- Canonical run context fields and hashes (`run_id`, `contract_hash`, `global_ceilings_hash`, `exemption_manifest_hash`, `toolchain_hash`).
- Attempt index (or null when not applicable).
- Event payloads conforming to event-type schema.

## Outputs
- Deterministically serialized canonical JSON event records using the ledger schema field order.
- Append-only event sequence that conforms to the ledger grammar.
- Exactly one terminal event (`RUN_SUCCESS` or `RUN_ABORTED`) per run.

## Invariants
- Passive subsystem: no validation, no semantic reinterpretation, no retries, no artifact storage.
- Deterministic serialization, canonical field ordering, explicit nulls, UTF-8 JSON, and no nondeterministic metadata.
- Single-writer serial emission with no parallel writes and no asynchronous buffering.
- No event reordering, rewriting, deletion, aggregation, or summarization.
- `run_id` is deterministic and stable within a run.
- Attempt numbering starts at 1, increments by 1, never skips, and never repeats.
- If `ARTIFACT_FROZEN` appears, `RUN_SUCCESS` appears immediately after.

## Failure Modes
- `LEDGER_E_SERIALIZATION`
- `LEDGER_E_APPEND_FAILURE`
- `LEDGER_E_INVALID_EVENT_SCHEMA`
- Gate failures are recorded using subsystem error namespaces in event payloads; ledger does not invent gate-failure error codes.
