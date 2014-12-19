#ifndef BUF_H
#define BUF_H

#include "types.h"
#define BUF_BUSY    0x1     //该磁盘缓冲块是否正在被其他进程使用
#define BUF_VALID   0x2     //该缓存块的内容已经从磁盘读入
#define BUF_DIRTY   0x4     //该缓存块的内容已经被修改

struct buf {
    int     flags;
    uint    dev;
    uint    sector;

    struct buf *prev;
    struct buf *next;
    struct buf *qnext;
    uchar   data[512];
};

void binit(void);   //初始化磁盘缓冲区
//从磁盘读数据
struct buf* bread(uint dev, uint sector);
//向磁盘写数据
void bwrite(struct buf* f);
void brelse(struct buf* cur);

#endif 
