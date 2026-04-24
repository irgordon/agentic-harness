#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

./harness_cli run \
  --artifact fixtures/artifacts/basic.c \
  --out-ledger "$TMP_DIR/default-ledger.jsonl" \
  > "$TMP_DIR/default-stdout.txt" \
  2> "$TMP_DIR/default-stderr.txt"

grep -q "run completed successfully" "$TMP_DIR/default-stdout.txt"

./harness_cli run \
  --config tests/integration/fixtures/config/local_config_json_logging.json \
  --artifact fixtures/artifacts/basic.c \
  --out-ledger "$TMP_DIR/json-ledger.jsonl" \
  > "$TMP_DIR/json-stdout.txt" \
  2> "$TMP_DIR/json-stderr.txt"

grep -q '"t":1' "$TMP_DIR/json-stderr.txt"
grep -q '"level":"INFO"' "$TMP_DIR/json-stderr.txt"

set +e
./harness_cli run \
  --config tests/integration/fixtures/config/local_config_invalid.json \
  --artifact fixtures/artifacts/basic.c \
  --out-ledger "$TMP_DIR/bad-ledger.jsonl" \
  > "$TMP_DIR/bad-stdout.txt" \
  2> "$TMP_DIR/bad-stderr.txt"
RC=$?
set -e

if [ "$RC" -ne 2 ]; then
  echo "expected exit code 2 for invalid config, got $RC" >&2
  exit 1
fi

grep -q '^config error: failed to load deterministic configuration$' "$TMP_DIR/bad-stderr.txt"

echo "config integration test passed"
