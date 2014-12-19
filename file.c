#include "file.h"
#include "param.h"

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


    }
    else if (f->type == FD_INODE)
    {
        f->type = FD_NONE;


    }
}

//获取一个文件的状态

//从文件中读取数据
int fileread(struct file *f, char *addr, int n)
{
    if (f->readable == 0)
        return -1;
    if (f->type == FD_PIPE)
    {

    }
    if (f->type == FD_INODE)
    {

    }
}

//向文件中写数据
int filewrite(struct file *f, char *addr, int n)
{
    if (f->writeable == 0)
        return -1;
    if (f->type == FD_PIPE)
    {

    }
    if (f->type == FD_INODE)
    {

    }
}
