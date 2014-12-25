#include "timer.h"
#include "picirq.h"
#include "x86.h"

void timerinit(void)
{
    //设置时钟中断频率为每秒100次
    outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
    outb(IO_TIMER1, TIMER_DIV(100) % 256);
    outb(IO_TIMER1, TIMER_DIV(100) / 256);
    picenable(IRQ_TIMER);
}
