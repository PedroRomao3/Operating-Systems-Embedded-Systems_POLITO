#include "test.h"
#include "uart.h"
#include "setup.h"
#include <string.h>

extern int RUNTIME_TEST_CASE;

// static void vBurnCPU(TickType_t ticks_to_wait)
// {
//     TickType_t start = xTaskGetTickCount();
//     while ((xTaskGetTickCount() - start) < ticks_to_wait)
//     {
//         __asm("nop");
//     }
// }

static void vBurnCPU(TickType_t target_budget_ticks)
{
    TaskHandle_t xHandle = xTaskGetCurrentTaskHandle();

    // ulTaskGetRunTimeCounter returns the exact amount of time THIS
    // task has been in the Running state since it was created.
    configRUN_TIME_COUNTER_TYPE start_time = ulTaskGetRunTimeCounter(xHandle);

    // Loop until the task's personal active execution time reaches the budget.
    // If preempted, ulTaskGetRunTimeCounter() pauses for this task.
    while ((ulTaskGetRunTimeCounter(xHandle) - start_time) < target_budget_ticks)
    {
        // Keep the CPU busy. volatile prevents the compiler from removing the loop.
        __asm volatile("nop");
    }
}

static void TaskGeneric(void *arg)
{
    TickType_t duration = (TickType_t)arg;
    vBurnCPU(duration);
}

static void TaskBursty(void *arg)
{
    static int cycle_count = 0;
    cycle_count++;

    if ((cycle_count % 10) == 0)
    {
        vBurnCPU(pdMS_TO_TICKS(15));
    }
    else
    {
        vBurnCPU(pdMS_TO_TICKS(1));
    }
}

static void TaskInfinite(void *arg)
{
    char *name = (char *)arg;
    for (;;)
    {
        SdkLog("[TRACE] %d:%s:RUNNING\n", xTaskGetTickCount(), name);

        vBurnCPU(pdMS_TO_TICKS(1));
    }
}

bool InitTesting(SchedulerConfig_t *testConfiguration, List_t *periodicTaskConfigList, List_t *nonPeriodicTaskConfigList)
{
    if (testConfiguration == NULL)
    {
        LogAlways("InitTesting: testConfiguration is NULL\n");
        return false;
    }

    if (testConfiguration->num_tasks > 0 && testConfiguration->tasks == NULL)
    {
        LogAlways("InitTesting: num_tasks > 0 but tasks array is NULL\n");
        return false;
    }

    if (testConfiguration->num_tasks > testConfiguration->max_tasks)
    {
        LogAlways("InitTesting: num_tasks (%u) exceeds max_tasks (%u)\n",
                  testConfiguration->num_tasks, testConfiguration->max_tasks);
        return false;
    }

    for (uint32_t i = 0; i < testConfiguration->num_tasks; i++)
    {
        TaskConfiguration_t currentTask = testConfiguration->tasks[i];

        if (currentTask.func == NULL)
        {
            LogAlways("InitTesting: Task %u (%s) has NULL function\n", i, currentTask.name);
            return false;
        }

        if (currentTask.stack_size == 0)
            currentTask.stack_size = 512;

        overrun_policy_t policyToUse = testConfiguration->policy;
        if (currentTask.policy_override != POLICY_NONE)
            policyToUse = currentTask.policy_override;

        vCreateAndAddTask(
            currentTask.name,
            currentTask.func,
            currentTask.arg,
            currentTask.priority,
            currentTask.stack_size,
            ((currentTask.deadline == 0) && (currentTask.period == 0)) ? false : true,
            currentTask.trace_enabled,
            pdMS_TO_TICKS(currentTask.period),
            pdMS_TO_TICKS(currentTask.deadline),
            policyToUse,
            periodicTaskConfigList, nonPeriodicTaskConfigList);
    }

    return true;
}

