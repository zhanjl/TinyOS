#include "kalloc.h"
#include "mmu.h"

extern char end[];  //内核区域的最后一个地址


struct run {
    struct run *next;
};

struct freeblock {
    //可以加上锁
    struct run *freelist;
};

struct freeblock kmem;

void kinit(void *vstart, void *vend)
{
    char *p = (char*)PGROUNDUP((int)vstart);
    struct run *r;
    for (; p + PGSIZE <= (char*)vend; p += PGSIZE)
    {
        r = (struct run*)p;
        r->next = kmem.freelist;
        kmem.freelist = r;
    }
}

void kfree(char *v)
{
    struct run *r;
    r = (struct run*)v;
    r->next = kmem.freelist;
    kmem.freelist = r;

    return;
}

char *kalloc(void)
{
    struct run *r = kmem.freelist;

    if (r)
    {
        kmem.freelist = r->next;
    }
    return (char*)r;
}
