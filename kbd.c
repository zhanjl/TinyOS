#include "kbd.h"
#include "monitor.h"
#include "types.h"
#include "x86.h"

//获取按键代表的字符
int kbdgetc(void)
{
    static uint shift;
    static uchar *charcode[4] = {
        normalmap, shiftmap, ctlmap, ctlmap
    };
    uint st, data, c;
    
    st = inb(KBSTATP);    
    if ((st & KBS_DIB) == 0)
        return -1;
    data = inb(KBDATAP);

    if (data == 0xE0)
    {
        shift |= E0ESC;
        return 0;
    }
    else if (data & 0x80)
    {
        data = (shift & E0ESC ? data : data & 0x7E);
        shift &= ~(shiftcode[data] | E0ESC);
        return 0;
    }
    else if (shift & E0ESC)
    {
        data |= 0x80;
        shift &= ~E0ESC;
    }

    shift |= shiftcode[data];
    shift ^= togglecode[data];
    c = charcode[shift & (CTL | SHIFT)][data];
    if (shift & CAPSLOCK)
    {
        if (c >= 'a' && c <= 'z')
            c += 'A' - 'a';
        else if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';
    }
    return c;
}

void kbdintr(void)
{
    consoleintr(kbdgetc);
}
