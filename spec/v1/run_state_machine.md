# Run State Machine (Spec Freeze v1)

This file freezes the minimum state machine implemented by the local runnable harness.

```text
CONTRACT_VALIDATION
  -> BUDGET_DERIVATION
  -> GENERATION_ATTEMPT(1)
  -> STATIC_ANALYSIS
  -> FREEZE
  -> RUN_SUCCESS
```

## Deterministic constraints

- Input files are read as bytes.
- Candidate artifact is normalized before freeze.
- Freeze hash is derived from normalized artifact and run inputs.
- Ledger event order is fixed.
