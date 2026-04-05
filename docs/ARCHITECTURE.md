# ARCHITECTURE DOCUMENT (V1 CANONICAL)
**Deterministic Agentic Software Generation Harness**
*Spec Version: 1.0*

---

## 1. SYSTEM PURPOSE

This system transforms a validated Contract into a Generated Artifact through a deterministic validation and freeze pipeline. The V1 system guarantees:

* bounded structural complexity
* local reasoning
* deterministic validation and freeze
* auditability
* zero silent debt

The generator (LLM/agent) is explicitly OUTSIDE the trusted core unless fully pinned and reproducible. The trusted core validates artifacts; it does not guarantee end-to-end generation determinism.

---

## 2. TRUST MODEL

### 2.1 HARD GATES (Trusted Computing Base)
These gates may block freeze and MUST be deterministic:

* Contract schema validation
* Contract cross-field validity
* Budget derivation (Budget Compiler)
* Static analysis vs budgets
* Test execution
* Freeze hashing

### 2.2 SOFT GATES (Advisory Only in V1)
These may emit warnings but CANNOT block freeze:

* LLM clarity scoring
* Semantic naming review
* Local reasoning heuristics

### 2.3 GENERATOR TRUST BOUNDARY
The generator is NOT part of the trusted core unless:

* generator version is pinned
* prompts are pinned
* behavior is empirically reproducible

Absent these, the system guarantees deterministic validation of a candidate artifact, not deterministic generation.

---

## 3. SUBSYSTEMS (V1)

The V1 architecture contains four active subsystems and two passive ones:

1. Contract Surface (Minimal Schema)
2. Budget Compiler (Deterministic)
3. Static Analysis Engine (Deterministic)
4. Harness (State Machine for Hard Gates)

Passive:

5. Ledger (Strict Event Model)
6. Exemption Manifest (Append-Only, Deterministic)

---

## 4. CONTRACT SURFACE (Minimal V1 Schema)

The contract expresses only what the Budget Compiler and Static Analysis Engine can enforce deterministically.

```json
{
  "artifact_id": "string",
  "artifact_kind": "pure_function | stateful_module | orchestrator | adapter | io_boundary",
  "public_surface_target": "integer >= 0",
  "is_stateful": "boolean",
  "has_io": "boolean",
  "has_concurrency": "boolean",
  "declared_state_count": "integer >= 0",
  "declared_transition_count": "integer >= 0",
  "declared_invariant_count": "integer >= 0",
  "test_obligation_class": "light | normal | heavy"
}
```

---

## 5. BUDGET COMPILER (Trusted Core)

### 5.1 RESPONSIBILITIES
* Validate contract schema
* Validate cross-field rules
* Select base profile
* Apply deterministic adjustments
* Clamp to global ceilings
* Emit local budget
* Reject incompatible contracts

### 5.2 GUARANTEES
* Same inputs → byte-identical output
* No randomness, clocks, environment access, or model calls
* Canonical field ordering
* Pure function of `(contract, global_ceilings, exemption_manifest)`

### 5.3 EXEMPTIONS AS INPUT
Exemptions are explicit inputs to budget derivation:

```text
local_budget = f(global_ceilings, contract, exemption_manifest)
```

Any change to the exemption manifest:
* changes the run configuration
* must be logged
* must produce a new `local_budget_hash`

### 5.4 EXEMPTION SCOPE (V1)
Exemptions may widen ONLY size-oriented limits:
* `max_lines_per_function`
* `max_file_size`

Exemptions may NEVER widen:
* `max_public_surface`
* `max_states`
* `max_transitions_per_state`
* `max_fan_out`

These remain governed solely by contract + global ceilings.

### 5.5 OUTPUT SCHEMA
```json
{
  "artifact_id": "string",
  "max_lines_per_function": "integer",
  "max_nesting_depth": "integer",
  "max_cyclomatic_complexity": "integer",
  "max_fan_out": "integer",
  "max_file_size": "integer",
  "max_public_surface": "integer",
  "max_states": "integer",
  "max_transitions_per_state": "integer"
}
```

### 5.6 OUTPUT INVARIANTS
* `artifact_id == contract.artifact_id`
* all numeric fields >= 0
* all numeric fields <= global ceilings
* `max_public_surface <= contract.public_surface_target`
* `max_states <= contract.declared_state_count`
* if `is_stateful = false` → `max_states = 0`, `max_transitions_per_state = 0`
* if `artifact_kind = pure_function` → same as above

### 5.7 CONTRACT VALIDITY RULES
Reject if:
* pure_function `has_io = true`
* pure_function `has_concurrency = true`
* pure_function `is_stateful = true`
* `declared_state_count > 0` but `is_stateful = false`
* `declared_state_count = 0` but `declared_transition_count > 0`
* `declared_transition_count > declared_state_count * global_ceilings.max_transitions_per_state`

