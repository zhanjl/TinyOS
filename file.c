#include "file.h"
#include "param.h"
#include "fs.h"
#include "monitor.h"
#include "log.h"
#include "stat.h"
#include "pipe.h"
struct devsw devsw[NDEV];   //设备驱动程序数组

//文件描述符表,每个进程最多只能打开NFILE个文件
struct file ftable[NFILE];

void fileinit(void)
{
    //初始化filetable锁，但是本内核没实现锁机制，所以
    //该函数不做任何工作。
}

//在文件描述符表中寻找一个空闲格
struct file* filealloc(void)
{
    int i;
    for (i = 0; i < NFILE; i++)
    {
        if (ftable[i].ref == 0)
        {
            ftable[i].ref++;
            return &ftable[i];
        }
    }
    return 0;
}

struct file* filedup(struct file *f)
{
    f->ref++;
     
    return f;
}

//关闭一个文件
void fileclose(struct file *f)
{
    if (--f->ref > 0)
        return;

    if (f->type == FD_PIPE)
    {
        f->type = FD_NONE;
        pipeclose(ff.pipe, ff.writeable);
    }
    else if (f->type == FD_INODE)
    {
        f->type = FD_NONE;
        begin_op();
        iput(f->ip);
        end_op();
    }
}

//从文件中读取数据
int fileread(struct file *f, char *addr, int n)
{
    int r;
    if (f->readable == 0)
        return -1;
    if (f->type == FD_PIPE)
    {
        return piperead(f->pipe, addr, n);
    }
    if (f->type == FD_INODE)
    {
        if ((r = readi(f->ip, addr, f->off, n)) > 0)
            f->off += r;
        return r;
    }
}

//向文件中写数据
int filewrite(struct file *f, char *addr, int n)
{
    if (f->writeable == 0)
        return -1;
    if (f->type == FD_PIPE)
    {
        return pipewrite(f->pipe, addr, n);
    }
    if (f->type == FD_INODE)
    {
        int max = ((LOGSIZE - 1 - 1 - 2) / 2) * 512;
        int i = 0;
        int n1, r;
        while (i < n)
        {
            n1 = n - i;
            if (n1 > max)
                n1 = max;
            
            begin_op();
            if ((r = writei(f->ip, addr+i, f->off, n1)) > 0)
                f->off += r;
            end_op();

            if (r < 0)
                break;
            if (r != n1)
                PANIC("short filewrite");
            i += r;
        }
        return i == n ? n : -1;
    }
    PANIC("filewrite");
}
//读取文件状态
int filestat(struct file *f, struct stat *st)
{
    if (f->type == FD_INODE)
    {
        stati(f->ip, st);
        return 0;
    }
    return -1;
}
