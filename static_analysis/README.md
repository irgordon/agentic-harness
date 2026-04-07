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
