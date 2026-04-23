#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

LEDGER_PATH="$TMP_DIR/ledger.jsonl"
STDOUT_PATH="$TMP_DIR/stdout.txt"

./harness_cli run \
  --contract fixtures/contracts/basic.json \
  --ceilings fixtures/ceilings/basic.json \
  --exemptions fixtures/exemptions/basic.json \
  --artifact fixtures/artifacts/basic.c \
  --out-ledger "$LEDGER_PATH" \
  > "$STDOUT_PATH"

diff -u fixtures/expected/ledger.jsonl "$LEDGER_PATH"

grep -q "freeze_hash=$(cat fixtures/expected/freeze_hash.txt)" "$STDOUT_PATH"

echo "integration replay test passed"
