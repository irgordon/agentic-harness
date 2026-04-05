RUN_MODEL.md
Deterministic Run & Attempt Execution Model (V1)
Spec Version: 1.0.1


============================================================
1. PURPOSE
============================================================

The Run Model defines the complete lifecycle of a Harness execution,
including:

  • run identity and configuration boundaries
  • attempt structure and numbering
  • deterministic vs non-deterministic gate behavior
  • freeze reproducibility guarantees
  • ledger event ordering and terminal conditions
  • integration with agentic generation workflows

This document binds all V1 subsystems into a single normative execution
model.


============================================================
2. RUN CONFIGURATION & IDENTITY
============================================================

A **run** is defined by the following fixed configuration inputs:

  • contract_hash
  • global_ceilings_hash
  • exemption_manifest_hash (applicable subset only)
  • toolchain_hash
  • generator_timeout_ms
  • max_attempts

These MUST remain constant for the entire run.

Run identity MUST include all configuration inputs that affect execution:

run_id = hash(
    contract_hash,
    global_ceilings_hash,
    exemption_manifest_hash,
    toolchain_hash,
    generator_timeout_ms,
    max_attempts
)

If ANY of these change → a new run MUST be created.


============================================================
3. ATTEMPT MODEL
============================================================

A run consists of up to max_attempts **attempts**.

Each attempt i consists of exactly:

  1. GENERATION_ATTEMPT(i)
  2. VERIFICATION_ATTEMPT(i)

Attempt numbering invariants:

  • attempts start at 1
  • attempts begin only after successful BUDGET_DERIVATION
  • attempts increase by exactly 1
  • attempts never skip values
  • attempts never repeat
  • one generator invocation == one attempt


============================================================
4. STATE MACHINE
============================================================

The run MUST execute the following fixed state machine:

  CONTRACT_VALIDATION
        ↓
  BUDGET_DERIVATION
        ↓
  ATTEMPT_LOOP:
      GENERATION_ATTEMPT(i)
            ↓
      VERIFICATION_ATTEMPT(i)
            ↓
      if passed → FREEZE
      if failed → next attempt
        ↓
  FREEZE
        ↓
  DONE


============================================================
5. GATE CLASSES
============================================================

5.1 Deterministic Validation Gates (Trusted Core)

  • CONTRACT_VALIDATION
  • BUDGET_DERIVATION
  • VERIFICATION_ATTEMPT
  • FREEZE

Deterministic gates MUST treat inputs as unchanged if and only if their
canonical hashes are identical.

Deterministic gates MUST NOT retry unchanged inputs.

5.2 External Acquisition Gate (Non-Deterministic)

  • GENERATION_ATTEMPT

This gate MAY produce different outputs for identical inputs.


============================================================
6. CANONICAL REQUEST & RESPONSE FLOW
============================================================

6.1 Request Construction

Before each GENERATION_ATTEMPT(i):

  • Construct canonical GeneratorRequest_without_id
  • Compute request_id = hash(normalized(request_without_id))
  • Send request to generator

6.2 Response Validation

The Harness MUST:

  • canonicalize the response
  • validate request_id match
  • validate status
  • validate null/non-null constraints
  • treat malformed responses as GEN_E_PROTOCOL

6.3 Artifact Normalization

If status = success:

  • normalize candidate_artifact using the canonical normalization spec
  • normalization behavior MUST be included in toolchain_hash
  • compute candidate_artifact_hash
  • proceed to VERIFICATION_ATTEMPT(i)


============================================================
7. VERIFICATION ATTEMPT
============================================================

Each candidate artifact is verified exactly once.

Verification consists of:

  1. Static Analysis Engine
  2. Test execution (based on test_obligation_class)

Rules:

  • deterministic gates MUST NOT retry unchanged inputs
  • failure MUST be classified as:
        - generation_failed
        - static_analysis_failed
        - tests_failed
        - verification_failed
  • failure → ATTEMPT_FAILED(i)
  • success → ATTEMPT_PASSED(i)


