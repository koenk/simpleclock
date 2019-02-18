#include <string.h>
#include <stdlib.h>

#include "rtc.h"
#include "twi.h"
#include "uart.h"

#define TWI_ADDR 0x68

#define REG_TIME_SEC        0x00
#define REG_TIME_MIN        0x01
#define REG_TIME_HOUR       0x03

#define REG_DATE_DAY        0x04
#define REG_DATE_MONTH      0x05
#define REG_DATE_YEAR       0x06

#define REG_ALARM1_SEC      0x07
#define REG_ALARM1_MIN      0x08
#define REG_ALARM1_HOUR     0x09
#define REG_ALARM1_DAY      0x0a

#define REG_ALARM2_MIN      0x0b
#define REG_ALARM2_HOUR     0x0c
#define REG_ALARM2_DAY      0x0d

#define REG_CONTROL         0x0e
#define REG_STATUS          0x0f
#define REG_AGING           0x10
#define REG_TEMPI           0x11
#define REG_TEMPF           0x12

#define INTCN 2
#define A2IE 1
#define A1IE 0

#define A1F 0

void rtc_init(void)
{
    /* Disable square wave signal and alarm interrupts */
    twi_start(TWI_ADDR, false);
    twi_write(REG_CONTROL);
    twi_write(1<<INTCN);
    twi_stop();
}

static inline u8 bcd_decode(u8 bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0xf);
}
static inline u8 bcd_encode(u8 dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}

static inline u8 asc_decode(const char *s)
{
    return (s[0] - '0') * 10 + s[1] - '0';
}

void rtc_read_time(struct rtc_time *ret)
{
    u8 h, m, s;

    twi_start(TWI_ADDR, false);
    twi_write(REG_TIME_SEC);

    twi_start(TWI_ADDR, true);
    s = twi_read(false);
    m = twi_read(false);
    h = twi_read(true);
    twi_stop();

    ret->sec = bcd_decode(s);
    ret->min = bcd_decode(m);
    ret->hour = bcd_decode(h);
}

void rtc_write_time(struct rtc_time *time)
{
    twi_start(TWI_ADDR, false);
    twi_write(REG_TIME_SEC);
    twi_write(bcd_encode(time->sec));
    twi_write(bcd_encode(time->min));
    twi_write(bcd_encode(time->hour));
    twi_stop();
}

void rtc_write_time_from_string(const char *s)
{
    struct rtc_time time;
    time.hour = asc_decode(s);
    time.min = asc_decode(&s[3]);
    time.sec = asc_decode(&s[6]);
    rtc_write_time(&time);
}

void rtc_read_temp(struct rtc_temp *ret)
{
    s8 tempi;
    u8 tempf;

    twi_start(TWI_ADDR, false);
    twi_write(REG_TEMPI);

    twi_start(TWI_ADDR, true);
    tempi = twi_read(false);
    tempf = twi_read(true);
    twi_stop();

    ret->temp = tempi;
    ret->fraction = (tempf >> 6) * 25;
}


void rtc_enable_notifier(void)
{
    twi_start(TWI_ADDR, false);
    twi_write(REG_ALARM1_SEC);
    twi_write(0x00); /* Match on seconds = 0 */
    twi_write(0x80); /* Ignore minutes */
    twi_write(0x80); /* Ignore hours */
    twi_write(0x80); /* Ignore date */
    twi_stop();

    /* Enable alarm0 interrupts */
    twi_start(TWI_ADDR, false);
    twi_write(REG_CONTROL);
    twi_write(1<<INTCN | 1<<A1IE);
    twi_stop();

    rtc_notifier_handled();
}

void rtc_notifier_handled(void)
{
    u8 sts;

    twi_start(TWI_ADDR, false);
    twi_write(REG_STATUS);
    twi_start(TWI_ADDR, true);
    sts = twi_read(true);
    twi_stop();

    sts &= ~(1<<A1F);

    twi_start(TWI_ADDR, false);
    twi_write(REG_STATUS);
    twi_write(sts);
    twi_stop();
}
