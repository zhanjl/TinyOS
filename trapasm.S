#include "mmu.h"

.globl  alltraps

alltraps:
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    pushal

    movw $(SEG_KDATA<<3), %ax
    movw %ax, %ds
    movw %ax, %es

    pushl %esp
    call trap   #真正的中断处理程序
    addl $4, %esp

.globl trapret
trapret:
    popal
    popl %gs
    popl %fs
    popl %es
    popl %ds
    addl $0x8, %esp #跳过中断号和错误码
    iret
