# normalization

## Purpose
The Normalization subsystem defines and applies canonical normalization rules so that identical logical inputs and artifacts produce identical bytes for hashing, equality checks, replay, verification, and freeze.

## Inputs
- Text content participating in deterministic processing.
- JSON objects participating in deterministic processing.
- Candidate artifacts (single-file or multi-file).
- Configuration inputs used for hashing, including contract, global ceilings, applicable exemptions, and toolchain identity components.
- Generator request objects before `request_id` hashing.

## Outputs
- Canonically normalized UTF-8 byte representations for normalized domains.
- Canonical artifact representation (normalized single file or canonical tar stream for multi-file artifact).
- Deterministic hash input bytes used by `run_id`, `request_id`, `artifact_hash`, `freeze_hash`, and other `*_hash` fields.

## Invariants
- Deterministic, stateless, and environment-independent behavior.
- No semantic interpretation, no code rewriting, no statement reordering, and no unspecified formatting.
- Text normalization enforces UTF-8 without BOM and LF line endings; final newline presence is preserved.
- JSON normalization enforces deterministic key ordering (schema order for spec-defined objects; lexicographic for generic objects), preserved array order, explicit null handling, and deterministic serialization.
- Multi-file artifact normalization enforces deterministic archive path ordering and canonical metadata values.
- Hashes are computed over normalized bytes only; unnormalized data is outside contract.

## Failure Modes
- Any deviation from the canonical normalization rules invalidates deterministic equivalence and causes consuming deterministic gates to fail their validation contracts.
- V1 authoritative documents define no standalone normalization-specific error-code namespace.

## 6. Implementation Boundary
- MAY implement canonical text, JSON, request, configuration, and artifact normalization exactly as specified.
- MAY implement deterministic byte emission for all hash domains that require normalized inputs.
- MUST NOT implement policy decisions, gate pass/fail decisions, semantic interpretation, or subsystem-specific business rules.
- Responsibility ends at returning normalized bytes/representations or signaling normalization contract violation to the caller.

## 7. Forbidden Responsibilities
- MUST NEVER rewrite program semantics, reorder statements, or apply unspecified formatting transforms.
- MUST NEVER introduce implicit defaults, nondeterministic metadata, or environment-derived fields.
- MUST NEVER redefine hash domains, spec key order, or run/attempt lifecycle rules.
- MUST NEVER mutate source configuration membership (contract, ceilings, manifest) beyond canonical representation.

## 8. External Dependencies
- MAY depend on authoritative schema ordering and normalization-spec rules as read-only inputs.
- MAY serve `harness`, `generator_interface`, `budget_compiler`, `freeze`, and `ledger` as a read-only canonicalization service.
- MUST NOT depend on generator internals, agent state, or nondeterministic external services.
- MUST NOT require write access to subsystem-owned state to produce normalized output.

## 9. State & Mutability Rules
- MUST be stateless across invocations.
- MUST treat input values as immutable and produce new canonical output representations.
- MUST NOT persist caches that affect canonical output across runs.
- MUST NOT persist transient parser/serializer state as durable execution state.

## 10. Deterministic Data Models

### Generator Request Without ID (consumed and produced for hashing)
- Name: `GeneratorRequest_without_id`
- Field list (ordered):
  1. `run_id` — string — non-null
  2. `attempt` — integer (`>= 1`) — non-null
  3. `contract` — object — non-null
  4. `local_budget` — object — non-null
  5. `toolchain_capabilities` — object — nullable/omissible where explicitly allowed
- Canonical ordering rules: top-level keys are normalized in the order listed above; nested spec-defined objects use schema order.
- Hash participation rules: normalized bytes define `request_id`.
- Immutability guarantees: treated as immutable for normalization and hashing.
- Lifecycle constraints: one normalized request payload per attempt.

### Applicable Exemption Sequence (consumed and produced for hashing)
- Name: `ApplicableExemptions`
- Field list (ordered per entry):
  1. `exemption_id` — string — non-null
  2. `artifact_id` — string — non-null
  3. `scope` — enum (`function | module`) — non-null
  4. `target` — string — non-null
  5. `reason` — string — non-null
  6. `max_lines_per_function_override` — integer — nullable (explicit `null` allowed)
  7. `max_file_size_override` — integer — nullable (explicit `null` allowed)
- Canonical ordering rules: entries are ordered by `(artifact_id, scope, target, exemption_id)`; keys inside each entry follow schema order.
- Hash participation rules: normalized sequence defines `exemption_manifest_hash`.
- Immutability guarantees: input membership is append-only; normalized sequence is immutable for the run.
- Lifecycle constraints: only applicable entries participate in run identity and budget derivation.

### Canonical Artifact Representation (produced)
- Name: `CanonicalArtifact`
- Field list (ordered):
  1. `normalized_bytes` — byte sequence — non-null
- Canonical ordering rules: single-file content uses text normalization; multi-file content uses deterministic archive ordering by lexicographic path and fixed metadata.
- Hash participation rules: normalized bytes define `artifact_hash` and `freeze_hash` domains.
- Immutability guarantees: canonical bytes are immutable once emitted for a verification/freeze step.
- Lifecycle constraints: consumed by deterministic verification and freeze gates only.

### Canonical JSON Value (produced)
- Name: `CanonicalJson`
- Field list (ordered):
  1. `normalized_bytes` — UTF-8 JSON byte sequence — non-null
- Canonical ordering rules: schema-order keys for spec-defined objects; lexicographic key ordering for generic objects; array order preserved.
- Hash participation rules: defines bytes for `contract_hash`, `global_ceilings_hash`, `request_id`, and ledger `*_hash` fields where applicable.
- Immutability guarantees: immutable normalized output for a given logical value.
- Lifecycle constraints: reused as deterministic input identity across run lifecycle gates.
