#ifndef _RTOS_CONFIG_H_
#define _RTOS_CONFIG_H_

#define configUSE_16_BIT_TICKS 0
#define configMAX_TASK_NAME_LEN 16
#define configSUPPORT_STATIC_ALLOCATION 1
// 最大任务优先级, 默认定义为 5, 最大支持 256 个优先级
#define configMAX_PRIORITIES 5

#endif // _RTOS_CONFIG_H_
