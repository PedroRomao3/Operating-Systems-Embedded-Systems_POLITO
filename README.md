# FreeRTOS Periodic Task Layer: Deterministic Real-Time Scheduling
[![C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![RTOS](https://img.shields.io/badge/OS-FreeRTOS-green.svg)](https://www.freertos.org/)
[![Architecture](https://img.shields.io/badge/Arch-ARM_Cortex--M3-orange.svg)](https://developer.arm.com/ip-products/processors/cortex-m)

## Project Overview
This repository contains a custom **Periodic Task Layer (PTL)** engineered on top of the standard FreeRTOS kernel.

**Note on Contributions:** The repository includes the standard FreeRTOS source tree. Contributions are concentrated entirely in the custom PTL scheduling logic, driver implementation, and the automated testing frameworks designed to verify system timing.

* **Bare-Metal C & ARM Cortex-M:** The implementation is written in strict, bare-metal C, designed to run efficiently on ARM Cortex-M microcontrollers. It interacts directly with hardware timers and RTOS internals.
* **Deterministic Timing for Sensors:** In automotive networks, polling a sensor or writing to a communication bus (like CAN) must happen exactly on schedule. This PTL allows developers to define exact Release Times (R) and Deadlines (D), ensuring data is processed within strict timing windows.
* **Fault Detection & Handling:** A core component of functional safety is detecting when a system fails to meet its timing constraints. The PTL automatically detects **Deadline Misses** and **Period Overruns**, logging the violations and executing predefined recovery policies.

## Key Features Developed

1. **First-Class Periodic Tasks:** Introduced a dedicated API to instantiate tasks with explicit periods and relative deadlines. The PTL handles task activation and yielding, reducing developer boilerplate and minimizing human error.
2. **Strict Timing Verification:**
   * Automatically compares job finish time against absolute deadlines.
   * Monitors for period overruns (when a new release time arrives but the previous job is still executing).
3. **Pluggable Overrun Policies:** Implemented configurable recovery mechanisms to handle timing violations deterministically:
   * `SKIP`: Drops the new job, allowing the late job to finish. Prioritizes system stability.
   * `KILL`: Terminates the late job and immediately releases the new one. Prioritizes strict cadence.
   * `CATCH_UP`: Releases immediately, marks the previous as missed, and attempts to restore the original cadence.

## Development & Testing Methodology
To ensure the reliability of the scheduling extension, I placed a heavy emphasis on software testing and documentation:

* **Test Requirements Definition:** Defined specific edge-case scenarios (e.g., task preemption during an overrun, cascading deadline misses) to prove the scheduler behaves predictably under maximum load.
* **Automated Testing:** Utilized scripting (Python/Bash) and testing tools to automate the execution of the RTOS. 
* **State Verification:** Built an automated verification pipeline that captures the RTOS serial output and compares task execution traces against expected "golden" files to prevent regressions.

## Configuration Interface
The system relies on a centralized, statically defined configuration structure, making it easy to review software architecture and timing requirements before compilation.

```c
// Example System Configuration
SchedulerConfig cfg = {
  .policy = POLICY_SKIP,
  .trace_enabled = true,
  .max_tasks = 8,
  .tasks = {
    // Name, Entry Func, Args, Stack, Priority, Period (T), Deadline (D)
    { "Sensor_Read", TaskA, NULL, 512, 3, .period_ms = 10, .deadline_ms = 10 },
    { "Data_Filter", TaskB, NULL, 512, 2, .period_ms = 20, .deadline_ms = 15 },
    { "Diagnostics", TaskC, NULL, 512, 1, .period_ms = 50 }, // D defaults to T
  },
  .num_tasks = 3
};

Init(&cfg);
Start(); // Defines t0; all tasks are released synchronously.
