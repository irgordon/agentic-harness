# STATIC_ANALYSIS_ENGINE.md
**Deterministic Structural Metrics Engine (V1)**
*Spec Version: 1.0.1*

---

## 1. PURPOSE

The Static Analysis Engine (SAE) is a deterministic subsystem that measures structural properties of a candidate artifact and compares them to the local budget produced by the Budget Compiler.

The SAE is part of the trusted core. It MUST be fully deterministic.

The SAE does NOT:
* interpret semantics
* evaluate naming clarity
* perform heuristic reasoning
* call models or external tools
* infer behavior beyond explicit rules

It measures only what is explicitly defined in this document.

---

## 2. TRUST REQUIREMENTS

The SAE MUST satisfy:

* Same source input → same metric output (byte-identical)
* No randomness, clocks, environment probing, or model calls
* No language heuristics beyond explicit rules
* Canonical ordering of output fields
* Pure function of `(source_artifact, local_budget, contract)`

---

## 3. INPUTS

### 3.1 Source Artifact

A directory or file containing the generated code.

Language assumptions (V1):
* A single implementation language MUST be selected for V1.
* Parsing rules MUST be deterministic and fully specified.
* Multi-language support is out of scope for V1.

### 3.2 Local Budget

The budget object produced by the Budget Compiler:

```json
{
  "artifact_id": "string",
  "max_lines_per_function": "integer",
  "max_nesting_depth": "integer",
  "max_cyclomatic_complexity": "integer",
  "max_fan_out": "integer",
  "max_file_size": "integer",
  "max_public_surface": "integer",
  "max_states": "integer",
  "max_transitions_per_state": "integer"
}
```

### 3.3 Contract (for state/transition validation)

The SAE MUST read `declared_state_count` and `declared_transition_count` directly from the validated contract.

---

## 4. OUTPUT

### 4.1 Pass/Fail Decision

The SAE emits:

* `STATIC_ANALYSIS_PASSED`
* `STATIC_ANALYSIS_FAILED`

### 4.2 Structured Error Payload

On failure:

```json
{
  "error_code": "SAE_E_*",
  "artifact_id": "string",
  "message": "string",
  "details": {
    "metric": "string",
    "value": "any",
    "limit": "any",
    "location": "string or null"
  }
}
```

### 4.3 Required Error Codes

* `SAE_E_PARSE_ERROR`
* `SAE_E_LINES_PER_FUNCTION`
* `SAE_E_NESTING_DEPTH`
* `SAE_E_CYCLOMATIC_COMPLEXITY`
* `SAE_E_FAN_OUT`
* `SAE_E_FILE_SIZE`
* `SAE_E_PUBLIC_SURFACE`
* `SAE_E_STATE_COUNT`
* `SAE_E_TRANSITION_COUNT`
* `SAE_E_INTERNAL_ERROR`

---

## 5. METRIC DEFINITIONS (V1)

All metrics MUST be computed using the exact rules below. No heuristics.

### 5.1 Lines Per Function

**Definition:**
The number of non-blank, non-comment lines between the opening brace of a function and its matching closing brace.

**Rules:**
* Count only lexical lines.
* Ignore blank lines.
* Ignore comment-only lines.
* Inline lambdas/anonymous functions count as functions.

**Violation:**
If any function exceeds `local_budget.max_lines_per_function` → FAIL.

### 5.2 Nesting Depth

**Definition:**
The maximum depth of nested control structures inside a function.

**Count the following as +1 nesting:**
* `if` / `else if` / `else`
* `for` / `while` / `do`
* `switch`
* `try` / `catch` / `finally`
* function definitions inside functions

**Do NOT count:**
* braces used for scoping only
* object/struct literals

**Violation:**
If any function exceeds `local_budget.max_nesting_depth` → FAIL.

### 5.3 Cyclomatic Complexity

**Definition:**
$M = E - N + 2P$
where:
* $E$ = number of edges in control flow graph
* $N$ = number of nodes
* $P$ = number of connected components (usually 1)

