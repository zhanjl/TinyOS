#include "x86.h"
#include "monitor.h"
#include "memlayout.h"
#include "picirq.h"
#include "traps.h"
#include "types.h"
#include "file.h"
short *video_base = 0xB8000 + KERNBASE;        //IO缓存的起始地址(虚拟地址))
unsigned short cursor_x, cursor_y;    //默认初始化为0

//移动光标
static void movecur()
{
    unsigned short position;
    position = cursor_x + 80 * cursor_y;
    outb(0x3D4, 14);
    outb(0x3D5, position >> 8); //高8位
    outb(0x3D4, 15);
    outb(0x3D5, position);      //低8位
}

//scroll屏幕
static void scroll()
{

    if (cursor_y <= 24)
        return;
    int i;
    char attribute = (0 << 4) | (15 & 0x0F);
    short blank = (attribute << 8) | 0x20;
    for (i = 0; i < 24 * 80; i++)
        video_base[i] = video_base[i+80];

    for (i = 24 * 80; i < 25 * 80; i++)
        video_base[i] = blank;
    cursor_y--;
}

void monitor_putc(char c)
{
    char attribute = (0 << 4) | (15 & 0x0F);
    short val = (attribute << 8) | c;
    if (c == 0x08 && cursor_x)  //backspace
        cursor_x--;
    else if (c == 0x09)         //tab
        cursor_x = (cursor_x + 4) & ~3;
    else if (c == '\r')
        cursor_x = 0;
    else if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    else
    {
        video_base[cursor_x + cursor_y*80] = val;
        cursor_x++;
    }

    if (cursor_x >= 80)
    {
        cursor_x = 0;
        cursor_y++;
    }
    scroll();
    movecur();
}

void monitor_puts(const char *s)
{
    while (*s)
        monitor_putc(*(s++));
}

void monitor_clear()
{
    char attribute = (0 << 4) | (15 & 0x0F);
    short blank = (attribute << 8) | 0x20;
    int i;
    for (i = 0;i < 25 * 80; i++)
        video_base[i] = blank;
    cursor_x = 0;
    cursor_y = 0;
    movecur();
}

void monitor_hex(unsigned int val)
{
    char map[] = {  '0', '1', '2', '3',
                    '4', '5', '6', '7',
                    '8', '9', 'A', 'B',
                    'C', 'D', 'E', 'F',
                 };
    int base, index;
    base = 7;
    monitor_putc('0');
    monitor_putc('X');
    while (base >= 0)
    {
        index = (val >> (base*4)) & 0x0F;
        monitor_putc(map[index]);
        base--;
    }
    return;
}

void monitor_dec(int val)
{
    char map[] = {  '0', '1', '2', '3', '4',
                    '5', '6', '7', '8', '9',
                 };
    
    char digits[20];        //val的十进制位数不会超过20
    int remainder, index;
    index = 0;
    if (val == 0)
    {
        monitor_putc('0');
        return;
    }
    if (val < 0)
    {
        monitor_putc('-');
        val = -val;
    }
    while (val)
    {
        remainder = val % 10;
        val = val / 10;
        digits[index++] = map[remainder];
    }

    index--;
    while (index >= 0)
        monitor_putc(digits[index--]);
    return;
}

void panic(const char *message, const char *file, unsigned int line)
{
    asm volatile("cli");
    monitor_puts("PANIC(");
    monitor_puts(message);
    monitor_puts(") at ");
    monitor_puts(file);
    monitor_puts(" : ");
    monitor_dec(line);
    monitor_putc('\n');
    while (1)
        ;
}

void printf(const char* fmt, ...)
{
    char *argv;
    int *pi, *px;
    char *ps;
    argv = &fmt;
    argv = argv + sizeof(char*);; 
    while (*fmt) {
        if (*fmt != '%')
        {
            monitor_putc(*fmt);
            fmt++;
            continue;
        }
        fmt++;
        switch(*(fmt)) {
            case '%':
                monitor_putc('%');
                break;
            case 'c':
                monitor_putc(*argv);
                argv++;
                break;
            case 'd':
                pi = (int*)argv;
                monitor_dec(*pi);
                argv += sizeof(int);
                break;
            case 'x':
                px = (int*)argv;
                monitor_hex(*px);
                argv += sizeof(int);
                break;
            case 's':
                ps = *((char**)argv);
                argv += sizeof(char*);
                monitor_puts(ps);
                break;
        }
        fmt++;
    }

    return;
}


#define INPUT_BUF   128

struct {
    char buf[INPUT_BUF];
    uint r; //读字符的索引
    //uint w;
    uint e; //编辑索引
} input;

#define C(x)    ((x) - '@')

//键盘中断的处理函数
void consoleintr(int (*getc)(void))
{
    int c;
    while ((c = getc()) >= 0)
    {
        switch(c) {    
        
            default:
            //在此处还要加上特殊字符的处理
            if (c != 0 && input.e-input.r < INPUT_BUF)
            {
                c = (c == '\r') ? '\n' : c;
                input.buf[input.e++%INPUT_BUF] = c;
                monitor_putc(c);

            }
            break; 
        }
    }
    return;
}

//从键盘缓冲区中读取字符
int consoleread(struct inode *ip, char *dst, int n)
{
    int c;
    int target;
    target = n;
    while (n > 0)
    {
        if (input.r == input.e)
            continue;
        c = input.buf[input.r++%INPUT_BUF];
        
        *dst++ = c;
        --n;
        if (c == '\n')
            break;
    }
    return target - n;
}

//把buf缓冲区的内容显示到屏幕上
int consolewrite(struct inode *ip, char *buf, int n)
{
    int i;
    for (i = 0; i < n; i++)
        monitor_putc(buf[i]);
    return n;
}

extern struct devsw devsw[];
void consoleinit(void)
{
    //设置显示设备的读写函数
    devsw[CONSOLE].read = consoleread;
    devsw[CONSOLE].write = consolewrite;
    picenable(IRQ_KBD); //打开键盘中断
}
