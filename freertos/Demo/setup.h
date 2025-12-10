#ifndef __SETUP__
#define __SETUP__

#include "FreeRTOS.h"
#include "task.h"
#include "setup.h"
#include "uart.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAXIMUM_NAME_SIZE 128

typedef enum {
    PTask = 0,
    NPTask = 1
} TaskType_t;

typedef struct vProcessus{
    uint32_t pid;
    char name[MAXIMUM_NAME_SIZE];
    uint32_t priority;
	unsigned long start_waiting_date;
    uint32_t (*function)(void*);
    void *arg;
    uint32_t wake_up_time;
	uint32_t period_time;
    TaskType_t type;
    ListItem_t listItem;
} vProcessus;

/**
 * @brief Adds a process (vProcessus) to a FreeRTOS list.
 *
 * This function initializes the internal list item of a process,
 * sets the list item's owner pointer to the process itself, 
 * and then inserts the item at the end of a FreeRTOS list.
 *
 * It is used to register a process structure into a FreeRTOS List_t
 * for scheduling, tracking, or management purposes.
 *
 * @param[in,out] slot 
 *      Pointer to a vProcessus structure whose internal ListItem_t 
 *      (`slot->listItem`) will be initialized and inserted into the list.
 *      - Must not be NULL.
 *      - `slot->listItem` must be a valid ListItem_t member.
 *
 * @param[in,out] List 
 *      Pointer to a FreeRTOS list (`List_t`) where the process will be inserted.
 *      - Must not be NULL.
 *      - The list must already be initialized using `vListInitialise()`.
 *
 * @note 
 * - This function performs no NULL pointer checks; the caller must ensure that 
 *   the parameters are valid.
 * - The process is always inserted at the **end** of the list.
 * - The function uses the following FreeRTOS primitives:
 *      - `vListInitialiseItem()`
 *      - `listSET_LIST_ITEM_OWNER()`
 *      - `vListInsertEnd()`
 *
 * @return
 *      None. The function modifies the provided list in place.
 */
void TaskConfigList_Add(vProcessus *slot, List_t* hrtList);


/**
 * @brief Adds a process to either the periodic or non-periodic FreeRTOS list.
 *
 * This function checks the process type and inserts it into the appropriate
 * FreeRTOS list. Periodic tasks are added to the PeriodList, while 
 * non-periodic tasks are added to the NonPeriodList.
 *
 * @param[in] process 
 *      Pointer to the vProcessus structure to be added.
 *      - Must not be NULL.
 *      - The process must have a valid `type` field.
 *
 * @param[in,out] PeriodList 
 *      Pointer to the FreeRTOS list that stores periodic tasks.
 *      - Must not be NULL.
 *
 * @param[in,out] NonPeriodList 
 *      Pointer to the FreeRTOS list that stores non-periodic tasks.
 *      - Must not be NULL.
 *
 * @note
 * - This function performs no NULL checks; the caller must ensure that the input
 *   pointers are valid.
 * - The function internally calls `TaskConfigList_Add()`.
 *
 * @return
 *      None.
 */
void TaskConfigListPNP_Add(vProcessus* process, List_t* PeriodList, List_t* NonPeriodList);


/**
 * @brief Creates and initializes a new process (vProcessus).
 *
 * This function allocates memory for a new vProcessus structure,
 * assigns it a unique process ID, copies the task name safely,
 * stores the task function pointer and its argument, and initializes 
 * timing and priority parameters.
 *
 * The task type (periodic or non-periodic) is determined based on
 * the `is_Period` parameter.
 *
 * @param[in] name 
 *      Null-terminated string representing the name of the process.
 *      - Must not be NULL.
 *      - Will be safely copied into the internal name buffer.
 *
 * @param[in] function 
 *      Pointer to the function executed by the process.
 *      - Must match the signature: uint32_t func(void* arg)
 *
 * @param[in] period_time 
 *      The task period (for periodic tasks) or delay value.
 *
 * @param[in] priority 
 *      Priority assigned to the process.
 *
 * @param[in] is_Period 
 *      Boolean indicating whether the task is periodic.
 *      - true  → PTask  
 *      - false → NPTask
 *
 * @param[in] arg 
 *      Pointer passed as an argument to the process function.
 *
 * @return 
 *      Pointer to a fully initialized vProcessus structure.
 *      - The memory is allocated using `pvPortMalloc()`.
 *      - The caller is responsible for freeing it if required.
 *
 * @note
 * - No NULL checks are performed; the caller must validate inputs.
 * - The process ID is generated using an external `id_counter`.
 * - The function uses FreeRTOS memory allocation.
 */
vProcessus* vTaskProcessusCreate(const char* name, uint32_t (*function)(void*), uint32_t period_time, uint32_t priority, bool is_Period, void *arg);

void vListProcLaunchPerioc(List_t* PeriodicTaskConfigList);
void vListProcLaunchNonPerioc(List_t* NonPeriodicTaskConfigList);

#endif
