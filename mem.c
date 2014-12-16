#include "mem.h"

void *memset(void *src, char c, uint size)
{
    char *s;
    s = src;
    uint i;
    for (i = 0; i < size; i++)
        s[i] = c;

    return src;
}
void *memcpy(void *dst, void *src, uint size)
{
    uchar *d, *s;
    uint i;
    d = dst;
    s = src;
    for (i = 0; i < size; i++)
        d[i] = s[i];
    return dst;
}
