#include "FreeRTOS.h"
#include "logging.h"
#include "task.h"
#include "setup.h"
#include "test.h"
#include "uart.h"
#include "list.h"
#include "logging_stack.h"

#include <stdio.h>

/* Global task lists */
List_t PeriodicTaskConfigList;
List_t NonPeriodicTaskConfigList;

extern void initialise_monitor_handles(void);

int main(void)
{
    /* Initialise process lists */
    UART_init();

    /* Initializes semihosting (gives access to host computer files) */
    initialise_monitor_handles();

    /* Reads the arguments given in the command line */
    char *cmd = (char *)get_cmdline();

    vLoggingPrintf(cmd);
    UART_printf("\n");

    /*  Example of writing/reading a file and testing its content 
        You should also find the file on your pc after running these lines */
    write_file("./Output/test.txt", "TEST");
    print_bool(cmp_file("./Output/test.txt", "TEST")); // should print True

    print_bool(cmp_file("./Output/test.txt", " ")); // should print False

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
