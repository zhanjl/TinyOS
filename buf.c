#include "buf.h"
#include "monitor.h"
#include "mem.h"
#include "ide.h"
#include "param.h"
struct {
    struct buf buf[NBUF];   //缓存块数组
    struct buf head;        //链表头部
} bcache;

void binit(void)
{
    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;
    
    int i;
    for (i = 0; i < NBUF; i++)
    {
        bcache.buf[i].next = bcache.head.next;
        bcache.buf[i].prev = &bcache.head;
        bcache.buf[i].dev = -1;
        bcache.head.next->prev = &bcache.buf[i];
        bcache.head.next = &bcache.buf[i];
    }
}

void brelse(struct buf* cur)
{
    if (cur->flags & BUF_BUSY)
        PANIC("brelse");
    cur->prev->next = cur->next;
    cur->next->prev = cur->prev;

    cur->next = bcache.head.next;
    cur->prev = &bcache.head;
    bcache.head.next->prev = cur;
    bcache.head.next = cur;
}

static struct buf* bget(uint dev, uint sector)
{
    int i;
    for (i = 0; i < NBUF; i++)
    {
            if (bcache.buf[i].dev == dev && bcache.buf[i].sector == sector)
            {
                if (bcache.buf[i].flags & BUF_BUSY == 0)
                {
                    brelse(&bcache.buf[i]);
                    return &bcache.buf[i];
                }
                else //该缓冲块正在被其他进程使用, 采用效率较低的轮询方式
                {
                    i--;
                    continue;
                }
            }
    }

    //寻找其他的缓冲块,采用的算法是最近最少使用
    struct buf *temp;
    for (temp = bcache.head.prev; temp != &bcache.head; temp = temp->prev)
    {
        if (temp->flags & BUF_BUSY == 0)
        {
            //把该缓存块的值写入磁盘
            if (temp->flags & BUF_DIRTY != 0)
                iderw(temp);
            temp->dev = dev;
            temp->sector = sector;
            temp->flags = 0;    //清空该缓存块的标识
            brelse(temp);
            return temp;
        }
    }

    PANIC("can't find buffer block");
}

//从磁盘读数据
struct buf* bread(uint dev, uint sector)
{
    struct buf *b;
    b = bget(dev, sector);
    b->flags |= BUF_BUSY;
    //如果不可用
    if ( !(b->flags & BUF_VALID))
    {
        iderw(b);   //从磁盘向该缓存中读数据
        b->flags |= BUF_VALID;
    }

    b->flags &= ~BUF_BUSY;
    return b;
}

//向磁盘写数据，在写之前一定要先读
//防止一个缓冲块中有多个磁盘块的数据
void bwrite(struct buf *b)
{
    b->flags |= BUF_BUSY;
    b->flags |= BUF_DIRTY;
    iderw(b);    
    b->flags &= ~BUF_BUSY;
}
