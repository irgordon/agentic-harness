# LOCAL_PRODUCTION_READINESS_ASSESSMENT.md

## Scope

This assessment reviews the repository as of **2026-04-23** for readiness to run the Agentic Harness in a local production-like environment.

## Current Implementation State

### What is already present

1. **Architecture and subsystem specifications are extensive** under `docs/` and per-subsystem READMEs.
2. **Deterministic core build wiring exists** in `Makefile` and produces `core/libdeterministic_core.a`.
3. **Ledger appears to be the only materially implemented core subsystem** (`core/ledger/ledger.c` has full logic and schema-constrained APIs).

### What is currently missing or incomplete

1. **Core subsystem implementations are mostly stubs**:
   - `core/budget_compiler/budget_compiler.c` (empty)
   - `core/static_analysis/static_analysis.c` (empty)
   - `core/normalization/normalization.c` (empty)
   - `core/freeze/freeze.c` (empty)
   - `core/harness/harness.c` (empty)
   - `core/generator_interface/generator_interface.c` (empty)
2. **Expected repository surfaces from top-level documentation are absent** (e.g., `contract/`, `ceilings/`, `exemptions/`, and `agent/` are referenced but not present).
3. **No executable harness binary or CLI entrypoint** is defined; build currently outputs only a static library.
4. **No schema artifacts or validators** are present for the documented contract/ceilings/exemption/toolchain JSON validation flow.
5. **No deterministic calibration fixtures** (reference datasets/replay assets) are present for reproducibility verification.
6. **No tests/CI gates** are present for deterministic behavior, grammar conformance, or subsystem-level correctness.
7. **No operational bootstrapping command implementation** (docs describe `harness init` / `toolchain init`, but no corresponding executable implementation is present).
8. **No integration runner** that executes the run model (contract validation → budget derivation → attempt loop → verification → freeze → terminal event).

## Production Readiness Verdict

**Verdict: Not production-ready for local deployment.**

The repository currently resembles a **spec-first scaffold with partial core implementation (ledger-heavy)**, not an end-to-end runnable system.

## Completion Plan for Local Production Usability

### Phase 1 — Minimum runnable deterministic pipeline (must-have)

1. Implement deterministic `core/harness/harness.c` state machine per `docs/RUN_MODEL.md`.
2. Implement deterministic `core/budget_compiler/budget_compiler.c` against documented error and invariant rules.
3. Implement deterministic `core/static_analysis/static_analysis.c` for all required metrics and failure payloads.
4. Implement deterministic `core/normalization/normalization.c` and canonical JSON/file normalization support.
5. Implement deterministic `core/freeze/freeze.c` canonical archive + hash flow.
6. Implement deterministic `core/generator_interface/generator_interface.c` request/response validation and request_id handling.
7. Add a CLI binary target (for example `harness`) with at least:
   - `harness init`
   - `harness run --contract ... --ceilings ... --exemptions ...`

### Phase 2 — Deterministic input surfaces

1. Add missing input directories and schemas:
   - `contract/`
   - `ceilings/`
   - `exemptions/`
2. Add JSON schema files and strict validators that reject unknown fields.
3. Add `toolchain_manifest.json` generation and validation path.
4. Add canonical hash computation utilities shared across subsystems.

### Phase 3 — Reproducibility and quality gates

1. Add deterministic self-test fixtures (contract/artifact/ledger references).
2. Add repeat-build reproducibility tests (binary/library hash equality across clean builds).
3. Add subsystem unit tests for all documented error code paths.
4. Add integration tests that assert exact ledger event ordering and terminal conditions.
5. Add CI pipeline enforcing `make`, tests, and reproducibility checks.

### Phase 4 — Local production operations

1. Add configuration management for local runs (paths, generator timeout, max attempts).
2. Add structured logs/diagnostics while preserving deterministic core behavior.
3. Add failure triage utilities (ledger replay and run diff tools).
4. Add versioned release packaging for local deployment.

## Suggested “Done” Criteria for Local Production

A local production deployment should be considered usable only when all of the following are true:

- `harness init` deterministically generates and validates `toolchain_manifest.json`.
- `harness run` executes the full documented run model end-to-end.
- Same normalized inputs produce the same run identity and deterministic gate results.
- Ledger output is append-only, grammar-valid, and reproducible across replay.
- Freeze artifact and freeze hash are reproducible under repeated runs.
- CI proves deterministic behavior and guards regressions.

## Near-Term Priority Recommendation

If prioritizing only the fastest path to local production usability, implement in this order:

1. `harness.c` orchestration + minimal CLI
2. `budget_compiler.c`
3. `generator_interface.c`
4. `normalization.c`
5. `static_analysis.c`
6. `freeze.c`
7. deterministic integration tests + reproducibility fixtures

This order enables an end-to-end vertical slice quickly, then tightens correctness and reproducibility.
