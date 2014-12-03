#include "kalloc.h"
#include "mmu.h"

extern char end[];  //内核区域的最后一个地址
struct run {
    struct run *next;
};

struct {
    struct run *freelist;
} kmem;

void kinit(void *vstart, void *vend)
{
    char *p = PGROUNDUP(vstart);
    struct run *r;
    for (; p + PGSIZE <= vend; p += PGSIZE)
    {
        r = (struct run*)p;
        r->next = kmem->freelist;
        kmem->freelist = r;
    }
}

void kfree(char *v)
{
    if (v % PGSIZE || v <= end)
        while (1) ;
        //panic
    struct run *r;
    r = (struct run*)v;
    r->next = kmem->freelist;
    kmem->freelist = r;

    return;
}

void *kalloc(void)
{
    struct run *r = kmem->freelist;

    if (r)
    {
        kmem->freelist = r->next;
    }
    return (char*)r;
}
