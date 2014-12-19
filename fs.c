#include "fs.h"
#include "file.h"
#include "mem.h"
#include "buf.h"
#include "monitor.h"
//读取超级块结构
void readsb(int dev, struct superblock *sb)
{
    struct buf *b;

    b = bread(dev, 1);
    memcpy(sb, b->buf, sizeof(struct superblock));
    brelse(b);    
}


//把一块磁盘置为0
static void bzero(int dev, int bno)
{
    struct buf *bp;

    bp = bread(dev, bno);
    memset(bp->data, 0, BSIZE);
    //log_write(bp);
    brelse(bp);
}

//分配一个空闲的磁盘块
static uint balloc(uint dev)
{
    int b, bi, m;
    struct buf *bp;
    struct superblock sb;
    
    bp = 0;
    readsb(dev, &sb);
    
    for (b = 0; b < sb.size; b += BPB)
    {
        bp = bread(dev, BBLOCK(b, sb.ninodes));

        for (bi = 0; bi < BPB && b + bi < sb.size; bi++)
        {
            m = 1 << (bi % 8);
            if ((bp->data[bi/8] & m) == 0)
            {
                bp->data[bi/8] |= m;
                //log_write(bp);
                brelse(bp);
                bzero(b+bi);  
                return b + bi;
            }
        }
        brelse(bp);
    }
    PANIC("out of blocks");
}

//释放一个磁盘块
static void bfree(int dev, uint b)
{
    struct buf *bp;
    struct superblock sb;
    int bi, m;
    
    readsb(dev, &sb);
    bp = bread(dev, BBLOCK(b, sb.inode));
    bi = b % BPB;
    m = 1 << (bi % 8);
    
    if ((bp->data[bi/8] & m) == 0)
        PANIC("freeing free block");
    
    bp->data[bi/8] &= ~m;
    //log_write(bp);
    brelse(bp);    
}