### 5.8 DERIVATION ALGORITHM
1. Select base profile by `artifact_kind`
2. Apply flag adjustments (concurrency, I/O, invariants)
3. Clamp to global ceilings
4. Post-clamp sanity checks

### 5.9 POST-CLAMP SANITY CHECKS
Reject if:
* `max_public_surface < public_surface_target`
* `max_states < declared_state_count`
* `max_transitions_per_state * declared_state_count < declared_transition_count`

### 5.10 ERROR MODEL
Error codes:
* `BUDGET_E_INVALID_CONTRACT_SCHEMA`
* `BUDGET_E_INVALID_GLOBAL_CEILINGS_SCHEMA`
* `BUDGET_E_CONTRACT_FIELD_CONFLICT`
* `BUDGET_E_CONTRACT_INCOMPATIBLE_WITH_GLOBAL_LIMITS`
* `BUDGET_E_INTERNAL_RULE_TABLE_INVALID`

### 5.11 LEDGER EMISSION
The harness emits:
* `BUDGET_DERIVED`
* `EXEMPTION_APPLIED` (if applicable)

---

## 6. STATIC ANALYSIS ENGINE (Deterministic)

### 6.1 RESPONSIBILITIES
* Measure actual artifact metrics
* Compare metrics to local budget
* Emit pass/fail with structured errors

### 6.2 REQUIRED METRICS (V1)
* lines per function
* nesting depth
* cyclomatic complexity
* fan-out (unique direct module dependencies)
* file size (lines)
* public surface (exported functions + exported types)
* state count (enum members or annotated states)
* transition count (explicit edges in annotated state tables)

### 6.3 DETERMINISM REQUIREMENTS
* No heuristics
* No fuzzy parsing
* Same source → same metrics

---

## 7. HARNESS (Deterministic State Machine)

### 7.1 STATES
* `CONTRACT_VALIDATION`
* `BUDGET_DERIVATION`
* `GENERATION`
* `MECHANICAL_VERIFICATION`
* `FREEZE`
* `DONE`

### 7.2 TRANSITION RULES
Each gate reads ONLY artifacts from prior gates.
No gate may mutate prior artifacts.

### 7.3 BOUNDED REVISION
Each gate allows N retries.
After N failures → ABORT RUN.
No implicit rewind.

### 7.4 FREEZE REPRODUCIBILITY
Two levels:

**(1) Artifact-Verification Reproducibility (V1 guarantee)**
Given same:
* contract
* global ceilings
* exemption manifest
* candidate artifact
* toolchain version

→ pipeline must produce same frozen hash OR same failure.

**(2) End-to-End Generation Reproducibility (NOT guaranteed in V1)**
Requires pinned generator + pinned prompts.

---

## 8. EXEMPTION MANIFEST (Append-Only, Deterministic)

### 8.1 FORMAT
```json
{
  "exemption_id": "string",
  "artifact_id": "string",
  "reason": "string",
  "scope": "function | module",
  "max_lines_per_function_override": "integer or null",
  "max_file_size_override": "integer or null"
}
```

### 8.2 RULES
* Append-only
* No overrides to public surface, fan-out, states, transitions
* Any change → new run configuration
* Harness emits `EXEMPTION_APPLIED` event

---

## 9. LEDGER (Strict Event Model)

### 9.1 REQUIRED EVENTS
* `CONTRACT_ACCEPTED`
* `BUDGET_DERIVED`
* `EXEMPTION_APPLIED`
* `STATIC_ANALYSIS_PASSED` / `FAILED`
* `TESTS_PASSED` / `FAILED`
* `ARTIFACT_FROZEN`

### 9.2 EVENT PROPERTIES
* Canonical schema
* No free-form logging
* All hashes canonicalized

---

## 10. META-INVARIANTS (Enforceable)

### 10.1 Contract Supremacy
Generated artifacts may not introduce undeclared public surface or states.

### 10.2 Deterministic Gate Inputs
Each gate reads only declared artifacts from prior gates.

### 10.3 Bounded Revision
Each gate has fixed retry count; after N failures → abort.

### 10.4 Budget Non-Escalation
Budgets cannot relax unless contract version or exemption manifest changes.

### 10.5 Exemption Isolation
Exemptions may widen size limits only; never graph-shape or surface limits.

### 10.6 Freeze Reproducibility
Same inputs → same frozen hash OR same failure.

---

## 11. IMPLEMENTATION ORDER

1. Minimal Contract Schema
2. Budget Compiler (deterministic)
3. Static Analysis Engine (deterministic)
4. Harness (hard gates only)
5. Ledger (strict event model)
6. Exemption Manifest (append-only)
7. Soft Gates (advisory)
8. Richer DSL (later)
9. Full Domain Exemption Service (later)

END OF DOCUMENT
