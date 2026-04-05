# LEDGER.md
**Deterministic Ledger & Event Model (V1)**
*Spec Version: 1.0.3*

---

## 1. PURPOSE

The Ledger is the authoritative, append-only event log for a Harness run. It records all deterministic gate outcomes, attempt-level results, exemption applications, and final run status.

The Ledger MUST:
* provide a complete, minimal, canonical trace of the run
* support reproducibility and auditability
* avoid free-form or implementation-specific logging
* serialize events deterministically
* never reorder or mutate previously written events

The Ledger is a passive subsystem. It does NOT:
* perform validation
* interpret semantics
* retry gates
* store artifacts
* store generator prompts or internal model state

---

## 2. TRUST REQUIREMENTS

The Ledger MUST satisfy:

* Deterministic serialization
* Append-only writes
* Canonical field ordering
* No nondeterministic metadata (timestamps, UUIDs, etc.)
* No environment-dependent fields
* No implicit events
* Single-writer, serial event emission

All events MUST be emitted explicitly by the Harness.

---

## 3. RUN MODEL

A **run** is defined by fixed versions of:
* contract
* global ceilings
* exemption manifest
* toolchain

A run contains up to `N` **attempts**, each consisting of:
1. `GENERATION_ATTEMPT(i)`
2. `VERIFICATION_ATTEMPT(i)`

Attempt numbering invariant:
* attempts start at 1
* attempts increase by exactly 1
* attempts never skip values
* attempts never repeat

Exactly one terminal event MUST occur per run:
* `RUN_SUCCESS`
OR
* `RUN_ABORTED`

Both MUST NOT occur in the same run.

---

## 4. RUN IDENTIFIER

### 4.1 run_id Definition

`run_id` MUST be computed deterministically as:

```text
run_id = hash(
    contract_hash,
    global_ceilings_hash,
    exemption_manifest_hash,
    toolchain_hash
)
```

Properties:
* deterministic
* reproducible across environments
* stable across attempts within a run
* changes only when run configuration changes

---

## 5. EVENT GRAMMAR (V1)

Events MUST follow this grammar and MUST appear only where allowed.

### 5.1 Contract Phase
* `CONTRACT_ACCEPTED`
* `CONTRACT_REJECTED` → `RUN_ABORTED` (terminal)

### 5.2 Budget Phase
* `BUDGET_DERIVED`
* `EXEMPTION_APPLIED` (0 or more)
* `BUDGET_FAILED` → `RUN_ABORTED` (terminal)

### 5.3 Attempt Loop (for attempt i)
* `GENERATION_ATTEMPTED`
* `GENERATION_FAILED` → `ATTEMPT_FAILED`
* `GENERATION_SUCCEEDED`

If generation succeeded:
* `STATIC_ANALYSIS_PASSED` OR `STATIC_ANALYSIS_FAILED`
* If `STATIC_ANALYSIS_PASSED`:
    * `TESTS_PASSED` OR `TESTS_FAILED`

If either static analysis or tests fail:
* `ATTEMPT_FAILED`

If both pass:
* `ATTEMPT_PASSED` → `FREEZE`

### 5.4 Freeze Phase
* `ARTIFACT_FROZEN`
OR
* `FREEZE_FAILED` → `RUN_ABORTED` (terminal)

If `ARTIFACT_FROZEN` is emitted, it MUST be immediately followed by:
* `RUN_SUCCESS` (terminal)

### 5.5 Run Terminal
* `RUN_SUCCESS`
* `RUN_ABORTED`

---

## 6. EVENT SCHEMA

All events MUST conform to the canonical schema:

```json
{
  "event_type": "string",
  "run_id": "string",
  "attempt": "integer or null",
  "artifact_id": "string or null",
  "contract_hash": "string or null",
  "global_ceilings_hash": "string or null",
  "exemption_manifest_hash": "string or null",
  "toolchain_hash": "string or null",
  "payload": { ... }
}
```

Rules:
* Fields MUST appear in the exact order listed above.
* Nulls MUST be explicit.
* Hashes MUST be canonical digests of normalized inputs.
* No timestamps, UUIDs, or nondeterministic metadata.

---

## 7. EVENT PAYLOADS

The attempt number is carried in the envelope (`attempt` field), NOT in payloads.

### 7.1 CONTRACT_ACCEPTED
```json
{
  "contract_version": "string",
  "validation": "passed"
}
```

### 7.2 CONTRACT_REJECTED
```json
{
  "contract_version": "string",
  "error_code": "CONTRACT_E_*",
  "details": { }
}
```

### 7.3 BUDGET_DERIVED
```json
{
  "local_budget_hash": "string"
}
```

### 7.4 EXEMPTION_APPLIED
```json
{
  "exemption_id": "string",
  "exemption_hash": "string"
}
```

### 7.5 BUDGET_FAILED
```json
{
  "error_code": "BUDGET_E_*",
  "details": { }
}
```

### 7.6 GENERATION_ATTEMPTED
```json
{ }
```

### 7.7 GENERATION_FAILED
```json
{
  "error_code": "GEN_E_*",
  "details": { }
}
```

### 7.8 GENERATION_SUCCEEDED
```json
{
  "candidate_artifact_hash": "string"
}
```

### 7.9 STATIC_ANALYSIS_PASSED
```json
{
  "metrics_hash": "string"
}
```

### 7.10 STATIC_ANALYSIS_FAILED
```json
{
  "error_code": "SAE_E_*",
  "details": { }
}
```

### 7.11 TESTS_PASSED
```json
{
  "test_suite_hash": "string"
}
```

### 7.12 TESTS_FAILED
```json
{
  "error_code": "TEST_E_*",
  "details": { }
}
```

### 7.13 ATTEMPT_FAILED
```json
{
  "reason": "generation_failed | static_analysis_failed | tests_failed | verification_failed"
}
```

### 7.14 ATTEMPT_PASSED
```json
{ }
```

### 7.15 ARTIFACT_FROZEN
```json
{
  "freeze_hash": "string"
}
```

### 7.16 FREEZE_FAILED
```json
{
  "error_code": "FREEZE_E_*",
  "details": { }
}
```

### 7.17 RUN_SUCCESS
```json
{
  "attempts_used": "integer"
}
```

### 7.18 RUN_ABORTED
```json
{
  "attempts_used": "integer",
  "error_code": "HARNESS_E_*",
  "details": { }
}
```

---

## 8. SERIALIZATION RULES

### 8.1 Canonical JSON
Events MUST be serialized using:
* keys in the exact schema order defined in Section 6
* no extra whitespace beyond what is required for valid JSON
* UTF-8 encoding
* no trailing commas
* explicit nulls

### 8.2 Append-Only
Events MUST be appended in the exact order emitted.

### 8.3 No Rewrites
No event may be modified or deleted after emission.

### 8.4 No Aggregation
Events MUST NOT be merged or summarized.

---

## 9. ERROR MODEL

Ledger errors MUST be limited to:
* `LEDGER_E_SERIALIZATION`
* `LEDGER_E_APPEND_FAILURE`
* `LEDGER_E_INVALID_EVENT_SCHEMA`

The Ledger MUST NOT:
* reinterpret subsystem errors
* generate new error codes for gate failures

---

## 10. CONCURRENCY MODEL

The Ledger MUST operate in:
* single-writer mode
* serial event emission
* no parallel writes
* no asynchronous buffering

---

## 11. VERSIONING

`ledger_spec_version = 1.0.3`

Any change to:
* event grammar
* event schemas
* serialization rules
* error codes
* run_id definition

MUST increment the spec version.

END OF DOCUMENT
