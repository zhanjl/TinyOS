#include "traps.h"
#include "mmu.h"
#include "x86.h"
#include "ide.h"
#include "types.h"
struct gatedesc idt[256];
extern uint vectors[];
uint ticks = 0;
//设置IDT表
void tvinit(void)
{
    int i;
    for (i = 0; i < 256; i++)
        SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
    SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
}
//真正的中断处理程序，不同的中断分别进行相应的处理
void trap(struct trapframe *tf)
{
    int intrnum;    //中断号
    intrnum = tf->trapno;
    //根据intrnum的值分别进行处理
    switch(intrnum) {
        case T_IRQ0 + IRQ_IDE:
            ideintr();
            break;
        case T_IRQ0 + IRQ_TIMER:    //时钟中断
            ticks++;
            //
            break;
    }
}

//加载idt表
void idtinit(void)
{
    lidt(idt, sizeof(idt));
}
