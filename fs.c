#include "fs.h"
#include "file.h"
#include "mem.h"
#include "buf.h"
#include "monitor.h"
#include "param.h"
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
    log_write(bp);
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
                log_write(bp);
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
    log_write(bp);
    brelse(bp);    
}

//i节点缓存区
struct inode icache[NINODE];

static struct inode *iget(uint dev, uint inum);

//从磁盘上寻找一个空闲i节点
struct inode* ialloc(uint dev, short type)
{
    int inum;
    struct  buf *bp;
    struct dinode *dip;
    struct superblock sb;

    readsb(dev, &sb);

    for (inum = 1; inum < sb.ninodes; inum++)
    {
        bp = bread(dev, IBLOCK(inum));
        dip = (struct dinode*)bp->data + inum % IPB;
        if (dip->type == 0)
        {
            memset(dip, 0, sizeof(struct dinode));
            dip->type = type;
            log_write(bp)
            brelse(bp);

            return iget(dev, inum);
        }
        brelse(bp);
    }
    PANIC("ialloc: no inodes");
}

//把一个i节点写到磁盘上
void iupdate(struct inode *ip)
{
    struct buf *bp;
    struct dinode *dip;

    bp = bread(ip->dev, IBLOCK(ip->inum));
    dip = (struct dinode*)bp->data + ip->inum%IPB;
    dip->type = ip->type;
    dip->major = ip->major;
    dip->minor = ip->minor;
    dip->nlink = ip->nlink;
    dip->size = ip->size;

    memcpy(dip->addrs, ip->addrs, sizeof(ip->addrs));
    log_write(bp);
    brelse(bp);
}

//在i节点缓冲区中寻找指定的i节点或分配一个新的i节点
static struct inode*
iget(uint dev, uint inum)
{
    struct inode *ip, *empty;

    empty = 0;
    for (ip = &icache[0]; ip < &icache[NINODE]; ip++)
    {
        if (ip->ref > 0 && ip->dev == dev && ip->inum == inum)
        {
            ip->ref++;
            return ip;
        }

        if (ip->ref == 0)
            empty = ip;
    }

    if (empty == 0)
        PANIC("iget: no inodes");

    ip = empty;
    ip->dev = dev;
    ip->inum = inum;
    ip->ref = 1;
    ip->flag = 0;

    return ip;
}


//增加i节点的引用次数
struct inode* idup(struct inode *ip)
{
    ip->ref++;
    return ip;
}

//占用一个i节点，必要时从磁盘读入
void ilock(struct inode *ip)
{
    struct buf *bp;
    struct dinode *dip;

    while (ip->flag & I_BUSY)
        ;
    ip->flag |= I_BUSY;

    if (!(ip->flag & I_VALID))
    {
        bp = bread(ip->dev, IBLOCK(ip->inum));
        dip = (struct dinode*)bp->data + ip->inum%IPB;

        ip->type = dip->type;
        ip->major = dip->major;
        ip->minor = dip->minor;
        ip->nlink = dip->nlink;
        ip->size = dip->size;   

        memcpy(ip->addrs, dip->addrs, sizeof(ip->addrs));
        ip->flag |= I_VALID;
    }
}

//释放一个i节点
void iunlock(struct inode *ip)
{
    ip->flag &= ~I_BUSY;
}


//
void iput(struct inode *ip)
{
    if (ip->ref == 1 && (ip->flag & I_VALID) && ip->nlink == 0)
    {
        ip->flag |= I_BUSY;
        itrunc(ip);
        ip->type = 0;
        iupdate(ip);
        ip->flag = 0;
        ip->flag &= ~I_BUSY;
    }
    ip->ref--;
}

//封装了iunlock和iput
void iunlockput(struct inode *ip)
{
    iunlock(ip);
    iput(ip);
}

//把一个文件长度截为0
static void itrunc(struct inode *ip)
{
    int i, j;
    struct buf *bp;
    uint *a;

    for (i = 0; i < NDIRECT; i++)
    {
        if (ip->addrs[i])
        {
            bfree(ip->dev, ip->addrs[i]);
            ip->addrs[i] = 0;
        }
    }

    if (ip->addrs[NDIRECT])
    {
        bp = bread(ip->dev, ip->addrs[NDIRECT]);
        a = (uint*)bp->data;
        for (i = 0; i < NINDIRECT; i++)
        {
            if (a[i])
                bfree(ip->dev, a[i]);
        }
        ip->addrs[NDIRECT] = 0;
    }
    ip->size = 0;
    iupdate(ip);
}

//返回一个i节点中address的第n项的值，如果该
//值为0,则新分配一个磁盘块
static uint bmap(struct inode *ip, uint bn)
{
    uint    addr, *a;
    struct buf *bp;

    if (bn < NDIRECT)
    {
        if ((addr = ip->addrs[bn]) == 0)
            ip->addrs[bn] = addr = balloc(ip->dev);
        return addr;
    }

    bn -= NDIRECT;
     
    if (bn < NINDIRECT)
    {
        if ((addr = ip->addrs[NDIRECT]) == 0)
            ip->addrs[NDIRECT] = addr = balloc(ip->dev);
        bp = bread(ip->dev, addr);
        a = (uint*)bp->data;
        if ((addr = a[bn]) == 0)
        {
            a[bn] = addr = balloc(ip->dev);
            log_write(bp);
        }
        return addr;
    }

    PANIC("out of range");
}


