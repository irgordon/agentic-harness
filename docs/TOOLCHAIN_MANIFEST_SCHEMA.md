# TOOLCHAIN_MANIFEST_SCHEMA.md
**Toolchain Manifest Schema (V1.1)**
*Spec Version: 1.1.0*

---

## 0. PURPOSE

The toolchain manifest defines the deterministic identity of the execution environment. It is the root of reproducibility for all runs.

The manifest:
* captures all components that influence deterministic behavior
* provides a canonical representation for hashing
* enables cross-environment replay
* ensures `toolchain_hash` is stable, meaningful, and immutable
* prevents silent drift in build or runtime environments

This document defines the required fields, normalization rules, and validation constraints for `toolchain_manifest.json`.

---

## 1. FILE LOCATION

The manifest MUST be stored at:

```text
<repo_root>/toolchain_manifest.json
```

This file MUST be created or verified by:

```bash
harness init
```

---

## 2. CANONICAL FORM

The manifest MUST:
* be a JSON object
* use canonical compact JSON serialization
* follow schema-order key ordering
* contain no null values
* omit optional fields entirely when absent
* be normalized according to `NORMALIZATION_SPEC.md`
* reject unknown fields

`toolchain_hash` MUST be computed over the normalized byte sequence of the manifest.

Presence of unknown fields MUST cause manifest validation failure.

---

## 3. TOP-LEVEL STRUCTURE

The manifest MUST contain the following top-level keys in this exact order:

```json
{
  "manifest_version": "...",
  "compiler": { },
  "runtime": { },
  "build": { },
  "hashing": { },
  "normalization": { },
  "interfaces": { },
  "platform": { },
  "subsystems": { }
}
```

---

## 4. FIELD DEFINITIONS

### 4.1 manifest_version

String. Required.

Represents the version of the toolchain manifest schema.

Example:
```json
"1.1.0"
```

### 4.2 compiler

Object. Required.

```json
{
  "name": "clang",
  "version": "15.0.7",
  "flags": ["-O2", "-fno-strict-aliasing"]
}
```

Fields:
* `name`: string (e.g., "clang", "gcc")
* `version`: string
* `flags`: array of strings

The `flags` array MUST be sorted lexicographically by UTF-8 byte order before serialization.

### 4.3 runtime

Object. Required.

```json
{
  "language": "c",
  "stdlib_implementation": "glibc",
  "stdlib_version": "2.38"
}
```

Fields:
* `language`: string
* `stdlib_implementation`: string (e.g., "glibc", "musl")
* `stdlib_version`: string

`stdlib_version` MUST identify the specific implementation version.

### 4.4 build

Object. Required.

```json
{
  "flags": ["-O2", "-fno-strict-aliasing"],
  "deterministic_mode": true
}
```

Fields:
* `flags`: array of strings (sorted lexicographically)
* `deterministic_mode`: boolean

If `deterministic_mode` is false, harness initialization MUST fail.

### 4.5 hashing

Object. Required.

```json
{
  "algorithm": "sha256"
}
```

Fields:
* `algorithm`: string

Changing the hash algorithm MUST invalidate all prior toolchain hashes.

### 4.6 normalization

Object. Required.

```json
{
  "normalization_spec_version": "1.0.1",
  "normalization_engine_version": "1.0.0"
}
```

Fields:
* `normalization_spec_version`: string
* `normalization_engine_version`: string

### 4.7 interfaces

Object. Required.

```json
{
  "generator_interface_spec_version": "1.0.0"
}
```

Fields:
* `generator_interface_spec_version`: string

### 4.8 platform

Object. Required.

```json
{
  "os": "linux",
  "arch": "x86_64",
  "kernel_version": "6.6.12",
  "timezone": "UTC"
}
```

Fields:
* `os`: string
* `arch`: string
* `kernel_version`: string
* `timezone`: string (MUST be "UTC")

All timestamps in the toolchain environment MUST be normalized to UTC.

### 4.9 subsystems

Object. Required.

```json
{
  "static_analysis_engine_version": "1.0.0",
  "budget_compiler_version": "1.0.0",
  "ledger_schema_version": "1.0.0",
  "freeze_engine_version": "1.0.0"
}
```

Fields:
* `static_analysis_engine_version`: string
* `budget_compiler_version`: string
* `ledger_schema_version`: string
* `freeze_engine_version`: string

Subsystem version identifiers MUST be immutable and content-addressable (e.g., semantic version tied to a release artifact or commit hash).

---

## 5. VALIDATION RULES

The manifest MUST satisfy:
* all required fields present
* no unknown fields
* no null values
* no empty strings
* arrays sorted lexicographically where required
* strings MUST be UTF-8
* manifest MUST be normalized before hashing
* `manifest_version` MUST match this schema version
* `stdlib_implementation` MUST be recognized
* `timezone` MUST be "UTC"

The manifest MUST be validated against this schema before hashing.

---

## 6. HASHING RULES

`toolchain_hash` MUST be computed as:

```text
hash(normalized(toolchain_manifest.json))
```

The hash algorithm MUST match:

```text
hashing.algorithm
```

---

## 7. IDEMPOTENCY & MUTABILITY RULES

`harness init` MUST:
* verify existing manifest if present
* regenerate manifest only if environment changes
* emit new `toolchain_hash` only when required
* never overwrite a valid manifest silently
* write updates using atomic file replacement

The manifest MUST be treated as immutable after successful initialization. If the environment changes, a new manifest MUST be generated rather than modifying the existing one.

---

## 8. EXAMPLE MANIFEST

```json
{
  "manifest_version": "1.1.0",
  "compiler": {
    "name": "clang",
    "version": "15.0.7",
    "flags": [
      "-O2",
      "-fno-strict-aliasing"
    ]
  },
  "runtime": {
    "language": "c",
    "stdlib_implementation": "glibc",
    "stdlib_version": "2.38"
  },
  "build": {
    "flags": [
      "-O2",
      "-fno-strict-aliasing"
    ],
    "deterministic_mode": true
  },
  "hashing": {
    "algorithm": "sha256"
  },
  "normalization": {
    "normalization_spec_version": "1.0.1",
    "normalization_engine_version": "1.0.0"
  },
  "interfaces": {
    "generator_interface_spec_version": "1.0.0"
  },
  "platform": {
    "os": "linux",
    "arch": "x86_64",
    "kernel_version": "6.6.12",
    "timezone": "UTC"
  },
  "subsystems": {
    "static_analysis_engine_version": "1.0.0",
    "budget_compiler_version": "1.0.0",
    "ledger_schema_version": "1.0.0",
    "freeze_engine_version": "1.0.0"
  }
}
```

END OF DOCUMENT
