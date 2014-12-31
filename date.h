#ifndef DATE_H
#define DATE_H
#include "types.h"
struct rtcdate {
    uint    second;
    uint    minute;
    uint    hour;
    uint    day;
    uint    month;
    uint    year;
};
#endif
