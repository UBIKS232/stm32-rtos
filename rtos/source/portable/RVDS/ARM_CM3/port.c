#include "portmacro.h"
#include "projectdefs.h"

#define portINITIAL_XPSR (0x01000000L)
#define portSTART_ADDRESS_MASK ((StackType_t)0xfffffffeUL)

/**
 * @brief 异常发生之后, 任务函数最终进入的函数
 */
static void prvtaskExitError(void)
{

    for (;;)
        ;
}

/**
 * @brief 任务栈初始化, 构建异常上下文, 与体系结构相关!
 * @param StackType_t *pxTopOfStack: 栈顶
 * @param TaskFuntion_t pxCode: 任务入口
 * @param void *pvParameters: 任务形参
 * @returns StackType_t pxTopOfStack: 此时 pxTopOfStack 指向空闲栈
 */
StackType_t pxPortInitialiseStack(StackType_t *pxTopOfStack,
                                  TaskFuntion_t pxCode,
                                  void *pvParameters)
{
    // 注意:
    // Cortex-M3 的栈是由高地址向低地址生长的,
    // 因此可以看到任务栈在初始化时 pxTopOfStack 这个指针一直在递减,
    // 而堆是从低地址向高地址生长的,
    // pxTopOfStack 这些变量仅仅表示指针, 并不一定表示 pxTopOfStack 就一定在高地址.
    // 异常: 中断,系统调用,任务切换等
    // 异常发生时, 以下栈中写入的内容(异常上下文)会被自动加载到 CPU 寄存器内:
    // CPU 自动从栈中加载到 CPU 寄存器的内容,
    // 包括 8 个寄存器, 分别为 R0,R1,R2,R3,R12,R14,R15 和 xPSR 的位 24
    pxTopOfStack--;
    // xPSR 的第 24 位, 必须置 1
    *pxTopOfStack = portINITIAL_XPSR;
    pxTopOfStack--;
    // R15(PC) 指向任务函数, 即任务入口地址
    *pxTopOfStack = ((StackType_t)pxCode) & portSTART_ADDRESS_MASK;
    pxTopOfStack--;
    // R14(LR) 任务的异常返回地址, 通常任务是不会返回的,
    // 如果返回了就跳转到 prvTaskExitError, 该函数是一个无限循环
    *pxTopOfStack = (StackType_t)prvtaskExitError;
    // R12, R3, R2 and R1 默认初始化为 0
    pxTopOfStack -= 5;
    // R0
    *pxTopOfStack = (StackType_t)pvParameters;

    // 异常发生时, 以下栈中写入内容被手动加载到 CPU 寄存器内
    // 即软件弹栈
    pxTopOfStack -= 8;

    // 返回栈顶指针, 此时 pxTopOfStack 指向栈中代表 R4 的位置
    return pxTopOfStack;
}

/**
 * @brief 1. 更新 MSP 的值,
 * @brief 2. 产生 SVC 系统调用, 然后去到 SVC 的中断服务函数里面真正切换到第一个任务
 */
