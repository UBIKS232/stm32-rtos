#ifndef _RTOS_CONFIG_H_
#define _RTOS_CONFIG_H_

#define configUSE_16_BIT_TICKS 0
#define configMAX_TASK_NAME_LEN 16
#define configSUPPORT_STATIC_ALLOCATION 1
// 最大任务优先级, 默认定义为 5, 最大支持 256 个优先级
#define configMAX_PRIORITIES 5
// 配置中断屏蔽寄存器 BASEPRI 的值, 高四位有效. 目前配置为 191, 因为是四位有效, 所以实际值等于 11, 即优先级高于或者等于11 的中断都将被屏蔽.
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 191

#define configPRIO_BITS 4
// 中断最低优先级
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#endif // _RTOS_CONFIG_H_
