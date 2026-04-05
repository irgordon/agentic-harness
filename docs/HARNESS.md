HARNESS.md
Deterministic Agentic Coding Harness (V1)
Spec Version: 1.0.2


============================================================
1. PURPOSE
============================================================

The Harness is the deterministic orchestrator of the V1 agentic
software‑generation pipeline. It executes deterministic validation gates
in a fixed order, coordinates acquisition of candidate artifacts from an
external generator, and enforces bounded attempts and freeze
reproducibility.

The Harness validates candidate artifacts; it does NOT guarantee
end‑to‑end generation determinism unless the generator is fully pinned
and reproducible.

The Harness itself MUST remain deterministic with respect to:
  • gate ordering
  • gate behavior
  • error semantics
  • freeze behavior
  • ledger emission


============================================================
2. GATE CLASSES
============================================================

The Harness contains two distinct classes of gates:

2.1 Deterministic Validation Gates (Trusted Core)
  • CONTRACT_VALIDATION
  • BUDGET_DERIVATION
  • VERIFICATION_ATTEMPT (Static Analysis + Tests)
  • FREEZE

These gates MUST be pure functions of their declared inputs.

2.2 External Acquisition Gate (Non‑Deterministic)
  • GENERATION_ATTEMPT

This gate MAY invoke a nondeterministic generator. It is NOT part of the
trusted core and MUST NOT be described as pure or deterministic.


============================================================
3. RUN MODEL
============================================================

A **run** is defined by fixed versions of:
  • contract
  • global ceilings
  • exemption manifest
  • toolchain

A run MAY contain up to N **attempts**, where each attempt consists of:
  1. GENERATION_ATTEMPT → produce candidate artifact
  2. VERIFICATION_ATTEMPT → validate that artifact exactly once

A new run is required if ANY of the following change:
  • contract version
  • global ceilings version
  • exemption manifest version
  • toolchain version

A new candidate artifact DOES NOT create a new run; it creates a new
attempt within the same run.


============================================================
4. INPUTS
============================================================

4.1 Contract  
Validated Minimal V1 Contract Schema.

4.2 Global Ceilings  
Validated ceilings object.

4.3 Exemption Manifest  
Append‑only, deterministic, validated before use.

4.4 Generator  
External system that produces candidate artifacts. May be
non‑deterministic.

4.5 Toolchain Version  
Version identifier for static analysis, test runner, and freeze hashing.


============================================================
5. OUTPUTS
============================================================

5.1 Run Terminal Status
  • RUN_SUCCESS
  • RUN_ABORTED

5.2 Attempt‑Level Results
  • ATTEMPT_PASSED
  • ATTEMPT_FAILED

5.3 Ledger Events
Events MUST follow the event grammar defined in Section 10.


============================================================
6. STATE MACHINE
============================================================

The Harness MUST implement the following fixed state machine:

  CONTRACT_VALIDATION
        ↓
  BUDGET_DERIVATION
        ↓
  ATTEMPT_LOOP:
      GENERATION_ATTEMPT
            ↓
      VERIFICATION_ATTEMPT
            ↓
      if passed → FREEZE
      if failed → next attempt (up to N)
        ↓
  FREEZE
        ↓
  DONE


============================================================
7. GATE DEFINITIONS
============================================================

------------------------------------------------------------
7.1 CONTRACT_VALIDATION (Deterministic)
------------------------------------------------------------

Inputs:
  • contract
  • global ceilings
  • exemption manifest (schema only)

Actions:
  • Validate contract schema
  • Validate cross‑field rules
  • Validate exemption manifest schema
  • Emit CONTRACT_ACCEPTED

Failure:
  • RUN_ABORTED with CONTRACT_E_*


------------------------------------------------------------
7.2 BUDGET_DERIVATION (Deterministic)
------------------------------------------------------------

Inputs:
  • contract
  • global ceilings
  • applicable exemptions

Actions:
  • Invoke Budget Compiler
  • Emit BUDGET_DERIVED
  • Emit EXEMPTION_APPLIED (if applicable)

Failure:
  • RUN_ABORTED with BUDGET_E_*


------------------------------------------------------------
7.3 GENERATION_ATTEMPT (Non‑Deterministic)
------------------------------------------------------------

Inputs:
  • contract
  • local budget
  • generator

Actions:
  • Invoke generator to produce candidate artifact
  • Emit GENERATION_ATTEMPTED

Failure:
  • Emit GENERATION_FAILED
  • Continue to next attempt until N attempts exhausted
  • After N failures → RUN_ABORTED


------------------------------------------------------------
7.4 VERIFICATION_ATTEMPT (Deterministic)
------------------------------------------------------------

