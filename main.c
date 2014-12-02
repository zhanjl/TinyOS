#include "types.h"
#include "mmu.h"
#include "memlayout.h"


//定义供内核人口(entry.s)使用的页表
__attribute__((__aligned__(PGSIZE)))
pde_t   entrypgdir[NPDENTRIES] = {
    //0 - 4MB映射到0 - 4MB
    [0] = (0) | PTE_P | PTE_W | PTE_PS,
    //KERNBASE - KERNBASE + 4MB映射到 0 - 4MB
    [KERNBASE>>PDXSHIFT] = (0) | PTE_P | PTE_W | PTE_PS,
};
int main()
{
    return 0;
}

