#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"
#include "uart.h"
#include <stdio.h>

int id_counter = 0;

void TaskConfigList_Add(vProcessus *slot, List_t* List){
    // Initialise intern item
    vListInitialiseItem(&slot->listItem);

    // Ling to get the slot
    listSET_LIST_ITEM_OWNER(&slot->listItem, slot);

    // Insert the item
    vListInsertEnd(List, &slot->listItem);
}

void TaskConfigListPNP_Add(vProcessus *process, List_t *PeriodList, List_t *NonPeriodList){
    // Check the Type and add it to the correct List_t
    TaskConfigList_Add(process, (process->type == PTask) ? PeriodList : NonPeriodList);
}

vProcessus* vTaskProcessusCreate(const char* name, uint32_t (*function)(void*), uint32_t period_time, uint32_t priority, bool is_Period, void *arg){
    
    vProcessus *new_task = pvPortMalloc(sizeof(vProcessus));

    //choose the id and increment it for the next process
    new_task->pid = id_counter;
    id_counter++;

    //strcpy and \'0' for security
    strncpy(new_task->name, name, MAXIMUM_NAME_SIZE);
    new_task->name[MAXIMUM_NAME_SIZE - 1] = '\0';

    new_task->priority = priority;
    new_task->function = function;
    new_task->start_waiting_date = 0;
    new_task->period_time = period_time;
    new_task->wake_up_time = period_time;
    new_task->arg = arg;

    new_task->type = is_Period ? PTask : NPTask;

    return new_task;
}
void vListProcLaunchPerioc(List_t* PeriodicTaskConfigList)
{
    ListItem_t *pxItem;

    pxItem = listGET_HEAD_ENTRY(PeriodicTaskConfigList);
    while (pxItem != listGET_END_MARKER(PeriodicTaskConfigList))
    {
        vProcessus *obj = (vProcessus *) listGET_LIST_ITEM_OWNER(pxItem);

        // Do something with obj
        // ...
        // End

        pxItem = pxItem->pxNext;  // move to next item
    }
}

void vListProcLaunchNonPerioc(List_t* NonPeriodicTaskConfigList){
        ListItem_t *pxItem;

    pxItem = listGET_HEAD_ENTRY(NonPeriodicTaskConfigList);
    while (pxItem != listGET_END_MARKER(NonPeriodicTaskConfigList))
    {
        vProcessus *obj = (vProcessus *) listGET_LIST_ITEM_OWNER(pxItem);

        // Do something with obj
        // ...
        // End

        pxItem = pxItem->pxNext;  // move to next item
    }
}
