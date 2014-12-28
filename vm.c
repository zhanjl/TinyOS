#include "types.h"
#include "vm.h"
#include "param.h"
#include "mmu.h"
#include "memlayout.h"
#include "x86.h"
#include "asm.h"
#include "mem.h"
#include "monitor.h"
#include "proc.h"
extern char data[];     //内核数据段起始地址
pde_t   *kpgdir;        //内核页目录表起始地址

struct segdesc gdt[NSEGS];

//初始化GDT表
void seginit(void)
{
    gdt[SEG_KCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, 0);
    gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
    gdt[SEG_UCODE] = SEG(STA_X | STA_R, 0, 0xffffffff, DPL_USER);
    gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);

    lgdt(gdt, sizeof(gdt));
}
//返回某个虚拟地址对应的页表项
//页表项如果不存在，分配一张页表(alloc == 1)或者直接返回0(alloc == 0)。
static pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
    pde_t *pde;
    pte_t *pte;

    pde = &pgdir[PDX(va)];
    if ((*pde) & PTE_P) {
        pte = (pte_t*)P2V(PTE_ADDR(*pde));
    } else {
        if (!alloc || (pte = kalloc()) == 0)
            return 0;

        memset(pte, 0, PGSIZE);
        *pde = V2P(pte) | PTE_P | PTE_W;
    }
    return &pte[PTX(va)];

}

//把从虚拟地址va开始的size大小地址段映射到物理起始地址pa处
static int
mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
{
    char *start, *end;
    start = (char*)PGROUNDDOWN((uint)va);
    end = (char*)PGROUNDDOWN((uint)va + size - 1);
    pa = PGROUNDDOWN(pa);
    pte_t *pte;
    while (start <= end)
    {
        pte = walkpgdir(pgdir, start, 1);
        if (pte == 0)
            return -1;
        //该页表项已经映射了物理内存
        if (*pte & PTE_P)
            return -1;

       *pte = pa | PTE_P | perm;
        
        start += PGSIZE;
        pa += PGSIZE;
    } 
    return 0;
}

//内核区划分成的内存段，不同的段有不同的读写权限

static struct kmap {
    void *vstart;
    uint  pstart;
    uint  pend;
    int   perm;
} kmap[] = {
    {KERNBASE, 0, EXTMEM, PTE_W},   //I/O空间
    {KERNBASE+EXTMEM, EXTMEM, V2P(data), 0}, //内核代码段
    {data, V2P(data), PHYSTOP, PTE_W}   //内核数据段和剩余的内存段
};

//为内核分配一张页表
pde_t* setupkvm(void)
{
    pde_t *pgdir;
    pgdir = (uint)kalloc();
    if (pgdir == 0)
        return 0;
    memset(pgdir, 0, PGSIZE);
    int i;
    for (i = 0; i < 3; i++)
    {
        if (mappages(pgdir, kmap[i].vstart, 
                    kmap[i].pend - kmap[i].pstart,
                    kmap[i].pstart, kmap[i].perm) < 0)
            return 0;
    }
    return pgdir;
}

static void switchkvm()
{
    lcr3(V2P(kpgdir));
}

//为内核分配页表，并切换到该页表
void kvmalloc(void)
{
    kpgdir = setupkvm();
    switchkvm();
}
//
void inituvm(pde_t *pgdir, char *init, uint sz)
{
    char    *mem;

    if (sz > PGSIZE)
        PANIC("bigger than a page");
    mem = kalloc();
    memset(mem, 0, PGSIZE);
    mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W | PTE_U);
    memcpy(mem, init, sz);
}

//切换到内核页表
void switchkvm(void)
{
    lcr3(V2P(kpgdir));
}

//切换到进程p的页表
void switchuvm(struct proc *p)
{
    ltr(SEG_TSS << 3);
    lcr3(V2P(p->pgdir));
}
