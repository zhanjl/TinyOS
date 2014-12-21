#ifndef LOG_H
#define LOG_H

#include "param.h"
#include "buf.h"

struct logheader {
    int         n;
    int sector[LOGSIZE];
};

struct log {
    int start;
    int size;
    int outstanding;
    int committing;
    int dev;
    struct logheader lh;
};

void    initlog(void);
void    log_write(struct buf*);
void    begin_op();
void    end_op();
#endif
