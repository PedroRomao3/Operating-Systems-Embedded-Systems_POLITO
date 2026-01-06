#include "setup.h"

static uint32_t pid_counter = 0;

/* -------- INITIALISATION -------- */

void vTaskProcessusInit(List_t *periodic, List_t *nonperiodic)
{
    vListInitialise(periodic);
    vListInitialise(nonperiodic);
}

/* -------- CREATION -------- */

vProcessus* vTaskProcessusCreate(const char *name,
                                 TaskFunction_t function,
                                 void *arg,
                                 UBaseType_t priority,
                                 bool is_periodic,
                                 TickType_t period,
                                 TickType_t deadline,
                                 overrun_policy_t policy)
{
    vProcessus *p = pvPortMalloc(sizeof(vProcessus));

    p->pid = pid_counter++;
    strncpy(p->name, name, MAXIMUM_NAME_SIZE);
    p->name[MAXIMUM_NAME_SIZE - 1] = '\0';

    p->function = function;
    p->arg = arg;
    p->priority = priority;
    p->type = is_periodic ? PTask : NPTask;

    p->period = period;
    p->deadline = (deadline == 0) ? period : deadline;
    p->overrun_policy = policy;

    p->state = JOB_IDLE;
    p->last_release = 0;
    p->job_id = 0;

    vListInitialiseItem(&p->listItem);
    listSET_LIST_ITEM_OWNER(&p->listItem, p);

    return p;
}

/* -------- LIST MANAGEMENT -------- */

void TaskConfigListPNP_Add(vProcessus *process, List_t *PeriodList, List_t *NonPeriodList)
{
    vListInsertEnd((process->type == PTask)
                    ? PeriodList
                    : NonPeriodList,
                   &process->listItem);
}


vProcessus* vCreateAndAddTask(const char *name,
                              TaskFunction_t function,
                              void *arg,
                              UBaseType_t priority,
                              bool is_periodic,
                              TickType_t period,
                              TickType_t deadline,
                              overrun_policy_t policy,
                              List_t *PeriodList,
                              List_t *NonPeriodList)
{
    vProcessus *p = vTaskProcessusCreate(name, function, arg,
                                         priority, is_periodic,
                                         period, deadline, policy);

    TaskConfigListPNP_Add(p, PeriodList, NonPeriodList);
    return p;
}

/* -------- PERIODIC WRAPPER -------- */

static void vTaskPeriodicWrapper(void *arg)
{
    vProcessus *p = (vProcessus *)arg;

    for (;;) {
        xSemaphoreTake(p->release_sem, portMAX_DELAY);

        p->state = JOB_RUNNING;
        p->function(p->arg);
        p->state = JOB_IDLE;
    }
}

/* -------- TASK LAUNCH -------- */

void vListProcLaunchPerioc(List_t *PeriodicTaskConfigList)
{
    ListItem_t *it = listGET_HEAD_ENTRY(PeriodicTaskConfigList);

    while (it != listGET_END_MARKER(PeriodicTaskConfigList)) {
        vProcessus *p = listGET_LIST_ITEM_OWNER(it);

        p->release_sem = xSemaphoreCreateBinary();

        xTaskCreate(vTaskPeriodicWrapper,
                    p->name,
                    512,
                    p,
                    p->priority,
                    &p->handle);

        vTaskSuspend(p->handle);
        it = it->pxNext;
    }
}

void vListProcLaunchNonPerioc(List_t *NonPeriodicTaskConfigList)
{
    ListItem_t *it = listGET_HEAD_ENTRY(NonPeriodicTaskConfigList);

    while (it != listGET_END_MARKER(NonPeriodicTaskConfigList)) {
        vProcessus *p = listGET_LIST_ITEM_OWNER(it);

        xTaskCreate(p->function,
                    p->name,
                    512,
                    p->arg,
                    p->priority,
                    &p->handle);

        it = it->pxNext;
    }
}

/* -------- SCHEDULER EXTENSION -------- */

static void vHandleOverrun(vProcessus *p, TickType_t now)
{
    switch (p->overrun_policy) {

    case POLICY_SKIP:
        p->last_release += p->period;
        break;

    case POLICY_KILL:
        vTaskSuspend(p->handle);
        p->state = JOB_IDLE;
        /* fallthrough */

    case POLICY_CATCH_UP:
        p->job_id++;
        p->last_release = now;
        p->abs_deadline = now + p->deadline;
        xSemaphoreGive(p->release_sem);
        vTaskResume(p->handle);
        break;
    }
}

static void vPeriodicReleaseManager(void *arg)
{
    List_t *list = (List_t *)arg;

    for (;;) {
        TickType_t now = xTaskGetTickCount();
        ListItem_t *it = listGET_HEAD_ENTRY(list);

        while (it != listGET_END_MARKER(list)) {
            vProcessus *p = listGET_LIST_ITEM_OWNER(it);

            if ((now - p->last_release) >= p->period) {
                if (p->state == JOB_RUNNING) {
                    vHandleOverrun(p, now);
                } else {
                    p->job_id++;
                    p->last_release = now;
                    p->abs_deadline = now + p->deadline;
                    xSemaphoreGive(p->release_sem);
                    vTaskResume(p->handle);
                }
            }
            it = it->pxNext;
        }
        vTaskDelay(1);
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
