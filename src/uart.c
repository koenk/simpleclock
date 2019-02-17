#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"

#define BAUDRATE 9600UL

FILE uart_fd = FDEV_SETUP_STREAM(uart_fputc, NULL, _FDEV_SETUP_WRITE);

#define RECV_BUF_MAX 32
static char recv_buf[RECV_BUF_MAX];
static unsigned char recv_buf_size = 0;
static uart_recv_cb_t recv_cb = NULL;

void uart_init(void)
{
    int lbt, div;

    /*
     * Software reset the LIN/UART controller.
     */
    LINCR = 1 << LSWRES;

    /*
     * Set baudrate (15.5.6.1, formula incorrect in datasheet).
     */
    lbt = 26; /* Prescaler - 26 works out nicely for 9600 @ 1MHz */
    /* Add half of baudrate for better rounding */
    div = (F_CPU + (lbt / 2) * BAUDRATE) / (lbt * BAUDRATE) - 1;
    LINBTR = (1<<LDISR) | lbt;
    LINBRRL = div & 0xff;
    LINBRRH = (div >> 8) & 0xff;

    /*
     * Enable Rx interrupts/
     */
    LINENIR = 1 << LENRXOK;

    /*
     * Enable controller, with full duplex (send and receive) in 8N1 mode.
     */
    LINCR = (1 << LENA) | /* Enable controller */
            (1 << LCMD2) | /* UART */
            (1 << LCMD1) | /* Rx */
            (1 << LCMD0) /* Tx */
            ; /* LCONF[0:1] = 0 - 8N1 */

    stdout = stderr = &uart_fd;
}

/* Rx interrupt */
ISR(LIN_TC_vect)
{
    char val;

    cli();

    val = LINDAT; /* Read data and re-enable Rx interrupts. */

    /* TODO */
    uart_putchar(val);

    if (val == '\n' || val == '\r') {
        if (recv_buf_size) {
            recv_buf[recv_buf_size] = '\0';
            recv_buf_size = 0;
            if (recv_cb)
                recv_cb(recv_buf);
        }
    } else {
        recv_buf[recv_buf_size++] = val;
        if (recv_buf_size == RECV_BUF_MAX - 1)
            recv_buf_size = 0;
    }

    sei();
}

char uart_putchar(const char c)
{
    loop_until_bit_is_clear(LINSIR, LBUSY);
    LINDAT = c;
    return c;
}

int uart_fputc(const char c, FILE *stream)
{
    (void)stream;
    return uart_putchar(c);
}

void uart_puts(const char *s)
{
    while (*s)
        uart_putchar(*s++);
}

void uart_set_recv_callback(uart_recv_cb_t func)
{
    recv_cb = func;
}
