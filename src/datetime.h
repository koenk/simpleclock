#ifndef DATETIME_H
#define DATETIME_H

#include "types.h"

void time_from_string(const char *str, struct time *ret);
void date_from_string(const char *str, struct date *ret);
void datetime_from_string(const char *str, struct datetime *ret);

s16 date_cmp(struct date *date1, struct date *date2);
u8 date_year_is_leap(u16 year);
u8 date_days_per_month(u8 month, u16 year);
void date_next(struct date *date);

u16 date_diff_days(struct date *date1, struct date *date2);

void datetime_print(struct datetime *datetime);
void date_print(struct date *date);
void time_print(struct time *time);

#endif
