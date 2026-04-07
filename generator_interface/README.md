# generator_interface

## Purpose
The Generator Interface defines the boundary contract between the deterministic Harness and an external generator that may be non-deterministic. It constrains request/response structure, identity, validation, and attempt integration.

## Inputs
- Canonical `GeneratorRequest` fields:
  - `request_id`
  - `run_id`
  - `attempt`
  - `contract`
  - `local_budget`
  - `toolchain_capabilities` (advisory in V1)
- Canonical request object without `request_id` for deterministic `request_id` derivation.
- Configured `generator_timeout_ms` for timeout interpretation.

## Outputs
- Exactly one `GeneratorResponse` per request with fields:
  - `request_id`
  - `status` (`success | failure`)
  - `candidate_artifact`
  - `error_code`
  - `error_details`
- On success, candidate artifact proceeds to canonical normalization and verification.
- On failure, harness attempt failure path is taken.

## Invariants
- Generator is outside trusted core in V1; harness does not assume generator determinism.
- Request identity is deterministic: `request_id = hash(normalized(GeneratorRequest_without_request_id))`.
- Attempt numbering is monotonic and matches ledger attempt numbering.
- Harness validates response `request_id`, `status`, and null/non-null constraints before use.
- Protocol violations are treated as generator failure with `GEN_E_PROTOCOL`.
- Timeout handling is deterministic at configuration level and uses configured `generator_timeout_ms`.
- Generator side effects are isolated and do not mutate harness-controlled deterministic inputs or outputs.

## Failure Modes
- Timeout: `GEN_E_TIMEOUT`.
- Protocol/shape mismatch: `GEN_E_PROTOCOL`.
- Other generator failures use `GEN_E_*` namespace (for example `GEN_E_INTERNAL`, `GEN_E_UNSUPPORTED_CONTRACT`, `GEN_E_RESOURCE_LIMIT`).
- Oversize artifact rejection before verification is treated as harness-level failure with `ATTEMPT_FAILED` reason `verification_failed`.

## 6. Implementation Boundary
- MAY implement canonical generator request construction, deterministic `request_id` derivation, and response-shape validation.
- MAY implement deterministic timeout/protocol failure classification at the interface boundary.
- MAY implement isolation enforcement at the interface contract boundary.
- MUST NOT implement generator model behavior, budget derivation, static analysis, test execution, freeze hashing, or ledger event grammar.
- Responsibility ends at validated `GeneratorResponse` handoff or generator-failure classification for the current attempt.

## 7. Forbidden Responsibilities
- MUST NEVER assume generator reproducibility in V1.
- MUST NEVER mutate contract, local budget, exemption data, run identity inputs, or ledger state.
- MUST NEVER reinterpret attempt numbering semantics defined by the run model.
- MUST NEVER accept malformed response field constraints as success.
- MUST NEVER bypass harness deterministic verification gates.

## 8. External Dependencies
- MAY depend on `harness`-provided run/attempt context, contract, local budget, and timeout configuration as read-only inputs.
- MAY depend on normalization rules as read-only rules for request canonicalization and identity hashing.
- MAY interact with external generator as write-only request / read-only response exchange.
- MUST NOT depend on `ledger`, `static_analysis`, `budget_compiler`, or `freeze` internals to validate protocol conformance.
- MUST NOT depend on generator-side hidden state for deterministic interface validation.

## 9. State & Mutability Rules
- MAY hold per-attempt transient request/response state only.
- Request identity fields (`request_id`, `run_id`, `attempt`) are immutable once request is emitted.
- MUST NOT persist generator-internal state as trusted deterministic state.
- MUST NOT persist mutable cross-run interface state that changes protocol validation behavior for identical normalized inputs.

## 10. Deterministic Data Models

### Generator Request Without ID (consumed for identity derivation)
- Name: `GeneratorRequest_without_id`
- Field list (ordered):
  1. `run_id` — string digest — non-null
  2. `attempt` — integer (`>= 1`) — non-null
  3. `contract` — object — non-null
  4. `local_budget` — object — non-null
  5. `toolchain_capabilities` — object — nullable/omissible where explicitly allowed
- Canonical ordering rules: normalized in the top-level schema order listed above; nested objects follow their schema order or deterministic generic ordering.
- Hash participation rules: normalized bytes are the sole input to `request_id`.
- Immutability guarantees: treated as immutable for request hashing.
- Lifecycle constraints: built once per attempt before generator invocation.

### Generator Request (emitted)
- Name: `GeneratorRequest`
- Field list (ordered):
  1. `request_id` — string digest — non-null
  2. `run_id` — string digest — non-null
  3. `attempt` — integer (`>= 1`) — non-null
  4. `contract` — object — non-null
  5. `local_budget` — object — non-null
  6. `toolchain_capabilities` — object — nullable/omissible where explicitly allowed
- Canonical ordering rules: key order follows schema order.
- Hash participation rules: `request_id` equals hash of normalized `GeneratorRequest_without_id`.
- Immutability guarantees: request identity fields are immutable once sent.
- Lifecycle constraints: exactly one request per attempt; attempt numbers are strictly monotonic.

### Generator Response (consumed)
- Name: `GeneratorResponse`
- Field list (ordered):
  1. `request_id` — string digest — non-null
  2. `status` — enum (`success | failure`) — non-null
  3. `candidate_artifact` — opaque artifact — nullable (required non-null when `status=success`)
  4. `error_code` — string (`GEN_E_*`) — nullable (required non-null when `status=failure`)
  5. `error_details` — object — nullable
- Canonical ordering rules: response is canonicalized before protocol validation.
- Hash participation rules: `request_id` binds response to request; successful `candidate_artifact` is normalized before downstream hashing.
- Immutability guarantees: response is immutable once validated for the attempt.
- Lifecycle constraints: one response per request; malformed shape is classified as `GEN_E_PROTOCOL`.

### Normalized Candidate Artifact (emitted to harness verification path)
- Name: `NormalizedCandidateArtifact`
- Field list (ordered):
  1. `normalized_bytes` — byte sequence — non-null
- Canonical ordering rules: normalization enforces canonical artifact representation before verification.
- Hash participation rules: normalized bytes define `candidate_artifact_hash` and freeze hash inputs.
- Immutability guarantees: immutable after normalization for the attempt.
- Lifecycle constraints: exists only on successful generation responses and is consumed by verification/freeze phases.
