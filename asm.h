//和建立GDT表项有关的宏定义
#ifndef ASM_H
#define ASM_H
#define SEG_NULLASM             \
        .word   0, 0;           \
        .byte   0, 0, 0, 0      


#define SEG_ASM(type,base,lim)                                  \
        .word (((lim) >> 12) & 0xffff), ((base) & 0xffff);      \
        .byte (((base) >> 16) & 0xff), (0x90 | (type)),         \
                (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)


#define     STA_X       0x8     //可执行权限 
#define     STA_R       0x2     //可读权限
#define     STA_W       0x2     //可写权限
#endif
