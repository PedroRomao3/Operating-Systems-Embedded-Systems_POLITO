#include "test.h"

#include "uart.h"

static void TaskA(void *arg)
{
    (void)arg;

    // UART_printf("A\n");
    vTaskDelay(pdMS_TO_TICKS(5));
}

static void TaskB(void *arg)
{
    (void)arg;

    // UART_printf("B\n");
    vTaskDelay(pdMS_TO_TICKS(18));
}

static void TaskC(void *arg)
{
    (void)arg;
    
    for (;;) {
        // UART_printf("C");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}

void vCreateTestTasks(List_t *Periodic, List_t *NonPeriodic){
    
    vCreateAndAddTask("TaskA", TaskA, NULL, 3, true, true,
                      pdMS_TO_TICKS(100), pdMS_TO_TICKS(100),
                      POLICY_SKIP,
                      Periodic, NonPeriodic);

    vCreateAndAddTask("TaskB", TaskB, NULL, 2, true, true,
                      pdMS_TO_TICKS(200), pdMS_TO_TICKS(200),
                      POLICY_KILL,
                      Periodic, NonPeriodic);

    vCreateAndAddTask("TaskC", TaskC, NULL, 1, false, true,
                      0, 0,
                      POLICY_SKIP,
                      Periodic, NonPeriodic);
}

