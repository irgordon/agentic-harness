# Developer Guide

## Deterministic setup

1. `make clean && make all`
2. `make test-unit`
3. `make test-integration`
4. `make test-logging`
5. `make test-schema-validation`
6. `make test-config`

## First deterministic run

```bash
./harness_cli run --artifact fixtures/artifacts/basic.c
```

## Explain a run ledger

```bash
./harness_cli explain --ledger run_ledger.jsonl
```
