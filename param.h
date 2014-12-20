//配置变量的大小
#ifndef PARAM_H
#define PARAM_H

#define KSTACKSIZE  4096    //每个进程的内核栈大小
#define NDEV        10      //内核支持的最大设备号
#define NFILE       100     //每个进程运行打开的最大文件数目
#define NINODE      50      //整个系统允许打开的最多不同文件数
#define NBUF        30
#endif
