#ifndef FS_H
#define FS_H
#include "types.h"
#define ROOTINO     1   //root的i节点号
#define BSIZE       512 //磁盘块大小

#define NDIRECT 12

//超级块结构体,记录着整个文件系统的信息
struct superblock {
    uint    size; //整个文件系统的大小
    uint    nblocks;    //存放数据的总的磁盘块大小 
    uint    ninodes;    //文件系统的总的inode数量
    uint    nlog;       //log块的数量
};

#define NDIRECT     12  //一个文件数据最多占用的磁盘块
#define NINDIRECT   (BSIZE / sizeof(uint))

#define MAXFILE     (NDIRECT + NINDIRECT)   //一个文件最多占用的磁盘块数

#define T_DIR   1   //目录文件
#define T_FILE  2   //普通磁盘文件
#define T_DEV   3   //设备文件

//磁盘上的inode结构体
struct  dinode {
    short   type;
    short   major;  //主设备号
    short   minor;  //次设备号
    short   nlink;  //链接到次inode的链接数
    uint    size;   //文件大小
    uint    addrs[NDIRECT+1]; //存储文件数据所在的磁盘块号
};

//一个磁盘块上的inode数
#define IPB     (BSIZE / sizeof(struct dinode))

//第i个inode节点所在的磁盘块号
#define IBLOCK(i)   ((i) / IPB + 2)

//每个磁盘块上的bitmap位数
#define BPB     (BSIZE * 8)

//包含第b个磁盘块的bitmap位所在的磁盘块
#define BBLOCK(b, ninodes)  ((ninodes)/IPB + 3 + (b)/BPB)

//目录文件的目录项结构体
#define DIRSIZ  14  //目录中的文件名

struct dirent {
    ushort  inum;           //该文件的inode号
    char    name[DIRSIZ];   //该文件使用的磁盘块
};
struct inode;
void readsb(int dev, struct superblock *sb);
int dirlink(struct inode*, char*, uint);
struct inode*   dirlookup(struct inode*, char*, uint*);
struct inode*   ialloc(uint, short);
struct inode*   idup(struct inode*);
void            iinit(void);
void            ilock(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
int             readi(struct inode*, char*, uint, uint);
//void            stati(struct inode*, struct stat*);
int             writei(struct inode*, char*, uint, uint);
#endif
