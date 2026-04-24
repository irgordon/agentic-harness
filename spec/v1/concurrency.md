# Concurrency Contract (v1)

The deterministic harness runtime is single-threaded by contract.

## Rules

1. No parallel attempts.
2. No asynchronous I/O in trusted-core execution paths.
3. No background worker threads.
4. No scheduler-dependent behavior in deterministic gates.
5. Ledger emission is strictly serial and append-only.
