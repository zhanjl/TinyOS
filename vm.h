#ifndef VM_H
#define VM_H
#include "types.h"

void kvmalloc(void);    //设置并切换到内核段页表
pde_t* setupkvm(void);  //为内核分配一张页目录表

struct proc;
void seginit(void);//把init起始的sz大小的内存映射到pgdir页目录表的0地址处
void inituvm(pde_t *pgdir, char *init, uint sz);//把initcode映射到0地址处
void switchkvm(void);   //切换到内核页表
void switchuvm(struct proc*);   //切换到某个进程的页表
char*  uva2ka(pde_t*, char*);
int    allocuvm(pde_t*, uint, uint);
int    deallocuvm(pde_t*, uint, uint);
void   freevm(pde_t*);
void   inituvm(pde_t*, char*, uint);
int    loaduvm(pde_t*, char*, struct inode*, uint, uint);
pde_t* copyuvm(pde_t*, uint);
void   switchuvm(struct proc*);
void   switchkvm(void);
int    copyout(pde_t*, uint, void*, uint);
void   clearpteu(pde_t *pgdir, char *uva);
#endif
