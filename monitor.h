#ifndef MONITOR_H
#define MONITOR_H
void monitor_putc(char c);          //在屏幕上打印一个字符
void monitor_puts(const char *s);         //在屏幕上打印一个字符串
void monitor_clear();               //清除屏幕
void monitor_hex(unsigned int val);  //以16进制方式显示一个非负数字
void monitor_dec(int val); //以10进制方式显示一个数字

void printf(const char* fmt, ...);
#define PANIC(msg) panic(msg, __FILE__, __LINE__)
void panic(const char*, const char*, unsigned int);
#endif
