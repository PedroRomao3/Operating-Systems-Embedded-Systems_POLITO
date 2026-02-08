#include "setup.h"

#include "FreeRTOSConfig.h"

// strncpy
#include <string.h>

static uint32_t pid_counter = 0;

/* -------- INITIALISATION -------- */

void vTaskListsInitialize(List_t *periodic, List_t *nonperiodic)
{
    vListInitialise(periodic);
    vListInitialise(nonperiodic);
}

/* -------- CREATION -------- */

/*
 * Here we set the last_release time to be under 0 because:
 * On startup:
 *  -> We Create periodic tasks then suspend them => When scheduler start the periodic tasks are seen as BLOCKED
 *  -> Non periodic tasks are not suspended on creation => scheduler sees them as READY
 *  -> -> For the scheduler, READY > prio (since FreeRTOS runs READY tasks even if their Prio is lower)
 *
 * When the Scheduler Runs:
 *  -> PTL manager runs first (vPeriodicReleaseManager) since it has highest priority
 *  -> It DOES NOT RELEASE any semaphores
 *  -> -> This happens because:
 *  -> -> -> In vPeriodicReleaseManager we have:
 *  -> -> -> -> *if ((now - p->last_release) >= p->period)*
 *  -> -> -> And when we create a periodic we used to set p->last_release to 0
 *  -> -> -> -> That means when we run the scheduler that condition will ALWAYS fail the first iteration since xTaskGetTickCount will be 0 or 1
 *
 *  -> At the end of first iteration scheduler delays for n ticks which means it blocks the highest priority PTL Manager task.
 *  -> -> This means now we can run the non periodic task.
 *  -> -> since no semaphores were released on the first iteration thats why the NON-PERIODIC task appears to run first.
 *
 * By setting p->last_release to something < 0:
 *  -> now the condition: *if ((now - p->last_release) >= p->period)* will succeed even at the first iteration and the last_release time will be updated
 *  -> That means the semaphores are freed and that means the periodic task runs first
 */
TaskDescription_t *vTaskProcessesCreate(const char *name,
                                        TaskFunction_t function,
                                        void *arg,
                                        UBaseType_t priority,
                                        StackType_t stack_size,
                                        bool is_periodic,
                                        TickType_t period,
                                        TickType_t deadline,
                                        overrun_policy_t policy,
                                        bool trace_enabled)
{
    TaskDescription_t *p = pvPortMalloc(sizeof(TaskDescription_t));

    p->pid = pid_counter++;
    strncpy(p->name, name, MAXIMUM_NAME_SIZE);
    // Note(Reda): This could be omitted since string literals are guaranteed to be null terminated. UNLESS the provided
    //             string literal has size >= MAXIMUM_NAME_SIZE then this is needed and will cut the name short.
    p->name[MAXIMUM_NAME_SIZE - 1] = '\0';

    p->function = function;
    p->arg = arg;
    p->priority = priority;
    p->stack_size = stack_size;
    p->type = is_periodic ? PTask : NPTask;

    p->period = period;
    if (deadline == 0)
    {
        p->deadline = period;
        //0 to satisfy regex
        SdkLog("[TRACE] 0:%s:Config: Defaulting Deadline to Period\n", name);
    }
    else
    {
        p->deadline = deadline;
    }
    p->overrun_policy = policy;
    p->trace_enabled = trace_enabled;

    p->state = JOB_IDLE;
    // NOTE: Same as p->last_release = -p->period since xTaskGetTickCount() here should return 0 or 1
    p->last_release = xTaskGetTickCount() - p->period;
    p->job_id = 0;

    vListInitialiseItem(&p->listItem);
    listSET_LIST_ITEM_OWNER(&p->listItem, p);

    return p;
}

/* -------- LIST MANAGEMENT -------- */

void TaskConfigListPNP_Add(TaskDescription_t *process, List_t *PeriodList, List_t *NonPeriodList)
{
    vListInsertEnd((process->type == PTask)
                       ? PeriodList
                       : NonPeriodList,
                   &process->listItem);
}

TaskDescription_t *vCreateAndAddTask(const char *name,
                                     TaskFunction_t function,
                                     void *arg,
                                     UBaseType_t priority,
                                     StackType_t stack_size,
                                     bool is_periodic,
                                     bool trace_enabled,
                                     TickType_t period,
                                     TickType_t deadline,
                                     overrun_policy_t policy,
                                     List_t *PeriodList,
                                     List_t *NonPeriodList)
{
    TaskDescription_t *p = vTaskProcessesCreate(name, function, arg,
                                                priority, stack_size, is_periodic,
                                                period, deadline, policy, trace_enabled);

    TaskConfigListPNP_Add(p, PeriodList, NonPeriodList);
    return p;
}

/* -------- PERIODIC WRAPPER -------- */

static void vTaskPeriodicWrapper(void *arg)
{
    TaskDescription_t *p = (TaskDescription_t *)arg;

    for (;;)
    {
        xSemaphoreTake(p->release_sem, portMAX_DELAY);
        TASK_LOG(p, "[TRACE] %d:%s:START\n", xTaskGetTickCount(), p->name);
        p->state = JOB_RUNNING;
        p->function(p->arg);
        TickType_t now = xTaskGetTickCount();
        if (now > p->abs_deadline)
        {
            SdkLog("[TRACE] %d:%s:MISS\n", now, p->name);
        }

        p->state = JOB_IDLE;
        TASK_LOG(p, "[TRACE] %d:%s:END\n", xTaskGetTickCount(), p->name);
    }
}

