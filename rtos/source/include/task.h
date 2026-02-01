#ifndef _TASK_H_
#define _TASK_H_

#include "portmacro.h"
#include "rtos.h"

/******************************************************************************/

typedef void *TaskHandle_t;

void prvInitialiseTaskLists(void);

#if (configSUPPORT_STATIC_ALLOCATION == 1)

TaskHandle_t xTaskCreateStatic(TaskFuntion_t pxTaskCode,
                               const char *const pcName,
                               const uint32_t ulStackDepth,
                               void *const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t *const puxStackBuffer,
                               TCB_t *const pxTaskBuffer);

#endif
/******************************************************************************/

/******************************************************************************/

void vTaskStartScheduler(void);

#define taskYIELD() portYIELD()

void vTaskSwitchContext(void);

void vTaskDelay(const TickType_t xTicksToDelay);
/******************************************************************************/

/******************************************************************************/
// 无中断保护关中断, 开中断, 进临界段, 出临界段
// 进入临界段，不带中断保护版本，不能嵌套
#define taskENTER_CRITICAL() portENTER_CRITICAL()
// 退出临界段，不带中断保护版本，不能嵌套
#define taskEXIT_CRITICAL() portEXIT_CRITICAL()
// 有中断保护关中断, 开中断, 进临界段, 出临界段
// 进入临界段，带中断保护版本，可以嵌套
#define taskENTER_CRITICAL_FROM_ISR() portSET_INTERRUPT_MASK_FROM_ISR()
// 退出临界段，带中断保护版本，可以嵌套
#define taskEXIT_CRITICAL_FROM_ISR(x) portCLEAR_INTERRUPT_MASK_FROM_ISR(x)
/******************************************************************************/

/******************************************************************************/
// 空闲任务优先级
#define tskIDLE_PRIORITY ((UBaseType_t)0U)
/******************************************************************************/


/******************************************************************************/
// 创建的任务的最高优先级
static volatile UBaseType_t uxTopReadyPriority = tskIDLE_PRIORITY;
// 查找最高优先级
#if (configUSE_PORT_OPTIMISED_TASK_SELECTION == 0)
// 通用方法
#define taskRECORD_READY_PRIORITY(uxPriority)  \
    do                                         \
    {                                          \
        if ((uxPriority) > uxTopReadyPriority) \
        {                                      \
            uxTopReadyPriority = (uxPriority); \
        }                                      \
    } while (0);

#define taskSELECT_HIGHEST_PRIORITY_TASK()                                              \
    do                                                                                  \
    {                                                                                   \
        UBaseType_t uxTopPriority = uxTopReadyPriority;                                 \
        while (listLIST_IS_EMPTY(&(pxReadyTasksLists[uxTopPriority])))                  \
        {                                                                               \
            uxTopPriority--;                                                            \
        }                                                                               \
        listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, &(pxReadyTasksLists[uxTopPriority])); \
        uxTopReadyPriority = uxTopPriority;                                             \
    } while (0);

#define taskRESET_READY_PRIORITY(uxPriority)

#define portRESET_READY_PRIORITY(uxPriorityt, uxTopReadyPriority)

#else
// 根据 Cortex-M3 优化后的方法
#define taskRECORD_READY_PRIORITY(uxPriority) \
    portRECORD_READY_PRIORITY(uxPriority, uxTopReadyPriority)

#define taskSELECT_HIGHEST_PRIORITY_TASK()                                              \
    do                                                                                  \
    {                                                                                   \
        UBaseType_t uxTopPriority = 0UL;                                                \
        portGET_HIGHEST_PRIORITY(uxTopPriority, uxTopReadyPriority);                    \
        listGET_OWNER_OF_NEXT_ENTRY(pxCurrentTCB, &(pxReadyTasksLists[uxTopPriority])); \
        uxTopReadyPriority = uxTopPriority;                                             \
    } while (0);

#if 0
#define taskRESET_READY_PRIORITY(uxPriority)                                                  \
    do                                                                                        \
    {                                                                                         \
        if (listCURRENT_LIST_LENGTH(&(pxReadyTasksLists[(uxPriorityt)])) == (UBaseType_t)0UL) \
        {                                                                                     \
            portRESET_READY_PRIORITY((uxPrioritty), (uxTopReadyPriority));                    \
        }                                                                                     \
    } while (0);
#else
// 按照 uxPriority 将 uxReadyPriorities(uint32_t) 的某一位清零
#define taskRESET_READY_PRIORITY(uxPriority)                          \
    do                                                                \
    {                                                                 \
        portRESET_READY_PRIORITY(uxPriority, uxTopReadyPriority); \
    } while (0);
#endif

#endif
/******************************************************************************/

#endif // _TASK_H_