============================================================
8. FREEZE PHASE
============================================================

If ATTEMPT_PASSED(i):

  • compute canonical freeze hash
  • emit ARTIFACT_FROZEN
  • emit RUN_SUCCESS (immediately after)

If freeze fails:

  • emit FREEZE_FAILED
  • emit RUN_ABORTED (final event)


============================================================
9. TERMINAL CONDITIONS
============================================================

A run MUST end with exactly one terminal event:

  • RUN_SUCCESS
  OR
  • RUN_ABORTED

RUN_ABORTED MUST be the final event in failure cases.

ARTIFACT_FROZEN MUST be immediately followed by RUN_SUCCESS.


============================================================
10. LEDGER INTEGRATION
============================================================

The run MUST emit events according to the Ledger grammar.

Key invariants:

  • deterministic ordering
  • deterministic serialization
  • EXEMPTION_APPLIED events MUST match the order used in budget derivation
  • attempt events MUST reflect monotonic attempt numbering
  • terminal event MUST be unique and final


============================================================
11. AGENTIC WORKFLOW INTEGRATION
============================================================

11.1 System Enforcement Boundary

The Harness enforces constraints regardless of agent behavior.

The agent cannot bypass deterministic gates.

11.2 Agent Responsibilities

The agent:

  • reads the contract
  • plans the implementation
  • generates candidate artifacts
  • responds to Harness feedback
  • iterates until success or attempt limit

11.3 Harness Responsibilities

The Harness:

  • enforces deterministic constraints
  • rejects invalid artifacts
  • provides structured feedback via ledger events
  • ensures reproducibility and auditability

11.4 Feedback Loop

The agent receives:

  • GENERATION_FAILED → adjust generation strategy
  • STATIC_ANALYSIS_FAILED → simplify structure
  • TESTS_FAILED → fix logic
  • ATTEMPT_FAILED → produce new artifact
  • RUN_ABORTED → revise contract or manifest
  • RUN_SUCCESS → finalize artifact

This forms a deterministic outer loop around a non-deterministic inner loop.


============================================================
12. REPOSITORY STRUCTURE
============================================================

/
├── contract/
│   ├── contract.json
│   ├── schema/ (versioned; participates in toolchain_hash)
│   └── validator/
│
├── ceilings/
│   ├── global_ceilings.json
│   └── schema/ (versioned; participates in toolchain_hash)
│
├── exemptions/
│   ├── manifest.json
│   ├── schema/ (versioned; participates in toolchain_hash)
│   └── validator/
│
├── budget_compiler/
│   ├── compiler.c
│   ├── rule_table.c
│   ├── schema/ (versioned)
│   └── tests/
│
├── static_analysis/
│   ├── engine.c
│   ├── normalization/ (versioned; participates in toolchain_hash)
│   ├── metrics/
│   └── tests/
│
├── harness/
│   ├── harness.c
│   ├── state_machine.c
│   ├── errors.c
│   └── tests/
│
├── generator_interface/
│   ├── request_builder.c
│   ├── response_validator.c
│   └── tests/
│
├── ledger/
│   ├── writer.c
│   ├── schema/ (versioned)
│   └── tests/
│
├── agent/
│   ├── planner/
│   ├── generator/
│   ├── adapters/
│   └── tests/
│
└── docs/
    ├── ARCHITECTURE_DOCUMENT.md
    ├── RUN_MODEL.md
    ├── HARNESS.md
    ├── LEDGER.md
    ├── EXEMPTION_MANIFEST.md
    ├── STATIC_ANALYSIS_ENGINE.md
    └── GENERATOR_INTERFACE.md


============================================================
13. VERSIONING
============================================================

run_model_spec_version = 1.0.1

Any change to:
  • run identity rules
  • attempt model
  • state machine
  • normalization rules
  • agent integration semantics
  • configuration boundaries

MUST increment the spec version.

END OF DOCUMENT
