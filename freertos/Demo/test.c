#include "test.h"
#include "uart.h"
#include "setup.h"

#define TEST_CASE 5

static void vBurnCPU(TickType_t ticks_to_wait)
{
    TickType_t start = xTaskGetTickCount();
    while ((xTaskGetTickCount() - start) < ticks_to_wait)
    {
        __asm("nop");
    }
}

static void TaskGeneric(void *arg)
{
    TickType_t duration = (TickType_t)arg;
    vBurnCPU(duration);
}

bool InitTesting(SchedulerConfig_t *testConfiguration, List_t *periodicTaskConfigList, List_t *nonPeriodicTaskConfigList)
{
    if (testConfiguration == NULL)
    {
        SdkLog(("InitTesting: testConfiguration is NULL\n"));
        return false;
    }

    if (testConfiguration->num_tasks > 0 && testConfiguration->tasks == NULL)
    {
        SdkLog(("InitTesting: num_tasks > 0 but tasks array is NULL\n"));
        return false;
    }

    if (testConfiguration->num_tasks > testConfiguration->max_tasks)
    {
        SdkLog(("InitTesting: num_tasks (%u) exceeds max_tasks (%u)\n",
                testConfiguration->num_tasks, testConfiguration->max_tasks));
        return false;
    }

    for (uint32_t i = 0; i < testConfiguration->num_tasks; i++)
    {
        TaskConfiguration_t currentTask = testConfiguration->tasks[i];

        if (currentTask.func == NULL)
        {
            SdkLog(("InitTesting: Task %u (%s) has NULL function\n", i, currentTask.name));
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
#if TEST_CASE == 1
    UART_printf("\n[TEST 1] Single Periodic Task\n");

    TaskConfiguration_t tasks[] = {
        {"TaskA", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 10, 10, TRACE_ENABLED, POLICY_SKIP}};

    SchedulerConfig_t config = {
        .policy = POLICY_SKIP,
        .tasks = tasks,
        .num_tasks = 1,
        .max_tasks = 5};

    InitTesting(&config, PeriodicList, NonPeriodicList);
#endif

#if TEST_CASE == 2
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
#endif

#if TEST_CASE == 3
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
#endif

#if TEST_CASE == 4
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
#endif

#if TEST_CASE == 5
    UART_printf("\n[TEST 5] Deadline Miss (Not Overrun)\n");

    TaskConfiguration_t tasks[] = {
        {"TaskMiss", TaskGeneric, (void *)pdMS_TO_TICKS(15), 512, 1, 20, 10, TRACE_ENABLED, POLICY_SKIP}};

    SchedulerConfig_t config = {
        .policy = POLICY_SKIP,
        .tasks = tasks,
        .num_tasks = 1,
        .max_tasks = 5};

    InitTesting(&config, PeriodicList, NonPeriodicList);
#endif
}