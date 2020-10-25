#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef int8_t s8;
typedef uint8_t u8;
typedef uint16_t u16;

typedef int16_t s16;

struct time {
    u8 sec;
    u8 min;
    u8 hour;
};

struct date {
    u8 day;
    u8 month;
    u16 year;
};

struct datetime {
    struct date date;
    struct time time;
};

#endif
