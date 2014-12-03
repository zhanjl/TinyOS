#ifndef KALLOC_H
#define KALLOC_H
//供内核使用的堆分配函数
//初始化vstart到vend之间的内存段
void kinit(void *vstart, void *vend);   
//释放一块内存
void kfree(char *v);
//分配一块内存
char *kalloc(void);
#endif
