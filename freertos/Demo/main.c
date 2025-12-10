#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"
#include "test.h"
#include "uart.h"
#include <stdio.h>

#define mainTASK_PRIORITY    ( tskIDLE_PRIORITY + 2 )
#define N 5000

//define global maybe change it by creation in main and parameters in Scheduler()?
List_t PeriodicTaskConfigList;
List_t NonPeriodicTaskConfigList;

int main(int argc, char **argv){

	(void) argc;
	(void) argv;

    UART_init();
	UART_printf("Initialisation of the Kernel\n");

	UART_printf("Creation of the processus list\n");
	// Création of the list of process
	vTaskProcessusInit(&PeriodicTaskConfigList, &NonPeriodicTaskConfigList);

	// TODO If something needed to initialise process do it here
	vListProcLaunchPerioc(&PeriodicTaskConfigList);
	vListProcLaunchNonPerioc(&NonPeriodicTaskConfigList);

	UART_printf("Launch of the Scheduler\n");

	// Give control to the scheduler
	vTaskStartScheduler();

	// If everything ok should never reach here
    for( ; ; );
}
