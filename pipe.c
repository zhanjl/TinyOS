#include "pipe.h"
#include "types.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "file.h"
#include "kalloc.h"

extern struct proc *curproc;    //当前进程
int pipealloc(struct file **f0, struct file **f1)
{
    struct pipe *p;

    p = 0;
    *f0 = *f1 = 0;
    if ((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
        goto bad;
    if ((p = (struct pipe*)kalloc()) == 0)
        goto bad;
    p->readopen = 1;
    p->writeopen = 1;
    p->nwrite = 0;
    p->nread = 0;

    (*f0)->type = FD_PIPE;
    (*f0)->readable = 1;
    (*f0)->writeable = 0;
    (*f0)->pipe = p;

    (*f1)->type = FD_PIPE;
    (*f1)->readable = 1;
    (*f1)->writeable = 0;
    (*f1)->pipe = p;
    return 0;
bad:
    if (p)
        kfree((char*)p);
    if (*f0)
        fileclose(*f0);
    if (*f1)
        fileclose(*f1);
    return -1;
}

void pipeclose(struct pipe *p, int write)
{
    if (write)
    {
        p->writeopen = 0;
        wakeup(&p->nread);
    }
    else
    {
        p->readopen = 0;
        wakeup(&p->nwrite);
    }

    if (p->readopen == 0 && p->writeopen == 0)
    {
        kfree((char*)p);
    }
}

int pipewrite(struct pipe *p, char *addr, int n)
{
    int i;

    for (i = 0; i < n; i++)
    {
        while (p->nwrite == p->nread + PIPESIZE)
        {
            if (p->readopen == 0 || curproc->killed)
                return -1;
            wakeup(&p->nread);  //唤醒读进程
            sleep(&p->nwrite);  //写进程睡眠
        }
        p->data[p->nwrite++ % PIPESIZE] = addr[i];
    }
    
    wakeup(&p->read);
    return n;
}

int piperead(struct pipe *p, char *addr, int n)
{
    int     i;
    while (p->nread == p->nwrite && p->writeopen)
    {
        if (curproc->killed)
            return -1;
        sleep(&p->nread);
    }

    for (i = 0; i < n; i++)
    {
        if (p->nread == p->nwrite)
            break;
        addr[i] = p->data[p->nread++ % PIPESIZE];
    }
    wakeup(&p->nwrite); //唤醒写管道的进程
    return i;
}
