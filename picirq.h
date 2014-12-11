#ifndef PICIRQ_H
#define PICIRQ_H

#define IO_PIC1     0x20    //主IRQ的地址
#define IO_PIC2     0xA0    //从IRQ的地址

#define IRQ_SLAVE   2       //从IRQ连接在主IRQ的2号端口上

#define T_IRQ0      32      //IRQ0端口被映射到的中断号

#define IRQ_TIMER   0       //时钟中断对应的IRQ端口号
#define IRQ_KBD     1       //键盘中断
void picenable(int irqport);    //打开IRQ的相应端口
void picinit(void);         //初始化8259A中断控制器
#endif
