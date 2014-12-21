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

int strncmp(const char *s, const char *t, uint n)
{
    uint count;
    count = 0;
    while (*s && *t)
    {
        if (*s > *t)
            return 1;
        else if (*s < *t)
            return -1;
        count++;
        if (count == n)
            return 0;
        s++;
        t++;
    }

    if (!(*s) && !(*t))
        return 0;
    else if (!(*s))
        return -1;
    return 1;
}

char *strncpy(char *dst, const char *src, uint size)
{
    uint i;
    for (i = 0; i < size; i++)
    {
        if (src[i] == 0)
            break;
        dst[i] = src[i];
    }
    dst[i] = 0;
    return dst;
}

