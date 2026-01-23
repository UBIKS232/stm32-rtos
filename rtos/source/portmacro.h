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

#define portSTACK_TYPE uint32_t

#define portBASE_TYPE long

typedef portSTACK_TYPE StackType_t;

typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if (configUSE_16_BIT_TICKS == 1)
typedef uint16_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffff
#else
typedef uint32_t TickType_t;
#define portMAX_DELAY (TickType_t)0xffffffffUL
#endif

#endif // _PORTMACRO_H_
