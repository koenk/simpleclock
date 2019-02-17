#ifndef RTC_H
#define RTC_H

#include "types.h"

struct rtc_time {
    u8 sec;
    u8 min;
    u8 hour;
};

struct rtc_temp {
    s8 temp;
    u8 fraction;
};

void rtc_init(void);

void rtc_read_temp(struct rtc_temp *ret);
void rtc_write_time(struct rtc_time *time);
void rtc_write_time_from_string(const char *s);

void rtc_read_time(struct rtc_time *ret);

#endif
