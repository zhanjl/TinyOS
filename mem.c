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
