# NORMALIZATION_SPEC.md
**Canonical Normalization Specification (V1)**
*Spec Version: 1.0.0*

---

## 1. PURPOSE

This document defines the **canonical normalization rules** for all inputs and artifacts that participate in:

* hashing
* equality checks
* freeze
* run identity
* replay and audit

Normalization is what makes:

* “same logical input” → “same bytes”
* “same logical artifact” → “same freeze hash”

This spec is **normative**. Implementations MUST follow it exactly.

---

## 2. SCOPE

Normalization applies to:

* `GeneratorRequest` (for `request_id` hashing)
* `GeneratorResponse` (for protocol validation)
* candidate artifacts (for verification + freeze)
* configuration inputs that are hashed:
    * contract
    * global ceilings
    * applicable exemptions
    * toolchain identity components
    * schemas where specified

Anything that participates in a hash used for:

* `run_id`
* `request_id`
* `artifact_hash`
* `freeze_hash`
* `*_hash` fields in the ledger

MUST be normalized according to this spec.

---

## 3. GENERAL PRINCIPLES

### 3.1 Determinism

Normalization MUST be:

* deterministic
* stateless
* independent of environment (OS, locale, filesystem, etc.)

### 3.2 Byte-Identical Output

For any normalized domain:

* identical logical inputs MUST produce byte-identical normalized outputs.

### 3.3 No Semantic Interpretation

Normalization MUST NOT:

* interpret language semantics
* rewrite code
* reorder statements
* “pretty print” or format beyond what is explicitly specified

---

## 4. TEXT NORMALIZATION

Applies to:

* source files
* JSON documents
* text-based configuration
* any text content used in hashing

### 4.1 Encoding

* All text MUST be encoded as UTF-8.
* No BOM (Byte Order Mark) is allowed.

### 4.2 Line Endings

* All line endings MUST be normalized to LF (`\n`).
* CRLF (`\r\n`) and CR (`\r`) MUST be converted to LF.

### 4.3 Trailing Whitespace

* Trailing whitespace at the end of lines MUST be preserved.
* No trimming is performed unless explicitly specified for a domain.

### 4.4 Final Newline

* Text MAY or MAY NOT end with a newline.
* Normalization MUST NOT add or remove a final newline.
* Whatever is present after line-ending normalization is preserved.

---

## 5. JSON NORMALIZATION

Applies to:

* `GeneratorRequest` (for `request_id`)
* configuration objects that are hashed
* ledger event payloads (before hashing, if applicable)

### 5.1 Structural Rules

Given a JSON value:

* **Objects:**
    * keys MUST appear in a deterministic order:
        * for spec-defined objects: schema order
        * for generic objects: lexicographic order by key
    * no duplicate keys allowed
* **Arrays:**
    * element order MUST be preserved
* **Numbers:**
    * MUST be represented in minimal decimal form (no leading zeros, no `+`, no trailing `.0` unless required)
* **Booleans:**
    * `true` or `false` in lowercase
* **Null:**
    * `null` in lowercase

### 5.2 Whitespace

* No insignificant whitespace is required.
* Implementations MAY choose any whitespace strategy as long as the resulting byte sequence is deterministic for the same logical value.
* Recommended: compact form (no spaces except where required).

### 5.3 Canonical Object Ordering

For spec-defined objects (e.g., contract, budget, ledger events):

* Key order MUST follow the schema order defined in the relevant spec.

For generic objects (e.g., error_details):

* Keys MUST be sorted lexicographically by UTF-8 byte order.

---

## 6. ARTIFACT NORMALIZATION

Applies to candidate artifacts produced by the generator and used for:

* static analysis
* tests
* freeze hashing

### 6.1 Single-File Artifacts

For a single text file:

* Apply text normalization (Section 4).
* No additional transformations.

### 6.2 Multi-File Artifacts

Multi-file artifacts MUST be represented as a canonical archive.

Canonical archive rules:

* **Archive format:** tar (ustar or pax is acceptable if deterministic).
* **File ordering:**
    * sort by path lexicographically (UTF-8 byte order).
* **Path format:**
    * use forward slashes `/`
    * no leading `./`
    * no trailing slashes for files
