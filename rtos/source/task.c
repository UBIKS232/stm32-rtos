#include "task.h"
#include "portmacro.h"
#include "rtos.h"
#include "rtos_config.h"
#include "projectdefs.h"
#include "list.h"

/******************************************************************************/
// 就绪列表: 任务创建好之后, 需要把任务添加到就绪列表里面, 表示任务已经就绪
// 同一优先级的任务统一插入到就绪列表的同一条链表中
List_t pxReadyTasksLists[configMAX_PRIORITIES] = {0};
// TCB_t *pxCurrentTCB
TCB_t *pxCurrentTCB = NULL;
// UBaseType_t uxCurrentNumberOfTasks
static volatile UBaseType_t uxCurrentNumberOfTasks = 0UL;
// 创建的任务的最高优先级
UBaseType_t uxTopReadyPriority = tskIDLE_PRIORITY;
// 任务延时列表
// FreeRTOS 定义了两个任务延时列表, 当系统时基计数器 xTickCount 没有溢出时, 用一条列表, 当 xTickCount 溢出后, 用另外一条列表
// xTickCount 溢出前
static List_t xDelayedTaskLists1 = {0};
// xTickCount 溢出后
static List_t xDelayedTaskLists2 = {0};
// 指向 xTickCount 没有溢出时使用的那条列表
List_t *pxDelayedTaskList = NULL;
// 指向 xTickCount 溢出时使用的那条列表
List_t *pxOverflowDelayedTaskList = NULL;
// 下一个任务的解锁时刻
volatile TickType_t xNextTaskUnblockTime = 0;
// xTickCount 溢出次数
BaseType_t xNumOfOverflows = 0;
extern TickType_t xTickCount;
/******************************************************************************/

/******************************************************************************/
/**
 * @brief 私有函数, 就序列表初始化
 */
void prvInitialiseTaskLists(void)
{
    UBaseType_t uxPriority = 0U;

    for (uxPriority = (UBaseType_t)0U;
         uxPriority < (UBaseType_t)configMAX_PRIORITIES;
         uxPriority++)
    {
        vListInitialise(&(pxReadyTasksLists[uxPriority]));
    }

    vListInitialise(&xDelayedTaskLists1);
    vListInitialise(&xDelayedTaskLists2);

    pxDelayedTaskList = &xDelayedTaskLists1;
    pxOverflowDelayedTaskList = &xDelayedTaskLists2;
}

#define DOUBLE_WORD_ALIGNMENT (0x0007)

/**
 * @brief 私有函数, 创建新任务
 * @param TaskFuntion_t pxTaskCode: 任务入口
 * @param const char * const pcName: 任务名称, 字符串形式
 * @param const uint32_t ulStackDepth: 任务栈大小, 单位为字
 * @param void * const pvParameters: 任务形参
 * @param TaskHandle_t *const pxCreateTask: 任务句柄
 * @param TCB_t *pxNewTCB: 任务控制块指针
 */
