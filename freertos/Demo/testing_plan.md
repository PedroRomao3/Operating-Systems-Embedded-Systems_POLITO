# Master Test Plan: PTL + Policies + Advanced Stress

## 1. Basic Scheduling Tests
*Verifies standard FreeRTOS fixed-priority behavior.*

| Test | Tasks | Deadline | Period | Priority | Duration | Expected Behavior |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **1** | **Periodic A** | 10ms | 10ms | 1 | <10ms | Task always meets deadline. |
| **2** | **Periodic A**<br>**Periodic B** | 10ms<br>10ms | 10ms<br>10ms | 1<br>1 | <5ms<br><5ms | Both meet deadlines. Equal priority = Round-Robin/FIFO order. |
| **3** | **Periodic A**<br>**Periodic B** | 10ms<br>10ms | 10ms<br>10ms | **3**<br>1 | <5ms<br><5ms | **Preemption Check:** Task A (Prio 3) must always finish before Task B (Prio 1) starts. |
| **4** | **Periodic A**<br>**Periodic B** | 10ms<br>15ms | 10ms<br>15ms | **3**<br>1 | <5ms<br><5ms | **Rate Monotonic Check:** Task A (shorter period) is assigned higher priority. Task A preempts Task B. |

## 2. Failure Detection (Deadline vs. Overrun)
*Verifies the system distinguishes between being "Late" and "Too Late".*

| Test | Tasks | Deadline | Period | Priority | Duration | Expected Behavior |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **5** | **Task A** | **10ms** | **20ms** | 1 | **15ms** | **Pure Deadline Miss:**<br>Job finishes at t=15ms.<br>**Log:** "DEADLINE MISS".<br>**Policy:** NO Action taken (Job 2 starts normally at t=20ms).<br>*(Verifies logic: `now > D` but `now < T`)*. |
| **6** | **Task A**<br>*(Config D=0)* | **(Auto)** | 10ms | 1 | 9ms | **Default Config Check:**<br>Deadline configured as `0`. System sets `D = T` (10ms).<br>Job takes 9ms -> **Success** (No logs).<br>Job takes 11ms -> **Overrun** (Log). |

## 3. Overrun Policy Tests
*Verifies the PTL correctly handles cases where Duration > Period.*

| Test | Tasks | Deadline | Period | Priority | Duration | Expected Behavior |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **7** | **Task A**<br>*(Policy: SKIP)* | 10ms | 10ms | 1 | **15ms** | **Job 1** finishes late (t=15).<br>**Job 2** is **SKIPPED**.<br>**Job 3** releases correctly at t=20ms. |
| **8** | **Task B**<br>*(Policy: KILL)* | 10ms | 10ms | 1 | **15ms** | **Job 1** is killed/suspended at t=10ms.<br>**Job 2** starts **immediately** at t=10ms.<br>Job 1 never completes. |
| **9** | **Task C**<br>*(Policy: CATCH_UP)* | 10ms | 10ms | 1 | **12ms** | **Job 1** finishes late (t=12).<br>**Job 2** starts **immediately** at t=12 (processing backlog).<br>System eventually catches up. |

## 4. Mixed Workload Tests
*Verifies interaction between Periodic PTL tasks and standard FreeRTOS tasks.*

| Test | Tasks | Deadline | Period | Priority | Duration | Expected Behavior |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **10** | **Periodic A**<br>**Background B** | 10ms<br>N/A | 10ms<br>N/A | **2**<br>1 | <5ms<br>Inf | **Standard Preemption:** Periodic A interrupts Background B every 10ms. B runs in the gaps. |
| **11** | **Periodic A**<br>**Background B** | 10ms<br>N/A | 10ms<br>N/A | **2**<br>1 | **11ms** | **Starvation (Periodic > NP):** Task A hogs 100% CPU. Task B **never** runs. |

## 5. Stress & Stability Tests
*Verifies system overhead and long-term timing stability.*

| Test | Tasks | Deadline | Period | Priority | Duration | Expected Behavior |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **12** | **Periodic A** | 5ms | 5ms | 1 | **4.95ms** | **Context Switch Overhead:** Task meets deadline. Tests if PTL overhead fits in the remaining 0.05ms. |
| **13** | **Periodic A** | 10ms | 10ms | 1 | 1ms | **Drift Check (1 Hour):** Run for 360,000 iterations. Expected release of last job: **t=3600.00s**. |
| **14** | **Periodic A** | 10ms | 10ms | 1 | Varied | **Domino Recovery:** Inject overload (Duration > 10ms) for cycles 1-5. Return to normal at cycle 6. System must stop reporting errors and resume normal operation. |

## 6. Priority Interactions (NP vs Periodic)
*Verifies that Priority Numbers override Task Types.*

| Test | Tasks | Deadline | Period | Priority | Duration | Expected Behavior |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **15** | **Periodic A**<br>**Background B** | 10ms<br>N/A | 10ms<br>N/A | 1<br>**2** | <5ms<br>Inf | **Inversion (NP > Periodic):** Task B (Prio 2) hogs 100% CPU.<br>Task A (Prio 1) **never** runs (misses every deadline), despite being "Periodic". |
| **16** | **Periodic A**<br>**Background B** | 10ms<br>N/A | 10ms<br>N/A | **1**<br>**1** | <5ms<br>Inf | **Equal Priority Slicing:** Task A becomes READY every 10ms.<br>Since Prio A == Prio B, Task A should eventually get a slice and run.<br>Task B runs in the remaining time. |

## 7. Advanced Stress & Complexity
*Verifies robustness under complex conditions.*

| Test | Tasks | Deadline | Period | Priority | Duration | Expected Behavior |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **17** | **Task A** (7ms)<br>**Task B** (13ms)<br>**Task C** (17ms) | N/A | 7, 13, 17 | 3, 2, 1 | 1ms | **Prime Number Stress:** Periods are prime numbers. They will rarely align (hyperperiod = 1547ms).<br>Verify no drift or crashes over 10 minutes. |
| **18** | **Periodic A**<br>**Mutex Holder** | 10ms<br>N/A | 10ms<br>N/A | **5**<br>1 | 1ms<br>Inf | **Resource Contention:** Task A needs a Mutex held by Task B.<br>Task A must **Wait** (consuming Deadline time).<br>Task A finishes only after B releases mutex. |
| **19** | **Task A**<br>*(Bursty)* | 10ms | 10ms | 1 | 1ms..15ms | **Sporadic Overload:** Task usually takes 1ms, but every 10th job takes 15ms.<br>Verify system logs a miss *only* for the 10th job and recovers immediately for the 11th. |
| **20** | **Task A**<br>**Task B** | 10ms<br>10ms | 10ms<br>10ms | 2<br>1 | **5ms**<br>**5ms** | **Utilization Wall (100% Load):** Total execution time = 10ms. Period = 10ms.<br>Theoretical limit. With context switch overhead, Task B *should* barely miss or barely make it. |