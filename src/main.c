#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>

#include "types.h"
#include "datetime.h"
#include "pins.h"
#include "uart.h"
#include "twi.h"
#include "rtc.h"
#include "display.h"

/* Set by makefile based on git version. */
#ifndef VERSION
# define VERSION "undef"
#endif

static u8 display_brightness_ee EEMEM = 1; /* 0..7 */
static u8 display_brightness;

static u8 datediff_enabled_ee EEMEM = 0;
static u8 datediff_enabled;

static struct datetime datediff_target_ee EEMEM = {
    .date = { .day = 1, .month = 1, .year = 2019 },
    .time = { .hour = 0, .min = 0, .sec = 0 },
};
static struct datetime datediff_target;


void init(void)
{
    DDRA = 0;
    DDRB = 0;
    PORTA = 0;
    PORTB = 0;

    pin_set_mode(PIN_LED1, OUTPUT);
    pin_set_mode(PIN_LED2, OUTPUT);

    pin_set_mode(PIN_RTC_INT, INPUT);

    EICRA = 1<<ISC11 | 0 << ISC10; /* INT1 falling edge */
    EIMSK = 1<<INT1; /* Enable external INT1 */

    display_brightness = eeprom_read_byte(&display_brightness_ee);

    datediff_enabled = eeprom_read_byte(&datediff_enabled_ee);
    eeprom_read_block(&datediff_target, &datediff_target_ee,
            sizeof(datediff_target));

}

void update_display(void)
{
    if (datediff_enabled) {
        u16 days;
        struct date date;
        rtc_read_date(&date);
        days = date_diff_days(&datediff_target.date, &date);
        display_shownum(days, false, false, display_brightness);
    } else {
        struct time time;
        rtc_read_time(&time);
        display_shownum(time.hour * 100 + time.min, true, true, display_brightness);
    }
}

void handle_command(char *msg)
{
    struct time time;
    struct date date;
    struct rtc_temp temp;

    _delay_ms(10);

    if (!strcmp(msg, "tg")) {
        rtc_read_time(&time);
        time_print(&time);
    } else if (!strncmp(msg, "ts ", 3)) {
        time_from_string(&msg[3], &time);
        rtc_write_time(&time);
        rtc_read_time(&time);
        time_print(&time);
        update_display();

    } else if (!strcmp(msg, "dg")) {
        rtc_read_date(&date);
        date_print(&date);
    } else if (!strncmp(msg, "ds ", 3)) {
        date_from_string(&msg[3], &date);
        rtc_write_date(&date);
        rtc_read_date(&date);
        date_print(&date);
        update_display();

    } else if (!strcmp(msg, "ddg")) {
        datetime_print(&datediff_target);
    } else if (!strncmp(msg, "dds ", 4)) {
        datetime_from_string(&msg[4], &datediff_target);
        datetime_print(&datediff_target);
        update_display();
        datetime_print(&datediff_target);
        eeprom_write_block(&datediff_target, &datediff_target_ee,
                sizeof(datediff_target));
    } else if (!strcmp(msg, "dde 0")) {
        LOG("Datediff disabled");
        datediff_enabled = 0;
        eeprom_write_byte(&datediff_enabled_ee, datediff_enabled);
        update_display();
    } else if (!strcmp(msg, "dde 1")) {
        LOG("Datediff enabled");
        datediff_enabled = 1;
        eeprom_write_byte(&datediff_enabled_ee, datediff_enabled);
        update_display();

    } else if (!strcmp(msg, "bg")) {
        LOGF("Brightness %u/7", display_brightness);
    } else if (!strncmp(msg, "bs ", 3)) {
        display_brightness = atoi(&msg[3]) & 0x7;
        eeprom_write_byte(&display_brightness_ee, display_brightness);
        update_display();
        LOGF("Brightness %u/7", display_brightness);

    } else if (!strcmp(msg, "temp")) {
        rtc_read_temp(&temp);
        LOGF("Temp %d.%u C", temp.temp, temp.fraction);

    } else if (!strncmp(msg, "ver", 3)) {
        LOGF("Version %s", VERSION);

    } else {
        LOGF("Unknown cmd \"%s\"", msg);
    }
}

int main(void)
{
    init();
    uart_init();
    uart_set_recv_callback(handle_command);
    twi_init();
    rtc_init();
    rtc_enable_notifier();
    display_init();

    LOG("*** Simpleclock initialized");
    _delay_ms(1000);
    update_display();

    sei();

    while (1) {
        _delay_ms(10);
        sleep_mode();
    }
}

ISR(INT1_vect)
{
    cli();
    rtc_notifier_handled();
    update_display();
    sei();
}
