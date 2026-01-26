#ifndef _PORTMACRO_H_
#define _PORTMACRO_H_

#include "stdint.h"
#include "stddef.h"
#include "projectdefs.h"
#include "rtos_config.h"

// "??"为带有疑问的语句

// object oriented
// #define OO(name)                  \
//     typedef struct name name##_t; \
//     struct name

#define portCHAR char
#define portFLOAT float
#define portDOUBLE double
#define portLONG long
#define portSHORT short

// 4B
#define portSTACK_TYPE uint32_t
typedef portSTACK_TYPE StackType_t;

// 8B??
#define portBASE_TYPE long
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if (configUSE_16_BIT_TICKS == 1)
typedef uint16_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffff
#else
typedef uint32_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffffffffUL
#endif

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack,
                                  TaskFuntion_t pxCode,
                                  void *pvParameters);
BaseType_t xPortStartScheduler(void);

#define portNVIC_INT_CTRL_REG (*((volatile uint32_t *)0xE000ED04))
#define portNVIC_PENDSVSET_BIT (1UL << 28UL)
#define portSY_FULL_READ_WRITE (15)

// 将 PendSV 的悬起位置 1, 当没有其它中断运行的时候响应 PendSV 中断, 去执行写好的 PendSV断服务函数, 在里面实现任务切换
#define portYIELD()                                     \
    {                                                   \
        /* 触发PendSV, 产生上下文切换 */                \
        portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT; \
        __dsb(portSY_FULL_READ_WRITE);                  \
        __isb(portSY_FULL_READ_WRITE);                  \
    }

#endif // _PORTMACRO_H_
