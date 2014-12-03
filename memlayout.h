//内存布局头文件
#include "types.h"
#ifndef MEMLAYOUT_H
#define MEMLAYOUT_H

#define EXTMEM  0x100000        //内核的起始物理地址

#define KERNBASE 0x80000000  //内核的起始虚拟地址

#define PHYSTOP  0x8000000  //物理内存的大小为128MB

#define V2P(a)  (((uint)(a)) - KERNBASE)    //虚拟地址转换为物理地址
#define P2V(a)  (((void *)(a)) + KERNBASE)  //物理地址转换为虚拟地址

#endif