static void prvInitialiseNewTask(TaskFuntion_t pxTaskCode,
                                 const char *const pcName,
                                 const uint32_t ulStackDepth,
                                 void *const pvParameters,
                                 UBaseType_t uxPriority,
                                 TaskHandle_t *const pxCreateTask,
                                 TCB_t *pxNewTCB)
{
    StackType_t *pxTopOfStack = NULL; // uint32_t -> 4B
    UBaseType_t x = 0U;

    // ARM AAPCS(ARM Architecture Procedure Call Standard):
    // 任何时候：SP 必须 4 字节对齐(硬件强制, SP[1:0] = 0),
    // 函数调用边界(public interface): SP 必须 8 字节对齐(double-word aligned);
    // Cortex-M3 权威指南: "Cortex-M3 使用的是“向下生长的满栈”模型。
    // 堆栈指针 SP 指向最后一个被压入堆栈的 32 位数值。在下一次压栈时, SP 先自减 4, 再存入新的数值。"
    // 令指向 task stack 的 pxStack 从数组中的最低地址指向数组中的最高地址, 人为构造一个栈以及栈指针
    pxTopOfStack = pxNewTCB->pxStack + (ulStackDepth - (uint32_t)1U);
    // 令栈指针向下做 8 字节对齐
    pxTopOfStack = (StackType_t *)(((uint32_t)pxTopOfStack) & (~((uint32_t)DOUBLE_WORD_ALIGNMENT)));

    // 将任务的名字存储在 TCB 中
    for (x = (UBaseType_t)0U; x < (UBaseType_t)configMAX_TASK_NAME_LEN; x++)
    {
        pxNewTCB->pcTaskName[x] = pcName[x];

        // if (pcName[x] == '\0')
        if (pcName[x] == 0x00)
            break;
    }
    // 限制字符串长度
    pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN - 1] = '\0';

    // 初始化 TCB 中的 xStateListItem 节点
    vListInitialiseItem(&(pxNewTCB->xStateListItem));
    // 设置 xStateListItem 节点的拥有者
    listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem), pxNewTCB);

    // 初始化优先级
    if (uxPriority >= (UBaseType_t)configMAX_PRIORITIES)
    {
        uxPriority = (UBaseType_t)configMAX_PRIORITIES - (UBaseType_t)1U;
    }
    pxNewTCB->uxPriority = uxPriority;

    // 初始化任务栈
    pxNewTCB->pxTopOfStack = pxPortInitialiseStack(pxTopOfStack,
                                                   pxTaskCode,
                                                   pvParameters);

    // 让任务句柄指向任务控制块
    if ((void *)pxCreateTask != NULL)
    {
        *pxCreateTask = (TaskHandle_t)pxNewTCB;
    }
}

/**
 * @brief 添加新任务到任务就绪列表
 * @param TCB_t *pxNewTCB
 */
static void prvAddNewTaskToReadyList(TCB_t *pxNewTCB)
{
    taskENTER_CRITICAL();
    {
        uxCurrentNumberOfTasks++;

        if (pxCurrentTCB == NULL)
        {
            pxCurrentTCB = pxNewTCB;

            if (uxCurrentNumberOfTasks == (UBaseType_t)1)
            {
                prvInitialiseTaskLists();
            }
        }
        else
        {
            // 如果 pxCurrentTCB 非空且新任务优先级更高, 则将其指向新任务
            if (pxCurrentTCB->uxPriority <= pxNewTCB->uxPriority)
            {
                pxCurrentTCB = pxNewTCB;
            }
        }
        prvAddTaskToReadyList(pxNewTCB);
    }
    taskEXIT_CRITICAL();
}

/**
 * @brief 将任务插入到延时列表
 * @param TickType_t xTicksToWait
 */
static void prvAddCurrentTaskToDelayedList(TickType_t xTicksToWait)
{
    TickType_t xTimeToWake = 0;

    const TickType_t xConstTickCount = xTickCount;

    // 因为任务将要被添加到延时列表, 将任务从就绪列表中移除, 之后延时完成, 重新将任务添加到就绪列表
    if (uxListRemove(&(pxCurrentTCB->xStateListItem)) == (UBaseType_t)0)
    {
        // 调用函数 uxListRemove()将任务从就绪列表移除, uxListRemove()
        // 会返回当前链表下节点的个数, 如果为 0, 则表示当前链表下没有任务就绪, 则调用函数
        // portRESET_READY_PRIORITY()将任务在优先级位图表 uxTopReadyPriority 中对应
        // 的位清除, 因为 FreeRTOS 支持同一个优先级下可以有多个任务, 所以在清除优先级位图表
        // uxTopReadyPriority 中对应的位时要判断下该优先级下的就绪列表是否还有其它的任务
        portRESET_READY_PRIORITY(pxCurrentTCB->uxPriority, uxTopReadyPriority);
    }

    // 计算任务延时到期时, 系统时基计数器 xTickCount 的值是多少
    xTimeToWake = xConstTickCount + xTicksToWait;

    // 将延时到期的值设置为节点的排序值
    listSET_LIST_ITEM_VALUE(&(pxCurrentTCB->xStateListItem), xTimeToWake);

    if (xTimeToWake < xConstTickCount)
    {
        // 加法计算溢出了
        vListInsert(pxOverflowDelayedTaskList, &(pxCurrentTCB->xStateListItem));
    }
    else
    {
        vListInsert(pxDelayedTaskList, &(pxCurrentTCB->xStateListItem));

        // 更新下一个任务解锁时刻变量 xNextTaskUnblockTime 的值
        if (xTimeToWake < xNextTaskUnblockTime)
        {
            // 每次将任务加入延时列表都比较一下, 可以得到延时最短的任务的 unblocktime
            xNextTaskUnblockTime = xTimeToWake;
        }
    }
}