Inputs:
  • candidate artifact
  • local budget
  • contract

Actions:
  • Run Static Analysis Engine
  • Emit STATIC_ANALYSIS_PASSED or STATIC_ANALYSIS_FAILED
  • If static analysis passes → run tests
  • Emit TESTS_PASSED or TESTS_FAILED

Rules:
  • EXACTLY ONE verification per candidate artifact
  • NO retries on unchanged inputs
  • If verification fails → ATTEMPT_FAILED

Failure:
  • Return to GENERATION_ATTEMPT for next candidate
  • After N attempts → RUN_ABORTED

Success:
  • ATTEMPT_PASSED → FREEZE


------------------------------------------------------------
7.5 FREEZE (Deterministic)
------------------------------------------------------------

Inputs:
  • candidate artifact
  • contract
  • global ceilings
  • exemption manifest
  • toolchain version

Actions:
  • Compute canonical freeze hash
  • Emit ARTIFACT_FROZEN
  • RUN_SUCCESS

Failure:
  • RUN_ABORTED with FREEZE_E_*


------------------------------------------------------------
7.6 DONE
------------------------------------------------------------

Terminal state. No further actions permitted.


============================================================
8. BOUNDED ATTEMPT RULES
============================================================

8.1 Attempt Limit
A run MAY contain up to N attempts.

8.2 Attempt Definition
An attempt consists of:
  • one GENERATION_ATTEMPT
  • one VERIFICATION_ATTEMPT

8.3 Deterministic Gates Are Not Retried
Deterministic gates MUST NOT retry unchanged inputs.

8.4 Attempt Failure
If VERIFICATION_ATTEMPT fails:
  • control returns to GENERATION_ATTEMPT
  • a new candidate artifact MUST be supplied

8.5 Run Failure
After N failed attempts → RUN_ABORTED.


============================================================
9. FREEZE REPRODUCIBILITY
============================================================

Two levels:

9.1 Artifact‑Verification Reproducibility (Guaranteed in V1)
Given identical:
  • contract
  • global ceilings
  • exemption manifest
  • candidate artifact
  • toolchain version

→ FREEZE MUST produce the same hash OR the same failure.

9.2 End‑to‑End Generation Reproducibility (NOT guaranteed in V1)
Requires:
  • pinned generator
  • pinned prompts
  • reproducible execution behavior


============================================================
10. LEDGER EVENT GRAMMAR
============================================================

Events MUST follow this grammar:

10.1 Contract Phase
  • CONTRACT_ACCEPTED
  • (optional) CONTRACT_REJECTED → RUN_ABORTED

10.2 Budget Phase
  • BUDGET_DERIVED
  • (optional) EXEMPTION_APPLIED
  • (optional) BUDGET_FAILED → RUN_ABORTED

10.3 Attempt Loop
For each attempt i:

  • GENERATION_ATTEMPTED(i)
  • GENERATION_FAILED(i) OR GENERATION_SUCCEEDED(i)

  If generation succeeded:
      • STATIC_ANALYSIS_PASSED(i) OR STATIC_ANALYSIS_FAILED(i)
      • If static analysis passed:
            TESTS_PASSED(i) OR TESTS_FAILED(i)

  If verification failed:
      • ATTEMPT_FAILED(i)

10.4 Freeze Phase
  • ARTIFACT_FROZEN (only if ATTEMPT_PASSED)

10.5 Run Terminal
  • RUN_SUCCESS OR RUN_ABORTED


============================================================
11. ERROR MODEL
============================================================

11.1 Harness Error Codes
  • HARNESS_E_INVALID_STATE
  • HARNESS_E_UNEXPECTED_INPUT
  • HARNESS_E_MAX_ATTEMPTS_EXCEEDED

11.2 Wrapped Subsystem Errors
Subsystem errors MUST be wrapped:

{
  "error_code": "HARNESS_E_GATE_FAILURE",
  "gate": "string",
  "cause": {
      "subsystem_error_code": "string",
      "details": { ... }
  }
}


============================================================
12. CONCURRENCY AND EXECUTION MODEL
============================================================

12.1 Single‑Threaded Execution
All gates MUST execute serially in V1.

12.2 No Parallel Attempts
Only one attempt may be active at a time.

12.3 No Asynchronous Execution
All gate transitions MUST be synchronous.


============================================================
13. VERSIONING
============================================================

harness_spec_version = 1.0.2

Any change to:
  • state machine
  • gate definitions
  • attempt model
  • event grammar
  • error codes

MUST increment the spec version.

END OF DOCUMENT
