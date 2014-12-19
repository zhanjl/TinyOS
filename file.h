#ifndef FILE_H
#define FILE_H

#include "types.h"
#include "fs.h"

#define FD_NONE     0
#define FD_PIPE     2
#define FD_INODE    4

struct file {
    int     type;
    int     ref;    //引用计数
    char    readable;
    char    writeable;
    struct pipe *pipe;
    struct inode *ip;
    uint    off;    //文件偏移
};

struct inode {
    uint    dev;    //设备号
    uint    num;    //文件的inode号
    int     ref;    //引用计数
    int     flag;   //标识
    //下面是磁盘上的inode拷贝
    short   type;   
    short   major;
    short   minor;
    short   nlink;
    uint    size;
    uint    addrs[NDIRECT+1];   //该文件的内容所在的块号
};

#define I_BUSY  0x1
#define I_VALID 0x2

struct devsw {
    int (*read)(struct inode*, char*, int);
    int (*write)(struct inode*, char*, int);
};

#define CONSOLE 1   //显示器的设备号是1
#endif
