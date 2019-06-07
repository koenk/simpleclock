#ifndef UART_H
#define UART_H

#include <stdio.h>
#include <avr/pgmspace.h>

typedef void (*uart_recv_cb_t)(char *msg);

extern FILE uart_fd;

#define LOG(msg) \
    do { \
        char ___logbuf[sizeof(msg) + 2]; \
        strcpy_P(___logbuf, PSTR(msg "\r\n")); \
        uart_puts(___logbuf); \
    } while (0)

#define LOGF(msg, ...) \
    do { \
        char ___logbuf[sizeof(msg) + 2]; \
        strcpy_P(___logbuf, PSTR(msg "\r\n")); \
        fprintf(&uart_fd, ___logbuf, ## __VA_ARGS__); \
    } while (0)

void uart_init(void);
char uart_putchar(const char c);
int uart_fputc(const char c, FILE *stream);
void uart_puts(const char *s);
void uart_set_recv_callback(uart_recv_cb_t func);


#endif
