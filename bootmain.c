#include "types.h"
#include "x86.h"
#include "elf.h"

#define     SECTSIZE    512
//把内核文件offset偏移处的一块(512B)内容加载到内存dst处
void readsect(void *dst, uint offset);  
//把内核文件offset偏移处的count字节的内容加载到内存dst处
void readelf(void *dst, uint offset, uint count);
int bootmain()
{
    elf_header  *elfhdr;
    elf_phdr    *phdr;
    uint        i, pnum;   
    void (*entry)(void);
    //把elf文件头和程序头部表读入到内存地址0x10000处
    elfhdr = 0x10000;
    readelf(elfhdr, 0, 4096);
    phdr = (elf_phdr*)((uchar*)elfhdr + elfhdr->e_phoff);
    pnum = elfhdr->e_phnum;

    for (i = 0; i < pnum; i++)
    {
        readelf(phdr->p_paddr, phdr->p_offset, phdr->p_filesz);
        if (phdr->p_memsz > phdr->p_filesz)
            stosb(phdr->p_paddr + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
        phdr = phdr + 1;
    }
    
    //内核入口
    entry = (void (*)(void))elfhdr->e_entry;
    entry();
    return 1;
}

//等待磁盘准备完毕
void
waitdisk(void)
{
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

void
readsect(void *dst, uint offset)
{
  waitdisk();
  outb(0x1F2, 1);   
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F7, 0x20);  

  waitdisk();
  insl(0x1F0, dst, SECTSIZE/4);
}

void readelf(void *dst, uint offset, uint count)
{
    uchar *addr;
    uint   i, secnum;
     
    addr = dst;
    secnum = (count + SECTSIZE - 1) / SECTSIZE;
     
    for (i = 0; i < secnum; i++)
    {
        readsect(addr,offset);
        addr += SECTSIZE;
        offset += SECTSIZE;
    }

    return;
}
