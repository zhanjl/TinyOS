#include "traps.h"
#include "mmu.h"
#include "x86.h"
#include "ide.h"
#include "types.h"
#include "proc.h"
#include "monitor.h"
#include "lapic.h"
struct gatedesc idt[256];
extern uint vectors[];
extern struct proc *curproc;    //当前进程
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
    if (intrnum == T_SYSCALL)   //如果是系统调用
    {
        if (curproc->killed)
            exit();
        curproc->tf = tf;
        syscall();
        if (curproc->killed)
            exit();
        return;
    }
    //根据intrnum的值分别进行处理
    switch(intrnum) {
        case T_IRQ0 + IRQ_IDE:
            ideintr();
            lapiceoi();
            break;
        case T_IRQ0 + IRQ_TIMER:    //时钟中断
            ticks++;
            wakeup(&ticks);
            lapiceoi();
            break;
        default:    //缺页中断，本内核没有实现缺页中断处理函数，也没有实现磁盘页交换区
            if (curproc == 0 || (tf->cs&3) == 0)    //在内核态
            {
                printf("unexpected trap %d at %x\n", tr->trapno, rcr2());
                PANIC("trap");
            }
            //用户态
            printf("pid %d trap error at %x\n", curproc->pid, rcr2());
            curproc->killed = 1;
    }
    if (curproc && curproc->killed && (tf->cs&3) == DPL_USER)
       exit();
    //进程切换
    if (curproc && curproc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
        yield();
    //再次切换回此进程
    if (curproc && curproc->killed && (tf->cs&3) == DPL_USER)
       exit();
}

//加载idt表
void idtinit(void)
{
    lidt(idt, sizeof(idt));
}
