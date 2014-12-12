#include "picirq.h"
#include "x86.h"
#include "types.h"

static ushort irqmask = 0xFFFF & ~(1<<IRQ_SLAVE);

static void 
picsetmask(ushort mask)
{
    irqmask = mask;
    outb(IO_PIC1+1, mask);
    outb(IO_PIC2+1, mask >> 8);
}

void picenable(int irqport)
{
    picsetmask(irqmask & ~(1<<irqport));
}

void picinit(void)
{
    outb(IO_PIC1+1, 0xFF);
    outb(IO_PIC2+1, 0xFF);

    outb(IO_PIC1, 0x11);

    
    outb(IO_PIC1+1, T_IRQ0);

     
    outb(IO_PIC1+1, 1<<IRQ_SLAVE);

    outb(IO_PIC1+1, 0x3);

 
    outb(IO_PIC2, 0x11);                 
    outb(IO_PIC2+1, T_IRQ0 + 8);      
    outb(IO_PIC2+1, IRQ_SLAVE);           
  
    outb(IO_PIC2+1, 0x3);                 

  
    outb(IO_PIC1, 0x68);             
    outb(IO_PIC1, 0x0a);            

    outb(IO_PIC2, 0x68);            
    outb(IO_PIC2, 0x0a);            

    if(irqmask != 0xFFFF)
        picsetmask(irqmask);
}
