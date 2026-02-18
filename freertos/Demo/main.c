#include "FreeRTOS.h"
#include "logging.h"
#include "task.h"
#include "setup.h"
#include "test.h"
#include "uart.h"
#include "list.h"
#include "logging_stack.h"

#include <stdio.h>
#include <stdlib.h> // atoi()

/* Global task lists */
List_t PeriodicTaskConfigList;
List_t NonPeriodicTaskConfigList;

extern void initialise_monitor_handles(void);
int RUNTIME_TEST_CASE = 0;

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

int main(void)
{
    /* Initialise process lists */
    UART_init();

    /* Initializes semihosting (gives access to host computer files) */
    initialise_monitor_handles();

    char *cmd = (char *)get_cmdline();
    if (cmd != NULL)
    {
        char *ptr = strstr(cmd, "TEST=");
        if (ptr != NULL)
        {
            RUNTIME_TEST_CASE = atoi(ptr + 5);
        }
    }

    // Fallback: If no arg provided, default to 5 (or whatever)
    if (RUNTIME_TEST_CASE == 0)
        RUNTIME_TEST_CASE = 5;

    SdkLog("Running Test Case: %d\n", RUNTIME_TEST_CASE);

    // LogAlways(cmd);
    // UART_printf("\n");

    // /*  Example of writing/reading a file and testing its content
    //     You should also find the file on your pc after running these lines */
    // write_file("./Output/test.txt", "TEST");
    // print_bool(cmp_file("./Output/test.txt", "TEST")); // should print True

    // print_bool(cmp_file("./Output/test.txt", " ")); // should print False

    // UART_printf("1");
    vTaskListsInitialize(&PeriodicTaskConfigList, &NonPeriodicTaskConfigList);

    /* Create application tasks */
    vCreateTestTasks(&PeriodicTaskConfigList, &NonPeriodicTaskConfigList);

    /* Create FreeRTOS tasks */
    vListProcLaunchPeriodic(&PeriodicTaskConfigList);
    vListProcLaunchNonPeriodic(&NonPeriodicTaskConfigList);

    /* Start periodic task layer */
    vStartPeriodicScheduler(&PeriodicTaskConfigList);

#if LOG_USE_BUFFERING
    // Start logging task with lowest priority
    vStartLoggingTask(&NonPeriodicTaskConfigList);
#endif

    /* Start FreeRTOS scheduler */
    vTaskStartScheduler();

    for (;;)
        ;
}
