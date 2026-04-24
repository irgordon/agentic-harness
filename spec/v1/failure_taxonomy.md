# Failure Taxonomy (v1)

## Categories

- `CONFIG_FAILURE`
- `CONTRACT_FAILURE`
- `BUDGET_FAILURE`
- `GENERATION_FAILURE`
- `VERIFICATION_FAILURE`
- `FREEZE_FAILURE`
- `HARNESS_FAILURE`

## Deterministic mappings

- Config loading failures map to `E_CONFIG_*` and non-zero process exit.
- Harness-owned failures map to `HARNESS_E_*`.
- Generator protocol failures map to `GEN_E_*`.
- Static analysis failures map to `SAE_E_*`.
