#ifndef TRAPS_H
#define TRAPS_H

#define T_PGFLT     14      //页错误中断号

#define T_SYSCALL   128     //系统调用中断号0x80

#define T_IRQ0      32      //IRQ0被映射的中断号
#define IRQ_TIMER   0     //时钟对应的IRQ号

#define IRQ_KBD     1       //键盘对应的IRQ号
#endif
