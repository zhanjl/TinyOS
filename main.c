#include "types.h"
#include "mmu.h"
#include "memlayout.h"
#include "vm.h"
#include "monitor.h"
#include "picirq.h"
#include "traps.h"
#include "ide.h"
#include "buf.h"
#include "timer.h"
#include "proc.h"
extern char end[];
//定义供内核人口(entry.s)使用的页表
__attribute__((__aligned__(PGSIZE)))
pde_t   entrypgdir[NPDENTRIES] = {
    //0 - 4MB映射到0 - 4MB
    [0] = (0) | PTE_P | PTE_W | PTE_PS,
    //KERNBASE - KERNBASE + 4MB映射到 0 - 4MB
    [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};

static void mpmain(void) __attribute__((noreturn));

int main()
{
    //供内核使用的堆区只有end - 4MB区间 
    kinit(end, P2V(1024 * 1024 * 4));
    //为内核分配一张新的页表，并切换到该页表
    kvmalloc(); 
    //重新设置GDT表，包括用户代码段和用户数据段
    seginit();
    monitor_clear();               //清除屏幕
    printf("success %s\n", "siginit()");

    picinit();  //初始化IRQ芯片
    consoleinit();  //初始化显示设备
    tvinit();       //设置IDT表 具体的中断处理程序还没加上
    ideinit();      //初始化磁盘IO
    binit();        //初始化磁盘缓冲区
    timerinit();    //打开时钟中断
    userinit();     //设置第一个用户进程
    mpmain();       //开始运行进程
}

static void mpmain(void)
{
    idtinit();          //加载idt寄存器
    scheduler();        //开始运行进程
}
