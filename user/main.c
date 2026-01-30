#include "portmacro.h"
#include "rtos_config.h"
#include "projectdefs.h"
#include "task.h"
#include "rtos.h"
#include "list.h"

extern List_t pxReadyTasksLists[configMAX_PRIORITIES];

// idle task
TCB_t IdleTaskTCB = {0};
StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];

// task 1
portCHAR flag1 = 0;
TCB_t Task1TCB = {0};
TaskHandle_t Task1_Handle = NULL;
#define TASK1_STACK_SIZE 128
StackType_t Task1Stack[TASK1_STACK_SIZE];
void Task1_Entry(void *p_arg);

// task 2
portCHAR flag2 = 0;
TCB_t Task2TCB = {0};
TaskHandle_t Task2_Handle = NULL;
#define TASK2_STACK_SIZE 128
StackType_t Task2Stack[TASK2_STACK_SIZE];
void Task2_Entry(void *p_arg);

void delay(uint32_t count);

int main(void)
{
    // 通过将任务控制块的 xStateListItem 这个节点插入到就绪列表中来实现将任务插入到就绪列表里面,
    // 如果把就绪列表比作是晾衣架, 任务是衣服,
    // 那 xStateListItem 就是晾衣架上面的钩子,
    // 每个任务都自带晾衣架钩子, 就是为了把自己挂在各种不同的链表中
    prvInitialiseTaskLists();
    Task1_Handle = xTaskCreateStatic((TaskFuntion_t)Task1_Entry,
                                     (char *)"Task1",
                                     (uint32_t)TASK1_STACK_SIZE,
                                     (void *)NULL,
                                     (StackType_t *)Task1Stack,
                                     (TCB_t *)&Task1TCB);
    Task2_Handle = xTaskCreateStatic((TaskFuntion_t)Task2_Entry,
                                     (char *)"Task2",
                                     (uint32_t)TASK1_STACK_SIZE,
                                     (void *)NULL,
                                     (StackType_t *)Task2Stack,
                                     (TCB_t *)&Task2TCB);
    vListInsertEnd(&(pxReadyTasksLists[1]),
                   &(((TCB_t *)(&Task1TCB))->xStateListItem));
    vListInsertEnd(&(pxReadyTasksLists[2]),
                   &(((TCB_t *)(&Task2TCB))->xStateListItem));

    vTaskStartScheduler();

    while (1)
    {
    }
}

void delay(uint32_t count)
{
    for (; count != 0; count--)
        ;
}

void Task1_Entry(void *p_arg)
{
    for (;;)
    {
#if 0
        flag1 = 1;
        delay(100);
        flag1 = 0;
        delay(100);

        taskYIELD();
#else
        flag1 = 1;
        vTaskDelay(10);
        flag1 = 0;
        vTaskDelay(10);
#endif
    }
}

void Task2_Entry(void *p_arg)
{
    for (;;)
    {
#if 0
        flag2 = 1;
        delay(100);
        flag2 = 0;
        delay(100);

        taskYIELD();
#else
        flag2 = 1;
        vTaskDelay(10);
        flag2 = 0;
        vTaskDelay(10);
#endif
    }
}
