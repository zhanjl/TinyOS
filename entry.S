#把_start设为entry处的物理地址，因为此时分页机制还没打开，不能使用
#虚拟地址
#include "param.h"
#include "mmu.h"
#include "memlayout.h"

.globl _start
_start = V2P_WO(entry)


.globl entry
entry:
  #把一页大小设置为4MB
  movl    %cr4, %eax
  orl     $(CR4_PSE), %eax
  movl    %eax, %cr4
  #加载页目录表的物理地址，页目录表在main.c中定义
  movl    $(V2P_WO(entrypgdir)), %eax
  movl    %eax, %cr3
  # 打开分页机制
  movl    %cr0, %eax
  orl     $(CR0_PG|CR0_WP), %eax
  movl    %eax, %cr0

  # 设置栈指针
  movl $(stack + KSTACKSIZE), %esp

  
  # 跳到main函数处开始执行
  mov $main, %eax
  jmp *%eax
  #定义栈指针
  .comm stack, KSTACKSIZE
