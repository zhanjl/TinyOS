#include "x86.h"
#include "buf.h"
#include "traps.h"
#include "ide.h"
#include "picirq.h"
#include "monitor.h"
static int havedisk1;
struct buf *idequeue;

static int idewait(int checkerr)
{
    int r;
    while (((r = inb(0x1f7)) & (IDE_BSY | IDE_DRDY)) != IDE_DRDY)
        ;
    if (checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
        return -1;
    return 0;
}

void ideinit(void)
{
    picenable(IRQ_IDE);
    idewait(0);
    int i;
    outb(0x1f6, 0xe0 | (1<<4));
    for (i = 0; i < 1000; i++)
    {
        if (inb(0x1f7) != 0)
        {
            havedisk1 = 1;
            break;
        }
    }

    outb(0x1f6, 0xe0 | (0 << 4));
}


static void idestart(struct buf *b)
{
    if (b == 0)
        PANIC("idestart");

    idewait(0);
    outb(0x3f6, 0);
    outb(0x1f2, 1);
    outb(0x1f3, b->sector & 0xff);
    outb(0x1f4, (b->sector >> 8) & 0xff);
    outb(0x1f5, (b->sector >> 16) & 0xff);
    outb(0x1f6, 0xe0 | ((b->dev&1) << 4) | (b->sector>>24) & 0x0f);

    //请求写磁盘
    if (b->flags & BUF_DIRTY)
    {
        outb(0x1f7, IDE_CMD_WRITE);
        outsl(0x1f0, b->data, 512 / 4);
    }
    //请求读磁盘
    else 
    {
        outb(0x1f7, IDE_CMD_READ);
    }
}

void ideintr(void)
{
    struct buf *b;
    if ((b = idequeue) == 0)
        return;

    idequeue = b->qnext;

    if (!(b->flags & BUF_DIRTY) && idewait(1) >= 0)
        insl(0x1f0, b->data, 512 / 4);

    b->flags |= BUF_VALID;
    b->flags &= ~BUF_DIRTY;

    if (idequeue != 0)
        idestart(idequeue);
}

void iderw(struct buf *b)
{
    struct buf *temp;
    temp = idequeue;

    b->qnext = 0;

    //插入队列
    if (temp == 0)
        temp = b;
    else
    {
        while (temp->qnext)
            temp = temp->qnext;
        temp->qnext = b;
    }

    if (idequeue == b)
        idestart(b);

    //等待磁盘操作完成
    while ((b->flags & (BUF_VALID | BUF_DIRTY)) != BUF_VALID)
        ;
}
