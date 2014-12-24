#ifndef TIMER_H
#define TIMER_H

#define IO_TIMER1       0x040       //8253

#define TIMER_FREQ      1193182
#define TIMER_DIV(x)    ((TIMER_FREQ + (x) / 2) / (x))

#define TIMER_MODE      (IO_TIMER1 + 3)
#define TIMER_SEL0      0x00
#define TIMER_RATEGEN   0x04
#define TIMER_16BIT     0x30

void timerinit(void);

#endif
