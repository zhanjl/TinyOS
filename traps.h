#ifndef TRAPS_H
#define TRAPS_H
#include "types.h"
#define T_PGFLT     14      //页错误中断号

#define T_SYSCALL   128     //系统调用中断号0x80

#define T_IRQ0      32      //IRQ0被映射的中断号
#define IRQ_TIMER   0     //时钟对应的IRQ号

#define IRQ_KBD     1       //键盘对应的IRQ号



struct trapframe {
    uint nesp;
    //pushed by pusha
    uint edi;
    uint esi;
    uint ebp;
    uint oesp;
    uint ebx;
    uint edx;
    uint ecx;
    uint eax;

    ushort gs;
    ushort padding1;
    ushort fs;
    ushort padding2;
    ushort es;
    ushort padding3;
    ushort ds;
    ushort padding4;

    uint trapno;
    uint err;
    uint eip;
    ushort cs;
    ushort padding5;
    uint eflags;
    uint esp;
    ushort ss;
    ushort padding6;
};


struct gatedesc {
  uint off_15_0 : 16;   // low 16 bits of offset in segment
  uint cs : 16;         // code segment selector
  uint args : 5;        // # args, 0 for interrupt/trap gates
  uint rsv1 : 3;        // reserved(should be zero I guess)
  uint type : 4;        // type(STS_{TG,IG32,TG32})
  uint s : 1;           // must be 0 (system)
  uint dpl : 2;         // descriptor(meaning new) privilege level
  uint p : 1;           // Present
  uint off_31_16 : 16;  // high bits of offset in segment
};

// Set up a normal interrupt/trap gate descriptor.
// - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
//   interrupt gate clears FL_IF, trap gate leaves FL_IF alone
// - sel: Code segment selector for interrupt/trap handler
// - off: Offset in code segment for interrupt/trap handler
// - dpl: Descriptor Privilege Level -
//        the privilege level required for software to invoke
//        this interrupt/trap gate explicitly using an int instruction.
#define SETGATE(gate, istrap, sel, off, d)                \
{                                                         \
  (gate).off_15_0 = (uint)(off) & 0xffff;                \
  (gate).cs = (sel);                                      \
  (gate).args = 0;                                        \
  (gate).rsv1 = 0;                                        \
  (gate).type = (istrap) ? STS_TG32 : STS_IG32;           \
  (gate).s = 0;                                           \
  (gate).dpl = (d);                                       \
  (gate).p = 1;                                           \
  (gate).off_31_16 = (uint)(off) >> 16;                  \
}


void tvinit(void);  //设置IDT表
void idtinit(void); //加载IDT表
#endif
