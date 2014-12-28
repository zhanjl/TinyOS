#ifndef VM_H
#define VM_H
#include "types.h"

void kvmalloc(void);    //设置并切换到内核段页表
pde_t* setupkvm(void);  //为内核分配一张页目录表

//把init起始的sz大小的内存映射到pgdir页目录表的0地址处
void seginit(void);
void inituvm(pde_t *pgdir, char *init, uint sz);
struct proc;
void switchkvm(void);   //切换到内核页表
void switchuvm(struct proc*);   //切换到某个进程的页表
#endif
