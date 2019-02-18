/*
 * Abstraction layer for I/O of AVR ports.
 */

#ifndef PINS_H
#define PINS_H

#include <stdlib.h>

#include <avr/io.h>

#include "types.h"

/* Pin mappings, pins go from 1 to 20 */
#define PIN_UART_RX 1
#define PIN_UART_TX 2

#define PIN_RTC_INT 4 /* INT1 */

#define PIN_LED1 9
#define PIN_LED2 10

#define PIN_ISP_MISO 3
#define PIN_ISP_MOSI 7
#define PIN_ISP_SCK 8
#define PIN_ISP_RST 11

#define PIN_DISP_CLK 13
#define PIN_DISP_DIO 14

#define PIN_TWI_SDA 20
#define PIN_TWI_SCL 18
#define PIN_RTC_RST 19

#define INPUT 0
#define OUTPUT 1

/*
 * Pins that we can do I/O on.
 */
static inline u8 pin_valid(u8 pin)
{
    if (pin == 0 || pin > 20)
        return 0;
    /* VCC/GND and AVCC/AGND */
    if (pin == 5 || pin == 6 || pin == 15 || pin == 16)
        return 0;
    return 1;
}

/*
 * Map a pin (1..20) to the bit number for its registers (e.g., DDRA or PORTB).
 */
static inline u8 pin_to_bit(u8 pin)
{
    if (!pin_valid(pin))
        return 8;
    else if (pin <= 4)
        return pin - 1;
    else if (pin <= 10)
        return pin - 3; /* skip AVCC/AGND */
    else if (pin <= 14)
        return 18 - pin; /* skip VCC/GND */
    else
        return 20 - pin;
}

/*
 * Bitmask for a given pin (1..20) for its corresponding registers.
 */
static inline u8 pin_to_mask(u8 pin)
{
    return 1 << pin_to_bit(pin);
}

static inline volatile u8 *pin_to_control_reg(u8 pin)
{
    if (pin <= 10)
        return &DDRA;
    if (pin <= 20)
        return &DDRB;
    return NULL;
}
static inline volatile u8 *pin_to_output_reg(u8 pin)
{
    if (pin <= 10)
        return &PORTA;
    if (pin <= 20)
        return &PORTB;
    return NULL;
}
static inline volatile u8 *pin_to_input_reg(u8 pin)
{
    if (pin <= 10)
        return &PINA;
    if (pin <= 20)
        return &PINB;
    return NULL;
}

static inline void pin_set_mode(u8 pin, u8 mode)
{
    volatile u8 *reg = pin_to_control_reg(pin);
    u8 mask = pin_to_mask(pin);

    if (!pin_valid(pin))
        return;

    if (mode == INPUT)
        *reg &= ~mask;
    else
        *reg |= mask;
}

static inline void pin_write(u8 pin, u8 val)
{
    volatile u8 *reg = pin_to_output_reg(pin);
    u8 mask = pin_to_mask(pin);

    if (!pin_valid(pin))
        return;

    if (val)
        *reg |= mask;
    else
        *reg &= ~mask;
}

static inline u8 pin_read(u8 pin)
{
    volatile u8 *reg = pin_to_output_reg(pin);
    u8 mask = pin_to_mask(pin);

    if (!pin_valid(pin))
        return 0;

    return (*reg & mask) ? 1 : 0;
}

#endif
