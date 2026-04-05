# WALKTHROUGH.md
**Operational Lifecycle Walkthrough (V1)**
*Spec Version: 1.0.0*

---

## 0. PURPOSE

This document describes the complete operational lifecycle for using the Agentic Generation Harness, from a blank machine to a fully functioning agentic coding workflow. It defines the deterministic initialization boundary, the execution loop, the freeze boundary, and the audit trail.

This is a runbook, not a design document. All normative behavior is defined in the subsystem specifications in `/docs`.

---

## 1. STARTING FROM ZERO

You begin with an empty machine and no project state:

```text
~/projects/
    (empty)
```

---

## 2. CREATE PROJECT ROOT

Create the repository root:

```bash
mkdir agentic-harness
cd agentic-harness
```

---

## 3. SCAFFOLD REPOSITORY STRUCTURE

Create the canonical directory layout:

```text
/
├── contract/
├── ceilings/
├── exemptions/
├── budget_compiler/
├── static_analysis/
├── normalization/
├── freeze/
├── harness/
├── generator_interface/
├── ledger/
├── agent/
└── docs/
```

All directories are initially empty.

---

## 4. POPULATE SPECIFICATIONS

Copy the specification documents into `/docs`:

```text
ARCHITECTURE_DOCUMENT.md
RUN_MODEL.md
HARNESS.md
LEDGER.md
EXEMPTION_MANIFEST.md
GENERATOR_INTERFACE.md
STATIC_ANALYSIS_ENGINE.md
NORMALIZATION_SPEC.md
```

These documents are the authoritative source of truth.

---

## 5. TOOLCHAIN INITIALIZATION (MANDATORY)

Initialize the deterministic toolchain identity:

```bash
./toolchain init
```

This step MUST:
* detect compiler/runtime versions
* record build flags
* record hash algorithm identifier
* record `normalization_spec_version`
* record `generator_interface_spec_version`
* record platform identifier
* produce `toolchain_manifest.json`

`toolchain_hash` MUST be computed from the normalized `toolchain_manifest.json`.

---

## 6. VALIDATE SCHEMAS

Before implementation begins, validate:
* contract schema
* ceilings schema
* exemption schema
* ledger schema
* normalization spec compatibility

This prevents early drift between documentation and implementation.

---

## 7. IMPLEMENT DETERMINISTIC CORE

Implement the trusted core in a compiled language:

### 7.1 Budget Compiler
`contract` + `ceilings` + `exemptions` → `local_budget`

### 7.2 Static Analysis Engine
structural metrics + budget enforcement

### 7.3 Normalization Engine
implements `NORMALIZATION_SPEC.md` exactly

### 7.4 Harness State Machine
implements `RUN_MODEL.md` exactly

### 7.5 Ledger Writer
append-only, canonical serialization

### 7.6 Freeze Engine
* canonical archive creation
* canonical hashing
* final integrity validation
* MUST operate only on normalized artifacts

---

## 8. VERIFY DETERMINISTIC BUILD

Build the deterministic core twice:

```bash
make clean && make
sha256sum harness

make clean && make
sha256sum harness
```

The hashes MUST match.

Builds MUST disable embedded timestamps and nondeterministic metadata.

---

## 9. RUN DETERMINISTIC SELF-TEST (CALIBRATION)

Use a versioned reference dataset:

```text
/calibration/reference_contract/
/calibration/reference_artifact/
/calibration/reference_ledger/
```

Verify:
* `run_id` reproducibility
* `artifact_hash` reproducibility
* `freeze_hash` reproducibility
* ledger sequence reproducibility

---

## 10. IMPLEMENT GENERATOR INTERFACE

Implement deterministic request/response boundary:
* canonical request builder
* canonical response validator
* timeout enforcement
* request hashing
* deterministic staging directory creation

This is the boundary between deterministic and non-deterministic systems.

---

## 11. IMPLEMENT AGENT LAYER (NON-DETERMINISTIC)

Implement in Python:

```text
agent/
    planner/
    generator/
    adapters/
```

### 11.1 planner/
reads contract + ledger feedback → adjusts strategy

### 11.2 generator/
calls LLM (Codex, Gemini, Copilot, etc.)

### 11.3 adapters/
converts Harness requests → model prompts → artifacts

The agent MUST NOT write into deterministic workspace paths.

The agent MUST run in an isolated working directory with no write access to deterministic system paths.

---

## 12. CREATE FIRST CONTRACT (ILLUSTRATIVE)

Example simplified contract:

`contract/contract.json`:
```json
{
  "artifact_id": "my_first_project",
  "artifact_kind": "library",
  "language": "python",
  "test_obligation_class": "basic"
}
```

This is illustrative only. Actual schema is defined in `/docs`.

---

## 13. DEFINE GLOBAL CEILINGS

`ceilings/global_ceilings.json`:
```json
{
  "max_lines_per_function": 200,
  "max_file_size": 20000,
  "max_public_surface": 50
}
```

---

## 14. ADD EXEMPTIONS (OPTIONAL)

`exemptions/manifest.json`:
```json
[
  {
    "exemption_id": "allow_big_init",
    "artifact_id": "my_first_project",
    "scope": "function",
    "target": "my_first_project/__init__.py",
    "reason": "bootstrap complexity",
    "max_lines_per_function_override": 300,
    "max_file_size_override": null
  }
]
```

---

## 15. ENSURE WORKSPACE CLEANLINESS

Before running:
* workspace MUST contain only files declared in run configuration
* no leftover artifacts
* no stale staging directories

---

## 16. RUN THE HARNESS

```bash
./harness run contract/contract.json
```

The Harness:
* validates contract
* computes `local_budget`
* emits ledger events
* creates deterministic staging directory:
```text
/workspace/runs/<run_id>/staging/attempt_1/
```
* calls agent for `GENERATION_ATTEMPT(1)`

---

## 17. AGENT GENERATES CANDIDATE ARTIFACT

The agent writes ONLY to:

```text
/workspace/runs/<run_id>/staging/attempt_1/
```

It produces:
* code
* tests
* supporting files

---

## 18. HARNESS VERIFICATION

Verification order:
1. check raw artifact size
2. normalize artifact
3. run static analysis
4. run tests

Harness emits ledger events.

If failure → `ATTEMPT_FAILED(i)`
If success → `ATTEMPT_PASSED(i)`

If `i == max_attempts`:
* emit `ATTEMPT_FAILED(i)`
* emit `RUN_ABORTED`

---

## 19. FREEZE PHASE

Freeze MUST operate only on the normalized artifact.

Freeze:
* canonical normalization
* canonical archive creation
* canonical hashing
* final integrity validation
* record `normalization_spec_version`
* write frozen artifact to:
```text
/workspace/runs/<run_id>/artifacts/frozen.tar
```
* emit `ARTIFACT_FROZEN`
* emit `RUN_SUCCESS`

---

## 20. USING THE SYSTEM IN REAL WORK

You can now generate new artifacts:

```bash
./harness run contract/new_module.json
```

The agent generates code.
The Harness validates it.
The Ledger records everything.
Freeze produces a canonical artifact.

This completes the lifecycle.

END OF DOCUMENT
