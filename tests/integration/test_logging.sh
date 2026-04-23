#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

./harness_cli run \
  --contract fixtures/contracts/basic.json \
  --ceilings fixtures/ceilings/basic.json \
  --exemptions fixtures/exemptions/basic.json \
  --artifact fixtures/artifacts/basic.c \
  --out-ledger "$TMP_DIR/ledger.jsonl" \
  --log-json \
  > "$TMP_DIR/stdout.txt" \
  2> "$TMP_DIR/stderr.json"

grep -q '"t":1' "$TMP_DIR/stderr.json"
grep -q '"level":"INFO"' "$TMP_DIR/stderr.json"

echo "structured logging test passed"
