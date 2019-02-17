#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "types.h"
#include "pins.h"
#include "uart.h"
#include "twi.h"
#include "rtc.h"
#include "display.h"


void init(void)
{
    DDRA = 0;
    DDRB = 0;
    PORTA = 0;
    PORTB = 0;

    pin_set_mode(PIN_LED1, OUTPUT);
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
        LOG("Current time %02u:%02u:%02u", time.hour, time.min, time.sec);
    } else if (!strcmp(msg, "temp")) {
        rtc_read_temp(&temp);
        LOG("Temperature %d.%u C", temp.temp, temp.fraction);
    } else {
        LOG("Unknown command \"%s\"", msg);
    }
}

int main(void)
{
    struct rtc_time time;
    u8 old_min = 99;

    init();
    uart_init();
    uart_set_recv_callback(handle_command);
    twi_init();
    rtc_init();
    display_init();

    uart_puts("*** Simpleclock initialized\r\n");

    sei();

    while (1) {
        pin_write(PIN_LED1, 0);
        _delay_ms(1000);

        pin_write(PIN_LED1, 1);
        _delay_ms(1000);

        rtc_read_time(&time);
        if (time.min != old_min)
            display_shownum(time.hour * 100 + time.min, 1, 4);
    }
}
