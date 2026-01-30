#ifndef _RTOS_H_
#define _RTOS_H_

#include "portmacro.h"
#include "rtos_config.h"
#include "list.h"

// 任务控制块
typedef struct taskTaskControlBlock TCB_t;
struct taskTaskControlBlock
{
    // 栈顶
    volatile StackType_t *pxTopOfStack;
    // 任务节点
    ListItem_t xStateListItem;
    // 任务栈起始地址
    StackType_t *pxStack;
    // 任务名称, 字符串
    char pcTaskName[configMAX_TASK_NAME_LEN];
    // 延时, 等于0时延时结束
    TickType_t xTicksToDelay;
};

#endif // _RTOS_H_
