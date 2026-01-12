#include "test.h"
#include "uart.h"
#include "setup.h"

#define TEST_CASE 5

#define PERIODIC        true
#define NON_PERIODIC    false   // created macros because the trues and falses were confusing me
#define LOGGING         true
#define NO_LOGGING      false

static void vBurnCPU(TickType_t ticks_to_wait)
{
    TickType_t start = xTaskGetTickCount();
    while ((xTaskGetTickCount() - start) < ticks_to_wait) {
        __asm("nop"); 
    }
}

static void TaskGeneric(void *arg)
{
    TickType_t duration = (TickType_t)arg;
    vBurnCPU(duration);
}

void vCreateTestTasks(List_t *PeriodicList, List_t *NonPeriodicList)
{
    #if TEST_CASE == 1
    UART_printf("\n[TEST 1] Single Periodic Task\n");
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    #endif

    #if TEST_CASE == 2
    UART_printf("\n[TEST 2] Round Robin (Same Priority)\n");
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    
    vCreateAndAddTask("TaskB", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    #endif

    #if TEST_CASE == 3
    UART_printf("\n[TEST 3] Preemption\n");
    // TaskA (High Priority)
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 3, 
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    
    // TaskB (Low Priority)
    vCreateAndAddTask("TaskB", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    #endif

    #if TEST_CASE == 4
    UART_printf("\n[TEST 4] Rate Monotonic\n");
    // TaskA (Fast, High Prio)
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 3, 
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    
    // TaskB (Slow, Low Prio)
    vCreateAndAddTask("TaskB", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(15), pdMS_TO_TICKS(15), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    #endif

    #if TEST_CASE == 5
    UART_printf("\n[TEST 5] Deadline Miss (Not Overrun)\n");
    // Cost 15ms > Deadline 10ms, but < Period 20ms
    vCreateAndAddTask("TaskMiss", TaskGeneric, (void*)pdMS_TO_TICKS(15), 1, 
                      PERIODIC, LOGGING, 
                      pdMS_TO_TICKS(20), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, PeriodicList, NonPeriodicList);
    #endif
}