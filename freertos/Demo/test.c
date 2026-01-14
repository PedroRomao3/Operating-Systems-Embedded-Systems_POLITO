#include "test.h"
#include "uart.h"
#include "setup.h"
#include <string.h>

#define TEST_CASE 8

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
        SdkLog("InitTesting: testConfiguration is NULL\n");
        return false;
    }

    if (testConfiguration->num_tasks > 0 && testConfiguration->tasks == NULL)
    {
        SdkLog("InitTesting: num_tasks > 0 but tasks array is NULL\n");
        return false;
    }

    if (testConfiguration->num_tasks > testConfiguration->max_tasks)
    {
        SdkLog("InitTesting: num_tasks (%u) exceeds max_tasks (%u)\n",
                testConfiguration->num_tasks, testConfiguration->max_tasks);
        return false;
    }

    for (uint32_t i = 0; i < testConfiguration->num_tasks; i++)
    {
        TaskConfiguration_t currentTask = testConfiguration->tasks[i];

        if (currentTask.func == NULL)
        {
            SdkLog("InitTesting: Task %u (%s) has NULL function\n", i, currentTask.name);
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
        { "TaskA", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 10, 10, TRACE_ENABLED, POLICY_SKIP }
    };

    SchedulerConfig_t config = {
        .policy = POLICY_SKIP,
        .tasks = tasks,
        .num_tasks = 1,
        .max_tasks = 5
    };

    InitTesting(&config, PeriodicList, NonPeriodicList);
#endif

#if TEST_CASE == 2
    UART_printf("\n[TEST 2] Round Robin (Same Priority)\n");

    TaskConfiguration_t tasks[] = {
        { "TaskA", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 10, 10, TRACE_ENABLED, POLICY_SKIP },
        { "TaskB", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 10, 10, TRACE_ENABLED, POLICY_SKIP }
    };

    SchedulerConfig_t config = {
        .policy = POLICY_SKIP,
        .tasks = tasks,
        .num_tasks = 2,
        .max_tasks = 5
    };

    InitTesting(&config, PeriodicList, NonPeriodicList);
#endif

#if TEST_CASE == 3
    UART_printf("\n[TEST 3] Preemption\n");

    TaskConfiguration_t tasks[] = {
        { "TaskA", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 3, 10, 10, TRACE_ENABLED, POLICY_SKIP },
        { "TaskB", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 10, 10, TRACE_ENABLED, POLICY_SKIP }
    };

    SchedulerConfig_t config = {
        .policy = POLICY_SKIP,
        .tasks = tasks,
        .num_tasks = 2,
        .max_tasks = 5
    };

    InitTesting(&config, PeriodicList, NonPeriodicList);
#endif

#if TEST_CASE == 4
    UART_printf("\n[TEST 4] Rate Monotonic\n");

    TaskConfiguration_t tasks[] = {
        { "TaskA", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 3, 10, 10, TRACE_ENABLED, POLICY_SKIP },
        { "TaskB", TaskGeneric, (void *)pdMS_TO_TICKS(2), 512, 1, 15, 15, TRACE_ENABLED, POLICY_SKIP }
    };

    SchedulerConfig_t config = {
        .policy = POLICY_SKIP,
        .tasks = tasks,
        .num_tasks = 2,
        .max_tasks = 5
    };

    InitTesting(&config, PeriodicList, NonPeriodicList);
#endif

#if TEST_CASE == 5
    UART_printf("\n[TEST 5] Deadline Miss (Not Overrun)\n");

    TaskConfiguration_t tasks[] = {
        { "TaskMiss", TaskGeneric, (void *)pdMS_TO_TICKS(15), 512, 1, 20, 10, TRACE_ENABLED, POLICY_SKIP }
    };

    SchedulerConfig_t config = {
        .policy = POLICY_SKIP,
        .tasks = tasks,
        .num_tasks = 1,
        .max_tasks = 5
    };
    InitTesting(&config, PeriodicList, NonPeriodicList);

#endif

#if TEST_CASE == 7
    UART_printf("\n[TEST 5] Deadline Miss (Not Overrun)\n");

    TaskConfiguration_t tasks[] = {
        { "TaskMiss", TaskGeneric, (void *)pdMS_TO_TICKS(150),
            512, 1, pdMS_TO_TICKS(100), pdMS_TO_TICKS(100), TRACE_ENABLED, POLICY_SKIP }
    };

    SchedulerConfig_t config = {
        .policy = POLICY_SKIP,
        .tasks = tasks,
        .num_tasks = 1,
        .max_tasks = 5
    };
    InitTesting(&config, PeriodicList, NonPeriodicList);

#endif

#if TEST_CASE == 8
    UART_printf("\n[TEST 5] Deadline Miss (Not Overrun)\n");

    TaskConfiguration_t tasks[] = {
        { "TaskMiss", TaskGeneric, (void *)pdMS_TO_TICKS(150),
            512, 1, pdMS_TO_TICKS(100), pdMS_TO_TICKS(100), TRACE_ENABLED, POLICY_SKIP }
    };

    SchedulerConfig_t config = {
        .policy = POLICY_CATCH_UP,
        .tasks = tasks,
        .num_tasks = 1,
        .max_tasks = 5
    };

    InitTesting(&config, PeriodicList, NonPeriodicList);
#endif
}

/* Retrieving arguments from command line */

#define SYS_GET_CMDLINE 0x15

static inline int semihosting_call(int reason, void *arg)
{
    int value;
    __asm volatile (
        "mov r0, %1\n"
        "mov r1, %2\n"
        "bkpt 0xAB\n"
        "mov %0, r0\n"
        : "=r"(value)
        : "r"(reason), "r"(arg)
        : "r0", "r1", "memory"
    );
    return value;
}

#define CMDLINE_MAX 128

static char cmdline[CMDLINE_MAX];

const char *get_cmdline(void)
{
    struct {
        char *buf;
        int len;
    } args = {
        .buf = cmdline,
        .len = sizeof(cmdline)
    };

    if (semihosting_call(SYS_GET_CMDLINE, &args) != 0)
        return NULL;

    return cmdline;
}

/* File management */

void read_file(char* file_name)
{
    FILE *f = fopen(file_name, "r");
    if (!f) {
        vLoggingPrintf("ERROR: can't open file %s \n", file_name);
    }
    int c;

    while ((c = fgetc(f)) != EOF) {
        vLoggingPrintf("%c",c);
    }

    fclose(f);
}

void write_file(char* file_name, char* output)
{
    FILE *f = fopen(file_name, "w");
    if (!f) {
        vLoggingPrintf("ERROR: can't open file %s \n", file_name);
    }
    fprintf(f, "%s", output);
    fclose(f);
}

bool cmp_file(char* file_name, char* expected_output)
{
    FILE *f = fopen(file_name, "r");
    if (!f) {
        vLoggingPrintf("ERROR: can't open file %s \n", file_name);
    }
    int c;

    while ((c = fgetc(f)) != EOF && strcmp(expected_output, "\0")) {
        vLoggingPrintf("Comparing %d and %d\n", *expected_output, c);
        if((c != (int)*expected_output)){
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
    if(b)
        UART_printf("True\n");
    else
        UART_printf("False\n");
}