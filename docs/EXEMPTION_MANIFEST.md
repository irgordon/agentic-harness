EXEMPTION_MANIFEST.md
Deterministic Exemption Manifest (V1)
Spec Version: 1.0.4


============================================================
1. PURPOSE
============================================================

The Exemption Manifest is a deterministic, append‑only configuration
surface that allows narrowly scoped, size‑oriented overrides for cases of
irreducible domain complexity.

The manifest is NOT a general override mechanism. It MUST NOT relax any
architectural, graph‑shape, or public‑surface constraints.

The manifest participates directly in budget derivation. Therefore:

  • Only applicable exemptions influence budget derivation.
  • Only applicable exemptions influence run identity.
  • Any change to applicable exemptions requires a new run.
  • All applied exemptions MUST be logged via EXEMPTION_APPLIED events.


============================================================
2. TRUST REQUIREMENTS
============================================================

The Exemption Manifest MUST satisfy:

  • Deterministic parsing and normalization
  • Canonical field ordering
  • Append‑only membership semantics
  • No nondeterministic metadata (timestamps, UUIDs, etc.)
  • No environment‑dependent fields
  • No implicit defaults
  • No overrides to graph‑shape or public‑surface limits

The manifest is a trusted input to the Budget Compiler and Harness.


============================================================
3. SCOPE OF EXEMPTIONS (V1)
============================================================

Exemptions MAY widen ONLY size‑oriented limits:

  • max_lines_per_function
  • max_file_size

Exemptions MUST NOT widen:

  • max_public_surface
  • max_states
  • max_transitions_per_state
  • max_fan_out

Exemptions MUST NOT:

  • introduce new public surface
  • introduce new states or transitions
  • increase dependency fan‑out
  • alter artifact_kind
  • alter declared_state_count or declared_transition_count
  • alter test obligations


============================================================
4. MANIFEST STRUCTURE
============================================================

The manifest MUST be a list of exemption entries:

[
  { exemption_entry_1 },
  { exemption_entry_2 },
  ...
]

Ordering MUST be canonicalized deterministically after parsing:

  • sorted lexicographically by (artifact_id, scope, target)
  • sorting is a normalization step, not a mutation of membership


============================================================
5. EXEMPTION ENTRY SCHEMA
============================================================

Each entry MUST conform to the following schema:

{
  "exemption_id": "string",
  "artifact_id": "string",
  "scope": "function | module",
  "target": "string",   // canonical identifier
  "reason": "string",
  "max_lines_per_function_override": "integer or null",
  "max_file_size_override": "integer or null"
}

Rules:

  • exemption_id MUST be a stable, human‑assigned identifier.
  • target MUST use canonical naming:
        - function targets: fully qualified symbol names
        - module targets: canonical repository‑relative paths
  • reason MUST be a short, human‑readable justification.
  • At least one override field MUST be non‑null.
  • No additional fields are permitted.


============================================================
6. APPLICABILITY RULES
============================================================

An exemption entry is applicable to a given artifact if:

  • entry.artifact_id == contract.artifact_id
  • AND entry.scope matches the target type
  • AND entry.target matches a function or module in the artifact

Only applicable exemptions participate in:

  • budget derivation
  • exemption_manifest_hash
  • run_id computation
  • EXEMPTION_APPLIED events

Unrelated entries MUST NOT affect run identity or budget derivation.

At most one applicable exemption MAY exist per:

  (artifact_id, scope, target)

If duplicates exist → EXEMPTION_E_DUPLICATE_ENTRY.


============================================================
7. BUDGET DERIVATION INTEGRATION
============================================================

The Budget Compiler MUST incorporate exemptions as follows:

applicable_exemptions = filter(manifest, artifact_id)

exemption_manifest_hash = hash(normalized(applicable_exemptions))

local_budget = f(global_ceilings, contract, applicable_exemptions)

For each applicable exemption, overrides MUST be aggregated
monotonically:

effective_max_lines_per_function =
    max(all applicable max_lines_per_function_override values)

effective_max_file_size =
    max(all applicable max_file_size_override values)

Then apply:

local_budget.max_lines_per_function =
    min(global_ceilings.max_lines_per_function,
        max(local_budget.max_lines_per_function,
            effective_max_lines_per_function))

local_budget.max_file_size =
    min(global_ceilings.max_file_size,
        max(local_budget.max_file_size,
            effective_max_file_size))

Validation rules (Budget Compiler stage):

  • Overrides MUST NOT reduce derived budget values.
  • Overrides MUST NOT exceed global ceilings.
  • Overrides MUST NOT target forbidden fields.
  • Violations MUST produce BUDGET_E_CONTRACT_INCOMPATIBLE_WITH_GLOBAL_LIMITS.


============================================================
8. RUN CONFIGURATION BOUNDARY
============================================================

Run identity MUST depend ONLY on applicable exemptions:

exemption_manifest_hash =
    hash(normalized(applicable_exemptions))

run_id =
    hash(contract_hash,
         global_ceilings_hash,
         exemption_manifest_hash,
         toolchain_hash)

Any change to applicable exemptions requires a new run.

Changes to unrelated entries MUST NOT affect run identity.


============================================================
9. LEDGER INTEGRATION
============================================================

For each applicable exemption, the Harness MUST emit:

EXEMPTION_APPLIED

payload = {
  "exemption_id": "string",
  "exemption_hash": "string"
}

Rules:

  • One event per applicable exemption.
  • Events MUST be emitted in the same deterministic order used in
    budget derivation.
  • Events MUST appear after BUDGET_DERIVED and before any attempt events.


============================================================
10. ERROR MODEL
============================================================

Manifest parser MUST emit:

  • EXEMPTION_E_INVALID_SCHEMA
  • EXEMPTION_E_DUPLICATE_ENTRY
  • EXEMPTION_E_INVALID_SCOPE
  • EXEMPTION_E_INVALID_TARGET
  • EXEMPTION_E_EMPTY_OVERRIDE   // both override fields null
  • EXEMPTION_E_INTERNAL_ERROR

Budget Compiler MUST emit:

  • BUDGET_E_CONTRACT_INCOMPATIBLE_WITH_GLOBAL_LIMITS
  • BUDGET_E_FORBIDDEN_OVERRIDE
  • BUDGET_E_OVERRIDE_EXCEEDS_CEILING


============================================================
11. SERIALIZATION RULES
============================================================

The manifest MUST be serialized using:

  • keys in the exact schema order defined in Section 5
  • UTF‑8 encoding
  • no trailing commas
  • explicit nulls
  • deterministic ordering of entries


============================================================
12. APPEND‑ONLY RULE
============================================================

Append‑only applies to membership, not physical ordering.

Rules:

  • Existing entries MUST NOT be removed or modified.
  • New entries MUST be appended to the set, then the full set MUST be
    re‑sorted deterministically.
  • exemption_id MUST remain stable across versions.

Violations MUST cause EXEMPTION_E_INVALID_SCHEMA.


============================================================
13. VERSIONING
============================================================

exemption_manifest_spec_version = 1.0.4

Any change to:
  • entry schema
  • override rules
  • applicability rules
  • aggregation rules
  • normalization/canonicalization rules
  • integration with Budget Compiler
  • serialization rules

MUST increment the spec version.

END OF DOCUMENT
