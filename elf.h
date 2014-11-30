#include "types.h"

#ifndef ELF_H
#define ELF_H

//ELF头部结构
typedef struct {
    uchar       e_ident[16];    //目标文件标识信息
    ushort      e_type;         
    ushort      e_machine;
    uint        e_version;
    uint        e_entry;
    uint        e_phoff;
    uint        e_shoff;
    uint        e_flags;
    ushort      e_ehsize;
    ushort      e_phentsize;
    ushort      e_phnum;
    ushort      e_shentisze;
    ushort      e_shnum;
    ushort      e_shstrndx;
} elf_header;

//ELF程序头部
typedef struct {
    uint    p_type;
    uint    p_offset;
    uint    p_vaddr;
    uint    p_paddr;
    uint    p_filesz;
    uint    p_memsz;
    uint    p_flags;
    uint    p_align;
} elf_phdr;
#endif