* **File metadata:**
    * owner, group, timestamps, permissions MUST be set to fixed canonical values:
        * `uid = 0`
        * `gid = 0`
        * `uname = "root"`
        * `gname = "root"`
        * `mode = 0644` for files, `0755` for directories
        * `mtime = 0`
    * no extended attributes
* **Directory entries:**
    * optional; if present, MUST follow same canonical rules.
* **File content:**
    * each file’s content MUST be text-normalized (Section 4) before being written into the archive.

The final tar byte stream is the canonical artifact representation.

### 6.3 Binary Artifacts (V1)

V1 does not define binary artifact normalization beyond:

* the archive rules above
* no content rewriting

Binary content MUST be included as-is (no re-encoding), but archive metadata MUST still follow canonical rules.

---

## 7. REQUEST NORMALIZATION (GENERATOR INTERFACE)

Applies to `GeneratorRequest_without_id` before computing `request_id`.

### 7.1 Steps

1. Construct the logical request object:
```json
{
  "run_id": "...",
  "attempt": "...",
  "contract": { },
  "local_budget": { },
  "toolchain_capabilities": { } 
}
```
*(Note: `toolchain_capabilities` can be omitted).*

2. Normalize all nested JSON values according to Section 5.

3. Serialize to UTF-8 JSON using deterministic key ordering:
    * top-level keys in schema order: `"run_id"`, `"attempt"`, `"contract"`, `"local_budget"`, `"toolchain_capabilities"`
    * nested objects follow their own schema or lexicographic rules.

4. Compute:
```text
request_id = hash(normalized_bytes)
```

The normalization algorithm used here MUST be versioned and included in `toolchain_hash`.

---

## 8. CONFIGURATION NORMALIZATION

Applies to:

* contract
* global ceilings
* applicable exemptions
* schemas (where hashed)
* toolchain identity components

### 8.1 Contract

* Represented as JSON.
* Normalized using JSON rules (Section 5).
* Key order MUST follow the contract schema.

### 8.2 Global Ceilings

* Same as contract: JSON + schema-order keys.

### 8.3 Applicable Exemptions

* Filter manifest to applicable entries.
* Sort entries deterministically by: `(artifact_id, scope, target, exemption_id)`
* Normalize each entry as JSON with schema-order keys.
* The sequence of normalized entries is then serialized as a JSON array (with preserved order).

### 8.4 Schemas

If schemas are hashed:

* They MUST be normalized as text:
    * UTF-8
    * LF line endings
* No reformatting of schema content.

### 8.5 Toolchain Identity

`toolchain_hash` MUST be computed over a canonical bundle that includes:

* versions of:
    * budget compiler
    * static analysis engine
    * normalization spec version
    * generator interface spec version
    * ledger schema version
* any other components explicitly declared as part of `toolchain_hash`

The bundle MUST be represented as a JSON object with schema-order keys and normalized according to Section 5.

---

## 9. HASHING RULES

### 9.1 Hash Function

V1 does not mandate a specific hash algorithm, but:

* It MUST be cryptographically strong (e.g., SHA-256).
* It MUST be fixed for a given deployment.
* Any change to the hash algorithm MUST increment:
    * `toolchain_hash`
    * relevant spec versions

### 9.2 Hash Domain

All hashes used for:

* `run_id`
* `request_id`
* `artifact_hash`
* `freeze_hash`
* `*_hash` fields

MUST be computed over **normalized bytes** only.

No hash may be computed over unnormalized or partially normalized data.

---

## 10. EQUALITY & REPLAY

### 10.1 Input Equality

Two inputs are considered equal for deterministic gates if and only if:

* their normalized byte representations are identical
* therefore their canonical hashes are identical

### 10.2 Replay

For replay:

* Given the same normalized inputs and `toolchain_hash`, the system MUST produce:
    * the same `run_id`
    * the same `request_id` per attempt
    * the same `artifact_hash` for the same logical artifact
    * the same `freeze_hash`
    * the same ledger event sequence

---

## 11. VERSIONING

`normalization_spec_version = 1.0.0`

Any change to:

* text normalization rules
* JSON normalization rules
* archive format or rules
* request normalization behavior
* configuration normalization behavior
* hashing domain rules

MUST increment `normalization_spec_version` and MUST be reflected in:

* `toolchain_hash`
* any spec that depends on normalization behavior

END OF DOCUMENT
