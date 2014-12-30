#ifndef STAT_H
#define STAT_H

#include "types.h"
#define T_DIR   1   //目录文件
#define T_FILE  2   //普通文件
#define T_DEV   3   //设备文件
struct stat {
    short   type;   //文件类型  
    int     dev;    //文件系统的设备号
    uint    ino;    //i节点号
    short   nlink;  //链接到此文件的链接数
    uint    size;   //文件大小
};
#endif
