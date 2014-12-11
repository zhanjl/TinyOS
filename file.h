#ifndef FILE_H
#define FILE_H

struct inode {
        //inode结构体
};

struct devsw {
    int (*read)(struct inode*, char*, int);
    int (*write)(struct inode*, char*, int);
};

#define CONSOLE 1   //显示器的设备号是1
#endif
