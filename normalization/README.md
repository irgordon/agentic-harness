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
