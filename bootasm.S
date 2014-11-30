#加载器被BIOS加载到0x7c00地址处，此时cs = 0, eip = 7c00。其他寄存器的值
#不关心

#include "asm.h"
#include "mmu.h"
.code16
.global start
start:
    cli

    xorw    %ax, %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss

#打开A20总线，这是个历史遗留问题
seta20.1:
    inb     $0x64,%al
    testb   $0x2,%al
    jnz     seta20.1

    movb    $0xd1,%al
    outb    %al,$0x64

seta20.2:
    inb     $0x64,%al
    testb   $0x2,%al
    jnz     seta20.2

    movb    $0xdf,%al
    outb    %al,$0x60

#进入保护模式
    lgdt    gdtdesc
    movl    %cr0, %eax
    orl     $CR0_PE, %eax
    movl    %eax, %cr0

    ljmp    $(SEG_KCODE<<3), $start32

#保护模式代码
.code32
start32:
#设置段寄存器的值 
    movw    $(SEG_KDATA<<3), %ax
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    $0, %ax
    movw    %ax, %fs
    movw    %ax, %gs

#设置栈指针
    movl    $start, %esp
    call    bootmain

#不应该从bootmain函数中返回，如果返回了，进入无限循环

spin:
    jmp     spin
#加载器使用的GDT
.p2align    2
gdt:
    SEG_NULLASM
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   #代码段 
    SEG_ASM(STA_W, 0x0, 0xffffffff)         #数据段

gdtdesc:
    .word   (gdtdesc - gdt - 1)
    .long   gdt
