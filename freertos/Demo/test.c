#include "test.h"
#include "uart.h"
#include "setup.h"


#define TEST_CASE 5

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

void vCreateTestTasks(List_t *Periodic, List_t *NonPeriodic)
{
    
    #if TEST_CASE == 1
    UART_printf("\n[TEST 1] Single Periodic Task\n");
    // TaskA: T=10ms, D=10ms, Cost=2ms
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 
                      true, pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, true, Periodic, NonPeriodic);
    #endif

    #if TEST_CASE == 2
    UART_printf("\n[TEST 2] Round Robin (Same Priority)\n");
    // TaskA: Prio 1, Cost 2ms
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 
                      true, pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, true, Periodic, NonPeriodic);
    // TaskB: Prio 1, Cost 2ms
    vCreateAndAddTask("TaskB", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 
                      true, pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, true, Periodic, NonPeriodic);
    #endif

    #if TEST_CASE == 3
    UART_printf("\n[TEST 3] Preemption\n");
    // TaskA (High): Prio 3, Cost 2ms
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 3, 
                      true, pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, true, Periodic, NonPeriodic);
    // TaskB (Low): Prio 1, Cost 2ms
    vCreateAndAddTask("TaskB", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 
                      true, pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, true, Periodic, NonPeriodic);
    #endif

    #if TEST_CASE == 4
    UART_printf("\n[TEST 4] Rate Monotonic\n");
    // TaskA (Fast): T=10ms, Prio 3
    vCreateAndAddTask("TaskA", TaskGeneric, (void*)pdMS_TO_TICKS(2), 3, 
                      true, pdMS_TO_TICKS(10), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, true, Periodic, NonPeriodic);
    // TaskB (Slow): T=15ms, Prio 1
    vCreateAndAddTask("TaskB", TaskGeneric, (void*)pdMS_TO_TICKS(2), 1, 
                      true, pdMS_TO_TICKS(15), pdMS_TO_TICKS(15), 
                      POLICY_SKIP, true, Periodic, NonPeriodic);
    #endif

    
    #if TEST_CASE == 5
    UART_printf("\n[TEST 5] Deadline Miss (Not Overrun)\n");
    // TaskMiss: T=20ms, D=10ms, Cost=15ms
    vCreateAndAddTask("TaskMiss", TaskGeneric, (void*)pdMS_TO_TICKS(15), 1, 
                      true, pdMS_TO_TICKS(20), pdMS_TO_TICKS(10), 
                      POLICY_SKIP, true, Periodic, NonPeriodic);
    #endif
}