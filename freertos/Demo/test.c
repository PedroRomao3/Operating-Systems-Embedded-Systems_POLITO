#include "test.h"

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
            pdMS_TO_TICKS(currentTask.period),
            pdMS_TO_TICKS(currentTask.deadline),
            policyToUse,
            periodicTaskConfigList, nonPeriodicTaskConfigList
        );
    }

    return true;
}