//从i节点中读数据
int readi(struct inode *ip, char *dst, uint off, uint n)
{
    //如果此i节点是设备文件
    if (ip->type = T_DEV)
    {
        if (ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
            return -1;
        //调用设备驱动程序
        return devsw[ip->major].read(ip, dst, n);
    }
    //如果i节点是普通的磁盘文件，直接拷贝内容
    if (off > ip->size || off + n < off)
        return -1;
    if (off + n > ip->size)
        n = ip->size - off;

    uint tot, m;
    
    for (tot = 0; tot < n; tot += m, off += m, dst += m)
    {
        bp = bread(ip->dev, bmap(ip, off/BSIZE));
        m = (n - tot) < (BSIZE - off%BSIZE) ? n - tot : BSIZE - off%BSIZE;
        memcpy(dst, bp->data+off%BSIZE, m);
        brelse(bp);
    }
    return n;
}

//向i节点写数据
int writei(struct inode *ip, char *src, uint off, uint n)
{
    //如果此i节点是设备文件
    if (ip->type = T_DEV)
    {
        if (ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
            return -1;
        //调用设备驱动程序
        return devsw[ip->major].write(ip, src, n);
    }

    if (off > ip->size || off + n < off)
        return -1;

    if (off + n > MAXFILE * BSIZE)
        return -1;

    uint tot, m;
    struct buf *bp;

    for (tot = 0; tot < n; tot += m, off += m, src += m)
    {
        bp = bread(ip->dev, bmap(ip, off/BSIZE));
        m = (n - tot) < (BSIZE - off%BSIZE) ? n - tot : BSIZE - off%BSIZE;
        memcpy(bp->data+off%BSIZE, src, m);
        log_write(bp);
        brelse(bp);
    }

    //更新文件大小
    if (off > ip->size)
    {
        ip->size = off;
        iupdate(ip);
    }
    return n;
}

/*
 *和目录有关的函数
 */

int namecmp(const char *s, const char *t)
{
    return strncmp(s, t, DIRSIZ);
}

//在一个目录文件中查找指定文件的i节点号
struct inode *dirlookup(struct inode *dp, char *name, uint *poff)
{
    if (dp->type != T_DIR)
        PANIC("dirlookup not dir");

    uint off, inum;
    struct dirent de;

    for (off = 0; off < dp->size; off += sizeof(de))
    {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
            PANIC("readi error");
        if (de.inum == 0)
            continue;
        if (namecmp(name, de.name) == 0)
        {
            if (poff)
                *poff = off;
            inum = de.inum;
            return iget(dp->dev, inum);
        }
    }

    return 0;
}

//在一个目录中增加一个文件
int dirlink(struct inode *dp, char *name, uint inum)
{
    int     off;
    struct dirent de;
    struct inode *ip;

    //查找该文件是否已在目录文件中
    if ((ip = dirlookup(dp, name, 0)) != 0)
    {
        iput(ip);
        return -1;
    }

    //找到一个空闲项
    for (off = 0; off < dp->size; off += sizeof(de))
    {
        if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
            PANIC("dirlink read");
        if (de.inum == 0)
            break;
    }
    
    strncpy(de.name, name, DIRSIZ);
    de.inum = inum;
    if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
        PANIC("writei error");

    return 0;
}

static char *skipelem(char *path, char *name)
{
    char *s;
    int len;

    while (*path == '/')
        path++;
    if (*path == 0)
        return 0;
    
    s = path;
    while (*path != '/' && *path != 0)
        path++;

    len = path - s;
    
    if (len >= DIRSIZ) {
        memcpy(name, s, DIRSIZ - 1);
        name[DIRSIZ-1] = '\0';
    } else {
        memcpy(name, s, len);
        name[len] = '\0';
    }
    while (path == '/')
        path++;
    return path;
}

//返回一个给定文件的i节点
static struct inode*
namex(char *path, int nameiparent, char *name)
{
    struct inode *ip, *next;
    
    if (*path == '/')
        ip = iget(ROOTDEV, ROOTINO);
    else 
        ip = idup(proc->cwd);   //当前进程目录    

    while ((path = skipelem(path, name)) != 0)
    {
        ilock(ip);

        if (ip->type != T_DIR)
        {
            iunlockput(ip);
            return 0;
        }

        if (nameiparent && *path == '\0')
        {
            iunlock(ip);
            return ip;
        }

        if ((next = dirlookup(ip, name, 0)) == 0)
        {
            iunlockput(ip);
            return 0;
        }
        iunlockput(ip);
        ip = next;
    }
    if (nameiparent)
    {
        iput(ip);
        return 0;
    }

    return ip;
}


struct inode* namei(char *path)
{
    char name[DIRSIZ];
    return namex(path, 0, name);
}

struct inode* nameiparent(char *path, char *name)
{
    return namex(path, 1, name);
}
