# static_analysis

## Purpose
The Static Analysis Engine (SAE) is a deterministic trusted-core subsystem that computes structural metrics for a candidate artifact and enforces the local budget and contract-declared state/transition counts.

## Inputs
- Source artifact (single-file or directory artifact).
- Local budget object from Budget Compiler.
- Validated contract fields `declared_state_count` and `declared_transition_count`.

## Outputs
- Pass/fail decision:
  - `STATIC_ANALYSIS_PASSED`
  - `STATIC_ANALYSIS_FAILED`
- On failure, structured payload containing `error_code`, `artifact_id`, `message`, and `details` (`metric`, `value`, `limit`, `location`).

## Invariants
- Pure function of `(source_artifact, local_budget, contract)`.
- No randomness, clocks, environment probing, heuristics, fuzzy parsing, external model calls, or semantic inference.
- Same source input yields the same metric output.
- Metrics are computed only by explicit V1 rules for:
  - lines per function
  - nesting depth
  - cyclomatic complexity
  - fan-out
  - file size
  - public surface
  - state count
  - transition count
- State and transition checks enforce exact equality to contract declarations.
- Transition count also enforces `transition_count <= max_states * max_transitions_per_state`.

## Failure Modes
- Parse failure: `SAE_E_PARSE_ERROR`.
- Metric limit failures:
  - `SAE_E_LINES_PER_FUNCTION`
  - `SAE_E_NESTING_DEPTH`
  - `SAE_E_CYCLOMATIC_COMPLEXITY`
  - `SAE_E_FAN_OUT`
  - `SAE_E_FILE_SIZE`
  - `SAE_E_PUBLIC_SURFACE`
- Contract-count mismatch failures:
  - `SAE_E_STATE_COUNT`
  - `SAE_E_TRANSITION_COUNT`
- Internal failure: `SAE_E_INTERNAL_ERROR`.

## 6. Implementation Boundary
- MAY implement deterministic parsing and metric computation defined by the V1 static-analysis specification.
- MAY implement deterministic budget enforcement and contract state/transition equality checks.
- MUST NOT implement test execution, budget derivation, generator protocol handling, ledger grammar control, or freeze hashing.
- Responsibility ends at emitting `STATIC_ANALYSIS_PASSED` or `STATIC_ANALYSIS_FAILED` with structured failure payload.

## 7. Forbidden Responsibilities
- MUST NEVER perform semantic inference, fuzzy parsing, heuristic scoring, or model-assisted analysis.
- MUST NEVER reinterpret contract intent beyond explicit metric/count rules.
- MUST NEVER mutate source artifacts, contract data, or local budget values.
- MUST NEVER assign attempt lifecycle outcomes beyond static-analysis pass/fail.

## 8. External Dependencies
- MAY depend on normalized candidate artifact bytes, local budget, and validated contract count fields as read-only inputs.
- MAY depend on normalization rules as read-only canonical input rules.
- MUST NOT depend on `generator_interface`, `freeze`, or `ledger` to compute metrics or pass/fail decisions.
- Interaction with `harness` is input/output only: read-only verification inputs from harness, write-only static-analysis result back to harness.

## 9. State & Mutability Rules
- MUST execute statelessly per verification invocation.
- MUST treat source artifact, contract, and budget as immutable inputs.
- MUST NOT persist ASTs, metric caches, or cross-run analysis memory.
- Failure payload content is immutable after emission for the attempt.

## 10. Deterministic Data Models

### Normalized Candidate Artifact Representation (consumed)
- Name: `NormalizedCandidateArtifact`
- Field list (ordered):
  1. `normalized_bytes` — byte sequence — non-null
- Canonical ordering rules: for multi-file artifacts, file paths are lexicographically ordered in canonical archive form; text content is UTF-8 with LF line endings.
- Hash participation rules: normalized artifact bytes define `candidate_artifact_hash` and participate in freeze hashing.
- Immutability guarantees: consumed as read-only verification input.
- Lifecycle constraints: produced from successful generation response normalization; verified exactly once per attempt.

### Local Budget (consumed)
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
- Canonical ordering rules: keys follow budget schema order.
- Hash participation rules: participates in `request_id` when included in generator request payload; does not redefine run identity.
- Immutability guarantees: treated as immutable during verification.
- Lifecycle constraints: fixed for all attempts within a run.

### Contract Count View (consumed)
- Name: `ContractDeclaredCounts`
- Field list (ordered):
  1. `declared_state_count` — integer (`>= 0`) — non-null
  2. `declared_transition_count` — integer (`>= 0`) — non-null
- Canonical ordering rules: keys follow contract schema order.
- Hash participation rules: part of normalized contract bytes used for `contract_hash`.
- Immutability guarantees: treated as immutable during verification.
- Lifecycle constraints: fixed within the run; any contract change requires a new run.

### Static Analysis Failure Payload (produced)
- Name: `StaticAnalysisFailure`
- Field list (ordered):
  1. `error_code` — string (`SAE_E_*`) — non-null
  2. `artifact_id` — string — non-null
  3. `message` — string — non-null
  4. `details` — object — non-null
     - `metric` — string — non-null
     - `value` — scalar/object/array — non-null
     - `limit` — scalar/object/array — non-null
     - `location` — string — nullable (explicit `null` allowed)
- Canonical ordering rules: top-level keys are emitted in schema order; nested generic object keys are deterministic.
- Hash participation rules: when serialized in ledger payloads, fields participate in canonical event bytes.
- Immutability guarantees: immutable after emission for the attempt.
- Lifecycle constraints: emitted only with `STATIC_ANALYSIS_FAILED` for the current attempt.