/* -------- TASK LAUNCH -------- */

void vListProcLaunchPeriodic(List_t *PeriodicTaskConfigList)
{
    ListItem_t *it = listGET_HEAD_ENTRY(PeriodicTaskConfigList);

    while (it != listGET_END_MARKER(PeriodicTaskConfigList))
    {
        TaskDescription_t *p = listGET_LIST_ITEM_OWNER(it);

        p->release_sem = xSemaphoreCreateBinary();

        xTaskCreate(vTaskPeriodicWrapper,
                    p->name,
                    p->stack_size,
                    p,
                    p->priority,
                    &p->handle);

        vTaskSuspend(p->handle);
        it = it->pxNext;
    }
}

void vListProcLaunchNonPeriodic(List_t *NonPeriodicTaskConfigList)
{
    ListItem_t *it = listGET_HEAD_ENTRY(NonPeriodicTaskConfigList);

    while (it != listGET_END_MARKER(NonPeriodicTaskConfigList))
    {
        TaskDescription_t *p = listGET_LIST_ITEM_OWNER(it);

        xTaskCreate(p->function,
                    p->name,
                    p->stack_size,
                    p->arg,
                    p->priority,
                    &p->handle);

        it = it->pxNext;
    }
}

/* -------- SCHEDULER EXTENSION -------- */

static void vHandleOverrun(TaskDescription_t *p, TickType_t now)
{
    TASK_LOG(p, "[TRACE] %d:%s:OVERRUN Policy:%d\n", now, p->name, p->overrun_policy);
    switch (p->overrun_policy)
    {
    case POLICY_NONE:
        LogError("[ %d ] %s Task has POLICY_NONE which should not be allowed!\n", now, p->name);
        break;
    case POLICY_SKIP:
        p->last_release += p->period;
        break;
    case POLICY_KILL:
        vTaskDelete(p->handle);

        xTaskCreate(vTaskPeriodicWrapper,
                    p->name,
                    p->stack_size,
                    p,
                    p->priority,
                    &p->handle);

        p->state = JOB_IDLE;

        // NOTE: Here we want to fallthrough, since the teacher says to release immediatly

    case POLICY_CATCH_UP:
        p->job_id++;

        /* FIX: PREVENT PERMANENT SCHEDULE DRIFT
         * If we do: p->last_release = now;
         * Assuming we start at 0 with T=10, if an overrun makes us arrive at T=11,
         * the new schedule becomes 11 - 21 - 31... instead of 10 - 20 - 30...
         * This means we permanently "accept" the delay and shift the whole timeline.
         * By doing += period, we stay locked to the original grid.
         */
        p->last_release += p->period;

        /* NOTE ON "JOB SWAPPING" vs "SERIALIZED CATCH-UP":
         * pausing the current Job A (K) to release Job B (K+1)
         * and resuming A later is not feasible or logical here:
         * * 1. TECHNICAL: TaskDescription_t has one handle/stack. Resuming exactly
         * where a task stopped requires a "Task Pool" or stack-cloning which
         * isn't supported by the current architecture.
         * * 2. LOGICAL: In the real world, stopping a task mid-way (e.g., after reading
         * a sensor but before outputting the result) makes little sense. Outputting
         * that "stale" value later is often dangerous or useless.
         * * 3. THE SOLUTION: We "Release Immediately" by giving the semaphore now.
         * The late Job K finishes cleanly, then immediately takes the pending
         * semaphore to start Job K+1. This is still a Catch-Up policy.
         */

        p->abs_deadline = p->last_release + p->deadline;
        xSemaphoreGive(p->release_sem);
        vTaskResume(p->handle);
        break;
    }
}

static void vPeriodicReleaseManager(void *arg)
{
    List_t *list = (List_t *)arg;

    TickType_t ptlFirstWakeTime = xTaskGetTickCount();

    for (;;)
    {
        TickType_t now = xTaskGetTickCount();
        ListItem_t *it = listGET_HEAD_ENTRY(list);

        while (it != listGET_END_MARKER(list))
        {
            TaskDescription_t *p = listGET_LIST_ITEM_OWNER(it);

            if ((now - p->last_release) >= p->period)
            {
                /*  Task has exceeded deadline : deciding appropriate
                    response depending on policy */
                if (p->state == JOB_RUNNING)
                {
                    vHandleOverrun(p, now);
                }
                else
                {
                    // even if max priority ISR still higher priority, also critical zones stop interrupts that laos could cause delay
                    TASK_LOG(p, "[ %d ] %s released %s \n", now, "PTL_MGR", p->name);

                    p->job_id++;
                    p->last_release += p->period; // changed this, the explanation is the same as in catchup_policy code, delays still occur here (eg ISR)
                    p->abs_deadline = p->last_release + p->deadline;
                    xSemaphoreGive(p->release_sem);
                    vTaskResume(p->handle);
                }
            }
            it = it->pxNext;
        }
        // NOTE: vTaskDelayUntill here should be more precise and reduces cpu jitter that could be caused by vTaskDelay
        vTaskDelayUntil(&ptlFirstWakeTime, configPERIODIC_MANAGER_DELAY_TICKS);
    }
}

/* -------- LAUNCH SCHEDULER EXTENSION -------- */

void vStartPeriodicScheduler(List_t *PeriodicTaskConfigList)
{
    xTaskCreate(vPeriodicReleaseManager,
                "PTL_MGR",
                512,
                PeriodicTaskConfigList,
                configMAX_PRIORITIES - 1,
                NULL);
}
