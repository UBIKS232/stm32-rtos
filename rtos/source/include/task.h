#ifndef _TASK_H_
#define _TASK_H_

#include "portmacro.h"

typedef void *TaskHandle_t;

void prvInitialiseTaskLists(void);

#if (configSUPPORT_STATIC_ALLOCATION == 1)

TaskHandle_t xTaskCreateStatic(TaskFuntion_t pxTaskCode,
                               const char *const pcName,
                               const uint32_t ulStackDepth,
                               void *const pvParameters,
                               StackType_t *const puxStackBuffer,
                               TCB_t *const pxTaskBuffer);

#endif

void vTaskStartScheduler(void);

#endif // _TASK_H_