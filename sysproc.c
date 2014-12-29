#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

extern struct proc *curproc;    //指向当前进程
extern uint ticks;              //时钟数
int sys_fork(void)
{
    return fork();
}

int sys_exit(void)
{
    exit();
    return 0;
}

int sys_wait(void)
{
    return wait();
}

int sys_kill(void)
{
    int pid;
    if (argint(0, &pid) < 0)
        return -1;
    return kill(pid);
}

int sys_getpid(void)
{
    return curproc->pid;
}

int sys_sbrk(void)
{
    int addr;
    int n;
    if (argint(0, &n) < 0)
        return -1;
    addr = curproc->sz;
    if (growproc(n) < 0)
        return -1;
    return addr;
}

int sys_sleep(void)
{
    int n;
    uint ticks0;
    ticks0 = ticks;
    if (argint(0, &n) < 0)
        return -1;
    while (ticks - ticks < n)
    {
        if (curproc->killed)
            return -1;
        sleep(&ticks);
    }
    return 0;
}

int sys_uptime(void)
{
    uint    xticks;

    xticks = ticks;
    return xticks;
}
