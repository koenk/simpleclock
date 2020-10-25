#ifndef RTC_H
#define RTC_H

#include "types.h"

struct rtc_temp {
    s8 temp;
    u8 fraction;
};

void rtc_init(void);

void rtc_read_temp(struct rtc_temp *ret);
void rtc_write_time(struct time *time);
void rtc_read_time(struct time *ret);
void rtc_write_date(struct date *date);
void rtc_read_date(struct date *ret);
void rtc_enable_notifier(void);
void rtc_notifier_handled(void);

#endif