static void prvStartFirstTask(void)
{
    // ARM Cortex-M3 TRM:
    // ARM Cortex-M System Control Space (SCS) Register Map
    // Base: 0xE000E000
    // Address      Name    Type        Reset Value     Description
    // ──────────────────────────────────────────────────────────────────────────────
    // 0xE000E008   ACTLR   RW          0x00000000      Auxiliary Control Register
    // 0xE000E010   STCSR   RW          0x00000000      SysTick Control and Status Register
    // 0xE000E014   STRVR   RW          Unknown         SysTick Reload Value Register
    // 0xE000E018   STCVR   RW (clear)  Unknown         SysTick Current Value Register
    // 0xE000E01C   STCR    RO          Impl. spec.     SysTick Calibration Value Register
    // ──────────────────────────────────────────────────────────────────────────────
    // Standard SCB Registers (Base: 0xE000ED00)
    // ──────────────────────────────────────────────────────────────────────────────
    // 0xE000ED00   CPUID   RO          0x412FC231      CPUID Base Register
    // 0xE000ED04   ICSR    RW/RO       0x00000000      Interrupt Control and State Register
    // 0xE000ED08   VTOR    RW          0x00000000      Vector Table Offset Register
    // 0xE000ED0C   AIRCR   RW          0x00000000      Application Interrupt and Reset Control Register
    // 0xE000ED10   SCR     RW          0x00000000      System Control Register
    // 0xE000ED14   CCR     RW          0x00000200      Configuration and Control Register
    // 0xE000ED18   SHPR1   RW          0x00000000      System Handler Priority Register 1
    // 0xE000ED1C   SHPR2   RW          0x00000000      System Handler Priority Register 2
    // 0xE000ED20   SHPR3   RW          0x00000000      System Handler Priority Register 3
    // 0xE000ED24   SHCSR   RW          0x00000000      System Handler Control and State Register
    // 0xE000ED28   CFSR    RW          0x00000000      Configurable Fault Status Register
    // 0xE000ED2C   HFSR    RW          0x00000000      HardFault Status Register
    // 0xE000ED30   DFSR    RW          0x00000000      Debug Fault Status Register
    // 0xE000ED34   MMFAR   RW          Unknown         MemManage Fault Address Register
    // 0xE000ED38   BFAR    RW          Unknown         BusFault Address Register
    // 0xE000ED3C   AFSR    RW          0x00000000      Auxiliary Fault Status Register
    // 0xE000ED40   ID_PFR0 RO          0x00000030      Processor Feature Register 0
    // 0xE000ED44   ID_PFR1 RO          0x00000200      Processor Feature Register 1
    // 0xE000ED48   ID_DFR0 RO          0x00100000      Debug Features Register 0
    // 0xE000ED4C   ID_AFR0 RO          0x00000000      Auxiliary Features Register 0
    // PM0056:
    // 1. SCB_VTOR
    //      Vector table offset register (SCB_VTOR)
    //      Address offset: 0x08
    //      Reset value: 0x0000 0000
    //      Required privilege: Privileged
    //      Bits 31:30 Reserved, must be kept cleared
    //      Bits 29:9 TBLOFF[29:9]: Vector table base offset field.
    //           It contains bits [29:9] of the offset of the table base
    //           from memory address 0x00000000.
    //           When setting TBLOFF, you must align the offset
    //           to the number of exception entries in the vector table.
    //           The minimum alignment is 128 words.
    //           Table alignment requirements mean that
    //           bits[8:0] of the table offset are always zero.
    //           Bit 29 determines
    //           whether the vector table is in the code or SRAM memory region.
    //              0: Code
    //              1: SRAM
    //           Note: Bit 29 is sometimes called the TBLBASE bit.
    //       Bits 8:0 Reserved, must be kept cleared
    // 2. MSP: Main Stack Pointer
    //      The Stack Pointer (SP) is register R13.
    //      In Thread mode, bit[1] of the CONTROL register
    //      indicates the stack pointer to use:
    //           0 = Main Stack Pointer (MSP). This is the reset value.
    //           1 = Process Stack Pointer (PSP).
    //      On reset,
    //      the processor loads the MSP with the value from address 0x00000000.
    // 3. SVCall
    //      A supervisor call (SVC) is an exception
    //      that is triggered by the SVC instruction.
    //      In an OS environment, applications can use SVC instructions
    //      to access OS kernel functions and device drivers.
    // 4. PendSV
    //      PendSV is an interrupt-driven request for system-level service.
    //      In an OS environment, use PendSV for context switching when no other
    //      exception is active.
    // 5. SysTick
    //      A SysTick exception is an exception the system timer generates
    //      when it reaches zero. Software can also generate a SysTick
    //      exception. In an OS environment, the processor can use this
    //      exception as system tick.
    // 6. Vector table
    //      The vector table contains the reset value of the stack pointer,
    //      and the start addresses, also called exception vectors,
    //      for all exception handlers. Figure 12 on page 35 shows the order
    //      of the exception vectors in the vector table.
    //      The least-significant bit of each vector must be 1,
    //      indicating that the exception handler is Thumb code.
    //      On system reset, the vector table is fixed at address 0x00000000.
    //      Privileged software can write to the VTOR to relocate
    //      the vector table start address to a different memory location,
    //      in the range 0x00000080 to 0x3FFFFF80,
    //      see Vector table offset register (SCB_VTOR) on page 133.
    __asm volatile(
        /* 当前栈需按照 8 字节对齐 */
        "PRESERVE8\n"
        /*
         * 在Cortex-M 中, 0xE000ED08 是 SCB_VTOR 寄存器的地址,
         * 里面存放的是向量表的起始地址, 即 MSP 的地址.
         * 向量表通常是从内部 FLASH 的起始地址开始存放,
         * 那么可知 memory：0x00000000 处存放的就是 MSP 的值.
         * 这个可以通过仿真时查看内存的值证实.
         */
        /* 将 0xE000ED08 这个立即数加载到寄存器 R0 */
        "ldr r0, =0xE000ED08\n"
        /*
         * 将 0xE000ED08 这个地址指向的内容加载到寄存器 R0,
         * 此时 R0 等于 SCB_VTOR 寄存器的值, 等于 0x00000000,
         * 即 memory 的起始地址
         */
        "ldr r0, [r0]\n"
        /* 将 0x00000000 这个地址指向的内容加载到 R0 */
        "ldr r0, [r0]\n"
        /* 设置主堆栈指针 msp 的值 */
        "msr msp, r0\n"
        /* 使能全局中断 */
        "cpsie i\n"
        "cpsie f\n"
        "dsb\n"
        "isb\n"
        /* 调用 SVC 去启动第一个任务 */
        "svc 0\n"
        "nop\n"
        "nop\n"
    );
}