void vCreateTestTasks(List_t *PeriodicList, List_t *NonPeriodicList)
{
    if (RUNTIME_TEST_CASE == 1)
    {
        UART_printf("\n[TEST 1] Single Periodic Task\n");

        TaskConfiguration_t tasks[] = {
            {"TaskA", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 10, 10, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 1,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }

    if (RUNTIME_TEST_CASE == 2)
    {
        UART_printf("\n[TEST 2] Round Robin (Same Priority)\n");

        TaskConfiguration_t tasks[] = {
            {"TaskA", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 10, 10, TRACE_ENABLED, POLICY_SKIP},
            {"TaskB", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 10, 10, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 2,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }

    if (RUNTIME_TEST_CASE == 3)
    {
        UART_printf("\n[TEST 3] Preemption\n");

        TaskConfiguration_t tasks[] = {
            {"TaskA", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 3, 10, 10, TRACE_ENABLED, POLICY_SKIP},
            {"TaskB", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 10, 10, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 2,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }

    if (RUNTIME_TEST_CASE == 4)
    {
        UART_printf("\n[TEST 4] Rate Monotonic\n");

        TaskConfiguration_t tasks[] = {
            {"TaskA", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 3, 10, 10, TRACE_ENABLED, POLICY_SKIP},
            {"TaskB", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 15, 15, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 2,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }

    if (RUNTIME_TEST_CASE == 5)
    {
        UART_printf("\n[TEST 5] Deadline Miss (Not Overrun)\n");

        TaskConfiguration_t tasks[] = {
            {"TaskMiss", TaskGeneric, (void *)pdMS_TO_TICKS(15), 512, 1, 20, 10, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 1,
            .max_tasks = 5};
        InitTesting(&config, PeriodicList, NonPeriodicList);
    }

    if (RUNTIME_TEST_CASE == 6)
    {
        UART_printf("\n[TEST 6] Default Config (D=0 -> D=T)\n");

        /* Scenario:
         * Period = 10ms. Deadline explicitly set to 0.
         * Work = 9ms.
         * Expectation: System treats D=T (10ms).
         * Result: 9ms < 10ms -> Success (End of Job).
         */
        TaskConfiguration_t tasks[] = {
            {"TaskDefault", TaskGeneric, (void *)pdMS_TO_TICKS(9), 512, 1, 10, 0, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 1,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }

    if (RUNTIME_TEST_CASE == 7)
    {
        UART_printf("\n[TEST 7] Overrun Policy Skip\n");

        TaskConfiguration_t tasks[] = {
            {"TaskMiss", TaskGeneric, (void *)pdMS_TO_TICKS(150),
             512, 1, pdMS_TO_TICKS(100), pdMS_TO_TICKS(100), TRACE_ENABLED, POLICY_NONE}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 1,
            .max_tasks = 5};
        InitTesting(&config, PeriodicList, NonPeriodicList);
    }

    if (RUNTIME_TEST_CASE == 8)
    {
        UART_printf("\n[TEST 8] Overrun Policy KILL\n");

        /* Scenario:
         * Work (150) > Period (100). Policy = KILL.
         * Expectation: At T=100, Task is Killed. New Task Starts immediately.
         * Log should show: START ... (No END) ... START
         */
        TaskConfiguration_t tasks[] = {
            {"TaskKill", TaskGeneric, (void *)pdMS_TO_TICKS(150),
             512, 1, pdMS_TO_TICKS(100), pdMS_TO_TICKS(100), TRACE_ENABLED, POLICY_KILL}
            // Explicitly set to KILL to match the test intent
        };

        SchedulerConfig_t config = {
            .policy = POLICY_KILL,
            .tasks = tasks,
            .num_tasks = 1,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }

    if (RUNTIME_TEST_CASE == 9)
    {
        UART_printf("\n[TEST 9] Overrun Policy CATCH_UP\n");

        /* Scenario:
         * Work (150) > Period (100). Policy = CATCH_UP.
         * Expectation:
         * T=0: Start Job 1.
         * T=100: Overrun detected. Next Release scheduled (Semaphore Given).
         * T=150: Job 1 Finishes. Job 2 Starts IMMEDIATELY (Catching up).
         */
        TaskConfiguration_t tasks[] = {
            {"TaskCatch", TaskGeneric, (void *)pdMS_TO_TICKS(150),
             512, 1, pdMS_TO_TICKS(100), pdMS_TO_TICKS(100), TRACE_ENABLED, POLICY_CATCH_UP}};

        SchedulerConfig_t config = {
            .policy = POLICY_CATCH_UP,
            .tasks = tasks,
            .num_tasks = 1,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 10)
    {
        UART_printf("\n[TEST 10] Periodic vs Background (Preemption)\n");

        /* Scenario:
         * Task A is Periodic (Prio 2).
         * Task B is Background (Prio 1).
         * Note: Passing 0 for Period and Deadline tells InitTesting it is Non-Periodic.
         */
        TaskConfiguration_t tasks[] = {
            {"PeriodicA", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 2, 10, 10, TRACE_ENABLED, POLICY_SKIP},
            {"BackgroundB", TaskInfinite, "BackgroundB", 512, 1, 0, 0, TRACE_ENABLED, POLICY_NONE}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 2,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 11)
    {
        UART_printf("\n[TEST 11] Starvation (Periodic > NP)\n");

        /* Scenario:
         * Task A is Periodic (Prio 2) but requires 11ms of work every 10ms.
         * Task B is Background (Prio 1).
         * IMPORTANT: with the skip policy there is no starvation, makes sense
         */
        TaskConfiguration_t tasks[] = {
            {"PeriodicA", TaskGeneric, (void *)pdMS_TO_TICKS(11), 512, 2, 10, 10, TRACE_ENABLED, POLICY_CATCH_UP},
            {"BackgroundB", TaskInfinite, "BackgroundB", 512, 1, 0, 0, TRACE_ENABLED, POLICY_NONE}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 2,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 12)
    {
        UART_printf("\n[TEST 12] Context Switch Overhead\n");

        /* Scenario:
         * Period = 5ms. Work = 4ms. Free time = 1ms.
         * Expectation: Task should consistently meet its deadline.
         */
        TaskConfiguration_t tasks[] = {
            {"TightTask", TaskGeneric, (void *)pdMS_TO_TICKS(4), 512, 1, 5, 5, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 1,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 19)
    {
        UART_printf("\n[TEST 19] Sporadic Overload (Catch Up)\n");

        /* Scenario:
         * Period: 10ms.
         * Normal Load: 1ms.
         * Burst Load: 15ms (Every 10th cycle).
         * Policy: CATCH_UP.
         *
         * PREDICTION:
         * Cycles 1-9: Normal (Start T=0, 10, 20...).
         * Cycle 10:   Start T=90. Ends T=105. (OVERRUN at T=100).
         * Cycle 11:   Start T=105 (Immediately!). Ends T=106. (Late start, but fast finish).
         * Cycle 12:   Start T=110. (Back on Schedule!).
         */
        TaskConfiguration_t tasks[] = {
            {"TaskBursty", TaskBursty, NULL, 512, 1, 10, 10, TRACE_ENABLED, POLICY_CATCH_UP}};

        SchedulerConfig_t config = {
            .policy = POLICY_CATCH_UP,
            .tasks = tasks,
            .num_tasks = 1,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }

    if (RUNTIME_TEST_CASE == 21)
    {
        UART_printf("\n[TEST 21] Stress Test (U = 85.1%%)\n");
        UART_printf("Tasks: Emergency(Highest), T1, T2(Early Deadline), T3(Lowest)\n");

        TaskConfiguration_t tasks[] = {
            /* 1. Emergency (Worst-Case Periodic)
             * T = 50ms (Min Inter-arrival time)
             * C = 5ms
             * Prio = 4 (Highest, because Shortest Period)
             */
            {"Emergency", TaskGeneric, (void *)pdMS_TO_TICKS(5), 512, 4, 50, 50, TRACE_ENABLED, POLICY_SKIP},

            /* 2. Tau 1
             * T = 100ms
             * C = 20ms
             * Prio = 3
             */
            {"Tau1", TaskGeneric, (void *)pdMS_TO_TICKS(20), 512, 3, 100, 100, TRACE_ENABLED, POLICY_SKIP},

            /* 3. Tau 2 (Strict Deadline)
             * T = 150ms
             * C = 40ms
             * D = 130ms (Must finish 20ms before period ends)
             * Prio = 2
             */
            {"Tau2", TaskGeneric, (void *)pdMS_TO_TICKS(40), 512, 2, 150, 130, TRACE_ENABLED, POLICY_SKIP},

            /* 4. Tau 3 (The Heavy Load)
             * T = 350ms
             * C = 100ms
             * Prio = 1 (Lowest)
             */
            {"Tau3", TaskGeneric, (void *)pdMS_TO_TICKS(100), 512, 1, 350, 350, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 4,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 22)
    {
        UART_printf("\n[TEST 22] Harmonic Razor's Edge (U = 97.5%%)\n");
        /* Scenario:
         * T1: T=10, C=5  -> 50.0% Load
         * T2: T=20, C=5  -> 25.0% Load
         * T3: T=40, C=9  -> 22.5% Load
         * Total Utilization = 97.5%.
         * * Expectation: Because the periods are harmonic (10, 20, 40), they align
         * perfectly. This SHOULD pass without a single miss.
         * If it throws an OVERRUN or MISS, OS overhead is taking >2.5%
         * of the CPU time (which is a great metric to know).
         */
        TaskConfiguration_t tasks[] = {
            {"Harmonic_High", TaskGeneric, (void *)pdMS_TO_TICKS(5), 512, 3, 10, 10, TRACE_ENABLED, POLICY_SKIP},
            {"Harmonic_Med", TaskGeneric, (void *)pdMS_TO_TICKS(5), 512, 2, 20, 20, TRACE_ENABLED, POLICY_SKIP},
            {"Harmonic_Low", TaskGeneric, (void *)pdMS_TO_TICKS(9), 512, 1, 40, 40, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 3,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 23)
    {
        UART_printf("\n[TEST 23] Guaranteed Overload (U = 107.5%%)\n");
        /* Scenario:
         * T1: T=10, C=6  -> 60.0% Load
         * T2: T=20, C=6  -> 30.0% Load
         * T3: T=40, C=7  -> 17.5% Load
         * Total Utilization = 107.5%.
         *
         * Expectation: The CPU physically cannot keep up.
         * T1 and T2 will preempt T3 constantly.
         * T3 will Starve/Miss its deadline guaranteed.
         * The Manager MUST log [TRACE]:Overload_Low:MISS.
         */
        TaskConfiguration_t tasks[] = {
            {"Overload_High", TaskGeneric, (void *)pdMS_TO_TICKS(6), 512, 3, 10, 10, TRACE_ENABLED, POLICY_SKIP},
            {"Overload_Med", TaskGeneric, (void *)pdMS_TO_TICKS(6), 512, 2, 20, 20, TRACE_ENABLED, POLICY_SKIP},
            {"Overload_Low", TaskGeneric, (void *)pdMS_TO_TICKS(7), 512, 1, 40, 40, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 3,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 24)
    {
        UART_printf("\n[TEST 24] Wiki Ex 1: Liu & Layland Safe (U = 72.5%%)\n");
        // Utilization = 72.5% | Guaranteed by Liu & Layland
        TaskConfiguration_t tasks[] = {
            // Priorities strictly follow RM: Shortest Period = Highest Priority
            {"P2", TaskGeneric, (void *)pdMS_TO_TICKS(20), 512, 3, 50, 50, TRACE_ENABLED, POLICY_SKIP},
            {"P1", TaskGeneric, (void *)pdMS_TO_TICKS(10), 512, 2, 80, 80, TRACE_ENABLED, POLICY_SKIP},
            {"P3", TaskGeneric, (void *)pdMS_TO_TICKS(20), 512, 1, 100, 100, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 3,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 25)
    {
        UART_printf("\n[TEST 25] Wiki Ex 2: Hyperbolic Bound Safe (U = 78.7%%)\n");
        // Utilization = 78.75% | Guaranteed by Hyperbolic Bound
        TaskConfiguration_t tasks[] = {
            {"P2", TaskGeneric, (void *)pdMS_TO_TICKS(20), 512, 3, 50, 50, TRACE_ENABLED, POLICY_SKIP},
            {"P3", TaskGeneric, (void *)pdMS_TO_TICKS(20), 512, 2, 100, 100, TRACE_ENABLED, POLICY_SKIP},
            {"P1", TaskGeneric, (void *)pdMS_TO_TICKS(30), 512, 1, 160, 160, TRACE_ENABLED, POLICY_SKIP}};
        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 3,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 26)
    {
        UART_printf("\n[TEST 26] Wiki Ex 3: Harmonic Subset Safe (U = 81.8%%)\n");
        // Utilization = 81.87% | Guaranteed by Harmonic Subsets
        TaskConfiguration_t tasks[] = {
            {"P2", TaskGeneric, (void *)pdMS_TO_TICKS(20), 512, 3, 50, 50, TRACE_ENABLED, POLICY_SKIP},
            {"P3", TaskGeneric, (void *)pdMS_TO_TICKS(20), 512, 2, 100, 100, TRACE_ENABLED, POLICY_SKIP},
            {"P1", TaskGeneric, (void *)pdMS_TO_TICKS(70), 512, 1, 320, 320, TRACE_ENABLED, POLICY_SKIP}};

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 3,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 27)
    {
        UART_printf("\n[TEST 27] Non-Harmonic Overrun (U = 92.8%%)\n");
        /* Scenario:
         * T1: T=50,  C=20 -> 40.0% Load
         * T2: T=70,  C=30 -> 42.8% Load
         * T3: T=100, C=10 -> 10.0% Load
         * Total Utilization = 92.8%. (Well under 100%)
         *
         * Expectation: 
         * At T=0, all tasks release. 
         * T1 runs [0-20]. 
         * T2 runs [20-50]. 
         * T1 runs [50-70] (2nd period).
         * T2 runs [70-100] (2nd period).
         * By T=100, T1 and T2 have consumed exactly 100ms of CPU time.
         * T3 (the lowest priority) got ZERO CPU time and will GUARANTEE a MISS at T=100,
         * even though the system mathematically had 7.2% idle time available overall.
         */
        TaskConfiguration_t tasks[] = {
            {"BadAlign_High", TaskGeneric, (void *)pdMS_TO_TICKS(20), 512, 3,  50,  50, TRACE_ENABLED, POLICY_SKIP},
            {"BadAlign_Med",  TaskGeneric, (void *)pdMS_TO_TICKS(30), 512, 2,  70,  70, TRACE_ENABLED, POLICY_SKIP},
            {"BadAlign_Low",  TaskGeneric, (void *)pdMS_TO_TICKS(10), 512, 1, 100, 100, TRACE_ENABLED, POLICY_SKIP}
        };

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 3,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
    if (RUNTIME_TEST_CASE == 28)
    {
        UART_printf("\n[TEST 28] Zero-Slack Mathematical Edge (U = 92.8%%)\n");
        /* Scenario:
         * T1: T=40, C=20 -> 50.0% Load
         * T2: T=70, C=30 -> 42.8% Load
         * Total Utilization = 92.8%.
         *
         * Math (Response Time Analysis):
         * R2 = C2 + ceil(R2/T1)*C1
         * R2 = 30 + ceil(70/40)*20 = 30 + 40 = 70.
         * The math says T2 will finish EXACTLY at its deadline (70).
         * * Expectation: 
         * Because the math leaves exactly ZERO slack, the tiny microseconds
         * of overhead required for FreeRTOS to context switch between T1 and T2 
         * will push T2's completion to ~70.1. 
         * Your monitor will proudly catch this as a MISS exactly at tick 70.
         */
        TaskConfiguration_t tasks[] = {
            {"ZeroSlack_High", TaskGeneric, (void *)pdMS_TO_TICKS(20), 512, 2,  40,  40, TRACE_ENABLED, POLICY_SKIP},
            {"ZeroSlack_Low",  TaskGeneric, (void *)pdMS_TO_TICKS(30), 512, 1,  70,  70, TRACE_ENABLED, POLICY_SKIP}
        };

        SchedulerConfig_t config = {
            .policy = POLICY_SKIP,
            .tasks = tasks,
            .num_tasks = 2,
            .max_tasks = 5};

        InitTesting(&config, PeriodicList, NonPeriodicList);
    }
}

static inline int semihosting_call(int reason, void *arg)
{
    int value;
    __asm volatile(
        "mov r0, %1\n"
        "mov r1, %2\n"
        "bkpt 0xAB\n"
        "mov %0, r0\n"
        : "=r"(value)
        : "r"(reason), "r"(arg)
        : "r0", "r1", "memory");
    return value;
}

static char cmdline[CMDLINE_MAX];

const char *get_cmdline(void)
{
    struct
    {
        char *buf;
        int len;
    } args = {
        .buf = cmdline,
        .len = sizeof(cmdline)};

    if (semihosting_call(SYS_GET_CMDLINE, &args) != 0)
        return NULL;

    return cmdline;
}

/* File management */

void read_file(char *file_name)
{
    FILE *f = fopen(file_name, "r");
    if (!f)
    {
        LogError("Can't open file %s \n", file_name);
    }
    int c;

    while ((c = fgetc(f)) != EOF)
    {
        vLoggingPrintf("%c", c);
    }
    vLoggingPrintf("\n");
    fclose(f);
}

void write_file(char *file_name, char *output)
{
    FILE *f = fopen(file_name, "w");
    if (!f)
    {
        LogError("Can't open file %s \n", file_name);
    }
    fprintf(f, "%s", output);
    fclose(f);
}

/*  Note: undefined behavior if content of file or expected output
    are blank (= "") */
bool cmp_file(char *file_name, char *expected_output)
{
    FILE *f = fopen(file_name, "r");
    if (!f)
    {
        LogError("Can't open file %s \n", file_name);
    }
    int c;

    while ((c = fgetc(f)) != EOF && strcmp(expected_output, "\0"))
    {
        LogInfo("Comparing %d and %d\n", *expected_output, c);
        if ((c != (int)*expected_output))
        {
            fclose(f);
            return false;
        }
        expected_output++;
    }
    fclose(f);
    return true;
}

void print_bool(bool b)
{
    if (b)
    {
        LogAlways("True\n");
    }
    else
    {
        LogAlways("False\n");
    }
}