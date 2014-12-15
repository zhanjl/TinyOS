#ifndef IDE_H
#define IDE_H
#include "buf.h"

#define IDE_BSY     0x80
#define IDE_DRDY    0x40
#define IDE_DF      0x20
#define IDE_ERR     0x01

#define IDE_CMD_READ    0x20
#define IDE_CMD_WRITE   0x30

void ideinit(void);
void ideintr(void);
void iderw(struct buf *b);
#endif
