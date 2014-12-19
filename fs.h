#ifndef FS_H
#define FS_H
#include "types.h"
#define NDIRECT 12

//超级块结构体,记录着整个文件系统的信息
struct superblock {
    uint    size; //整个文件系统的大小
    uint    nblocks;    //存放数据的总的磁盘块大小 
    uint    ninodes;    //文件系统的总的inode数量
    uint    nlog;       //log块的数量
};

#define NDIRECT     12  //一个文件数据最多占用的磁盘块

//磁盘上的inode结构体
struct  dinode {
    short   type;
    short   major;  //主设备号
    short   minor;  //次设备号
    short   nlink;  //链接到次inode的链接数
    uint    size;   //文件大小
    uint    addrs[NDIRECT+1]; //存储文件数据所在的磁盘块号
};

//目录文件的目录项结构体
#define DIRSIZ  14  //目录中的文件名

struct dirent {
    ushort  inum;           //该文件的inode号
    char    name[DIRSIZ];   //该文件使用的磁盘块
};
#endif
