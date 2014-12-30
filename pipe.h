#ifndef PIPE_H
#define PIPE_H
#include "types.h"
#define PIPESIZE    512 //管道数据大小

struct pipe {
    char    data[PIPESIZE];
    uint    nread;  //读取的字节数
    uint    nwrite;  //写入的字节数
    int     readopen;//是否读打开管道
    int     writeopen;//是否写打开管道
};

struct file;

int pipealloc(struct file**, struct file**);    //新建一个管道
void pipeclose(struct pipe*, int);              //关闭一个管道
int pipewrite(struct pipe*, char*, int);        //向管道中写数据
int piperead(struct pipe*, char*, int);         //从管道中读数据
#endif
