#include "test.h"
#include "uart.h"
#include "setup.h"

#define TEST_CASE 5

#define PERIODIC        true
#define NON_PERIODIC    false   // created macros because the trues and falses were confusing me
#define LOGGING         true
#define NO_LOGGING      false

static void vBurnCPU(TickType_t ticks_to_wait)
{
    TickType_t start = xTaskGetTickCount();
    while ((xTaskGetTickCount() - start) < ticks_to_wait) {
        __asm("nop"); 
    }
}

bool InitTesting(SchedulerConfig_t* testConfiguration, List_t* periodicTaskConfigList, List_t* nonPeriodicTaskConfigList)
{
    // TODO(Reda): Maybe trace here anywhere there is a possibile failing spot
    if (testConfiguration == NULL)
        return false;

    if (testConfiguration->num_tasks > 0 && testConfiguration->tasks == NULL)
        return false;

    if (testConfiguration->num_tasks > testConfiguration->max_tasks)
        return false;

    for (uint32_t i = 0; i < testConfiguration->num_tasks; i++)
    {
        TaskConfiguration_t currentTask = testConfiguration->tasks[i];

        if (currentTask.func == NULL)
            return false; // Since we have a task without a task function

        // TODO(Reda): Add some logging here? Set default stack_size if user forgot to specify and auto initialization sets it to 0
        if (currentTask.stack_size == 0)
            currentTask.stack_size = 512;

        // If the task config has an explicit policy then we use that otherwise just use the global scheduler policy
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
            LOGGING,
            pdMS_TO_TICKS(currentTask.period),
            pdMS_TO_TICKS(currentTask.deadline),
            policyToUse,
            periodicTaskConfigList, nonPeriodicTaskConfigList
        );
    }

    return true;
}

static void TaskGeneric(void *arg)
{
    TickType_t duration = (TickType_t)arg;
    vBurnCPU(duration);
}

void vCreateTestTasks(List_t *PeriodicList, List_t *NonPeriodicList)
{
    #if TEST_CASE == 1
    UART_printf("\n[TEST 1] Single Periodic Task\n");
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 512,
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    #endif

    #if TEST_CASE == 2
    UART_printf("\n[TEST 2] Round Robin (Same Priority)\n");
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 512,
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    
    vCreateAndAddTask("TaskB", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 512,
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    #endif

    #if TEST_CASE == 3
    UART_printf("\n[TEST 3] Preemption\n");
    // TaskA (High Priority)
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 3, 512,
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    
    // TaskB (Low Priority)
    vCreateAndAddTask("TaskB", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 512,
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    #endif

    #if TEST_CASE == 4
    UART_printf("\n[TEST 4] Rate Monotonic\n");
    // TaskA (Fast, High Prio)
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 3, 512,
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    
    // TaskB (Slow, Low Prio)
    vCreateAndAddTask("TaskB", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 512,
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(15), pdMS_TO_TICKS(15), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    #endif

    #if TEST_CASE == 5
    UART_printf("\n[TEST 5] Deadline Miss (Not Overrun)\n");
    // Cost 15ms > Deadline 10ms, but < Period 20ms
    vCreateAndAddTask("TaskMiss", TaskGeneric, (void*)pdMS_TO_TICKS(15), 1, 512,
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(20), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    #endif
}
