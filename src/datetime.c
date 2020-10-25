#include "datetime.h"
#include "uart.h"

static inline u8 asc_decode2(const char *s)
{
    return (s[0] - '0') * 10 + s[1] - '0';
}
static inline u16 asc_decode4(const char *s)
{
    return (u16)asc_decode2(&s[0]) * 100 + asc_decode2(&s[2]);
}


/* Expect "hh:mm:ss" */
void time_from_string(const char *str, struct time *ret)
{
    ret->hour = asc_decode2(str);
    ret->min = asc_decode2(&str[3]);
    ret->sec = asc_decode2(&str[6]);
}

/* Expect "dd-mm-yyyy" */
void date_from_string(const char *str, struct date *ret)
{
    ret->day = asc_decode2(str);
    ret->month = asc_decode2(&str[3]);
    ret->year = asc_decode4(&str[6]);
}

/* Expect "dd-mm-yyyy hh:mm:ss" */
void datetime_from_string(const char *str, struct datetime *ret)
{
    date_from_string(str, &ret->date);
    time_from_string(&str[11], &ret->time);
}

/* Returns 0 if dates are equal, <0 if date1 is before date2, >0 if date1 is
 * after date2.
 */
s16 date_cmp(struct date *date1, struct date *date2)
{
    if (date1->year != date2->year)
        return date1->year - date2->year;
    if (date1->month != date2->month)
        return date1->month - date2->month;
    return date1->day - date2->day;
}
u8 date_year_is_leap(u16 year)
{
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}
u8 date_days_per_month(u8 month, u16 year)
{
    u8 month_days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    u8 days = month_days[month - 1];
    if (month == 2 && date_year_is_leap(year))
        days++;
    return days;
}
void date_next(struct date *date)
{
    date->day++;
    if (date->day > date_days_per_month(date->month, date->year)) {
        date->day = 1;
        date->month++;
        if (date->month > 12) {
            date->month = 1;
            date->year++;
        }
    }
}
u16 date_diff_days(struct date *date1, struct date *date2)
{
    struct date iter, target;
    u16 days = 0;

    if (date_cmp(date1, date2) < 0) {
        iter = *date1;
        target = *date2;
    } else {
        iter = *date2;
        target = *date1;
    }

    while (date_cmp(&iter, &target)) {
        date_next(&iter);
        //LOGF("%d - %02u-%02u-%02u", days, iter.day, iter.month, iter.year);
        days++;
    }
    return days;
}

void datetime_print(struct datetime *datetime)
{
    LOGF("%02u-%02u-%04u %02u:%02u:%02u", datetime->date.day,
            datetime->date.month, datetime->date.year, datetime->time.hour,
            datetime->time.min, datetime->time.sec);
}

void date_print(struct date *date)
{
    LOGF("%02u-%02u-%04u", date->day, date->month, date->year);
}

void time_print(struct time *time)
{
    LOGF("%02u:%02u:%02u", time->hour, time->min, time->sec);
}
