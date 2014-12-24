#include "timer.h"
#include "picirq.h"
#include "x86.h"

void timerinit(void)
{
    outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
    outb(IO_TIMER1, TIMER_DIV(100) % 256);
    outb(IO_TIMER1, TIMER_DIV(100) / 256);
    picenable(IRQ_TIMER);
}
