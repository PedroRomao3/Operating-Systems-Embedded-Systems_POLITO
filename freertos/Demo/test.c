#include "test.h"

static void TaskA(void *arg)
{
    UART_printf("A");
    vTaskDelay(pdMS_TO_TICKS(5));
}

static void TaskB(void *arg)
{
    UART_printf("B");
    vTaskDelay(pdMS_TO_TICKS(18));
}

static void TaskC(void *arg)
{
    for (;;) {
        UART_printf("C");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void vCreateTestTasks(List_t *Periodic, List_t *NonPeriodic){
    
    vCreateAndAddTask("TaskA", TaskA, NULL, 3, true,
                      pdMS_TO_TICKS(10), pdMS_TO_TICKS(10),
                      POLICY_SKIP,
                      Periodic, NonPeriodic);

    vCreateAndAddTask("TaskB", TaskB, NULL, 2, true,
                      pdMS_TO_TICKS(20), pdMS_TO_TICKS(15),
                      POLICY_KILL,
                      Periodic, NonPeriodic);

    vCreateAndAddTask("TaskC", TaskC, NULL, 1, false,
                      0, 0,
                      POLICY_SKIP,
                      Periodic, NonPeriodic);
}

