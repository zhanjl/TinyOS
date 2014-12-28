#ifndef PROC_H
#define PROC_H
#include "types.h"
#include "x86.h"
#include "traps.h"
#include "file.h"
#include "param.h"
//保存本进程寄存器的值，段寄存器没必要保存，因为内核使用的是同一个内存段
struct context {
    uint    edi;
    uint    esi; 
    uint    ebx;
    uint    ebp;
    uint    eip;
};

//进程的状态
enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE};

//进程结构体
struct proc {
    uint    sz;     //占用物理内存的大小
    pde_t   *pgdir; //页目录表地址
    char    *kstack; //内核栈的栈顶地址
    enum procstate state;   //进程状态
    int     pid;     //进程号
    struct proc *parent;    //父进程
    struct trapframe *tf;   //系统调用时要使用，保存用户态的寄存器
    struct context *context;//进程调度时要使用，保存本进程的相关寄存器
    int killed;             //如果非0,表示本进程被killed
    struct file *ofile[NFILE];//打开的文件
    struct inode *cwd;      //当前目录
    char    *chan;          //sleep链
};



void pinit(void);   //进程初始化函数

void userinit(void);//设置第一个用户进程的进程结构体
void scheduler(void);   //调度器，选择要切换的进程

void sched(void);   //切换到选择的进程
void yield(void);   //把当前进程设置为RUNNABLE，并切换到另一个进程
void sleep(void *chan); //把当前进程设置为SLEEPING状态并切换到另一个进程
void wakeup(void *chan);//把所有sleep在chan上的进程状态设置为RUNNABLE。
#endif
