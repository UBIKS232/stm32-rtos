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
extern UBaseType_t uxTopReadyPriority;
#define taskRECORD_READY_PRIORITY(uxPriority) \
    portRECORD_READY_PRIORITY(uxPriority, uxTopReadyPriority)
/******************************************************************************/

#endif // _TASK_H_
