#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"
#include "uart.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

uint32_t vTaskFunction(void *pv) {
    for (;;) {
        //work here
        pv += 1;
        pv -= 1;
        for (volatile int i=0; i<150000; ++i) { __asm volatile("nop"); }
        vTaskSuspend(NULL);
    }
    return 0;
}

void vTaskProcessusInit(List_t* PeriodList, List_t* NonPeriodList){
    vProcessus *HardProcessOne = vTaskProcessusCreate("proccesP1", vTaskFunction, 5 , 1, false, NULL);
    vProcessus *HardProcessTwo = vTaskProcessusCreate("proccesP2", vTaskFunction, 5 , 1, false, NULL);
    vProcessus *SoftProcessThree = vTaskProcessusCreate("proccesNP1", vTaskFunction, 5 , 1, false, NULL);

    TaskConfigListPNP_Add(HardProcessOne, PeriodList, NonPeriodList);
    TaskConfigListPNP_Add(HardProcessTwo, PeriodList, NonPeriodList);
    TaskConfigListPNP_Add(SoftProcessThree, PeriodList, NonPeriodList);
}
