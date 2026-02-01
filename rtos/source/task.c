#include "task.h"
#include "portmacro.h"
#include "rtos.h"
#include "rtos_config.h"
#include "projectdefs.h"
#include "list.h"

/******************************************************************************/
// 就绪列表: 任务创建好之后, 需要把任务添加到就绪列表里面，表示任务已经就绪
// 同一优先级的任务统一插入到就绪列表的同一条链表中
List_t pxReadyTasksLists[configMAX_PRIORITIES] = {0};
// TCB_t *pxCurrentTCB
TCB_t *pxCurrentTCB = NULL;
// UBaseType_t uxCurrentNumberOfTasks
static volatile UBaseType_t uxCurrentNumberOfTasks = 0UL;
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

// 将任务添加到就序列表
#define prvAddTaskToReadyList(pxTCB)                              \
    do                                                            \
    {                                                             \
        taskRECORD_READY_PRIORITY((pxTCB)->uxPriority);           \
        vListInsertEnd(&(pxReadyTasksLists[(pxTCB)->uxPriority]), \
                       &((pxTCB)->xStateListItem));               \
    } while (0);

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
void vTaskSwitchContext(void){
    taskSELECT_HIGHEST_PRIORITY_TASK();
}
#endif
/******************************************************************************/

/******************************************************************************/
void vTaskDelay(const TickType_t xTicksToDelay)
{
    TCB_t *pxTCB = NULL;

    pxTCB = pxCurrentTCB;

    pxTCB->xTicksToDelay = xTicksToDelay;

    // 将任务从就绪列表移除, 与此同时需要存在一个延时列表
    // uxListRemove(&(pxTCB->xStateListItem));
    taskRESET_READY_PRIORITY(pxTCB->uxPriority);

    taskYIELD();
}
/******************************************************************************/
