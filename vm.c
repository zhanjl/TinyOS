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

void
inituvm(pde_t *pgdir, char *init, uint sz)
{
  char *mem;
  
  if(sz >= PGSIZE)
    panic("inituvm: more than a page");
  mem = kalloc();
  memset(mem, 0, PGSIZE);
  mappages(pgdir, 0, PGSIZE, v2p(mem), PTE_W|PTE_U);
  memmove(mem, init, sz);
}

// Load a program segment into pgdir.  addr must be page-aligned
// and the pages from addr to addr+sz must already be mapped.
int
loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
{
  uint i, pa, n;
  pte_t *pte;

  if((uint) addr % PGSIZE != 0)
    panic("loaduvm: addr must be page aligned");
  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, addr+i, 0)) == 0)
      panic("loaduvm: address should exist");
    pa = PTE_ADDR(*pte);
    if(sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;
    if(readi(ip, p2v(pa), offset+i, n) != n)
      return -1;
  }
  return 0;
}

// Allocate page tables and physical memory to grow process from oldsz to
// newsz, which need not be page aligned.  Returns new size or 0 on error.
int
allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  char *mem;
  uint a;

  if(newsz >= KERNBASE)
    return 0;
  if(newsz < oldsz)
    return oldsz;

  a = PGROUNDUP(oldsz);
  for(; a < newsz; a += PGSIZE){
    mem = kalloc();
    if(mem == 0){
      cprintf("allocuvm out of memory\n");
      deallocuvm(pgdir, newsz, oldsz);
      return 0;
    }
    memset(mem, 0, PGSIZE);
    mappages(pgdir, (char*)a, PGSIZE, v2p(mem), PTE_W|PTE_U);
  }
  return newsz;
}

// Deallocate user pages to bring the process size from oldsz to
// newsz.  oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz.  oldsz can be larger than the actual
// process size.  Returns the new process size.
int
deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
{
  pte_t *pte;
  uint a, pa;

  if(newsz >= oldsz)
    return oldsz;

  a = PGROUNDUP(newsz);
  for(; a  < oldsz; a += PGSIZE){
    pte = walkpgdir(pgdir, (char*)a, 0);
    if(!pte)
      a += (NPTENTRIES - 1) * PGSIZE;
    else if((*pte & PTE_P) != 0){
      pa = PTE_ADDR(*pte);
      if(pa == 0)
        panic("kfree");
      char *v = p2v(pa);
      kfree(v);
      *pte = 0;
    }
  }
  return newsz;
}

// Free a page table and all the physical memory pages
// in the user part.
void
freevm(pde_t *pgdir)
{
  uint i;

  if(pgdir == 0)
    panic("freevm: no pgdir");
  deallocuvm(pgdir, KERNBASE, 0);
  for(i = 0; i < NPDENTRIES; i++){
    if(pgdir[i] & PTE_P){
      char * v = p2v(PTE_ADDR(pgdir[i]));
      kfree(v);
    }
  }
  kfree((char*)pgdir);
}

// Clear PTE_U on a page. Used to create an inaccessible
// page beneath the user stack.
void
clearpteu(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if(pte == 0)
    panic("clearpteu");
  *pte &= ~PTE_U;
}

// Given a parent process's page table, create a copy
// of it for a child.
pde_t*
copyuvm(pde_t *pgdir, uint sz)
{
  pde_t *d;
  pte_t *pte;
  uint pa, i, flags;
  char *mem;

  if((d = setupkvm()) == 0)
    return 0;
  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, (void *) i, 0)) == 0)
      panic("copyuvm: pte should exist");
    if(!(*pte & PTE_P))
      panic("copyuvm: page not present");
    pa = PTE_ADDR(*pte);
    flags = PTE_FLAGS(*pte);
    if((mem = kalloc()) == 0)
      goto bad;
    memmove(mem, (char*)p2v(pa), PGSIZE);
    if(mappages(d, (void*)i, PGSIZE, v2p(mem), flags) < 0)
      goto bad;
  }
  return d;

bad:
  freevm(d);
  return 0;
}

//PAGEBREAK!
// Map user virtual address to kernel address.
char*
uva2ka(pde_t *pgdir, char *uva)
{
  pte_t *pte;

  pte = walkpgdir(pgdir, uva, 0);
  if((*pte & PTE_P) == 0)
    return 0;
  if((*pte & PTE_U) == 0)
    return 0;
  return (char*)p2v(PTE_ADDR(*pte));
}

// Copy len bytes from p to user address va in page table pgdir.
// Most useful when pgdir is not the current page table.
// uva2ka ensures this only works for PTE_U pages.
int
copyout(pde_t *pgdir, uint va, void *p, uint len)
{
  char *buf, *pa0;
  uint n, va0;

  buf = (char*)p;
  while(len > 0){
    va0 = (uint)PGROUNDDOWN(va);
    pa0 = uva2ka(pgdir, (char*)va0);
    if(pa0 == 0)
      return -1;
    n = PGSIZE - (va - va0);
    if(n > len)
      n = len;
    memmove(pa0 + (va - va0), buf, n);
    len -= n;
    buf += n;
    va = va0 + PGSIZE;
  }
  return 0;
}

