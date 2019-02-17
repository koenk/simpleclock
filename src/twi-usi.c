/*
 * Implement TWI (I2C) over the USI, following Note AVR310.
 */

#include <util/delay.h>

#include "twi.h"
#include "pins.h"
#include "uart.h"

/* Configuration data of USI module (we write this into USICR). */
#define USI_CONF (0<<USISIE | 0<<USIOIE |            /* Disable Interrupts */  \
                  1<<USIWM1 | 0<<USIWM0 |            /* Two-wire mode */       \
                  1<<USICS1 | 0<<USICS0 | 1<<USICLK) /* Software clock */

/* Reset USI status (written into USISR) */
#define USI_STATUS_RESET                                                       \
                (1<<USISIF | 1<<USIOIF | /* Clear interrupt flags */           \
                 1<<USIPF |              /* Clear stop condition flag */       \
                 1<<USIDC |              /* Clear data output collision flag */\
                 0x0<<USICNT0)           /* Reset 4-bit counter */

void twi_init(void)
{
    /* Pull-up on SDA and SCL and set as outputs */
    pin_write(PIN_TWI_SDA, 1);
    pin_write(PIN_TWI_SCL, 1);
    pin_set_mode(PIN_TWI_SCL, OUTPUT);
    pin_set_mode(PIN_TWI_SDA, OUTPUT);

    /* Preload data register with "released level" data */
    USIDR = 0xFF;

    USICR = USI_CONF;
    USISR = USI_STATUS_RESET;
}

/* Delays for standard mode, from AVR310. */
static inline void delay_short(void)
{
    _delay_us(4);
}
static inline void delay_long(void)
{
    _delay_us(5);
}

static u8 transfer(u8 num_bits)
{
    u8 transfer_clk;
    u8 data;

    /* We have to configure the clock to transfer either 1 or 8 bits. Each bit
     * takes two cycles, meaning we either set the clock to 14 (two ticks before
     * overflow/completion) or 0 (16 ticks) respectively. */
    transfer_clk = 16 - num_bits * 2;
    USISR = USI_STATUS_RESET |
            transfer_clk<<USICNT0;  /* 4-bit counter value */

    do {
        delay_long();
        USICR = USI_CONF | 1<<USITC; /* Positive SCL edge */
        while (!pin_read(PIN_TWI_SCL))
            ; /* Wait for high SCL */
        delay_short();
        USICR = USI_CONF | 1<<USITC; /* Negative SCL edge */
    } while (!(USISR & 1<<USIOIF)); /* Wait for transfer completion */

    /* Read data sent by slave (if applicable) and release SDA */
    delay_long();
    data = USIDR;
    USIDR = 0xFF;
    pin_set_mode(PIN_TWI_SDA, OUTPUT);

    return data;
}

/* Can be used for repeated start as well */
bool twi_start(u8 addr, bool do_read)
{
    /* Release SCL */
    pin_write(PIN_TWI_SCL, 1);
    while (!pin_read(PIN_TWI_SCL))
        ; /* Wait for SCL write */
    delay_long();

    /* Start condition */
    pin_write(PIN_TWI_SDA, 0);
    delay_short();
    pin_write(PIN_TWI_SCL, 0);
    pin_write(PIN_TWI_SDA, 1);

    /* Verify start condition detector picked up start condition */
    if (!(USISR & (1<<USISIF))) {
        LOG("ERROR: Could not start TWI transfer");
        return false;
    }

    /* Write target address and R/W flag */
    USIDR = (addr << 1) | do_read;
    transfer(8);

    /* Read ACK bit */
    pin_set_mode(PIN_TWI_SDA, INPUT);
    if (!transfer(1)) {
        LOG("ERROR: No ACK for address %u\n", addr);
        return false;
    }

    return true;
}

void twi_stop(void)
{
    pin_write(PIN_TWI_SDA, 0);
    pin_write(PIN_TWI_SCL, 1);
    while (!pin_read(PIN_TWI_SCL))
        ; /* Wait for SCL write */
    delay_short();
    pin_write(PIN_TWI_SDA, 1);
    delay_long();
}

bool twi_write(u8 data)
{
    pin_write(PIN_TWI_SCL, 0);
    USIDR = data;
    transfer(8);

    pin_set_mode(PIN_TWI_SDA, INPUT);
    if (!transfer(1)) {
        LOG("ERROR: No ack for sending %x", data);
        return false;
    }
    return true;
}

u8 twi_read(bool last_read)
{
    u8 data;

    pin_set_mode(PIN_TWI_SDA, INPUT);
    data = transfer(8);

    /* Send ACK, or NACK for last byte */
    USIDR = last_read ? 0xff : 0x00;
    transfer(1);

    return data;
}
