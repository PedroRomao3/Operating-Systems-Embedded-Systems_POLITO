#include "test.h"

static void TaskA(void *arg)
{
    UART_printf("A\n");
    vTaskDelay(pdMS_TO_TICKS(5));
}

static void TaskB(void *arg)
{
    UART_printf("B\n");
    vTaskDelay(pdMS_TO_TICKS(18));
}

static void TaskC(void *arg)
{
    UART_printf("This is a non periodic task.\n");
    vTaskDelay(pdMS_TO_TICKS(100));
}

void vCreateTestTasks(List_t *Periodic, List_t *NonPeriodic){
    
    vCreateAndAddTask("TaskA", TaskA, NULL, 3, true,
                      pdMS_TO_TICKS(100), pdMS_TO_TICKS(100),
                      POLICY_SKIP,
                      Periodic, NonPeriodic);

    vCreateAndAddTask("TaskB", TaskB, NULL, 2, true,
                      pdMS_TO_TICKS(200), pdMS_TO_TICKS(200),
                      POLICY_KILL,
                      Periodic, NonPeriodic);

    vCreateAndAddTask("TaskC", TaskC, NULL, 1, false,
                      0, 0,
                      POLICY_SKIP,
                      Periodic, NonPeriodic);
}

