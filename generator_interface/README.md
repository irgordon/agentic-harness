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
