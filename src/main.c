#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "types.h"
#include "pins.h"
#include "uart.h"
#include "twi.h"
#include "rtc.h"
#include "display.h"

/* Set by makefile based on git version. */
#ifndef VERSION
# define VERSION "undef"
#endif

static u8 display_brightness = 1; /* 0..7 */

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
}

void update_display_time(void)
{
    struct rtc_time time;
    rtc_read_time(&time);
    display_shownum(time.hour * 100 + time.min, true, display_brightness);
}

void handle_command(char *msg)
{
    struct rtc_time time;
    struct rtc_temp temp;
    if (!strcmp(msg, "get")) {
        rtc_read_time(&time);
        LOG("Current time %02u:%02u:%02u", time.hour, time.min, time.sec);
    } else if (!strncmp(msg, "set ", 4)) {
        rtc_write_time_from_string(&msg[4]);
        rtc_read_time(&time);
        update_display_time();
        LOG("Current time %02u:%02u:%02u", time.hour, time.min, time.sec);
    } else if (!strncmp(msg, "brightness ", 11)) {
        _delay_ms(10);
        display_brightness = atoi(&msg[11]) & 0x7;
        LOG("Brightness %u/7", display_brightness);
    } else if (!strcmp(msg, "temp")) {
        rtc_read_temp(&temp);
        LOG("Temperature %d.%u C", temp.temp, temp.fraction);
    } else if (!strncmp(msg, "ver", 3)) {
        _delay_ms(10);
        LOG("Version %s", VERSION);
    } else {
        _delay_ms(10);
        LOG("Unknown command \"%s\"", msg);
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

    uart_puts("*** Simpleclock initialized\r\n");
    _delay_ms(1000);
    update_display_time();

    sei();

#if 0
    while (1) {
        pin_write(PIN_LED1, 0);
        pin_write(PIN_LED2, 1);
        _delay_ms(500);
        pin_write(PIN_LED1, 1);
        pin_write(PIN_LED2, 0);
        _delay_ms(500);
    }
#endif

    while (1) {
        sleep_mode();
        _delay_ms(10);
    }
}

ISR(INT1_vect)
{
    cli();
    rtc_notifier_handled();
    update_display_time();
    sei();
}