// 按照startup中的向量表重新定义函数的名字
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickSVHandler SysTick_Handler
#define vPortSVCHandler SVC_Handler

/**
 * @brief vPortSVCHandler()函数开始真正启动第一个任务,  不再返回
 */
static void vPortSVCHandler(void)
{
    extern pxCurrentTCB;

    __asm volatile(
        "PRESERVE8\n"
        "ldr r3, =pxCurrentTCB\n"
        "ldr r1, [r3]\n"
        "ldr r0, [r1]\n" /* r0 = pxTopOfStack */
        "ldr r1, [r3]\n"
        "ldr r0, [r1]\n"
        "ldmia r0!, {r4-r11}\n" /* 以 r0 为基地址, 将栈中向上增长的 8 个字的内容加载到 CPU 寄存器 r4~r11, 同时 r0 也会跟着自增 */
        "msr psp, r0\n"         /* 将新的栈顶指针 r0 更新到 psp, 任务执行的时候使用的堆栈指针是 psp, 此时 psp(即pxTopOfStack) 指向原初始化好的 task stack 中代表 R4 的位置的下一位 */
        "isb\n"
        "mov r0, #0\n"
        "msr basepri, r0\n" /* 设置 basepri 寄存器的值为 0, 即打开所有中断. basepri 是一个中断屏蔽寄存器, 大于等于此寄存器值的中断都将被屏蔽 */
        "orr r14, #0xd\n"   /* ??在 SVC 中断服务里面, 使用的是 MSP 堆栈指针, 是处在 ARM 状态, 当从 SVC 中断服务退出前, 通过向 r14 寄存器最后 4 位按位或上0x0D, 使得硬件在退出时使用进程堆栈指针 PSP 完成出栈操作并返回后进入任务模式, 返回 Thumb 状态. 当 r14 为 0xFFFFFFFX时执行中断返回指令(bx r14), 按照cortext-m3 的做法, X 的 bit0 为 1 表示 返回 thumb 状态, bit1 和 bit2 分别表示返回后 sp 用 msp 还是 psp、以及返回到特权模式还是用户模式 */
        "bx r14\n"/* ??异常返回, 这个时候出栈使用的是 PSP 指针, 自动将栈中的剩下内容加载到 CPU 寄存器： xPSR, PC(任务入口地址), R14, R12, R3, R2, R1, R0(任务的形参)同时 PSP 的值也将更新, 即指向任务栈的栈顶 */
    );
}

// PM0056:
// The SHPR1-SHPR3 registers set the priority level,
// 0 to 15 of the exception handlers that have configurable priority.
// SHPR1-SHPR3 are byte accessible.
// Each PRI_N field is 8 bits wide,
// but the processor implements only bits[7:4] of each field,
// and bits[3:0] read as zero and ignore writes.
// System handler priority register 3 (SCB_SHPR3)
// Address: 0xE000 ED20
// Reset value: 0x0000 0000
// Required privilege: Privileged
// Bits 31:24 PRI_15[7:0]: Priority of system handler 15, SysTick exception
// Bits 23:16 PRI_14[7:0]: Priority of system handler 14, PendSV
// Bits 15:0 Reserved, must be kept cleared
#define portNVIC_SYSPRI2_REG (*((volatile uint32_t *)0xe000ed20))
#define portNVIC_PENDSV_PRI (((uint32_t)configKERNEL_INTERRUPT_PRIORITY) << 16UL)
#define portNVIC_SYSTICK_PRI (((uint32_t)configKERNEL_INTERRUPT_PRIORITY) << 24UL)

/**
 * @brief 启动调度器
 * @returns BaseType_t 0: 不应该有返回值
 */
BaseType_t xPortStartScheduler(void)
{
    // 配置 PendSV 和 SysTick 的中断优先级为最低,
    // SysTick 和 PendSV 都会涉及到系统调度, 系统调度的优先级要低于系统的其它硬件中断优先级,
    // 即优先相应系统中的外部硬件中断
    portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;
    portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;

    // 启动第一个任务, 不再返回
    prvStartFirstTask();

    // 不应该运行到这里
    return 0;
}
