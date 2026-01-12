#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"
#include "test.h"
#include "uart.h"
#include "list.h"

#include <stdio.h>

/* Global task lists */
List_t PeriodicTaskConfigList;
List_t NonPeriodicTaskConfigList;

static void TaskA(void *arg)
{
    (void)arg;

    UART_printf("A");
    vTaskDelay(pdMS_TO_TICKS(5));
}

static void TaskB(void *arg)
{
    (void)arg;

    UART_printf("B");
    vTaskDelay(pdMS_TO_TICKS(18));
}

static void TaskC(void *arg)
{
    (void)arg;
    
    for (;;) {
        UART_printf("C");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main(void)
{
    /* Initialise process lists */
    UART_init();
    
    TaskConfiguration_t taskList[] = {
        { "TaskA", TaskA, NULL, 512, 3, 10, 10, POLICY_SKIP },
        { "TaskB", TaskB, NULL, 512, 2, 20, 15, POLICY_KILL },
        { "TaskC", TaskC, NULL, 512, 1, 0, 0, POLICY_SKIP }
    };

    SchedulerConfig_t testingCfg = {
        .policy = POLICY_KILL,
        .trace_enabled = true,
        .max_tasks = 8,
        .tasks = taskList,
        .num_tasks = 3
    };
    
    UART_printf("1");
    vTaskListsInitialize(&PeriodicTaskConfigList, &NonPeriodicTaskConfigList);
    
    /* Create application tasks */

    UART_printf("2");
    // TODO(Reda): Maybe some logging here if test initialization failed?
    if (!InitTesting(&testingCfg, &PeriodicTaskConfigList, &NonPeriodicTaskConfigList))
        return -1;

    /* Create FreeRTOS tasks */
    UART_printf("3");
    vListProcLaunchPeriodic(&PeriodicTaskConfigList);
    vListProcLaunchNonPeriodic(&NonPeriodicTaskConfigList);

    /* Start periodic task layer */
    UART_printf("4");
    vStartPeriodicScheduler(&PeriodicTaskConfigList);

    /* Start FreeRTOS scheduler */
    vTaskStartScheduler();

    for (;;);
}
