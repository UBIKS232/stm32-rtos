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

/******************************************************************************/

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack,
                                   TaskFuntion_t pxCode,
                                   void *pvParameters);
BaseType_t xPortStartScheduler(void);
/******************************************************************************/

/******************************************************************************/
#define portNVIC_INT_CTRL_REG (*((volatile uint32_t *)0xE000ED04))
#define portNVIC_PENDSVSET_BIT (1UL << 28UL)
#define portSY_FULL_READ_WRITE (15)

// 触发上下文切换, 将 PendSV 的悬起位置 1, 当没有其它中断运行的时候响应 PendSV 中断, 去执行写好的 PendSV断服务函数, 在里面实现任务切换
#define portYIELD()                                     \
    {                                                   \
        /* 触发PendSV, 产生上下文切换 */                \
        portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT; \
        __dsb(portSY_FULL_READ_WRITE);                  \
        __isb(portSY_FULL_READ_WRITE);                  \
    }
/******************************************************************************/

/******************************************************************************/
// 临界段保护, 用于保护全局变量操作或者其他不能被打断的过程
#ifndef portFORCE_INLINE
#define portFORCE_INLINE inline __attribute__((always_inline))
#endif

// 无中断保护关中断, 开中断, 进临界段, 出临界段
// 不带返回值的关中断函数, 不能嵌套, 不能在中断里面使用
#define portDISABLE_INTERRUPTS() vPortRaiseBASEPRI()
/**
 * @brief 在往 BASEPRI 写入新的值的时候, 不用先将 BASEPRI 的值保存起来, 即不用管当前的中断状态是怎么样的, 既然不用管当前的中断状态, 也就意味着这样的函数不能在中断里面调用
 */
static portFORCE_INLINE void vPortRaiseBASEPRI(void)
{
    uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

    __asm
    {
        msr basepri, ulNewBASEPRI
        dsb
        isb
    }
}
void vPortEnterCritical(void);
// 不带返回值的进临界段函数, 不能嵌套, 不能在中断里面使用
#define portENTER_CRITICAL() vPortEnterCritical()
// 不带中断保护的开中断函数
#define portENABLE_INTERRUPTS() vPortSetBASEPRI(0)
void vPortExitCritical(void);
// 不带中断保护的退出临界段函数
#define portEXIT_CRITICAL() vPortExitCritical()

// 有中断保护关中断, 开中断, 进临界段, 出临界段
// 带返回值的关中断函数, 可以嵌套, 可以在中断里面使用
#define portSET_INTERRUPT_MASK_FROM_ISR() ulPortRaiseBASEPRI()
/**
 * @brief 带返回值的关中断函数, 可以嵌套, 可以在中断里面使用. 带返回值的意思是: 在往 BASEPRI 写入新的值的时候, 先将 BASEPRI 的值保存起来, 在更新完BASEPRI 的值的时候, 将之前保存好的 BASEPRI 的值返回, 返回的值作为形参传入开中断函数
 * @returns uint32_t ulReturn: BASEPRI的原始值
 */
static portFORCE_INLINE uint32_t ulPortRaiseBASEPRI(void)
{
    uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;

    __asm
    {
        mrs ulReturn, basepri
        msr basepri, ulNewBASEPRI
        dsb
        isb
    }
    return ulReturn;
}
// 带中断保护的开中断函数
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x) vPortSetBASEPRI(x)

/**
 * @brief 开中断函数, 将上一次关中断时保存的 BASEPRI 的值作为形参, 与 portSET_INTERRUPT_MASK_FROM_ISR()成对使用
 * @param uint32_t ulBASEPRI
 */
static portFORCE_INLINE void vPortSetBASEPRI(uint32_t ulBASEPRI)
{
    __asm
    {
        msr basepri, ulBASEPRI
    }
}



/******************************************************************************/

#endif // _PORTMACRO_H_
