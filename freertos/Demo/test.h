#ifndef __TEST_H__
#define __TEST_H__

//Function to create processus
void vTaskProcessusInit(List_t* PeriodList, List_t* NonPeriodList);

//Here functions for all process
void vTaskFunction();

#endif