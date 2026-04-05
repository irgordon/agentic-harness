# GENERATOR_INTERFACE.md
**External Generator Interface (V1)**
*Spec Version: 1.0.1*

---

## 1. PURPOSE

The Generator Interface defines the contract between the Harness and an external, potentially non-deterministic generator that produces candidate artifacts from a contract and local budget.

The generator is explicitly OUTSIDE the trusted core in V1.

The interface MUST:
* constrain inputs and outputs
* make non-determinism explicit
* support replay and debugging via stable identities
* integrate cleanly with the Harness attempt model

---

## 2. TRUST MODEL

The generator:
* MAY be non-deterministic by default.
* MUST be treated as an external acquisition component.
* MUST NOT be assumed pure or reproducible unless explicitly pinned.

The Harness:
* MUST NOT rely on generator determinism in V1.
* MUST treat each generator invocation as a distinct attempt.
* MUST validate all generator outputs via deterministic gates.
* MUST isolate generator side effects to its own environment.

---

## 3. INPUTS

For each attempt, the Harness MUST construct a canonical request object:

`GeneratorRequest`:
```json
{
  "request_id": "string",
  "run_id": "string",
  "attempt": "integer",
  "contract": { },
  "local_budget": { },
  "toolchain_capabilities": { }
}
```

### 3.1 Attempt Numbering

* Each generator invocation MUST increment the attempt counter.
* One invocation == one attempt.
* Attempt numbers MUST match the Ledger `attempt` field.

### 3.2 Request Identity

The request identifier MUST bind to the full canonical payload:

```text
request_hash = hash(normalized(GeneratorRequest_without_request_id))
request_id   = request_hash
```

The Harness MUST:
* compute `request_id` from the canonical request payload
* store `request_hash` for replay/debugging
* verify that any reconstructed request for the same attempt produces the same `request_id`

### 3.3 toolchain_capabilities

* Advisory only in V1.
* MUST NOT participate in `run_id` or `request_id` computation.
* MUST NOT be treated as part of the trusted core.

---

## 4. OUTPUTS

The generator MUST return a single response per request:

`GeneratorResponse`:
```json
{
  "request_id": "string",
  "status": "success | failure",
  "candidate_artifact": "opaque or null",
  "error_code": "GEN_E_* or null",
  "error_details": { }
}
```

Rules:
* `request_id` MUST echo the input `request_id`.
* On `status = "success"`:
    * `candidate_artifact` MUST be non-null.
    * `error_code` and `error_details` MUST be null.
* On `status = "failure"`:
    * `candidate_artifact` MUST be null.
    * `error_code` MUST be non-null.
    * `error_details` MAY be null or structured.

---

## 5. CANDIDATE ARTIFACT NORMALIZATION

The candidate artifact is opaque to the generator but MUST be normalized canonically by the Harness before hashing or verification.

Normalization MUST define:
* line endings: normalized to LF
* encoding: UTF-8
* multi-file artifacts:
    * deterministic file ordering (e.g., lexicographic path order)
    * deterministic packaging (e.g., tar with fixed metadata)
* file metadata:
    * ignore permission bits and timestamps
    * ignore non-content metadata

The freeze hash and any ledger hashes MUST be computed over the normalized artifact representation.

---

## 6. HARNESS INTEGRATION

### 6.1 Attempt Lifecycle

For each attempt `i`:
1. Harness constructs canonical `GeneratorRequest` (without `request_id`).
2. Harness computes `request_id = hash(normalized(request_without_id))`.
3. Harness sends `GeneratorRequest` (with `request_id`) to generator.
4. Harness receives `GeneratorResponse`.
5. Harness validates response (Section 7).
6. On failure:
    * emit `GENERATION_FAILED`
    * emit `ATTEMPT_FAILED` (reason = "generation_failed")
    * proceed to next attempt (if any).
7. On success:
    * normalize `candidate_artifact`
    * emit `GENERATION_SUCCEEDED`
    * pass normalized artifact to `VERIFICATION_ATTEMPT`.

### 6.2 Oversize Protection

Before `VERIFICATION_ATTEMPT`, the Harness MUST:
* compute artifact size (lines / bytes / files as configured)
* reject any artifact exceeding global maximum artifact size
* treat such rejection as:
    * `status = failure` at harness level
    * `ATTEMPT_FAILED` (reason = "verification_failed")

---

## 7. RESPONSE VALIDATION

The Harness MUST validate:
* `request_id` matches the sent `request_id`
* `status` ∈ `{"success", "failure"}`
* null/non-null constraints on fields
* `candidate_artifact` presence/absence consistent with `status`

Any violation MUST be treated as:
* `status = "failure"`
* `error_code = "GEN_E_PROTOCOL"`

The Harness MUST deserialize the response into a canonical in-memory representation before validation to avoid transport-induced variation.

---

## 8. TIMEOUTS

Timeout behavior MUST be deterministic at the configuration level.
* `generator_timeout_ms` MUST be part of the run configuration.
* The same run configuration MUST use the same timeout value.
* Timeout expiration MUST be interpreted relative to this value, not environment-specific defaults.

On timeout:
* treat as `status = "failure"`
* `error_code = "GEN_E_TIMEOUT"`

---

## 9. SECURITY AND ISOLATION

The generator:
* MUST NOT mutate contract, budget, manifest, or ledger.
* MUST NOT write to harness-controlled storage.
* MUST NOT access harness internal state except via this interface.

Operational invariant:
* The generator MUST execute in an isolated environment whose writes cannot affect Harness behavior except through `GeneratorResponse`.

---

## 10. ERROR CODE NAMESPACE

Generator error codes MUST:
* use the prefix `GEN_E_`
* remain stable in meaning across generator versions
* be documented by the generator implementation

Suggested codes (non-exhaustive):
* `GEN_E_TIMEOUT`
* `GEN_E_INTERNAL`
* `GEN_E_PROTOCOL`
* `GEN_E_UNSUPPORTED_CONTRACT`
* `GEN_E_RESOURCE_LIMIT`

---

## 11. PINNED MODE (OPTIONAL, FUTURE)

Pinned mode is OPTIONAL in V1 but, if used, MUST satisfy:
* `generator_version` fixed
* configuration (prompts, sampling params) fixed
* dependency graph fixed
* execution environment fixed (libraries, OS image, etc.)

In pinned mode, the generator SHOULD produce the same artifact for the same `request_id`, but this is NOT a V1 guarantee and MUST NOT be assumed by the Harness.

---

## 12. TRANSPORT NEUTRALITY

V1 does not mandate a specific transport (in-process, RPC, etc.), but:
* `GeneratorRequest` and `GeneratorResponse` MUST be representable as canonical JSON objects matching the schemas above.
* Transport-specific metadata MUST be ignored by the Harness.
* The Harness MUST always deserialize into canonical in-memory structures before validation and hashing.

---

## 13. VERSIONING

`generator_interface_spec_version = 1.0.1`

Any change to:
* request schema
* response schema
* normalization rules
* error semantics
* timeout semantics
* pinning semantics

MUST increment the spec version.

END OF DOCUMENT
