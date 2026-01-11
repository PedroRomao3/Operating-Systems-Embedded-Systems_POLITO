# group37


## 🕒 Project 2 — Priority-Based Scheduler for Periodic Tasks in FreeRTOS

> **Goal:** Add first-class periodic tasks (period + deadline) **on top of** the default FreeRTOS scheduler, preserving FreeRTOS’s preemptive, priority-based semantics.



### 🧭 1) Overview

FreeRTOS schedules tasks by priority but does not enforce **periodicity** or **deadlines**. This project introduces a thin **Periodic Task Layer (PTL)** that:

- lets users **declare periodic tasks** with *period* and *deadline*,
- performs **job releases** at the correct times,
- detects **deadline misses** and **period overruns**,
- and leaves **preemption** to FreeRTOS (unchanged).

> The PTL must patch the kernel scheduler with minimal intrusivity to make it easily portable.

### 📚 2) Terminology

- **Period (T):** time between releases.  
- **Deadline (D):** relative deadline from each release. If not specified, **D = T**.  
- **Release time (Rₖ):** k-th activation time; if all tasks start together, **R₀ = t₀** for all.  
- **Finish time (Fₖ):** time when job k completes.  
- **Deadline miss:** `Fₖ > Rₖ + D`.  
- **Overrun (period overrun):** previous job not finished at `Rₖ₊₁` (i.e., exceeds T).

### ⚙️ 3) Functional Requirements

1. **Introduce dedicated periodic tasks API.** To create and handle tasks with `(period, deadline, priority, stack, name, entry)`.  
2. **All tasks start together** (project requirement). *Optional*: allow a phase/offset parameter; if omitted, all start at t₀.  
3. **Priority-based, preemptive scheduling** stays as in FreeRTOS.  
4. **Round-robin** among tasks at **the same priority** (respecting FreeRTOS config).  
5. **Deadline & period checks are mandatory.** On every job completion and at every new release, log and handle violations.  
6. **Policy on overrun when a new release arrives** (pick one globally or per-task):  
   - **SKIP**: skip the new job; let the late one finish.  
   - **KILL**: terminate/suspend the running job immediately and release the new one.  
   - **CATCH_UP**: release now, mark previous job missed, keep nominal cadence.  
7. **Config structure** listing all periodic tasks and a `Configure/Start` entrypoint.  


### 🧠 4) Task Model & Runtime Semantics

Tasks are **functions with infinite loops** (or loops controlled by PTL termination). To minimize errors, the programmer must write only the body of the loop, and FreeRTOS must wrap it into the loop, minimizing the number of function calls.


### ⏱️ 5) Deadline & Period Checks  — with Examples

### Checks
1. **Deadline:** On `JobComplete()`, compare `Fₖ` vs `Rₖ + D`. If `Fₖ > Rₖ + D` → **DEADLINE_MISS**.  
2. **Period:** At `Rₖ₊₁`, if previous job isn’t complete → **OVERRUN**. Apply policy (SKIP/KILL/CATCH_UP), log action.

### Example – Deadline miss
- Task B: `T=20 ms`, `D=15 ms`.  
- k-th job starts at 40 ms, finishes at 56 ms → `Fₖ=56 ms`, `Rₖ + D = 55 ms` → **miss**.

### Example – Overrun with SKIP
- Task A: `T=10 ms`. Job k is still running at 30 ms (= Rₖ₊₁).  
- Policy **SKIP**: do not release job k+1 at 30 ms; continue running job k; next release at 40 ms; log `OVERRUN + SKIP`.

### Example – Overrun with KILL
- Same scenario, **KILL**: at 30 ms the PTL stops job k, releases job k+1 immediately; log `OVERRUN + KILL`.

### Example – Overrun with CATCH_UP
- At 30 ms release job k+1 immediately, mark job k as missed, maintain cadence; log `OVERRUN + CATCH_UP`.

---

### 🧰 6) Configuration Interface — with Examples

All scheduling parameters are provided in a dedicated configuration object you define.

### Must capture
- **Global:** policy (SKIP/KILL/CATCH_UP), tracing enable, max tasks.  
- **Per Task:** `name, entry, arg, stack, priority, T, D(optional)`

> If D not specified → **D = T**.

### Example – Config sketch (pseudocode)
```c
SchedulerConfig cfg = {
  .policy = POLICY_SKIP,
  .trace_enabled = true,
  .max_tasks = 8,
  .tasks = {
    { "A", TaskA, NULL, 512, 3, .period_ms = 10, .deadline_ms = 10 },
    { "B", TaskB, NULL, 512, 2, .period_ms = 20, .deadline_ms = 15 },
    { "C", TaskC, NULL, 512, 2, .period_ms = 50 },
  },
  .num_tasks = 3
};

Init(&cfg);
Start(); // defines t₀; all tasks start together
```
