#include "proc.h"
#include "param.h"
#include "x86.h"
#include "types.h"
#include "mmu.h"
#include "memlayout.h"
#include "kalloc.h"
#include "mem.h"
#include "vm.h"
#include "monitor.h"

void forkret(void);         //context中eip的值，用于返回用户态
extern void trapret(void);  //forkret中会执行这个程序，共同完成返回用户态的工作
struct proc proc[NPROC];    //系统进程结构体表
int nextpid = 1;        //下一个可用的进程号
struct proc *initproc;  //第一个进程
//初始化进程
void pinit(void)
{
    //本内核还没有实现锁，所以该函数不用做任何工作
}

static struct proc *
allocproc(void)
{
    struct proc *p;
    char *sp;
    int i;
    for (i = 0; i < NPROC; i++)
    {
        if (proc[i].state == UNUSED)
            break;
    }
    if (i == NPROC)
        return 0;
    p = &proc[i];
    p->state = EMBRYO;
    p->pid = nextpid++;

    //分配内核栈
    if ((p->kstack = kalloc()) == 0)
    {
        p->state = UNUSED;
        return 0;
    }
     
    sp = p->kstack + KSTACKSIZE;
     
    //分配存储trapframe的空间
    sp -= sizeof *p->tf;
    p->tf = (struct trapframe*)sp;
     
    //存储trapret函数指针
    sp -= 4;
    *(uint*)sp = (uint)trapret;
     
    //存储context 
    sp -= sizeof *p->context;
    p->context = (struct context*)sp;
    memset(p->context, 0, sizeof *p->context);
     
    p->context->eip = (uint)forkret;
    //只要执行了forkret函数，就会从内核态返回用户态继续执行程序
    //这个过程安排的很精巧
    return p;
}

//设置第一个进程的进程结构体
//第一个进程执行initcode.S的start入口函数
void userinit(void)
{
    struct proc *p;
    extern char _binary_initcode_start[], _binary_initcode_size[];

    p = allocproc();
    initproc = p;
    if ((p->pgdir = setupkvm()) == 0)
        PANIC("out of memory");

    inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
    p->sz = PGSIZE;

    memset(p->tf, 0, sizeof(*p->tf));

    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->esp = PGSIZE;    //用户栈
    p->tf->eip = 0;         //用户态代码地址

    p->cwd = namei("/");
    p->state = RUNNABLE;
}
