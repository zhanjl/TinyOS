#include "syscall.h"
#include "types.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "param.h"
#include "memlayout.h"

extern struct proc *curproc;    //当前进程
//获取地址addr处的整数值
int fetchint(uint addr, int *ip)
{
    if (addr >= proc->sz || addr+4 > proc->sz)
        return -1;
    *ip = *(int*)addr;
    return 0;
}
//获取addr地址处的字符串的值，返回字符串的长度
int fetchstr(uint addr, char **pp)
{
    char *s, *ep;
    if (addr >= proc->sz)
        return -1;
    *pp = (char*)addr;
    
    ep = proc->sz;
    for (s = *pp; s < ep; s++)
    {
        if (*s == 0)
            return s - *pp;
    }

    return -1;
}

//获取第n个32位的参数
int argint(int n, int *ip)
{
    return fetchint(curproc->tf->esp + 4 + 4*n, ip);
}

//获取第n个32位地址参数的值
int argptr(int n, char**pp, int size)
{
    int i;
    if (argint(n,&i) < 0)
        return -1;
    if ((uint)i >= proc->sz || (uint)i + size > proc->sz)
        return -1;
    *p = (char*)i;
    return 0;
}

//获取第n个32位地址参数的值，并把该地址当作字符串地址
int argstr(int n, char **pp)
{
    int addr;
    if (argint(n, &addr) < 0)
        return -1;
    return fetchstr(addr, pp);
}

//对应的系统调用函数
extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);

//系统调用函数数组，存储每个系统调用函数的地址
static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
};

void syscall(void)
{
    int num;
    num = curproc->tf->eax; //系统调用号存储在eax寄存器中
    if (num > 0 && num < 21 && syscalls[num])
    {
        curproc->tf->eax = syscalls[num]();
    }
    else
    {
        curproc->tf->eax = -1;
    }
}
