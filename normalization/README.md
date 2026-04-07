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
