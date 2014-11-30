//这个文件包含和所有和内存管理单元有关的定义
#ifndef MMU_H
#define MMU_H

#define CR0_PE          0x00000001  //保护模式位

//段选择子
#define SEG_KCODE       1   //内核代码段
#define SEG_KDATA       2   //内核数据段

#endif
