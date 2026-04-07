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

## 6. Implementation Boundary
- MAY implement canonical event-envelope serialization and append-only persistence for harness-emitted events.
- MAY implement ledger grammar conformance checks and event schema validation at write time.
- MAY implement deterministic single-writer serial emission behavior.
- MUST NOT implement gate execution, gate retries, generator control, artifact validation, or artifact storage.
- Responsibility ends at durable append of canonical event records or ledger write failure reporting.

## 7. Forbidden Responsibilities
- MUST NEVER reorder, rewrite, delete, aggregate, or summarize emitted events.
- MUST NEVER define, recompute, or reinterpret `run_id` semantics.
- MUST NEVER invent gate error codes or reinterpret subsystem error payload semantics.
- MUST NEVER emit implicit events not explicitly provided by harness.
- MUST NEVER attach nondeterministic metadata (timestamps, UUIDs, environment leakage).

## 8. External Dependencies
- MAY depend on `harness` as the sole event producer input channel.
- MAY depend on canonical schema and normalization rules as read-only serialization constraints.
- MAY write to ledger storage as append-only output.
- MUST NOT depend on `generator_interface`, `budget_compiler`, `static_analysis`, or `freeze` internals to alter event content.
- MUST NOT read or mutate subsystem-owned runtime state to synthesize ledger events.

## 9. State & Mutability Rules
- MAY hold only append cursor and transient serialization buffers required for ordered writes.
- Existing ledger records are immutable after append.
- `run_id` and attempt envelope values are immutable once serialized per event.
- MUST NOT persist hidden mutable state that changes canonical serialization for identical logical events.
