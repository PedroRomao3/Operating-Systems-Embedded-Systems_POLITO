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
    
    UART_printf("1");
    vTaskListsInitialize(&PeriodicTaskConfigList, &NonPeriodicTaskConfigList);
    
    /* Create application tasks */

    UART_printf("2");
    vCreateTestTasks(&PeriodicTaskConfigList, &NonPeriodicTaskConfigList);    

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
