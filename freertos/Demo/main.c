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

int main(void)
{
    /* Initialise process lists */
    UART_init();

    vTaskListsInitialize(&PeriodicTaskConfigList, &NonPeriodicTaskConfigList);

    /* Create application tasks */
    vCreateTestTasks(&PeriodicTaskConfigList, &NonPeriodicTaskConfigList);

    /* Create FreeRTOS tasks */
    vListProcLaunchPeriodic(&PeriodicTaskConfigList);
    vListProcLaunchNonPeriodic(&NonPeriodicTaskConfigList);

    /* Start periodic task layer */
    vStartPeriodicScheduler(&PeriodicTaskConfigList);

    /* Start FreeRTOS scheduler */
    vTaskStartScheduler();

    for (;;);
}
