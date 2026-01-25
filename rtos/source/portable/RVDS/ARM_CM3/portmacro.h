#ifndef _PORTMACRO_H_
#define _PORTMACRO_H_

#include "stdint.h"
#include "stddef.h"
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

StackType_t pxPortInitialiseStack(StackType_t *pxTopOfStack,
                                  TaskFuntion_t pxCode,
                                  void *pvParameters);
BaseType_t xPortStartScheduler(void);

#endif // _PORTMACRO_H_