**Simplified V1 counting rule:**
Complexity = 1
  + (# of `if`)
  + (# of `else if`)
  + (# of `for`)
  + (# of `while`)
  + (# of `case`)
  + (# of `catch`)
  + (# of `&&` or `||` in conditions)

**Violation:**
If any function exceeds `local_budget.max_cyclomatic_complexity` → FAIL.

### 5.4 Fan-Out

**Definition:**
The number of unique direct module dependencies imported or required by the artifact.

**Rules:**
* Count each unique imported module path once.
* Do NOT count:
    * standard library imports
    * type-only imports (if language supports them)
    * relative imports that resolve to the same file

**Violation:**
If `fan_out > local_budget.max_fan_out` → FAIL.

### 5.5 File Size

**Definition:**
The number of non-blank, non-comment lines in the file.

**Violation:**
If `file_size > local_budget.max_file_size` → FAIL.

### 5.6 Public Surface

**Definition:**
The number of exported functions AND exported types.

**Rules:**
* Count exported functions.
* Count exported classes, structs, interfaces, enums.
* Do NOT count:
    * private/internal symbols
    * re-exports of external modules (V1)

**Violation:**
If `public_surface > local_budget.max_public_surface` → FAIL.

### 5.7 State Count

**Definition:**
The number of states declared in the artifact’s state machine.

**Rules:**
* If annotated states exist → use annotated states.
* Else → use enum members.
* Both methods MUST NOT be combined.

**Violation:**
If `state_count != contract.declared_state_count` → FAIL.

### 5.8 Transition Count

**Definition:**
The number of explicit transitions in the state machine.

**Rules:**
* Count each (state → state) edge.
* Count only explicit transitions.
* Ignore unreachable states (V1).

**Violation:**
If `transition_count != contract.declared_transition_count` → FAIL.
If `transition_count > local_budget.max_states * local_budget.max_transitions_per_state` → FAIL.

---

## 6. ALGORITHM

```text
function run_static_analysis(source, local_budget, contract):
    ast = parse(source)
    if ast.error:
        fail(SAE_E_PARSE_ERROR)

    metrics = compute_metrics(ast)

    if metrics.lines_per_function > local_budget.max_lines_per_function:
        fail(SAE_E_LINES_PER_FUNCTION)

    if metrics.nesting_depth > local_budget.max_nesting_depth:
        fail(SAE_E_NESTING_DEPTH)

    if metrics.cyclomatic_complexity > local_budget.max_cyclomatic_complexity:
        fail(SAE_E_CYCLOMATIC_COMPLEXITY)

    if metrics.fan_out > local_budget.max_fan_out:
        fail(SAE_E_FAN_OUT)

    if metrics.file_size > local_budget.max_file_size:
        fail(SAE_E_FILE_SIZE)

    if metrics.public_surface > local_budget.max_public_surface:
        fail(SAE_E_PUBLIC_SURFACE)

    if metrics.state_count != contract.declared_state_count:
        fail(SAE_E_STATE_COUNT)

    if metrics.transition_count != contract.declared_transition_count:
        fail(SAE_E_TRANSITION_COUNT)

    if metrics.transition_count > local_budget.max_states * local_budget.max_transitions_per_state:
        fail(SAE_E_TRANSITION_COUNT)

    return STATIC_ANALYSIS_PASSED
```

---

## 7. LEDGER EMISSION

On pass:
* `STATIC_ANALYSIS_PASSED`

On fail:
* `STATIC_ANALYSIS_FAILED`

Payload includes:
* `artifact_id`
* `metric`
* `value`
* `limit`
* `location` (if applicable)

---

## 8. TEST CASES

### 8.1 Positive Cases
* Function sizes within limits
* Fan-out within limits
* Public surface matches contract
* State/transition counts match contract

### 8.2 Negative Cases
* Excessive lines per function
* Excessive nesting depth
* Excessive cyclomatic complexity
* Fan-out > budget
* File size > budget
* Public surface > budget
* State count mismatch
* Transition count mismatch

### 8.3 Determinism Cases
* Repeated runs produce identical metrics
* Reordered source file fields do not affect metrics
* Canonical JSON output identical across runs

---

## 9. VERSIONING

`static_analysis_spec_version = 1.0.1`

Any change to:
* metric definitions
* counting rules
* error codes
* parsing rules

MUST increment the spec version.

END OF DOCUMENT
