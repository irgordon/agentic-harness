#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

validate_case() {
  local schema_path="$1"
  local instance_path="$2"
  local expected="$3"
  local label="$4"

  python - "$schema_path" "$instance_path" "$expected" "$label" <<'PY'
import json
import sys

schema_path, instance_path, expected, label = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]


def load_json(path):
    with open(path, 'rb') as f:
        raw = f.read()
    text = raw.decode('utf-8')
    return json.loads(text)


def json_type_matches(schema_type, value):
    if schema_type == 'object':
        return isinstance(value, dict)
    if schema_type == 'array':
        return isinstance(value, list)
    if schema_type == 'string':
        return isinstance(value, str)
    if schema_type == 'integer':
        return isinstance(value, int) and not isinstance(value, bool)
    if schema_type == 'boolean':
        return isinstance(value, bool)
    if schema_type == 'null':
        return value is None
    raise ValueError(f'unsupported schema type: {schema_type}')


def validate(schema, value):
    expected_type = schema.get('type')
    if isinstance(expected_type, list):
        if not any(json_type_matches(t, value) for t in expected_type):
            return False
    elif isinstance(expected_type, str):
        if not json_type_matches(expected_type, value):
            return False

    if schema.get('type') == 'object':
        required = schema.get('required', [])
        for key in required:
            if key not in value:
                return False
        props = schema.get('properties', {})
        if schema.get('additionalProperties') is False:
            for key in value.keys():
                if key not in props:
                    return False
        for key, subschema in props.items():
            if key in value and not validate(subschema, value[key]):
                return False
    elif schema.get('type') == 'array':
        item_schema = schema.get('items')
        if item_schema is not None:
            for item in value:
                if not validate(item_schema, item):
                    return False

    if 'minimum' in schema:
        if not isinstance(value, int) or value < schema['minimum']:
            return False
    if 'minLength' in schema:
        if not isinstance(value, str) or len(value) < schema['minLength']:
            return False

    return True

try:
    schema = load_json(schema_path)
    instance = load_json(instance_path)
    valid = validate(schema, instance)
except Exception:
    valid = False

want_valid = expected == 'pass'
if valid != want_valid:
    print(
        f'validation mismatch for {label}: got valid={valid}, expected={want_valid}',
        file=sys.stderr,
    )
    sys.exit(1)
PY
}

validate_case "contract/schema.json" "fixtures/contracts/basic.json" "pass" "contract valid fixture"
validate_case "contract/schema.json" "tests/integration/fixtures/schema/contract_unknown_field.json" "fail" "contract unknown field"
validate_case "contract/schema.json" "tests/integration/fixtures/schema/contract_missing_required.json" "fail" "contract missing required"
validate_case "contract/schema.json" "tests/integration/fixtures/schema/contract_wrong_type.json" "fail" "contract wrong type"

validate_case "ceilings/schema.json" "fixtures/ceilings/basic.json" "pass" "ceilings valid fixture"
validate_case "ceilings/schema.json" "tests/integration/fixtures/schema/ceilings_unknown_field.json" "fail" "ceilings unknown field"
validate_case "ceilings/schema.json" "tests/integration/fixtures/schema/ceilings_missing_required.json" "fail" "ceilings missing required"
validate_case "ceilings/schema.json" "tests/integration/fixtures/schema/ceilings_wrong_type.json" "fail" "ceilings wrong type"

validate_case "exemptions/schema.json" "fixtures/exemptions/basic.json" "pass" "exemptions valid fixture"
validate_case "exemptions/schema.json" "tests/integration/fixtures/schema/exemptions_unknown_field.json" "fail" "exemptions unknown field"
validate_case "exemptions/schema.json" "tests/integration/fixtures/schema/exemptions_missing_required.json" "fail" "exemptions missing required"
validate_case "exemptions/schema.json" "tests/integration/fixtures/schema/exemptions_wrong_type.json" "fail" "exemptions wrong type"

UTF8_TMP="$(mktemp)"
trap 'rm -f "$UTF8_TMP"' EXIT
printf '{"artifact_id":"bad\xff"}' > "$UTF8_TMP"
validate_case "contract/schema.json" "$UTF8_TMP" "fail" "contract invalid utf-8"

echo "schema validation integration test passed"