/**
 * @brief 复位 xNextTaskUnblockTime 的值
 */
void prvResetNextTaskUnblockTime(void)
{
    TCB_t *pxTCB = NULL;

    if (listLIST_IS_EMPTY(pxDelayedTaskList) != pdFALSE)
    {
        xNextTaskUnblockTime = portMAX_DELAY;
    }
    else
    {
        // 当前列表不为空, 则有任务在延时, 则获取当前列表下第一个节点的排序值, 然后将该节点的排序值更新到 xNextTaskUnblockTime
        listGET_OWNER_OF_NEXT_ENTRY(pxTCB, pxDelayedTaskList);
        xNextTaskUnblockTime = listGET_LIST_ITEM_VALUE(pxTCB);
    }
}

#if (configSUPPORT_STATIC_ALLOCATION == 1)

/**
 * @brief 静态创建任务
 * @param TaskFuntion_t pxTaskCode: 任务入口
 * @param const char *const pcName: 任务名称, 字符串形式
 * @param const uint32_t ulStackDepth: 任务栈大小, 单位为字
 * @param void *const pvParameters: 任务形参
 * @param UbaseType_t uxPriority: 任务优先级, 数值越大优先级越高
 * @param StackType_t *const puxStackBuffer: 任务栈起始地址
 * @param TCB_t *const pxTaskBuffer: 任务控制块指针
 * @returns TaskHandle_t xReturn: 任务句柄, 用于指向任务的 TCB
 */
TaskHandle_t xTaskCreateStatic(TaskFuntion_t pxTaskCode,
                               const char *const pcName,
                               const uint32_t ulStackDepth,
                               void *const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t *const puxStackBuffer,
                               TCB_t *const pxTaskBuffer)
{
    TCB_t *pxNewTCB = NULL;
    TaskHandle_t xReturn = NULL;

    if ((pxTaskBuffer != NULL) && (puxStackBuffer != NULL))
    {
        pxNewTCB = (TCB_t *)pxTaskBuffer;
        pxNewTCB->pxStack = (StackType_t *)puxStackBuffer;

        prvInitialiseNewTask(pxTaskCode,
                             pcName,
                             ulStackDepth,
                             pvParameters,
                             uxPriority,
                             &xReturn,
                             pxNewTCB);

        prvAddNewTaskToReadyList(pxNewTCB);
    }
    else
    {
        xReturn = NULL;
    }

    return xReturn;
}

#endif
/******************************************************************************/

