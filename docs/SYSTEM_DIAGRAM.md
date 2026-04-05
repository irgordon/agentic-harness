# Agentic Generation Harness — Diagram Overview (Final V1)

A visual, intuitive map of the entire system — now aligned with the final `RUN_MODEL`, normalization spec, and trust boundaries.

---

## 1. TOP-LEVEL SYSTEM FLOW

```mermaid
flowchart TD
    Human["HUMAN / SYSTEM\nProvides Contract Input"] --> Contract["CONTRACT LAYER\nContract + Ceilings +\nApplicable Exemptions"]
    Contract --> Budget["BUDGET COMPILER\nDeterministic structural\nlimits (local_budget)"]
    Budget --> Harness["HARNESS\nDeterministic Outer Loop"]
    
    subgraph ATTEMPT_LOOP ["ATTEMPT LOOP"]
        direction TB
        Gen["GENERATION_ATTEMPT (i)\nNon-deterministic acquisition\n(Agent / LLM / Multi-Agent System)"]
        Ver["VERIFICATION_ATTEMPT (i)\nDeterministic validation:\n• Structural validation (budget)\n• Static analysis\n• Tests"]
        
        Gen --> Ver
        Ver -- "failure → next attempt" --> Gen
    end
    
    Harness --> ATTEMPT_LOOP
    
    Ver -- "success" --> Freeze["FREEZE\n• Canonical normalization\n• Canonical hashing\n• Final integrity validation\n• Immutable final artifact"]
    Ver -- "if i == max_attempts" --> Abort["RUN_ABORTED"]
    
    Freeze --> Done["DONE\nRUN_SUCCESS or RUN_ABORTED"]
    Abort --> Done
```

---

## 2. SUBSYSTEM RELATIONSHIPS

```mermaid
flowchart TD
    Contract["CONTRACT SURFACE\n- Defines intent and required artifact shape\n- Declares structural expectations"] --> Ceilings["GLOBAL CEILINGS\n- System-wide structural limits"]
    Ceilings --> Exemptions["EXEMPTION MANIFEST\n- Narrow, controlled overrides\n- Only size-oriented, never topology"]
    Exemptions --> Budget["BUDGET COMPILER\n- Deterministic derivation\n- Produces local_budget"]
```

---

## 3. HARNESS: THE DETERMINISTIC OUTER LOOP

```mermaid
flowchart TD
    Harness["HARNESS\nDeterministic state machine"] --> ATTEMPT_LOOP
    
    subgraph ATTEMPT_LOOP ["ATTEMPT LOOP"]
        direction TB
        Gen["GENERATION_ATTEMPT\n(non-deterministic)"] --> Ver["VERIFICATION_ATTEMPT\n(deterministic)"]
        Ver -- "failure → next attempt" --> Gen
    end
    
    Ver -- "success" --> Freeze["FREEZE"]
    Ver -- "i == max_attempts" --> Abort["RUN_ABORTED"]
```

---

## 4. AGENT: THE NON-DETERMINISTIC INNER LOOP

The agent (LLM or multi-agent system):

```mermaid
flowchart TD
    subgraph Agent ["Agent Execution Context"]
        direction TB
        Read["Reads contract + budget"] --> Plan["Plans implementation"]
        Plan --> Generate["Generates candidate artifact"]
        Generate --> Feedback["Reads Harness feedback"]
        Feedback --> Adjust["Adjusts strategy"]
        Adjust -- "Repeats until success\nor max_attempts" --> Read
    end
```

The agent can be creative.
The Harness cannot.

---

## 5. LEDGER: PASSIVE, APPEND-ONLY AUDIT TRAIL

The Harness emits events.
The Ledger stores them.
The Ledger never drives control flow.

**Recorded events include:**
* `CONTRACT_ACCEPTED`
* `BUDGET_DERIVED`
* `EXEMPTION_APPLIED`
* `GENERATION_ATTEMPTED(i)`
* `GENERATION_FAILED(i)`
* `GENERATION_SUCCEEDED(i)`
* `STATIC_ANALYSIS_PASSED(i)`
* `STATIC_ANALYSIS_FAILED(i)`
* `TESTS_PASSED(i)`
* `TESTS_FAILED(i)`
* `ATTEMPT_FAILED(i)`
* `ATTEMPT_PASSED(i)`
* `FREEZE_FAILED`
* `ARTIFACT_FROZEN`
* `RUN_SUCCESS` or `RUN_ABORTED`

---

## 6. FREEZE: THE FINAL TRUST BOUNDARY

When an attempt passes verification:

```mermaid
flowchart TD
    Freeze["FREEZE\n- Canonical normalization\n- Canonical hashing\n- Final integrity validation\n- Immutable final artifact"]
```

Freeze is not formatting.
Freeze is the final integrity checkpoint.

---

## 7. COMPLETE SYSTEM IN ONE DIAGRAM

```mermaid
flowchart TD
    Input["HUMAN / SYSTEM\nProvides Contract"] --> Contract["CONTRACT + CEILINGS + EXEMPTIONS"]
    Contract --> Budget["BUDGET COMPILER → local_budget"]
    Budget --> Harness["HARNESS (deterministic)"]
    
    subgraph ATTEMPT_LOOP ["ATTEMPT LOOP"]
        direction TB
        Gen["GENERATION_ATTEMPT (agent)"] --> Ver["VERIFICATION_ATTEMPT\n(budget + static analysis + tests)"]
        Ver -- "failure" --> Gen
    end
    
    Harness --> ATTEMPT_LOOP
    
    Ver -- "success" --> Freeze["FREEZE → canonical artifact"]
    Ver -- "if i == max_attempts" --> Abort["RUN_ABORTED"]
    
    Freeze --> Success["RUN_SUCCESS"]
```

---

## 8. WHAT THIS SYSTEM GUARANTEES

* ✔ Deterministic validation
* ✔ Bounded complexity
* ✔ Reproducible artifacts
* ✔ Full audit trail
* ✔ Explicit trust boundaries
* ✔ A safe outer loop around creative AI systems

This is how agentic coding becomes **reliable engineering**.
