#ifndef TEST_H
#define TEST_H

#include "setup.h"

#define TRACE_ENABLED true

typedef struct TaskConfiguration_s
{
    const char *name;
    void (*func)(void *);
    void *arg;
    uint32_t stack_size;
    uint32_t priority;

    // In ms. NOTE: If not initialized together with the deadline (default init to 0) then the task is considered NON Periodic
    uint32_t period;
    uint32_t deadline;
    bool trace_enabled;

    // NOTE: Just in case we want to have an override policy from the global one
    // Default value POLICY_NONE, if anything different then it is considered an override
    overrun_policy_t policy_override;
} TaskConfiguration_t;

typedef struct SchedulerConfig_s
{
    // NOTE: Agreed to have a global scheduler config policy
    overrun_policy_t policy;
    uint32_t max_tasks;
    TaskConfiguration_t *tasks;
    uint32_t num_tasks; // This could be extracted/deduced from the tasks array while parsing

    // At end for optimal alignment
    bool trace_enabled;
} SchedulerConfig_t;

/**
 * @brief Create test tasks and register them in PTL lists.
 * @param testConfiguration configuration of the scheduler for the testing
 * @param periodicTaskConfigList list for periodic tasks
 * @param nonPeriodicTaskConfigList list for NON periodic tasks
 * @return true on proper test initialization, false otherwise
 */
bool InitTesting(SchedulerConfig_t *testConfiguration, List_t *periodicTaskConfigList, List_t *nonPeriodicTaskConfigList);

void vCreateTestTasks(List_t *PeriodicList, List_t *NonPeriodicList);

#endif