/******************************************************************************/
extern TCB_t IdleTaskTCB;
extern StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];
void vApplicationGetIdleTaskMemory(TCB_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskStackBuffer = IdleTaskStack;
    *ppxIdleTaskTCBBuffer = &IdleTaskTCB;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

extern TCB_t Task1TCB;
extern TCB_t Task2TCB;

void prvIdleTask(void *p_arg)
{
    for (;;)
        taskYIELD();
}

#define PRIVILEGED_DATA
// Holds the handle of the idle task. The idle task is created automatically when the scheduler is started.
PRIVILEGED_DATA static TaskHandle_t xIdleTaskHandle = NULL;

/**
 * @brief 调度器启动
 */
void vTaskStartScheduler(void)
{
    // create idle task: start
    TCB_t *pxIdleTaskTCBBuffer = NULL;
    StackType_t *pxIdleTaskStackBuffer = NULL;
    uint32_t ulIldeTaskStackSize = 0;

    vApplicationGetIdleTaskMemory(&pxIdleTaskTCBBuffer,
                                  &pxIdleTaskStackBuffer,
                                  &ulIldeTaskStackSize);

    xIdleTaskHandle = xTaskCreateStatic((TaskFuntion_t)prvIdleTask,
                                        (char *)"IDLE",
                                        (uint32_t)ulIldeTaskStackSize,
                                        (void *)NULL,
                                        (UBaseType_t)tskIDLE_PRIORITY,
                                        (StackType_t *)pxIdleTaskStackBuffer,
                                        (TCB_t *)pxIdleTaskTCBBuffer);

    // vListInsertEnd(&(pxReadyTasksLists[0]),
    //             &(((TCB_t *)pxIdleTaskTCBBuffer)->xStateListItem));
    // create idle task: end

    xNextTaskUnblockTime = portMAX_DELAY;
    xTickCount = 0U;

#if 0
    // 目前不支持按优先级调度, 先指定一个最先运行的任务
    pxCurrentTCB = &Task1TCB;
#endif

    if (xPortStartScheduler() != pdFALSE)
    {
        // 如果调度器启动成功, 不会进入这个 if 块
    }
}

#if 0
/**
 * @brief 上下文切换, 更新pxCurrentTCB
 */
void vTaskSwitchContext(void)
{
    if (pxCurrentTCB == &Task1TCB)
        pxCurrentTCB = &Task2TCB;
    else
        pxCurrentTCB = &Task1TCB;
}
#endif
#if 0
/**
 * @brief 上下文切换, 更新pxCurrentTCB
 */
void vTaskSwitchContext(void)
{
    if (pxCurrentTCB == &IdleTaskTCB)
    {
        if (Task1TCB.xTicksToDelay == 0)
        {
            pxCurrentTCB = &Task1TCB;
        }
        else if (Task2TCB.xTicksToDelay == 0)
        {
            pxCurrentTCB = &Task2TCB;
        }
        else
        {
            return;
        }
    }
    else
    {
        if (pxCurrentTCB == &Task1TCB)
        {
            if (Task2TCB.xTicksToDelay == 0)
            {
                pxCurrentTCB = &Task2TCB;
            }
            else if (pxCurrentTCB->xTicksToDelay != 0)
            {
                pxCurrentTCB = &IdleTaskTCB;
            }
            else
            {
                return;
            }
        }
        else if (pxCurrentTCB == &Task2TCB)
        {
            if (Task1TCB.xTicksToDelay == 0)
            {
                pxCurrentTCB = &Task1TCB;
            }
            else if (pxCurrentTCB->xTicksToDelay != 0)
            {
                pxCurrentTCB = &IdleTaskTCB;
            }
            else
            {
                return;
            }
        }
    }
}
#else
/**
 * @brief 上下文切换, 更新pxCurrentTCB
 */
void vTaskSwitchContext(void)
{
#ifndef DEBUG___
    taskSELECT_HIGHEST_PRIORITY_TASK();
#else
    do
    {
        UBaseType_t uxTopPriority = 0UL;
        if (uxTopReadyPriority != 0UL)
            // portGET_HIGHEST_PRIORITY(uxTopPriority, uxTopReadyPriority);
            uxTopPriority = (31UL - (uint32_t)__clz(uxTopReadyPriority));
        // listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, &(pxReadyTasksLists[uxTopPriority]));
        do
        {
            List_t *const pxConstList = &(pxReadyTasksLists[uxTopPriority]);
            (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;
            if ((void *)(pxConstList)->pxIndex == (void *)&((pxConstList)->xListEnd))
            {
                (pxConstList)->pxIndex = (pxConstList)->pxIndex->pxNext;
            }
            pxCurrentTCB = (pxConstList)->pxIndex->pvOwner;
        } while (0);
        uxTopReadyPriority = uxTopPriority;
    } while (0);
#endif
}
#endif
/******************************************************************************/

/******************************************************************************/
void vTaskDelay(const TickType_t xTicksToDelay)
{
    // TCB_t *pxTCB = NULL;
    // pxTCB = pxCurrentTCB;
    // pxTCB->xTicksToDelay = xTicksToDelay;

    // 将任务从就绪列表移除, 与此同时需要存在一个延时列表
    // uxListRemove(&(pxTCB->xStateListItem));
    // taskRESET_READY_PRIORITY(pxTCB->uxPriority);

    // 将任务插入到延时列表
    prvAddCurrentTaskToDelayedList(xTicksToDelay);

    taskYIELD();
}
/******************************************************************************/
