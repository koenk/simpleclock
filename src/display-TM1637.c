/*
 * Display implementation using a TM1637 module.
 *
 * The TM1637 talks a variant of the I2C/TWI protocol, that among other things
 * does not use the address at the start of transactions. As such, we implement
 * this protocol in software on separate pins from other I2C devices.
 */

#include <util/delay.h>

#include "uart.h"
#include "display.h"
#include "pins.h"

#define BIT_DELAY 100 /* us */

#define COMM1 0x40
#define COMM2 0xc0
#define COMM3 0x80

#define DISP_ON 0x08

/*
 * Each 7-segment digit has thefollowing segments
 *
 *  AA
 * F  B
 * F  B
 *  GG
 * E  C
 * E  C
 *  DD
 *
 * Normally each digit has a dot (P) as well, but for some displays with a colon
 * (:) in the middle the dot is not available, and enabling the dot on the 2nd
 * digit turns on the colon.
 */
static u8 segment_lut[] =
{
                  // P G F E D C B A
    [0x0] = 0x3f, // 0 0 1 1 1 1 1 1
    [0x1] = 0x06, // 0 0 0 0 0 1 1 0
    [0x2] = 0x5b, // 0 1 0 1 1 0 1 1
    [0x3] = 0x4f, // 0 1 0 0 1 1 1 1
    [0x4] = 0x66, // 0 1 1 0 0 1 1 0
    [0x5] = 0x6d, // 0 1 1 0 1 1 0 1
    [0x6] = 0x7d, // 0 1 1 1 1 1 0 1
    [0x7] = 0x07, // 0 0 0 0 0 1 1 1
    [0x8] = 0x7f, // 0 1 1 1 1 1 1 1
    [0x9] = 0x6f, // 0 1 1 0 1 1 1 1
    [0xa] = 0x77, // 0 1 1 1 0 1 1 1
    [0xb] = 0x7c, // 0 1 1 1 1 1 0 0
    [0xc] = 0x39, // 0 0 1 1 1 0 0 1
    [0xd] = 0x5e, // 0 1 0 1 1 1 1 0
    [0xe] = 0x79, // 0 1 1 1 1 0 0 1
    [0xf] = 0x71, // 0 1 1 1 0 0 0 1
};

static u8 startup_state[] =
{
          //    P G F E D C B A
    0x00, //    0 0 0 0 0 0 0 0
    0x76, // H  0 1 1 1 0 1 1 0
    0x04, // i  0 0 0 0 0 1 0 0
    0x00, //    0 0 0 0 0 0 0 0
};

void display_init(void)
{
    pin_set_mode(PIN_DISP_CLK, INPUT);
    pin_set_mode(PIN_DISP_DIO, INPUT);
    pin_write(PIN_DISP_CLK, 0);
    pin_write(PIN_DISP_DIO, 0);

    display_setsegs(startup_state, 1);
}

static void disp_delay(void)
{
    _delay_us(BIT_DELAY);
}
static void start_command(void)
{
    pin_set_mode(PIN_DISP_DIO, OUTPUT);
    disp_delay();
}
static void end_command(void)
{
    pin_set_mode(PIN_DISP_DIO, OUTPUT);
    disp_delay();
    pin_set_mode(PIN_DISP_CLK, INPUT);
    disp_delay();
    pin_set_mode(PIN_DISP_DIO, INPUT);
    disp_delay();
}
static void write_byte(u8 val)
{
    u8 ack;

    for (u8 i = 0; i < 8; i++) {
        pin_set_mode(PIN_DISP_CLK, OUTPUT);
        disp_delay();

        if (val & (1 << i))
            pin_set_mode(PIN_DISP_DIO, INPUT);
        else
            pin_set_mode(PIN_DISP_DIO, OUTPUT);
        disp_delay();

        pin_set_mode(PIN_DISP_CLK, INPUT);
        disp_delay();
    }

    pin_set_mode(PIN_DISP_CLK, OUTPUT);
    pin_set_mode(PIN_DISP_DIO, INPUT);
    disp_delay();

    pin_set_mode(PIN_DISP_CLK, INPUT);
    disp_delay();

    ack = pin_read(PIN_DISP_DIO);
    if (ack == 0)
        pin_set_mode(PIN_DISP_DIO, OUTPUT);
    else
        LOG("ERROR: No ACK from TM1637");
    disp_delay();

    pin_set_mode(PIN_DISP_CLK, OUTPUT);
    disp_delay();
}

void display_setsegs(u8 segs[DISPLAY_NUM_DIGITS], u8 brightness)
{
    start_command();
    write_byte(COMM1);
    end_command();

    start_command();
    write_byte(COMM2 | 0);
    for (u8 i = 0; i < DISPLAY_NUM_DIGITS; i++)
        write_byte(segs[i]);
    end_command();

    start_command();
    write_byte(COMM3 | (brightness & 0xf) | DISP_ON);
    end_command();
}

void display_shownum(u16 num, bool colon, u8 brightness)
{
    u8 segs[DISPLAY_NUM_DIGITS];

    for (u8 i = 0; i < DISPLAY_NUM_DIGITS; i++) {
        u8 pos = DISPLAY_NUM_DIGITS - i - 1;
        u8 digit = num % 10;
        segs[pos] = segment_lut[digit];

        if (colon && pos == 1)
            segs[pos] |= 0x80;
        num /= 10;
    }

    display_setsegs(segs, brightness);
}
