# CONTRIBUTING_DETERMINISM.md

## Pre-commit setup

```bash
git config core.hooksPath .githooks
```

## Determinism guardrails enforced in pre-commit

1. `make env-normalize`
2. `make reproducible`
3. `make test-unit`
4. `make test-integration`

If any command fails, commit is blocked.

## Coding constraints

- Keep behavior independent from wall-clock time.
- Use deterministic ordering for file/collection traversal.
- Use explicit error codes and deterministic stderr/log output formats.
- Update `spec/` snapshots whenever governance surfaces change.
