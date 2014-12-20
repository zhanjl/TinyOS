#ifndef MEM_H
#define MEM_H
#include "types.h"
void *memset(void *src, char c, uint size);

void *memcpy(void *dst, void *src, uint size);

int strncmp(const char *s, const char *t, uint n);
char *strncpy(char *dst, const char *src, uint size);
#endif
