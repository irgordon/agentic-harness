# LOCAL_PRODUCTION_READINESS_ASSESSMENT.md

## Scope

This assessment reviews the repository as of **2026-04-24** for readiness to run the Agentic Harness in a local production-like environment.

> Historical note: the previous 2026-04-23 assessment has been preserved at
> `docs/archive/LOCAL_PRODUCTION_READINESS_ASSESSMENT_2026-04-23.md`.

## Current Implementation State

### What is already present

1. **Deterministic build and CLI wiring exists** in `Makefile` and produces both
   `core/libdeterministic_core.a` and `harness_cli`.
2. **Core subsystems are implemented with deterministic interfaces**:
   - `core/budget_compiler/`
   - `core/static_analysis/`
   - `core/normalization/`
   - `core/freeze/`
   - `core/harness/`
   - `core/generator_interface/`
   - `core/ledger/`
3. **Input schema surfaces are present** for contract, ceilings, and exemptions,
   with `additionalProperties: false` constraints.
4. **Integration and unit tests exist** for replay determinism, structured logging,
   schema-validation behavior, and subsystem unit coverage.

### What remains incomplete for stronger local production confidence

1. **Schema validation is not enforced by the C harness binary itself yet**; integration
   coverage validates schema behavior at the repository level.
2. **Static analysis remains heuristic and intentionally minimal** relative to the full
   specification depth documented in `docs/`.
3. **Toolchain manifest details are still partially placeholder (`unknown`)** and should
   be fully pinned for stronger operational reproducibility claims.
4. **No CI configuration is currently versioned in this repository** to automatically
   enforce the test/reproducibility gates on every change.

## Production Readiness Verdict

**Verdict: Conditionally ready for local deterministic development workflows; not yet fully hardened for production operations.**

The repository now supports an end-to-end local run with deterministic artifacts and
ledger outputs, backed by tests. Remaining work is mostly around hardening, enforcement,
and operationalization.

## Completion Plan for Local Production Hardening

### Phase 1 — Gate enforcement and parity

1. Move schema validation from test-only coverage into the harness run path.
2. Expand failure-mode testing for malformed inputs and boundary values.
3. Ensure CLI behavior and docs remain version-locked as interfaces evolve.

### Phase 2 — Reproducibility hardening

1. Replace placeholder toolchain manifest fields with pinned environment metadata.
2. Add repeat-run hash assertions beyond current fixture baseline.
3. Add deterministic replay/diff ergonomics for failure triage.

### Phase 3 — Operational readiness

1. Add CI that enforces `make all`, unit/integration tests, and reproducibility checks.
2. Add release packaging/versioning policy for local deployments.
3. Add runbook-level docs for incident/debug workflows.

## Suggested “Done” Criteria for Local Production

A local production deployment should be considered hardened only when all of the
following are true:

- Input schema validation is enforced at runtime by the harness.
- `harness init` and `harness run` behavior is deterministic and fully versioned.
- Replay and freeze hashes remain stable across clean rebuilds/reruns.
- Ledger output is append-only, grammar-valid, and replay-auditable.
- CI continuously enforces deterministic invariants and test gates.

## Near-Term Priority Recommendation

If prioritizing only the fastest path to stronger local production posture, implement in this order:

1. runtime schema validation in `harness run`
2. CI gate wiring
3. toolchain manifest pinning
4. expanded adversarial integration fixtures
