#ifndef UART_H_INCLUDED
#define UART_H_INCLUDED

#include <stdio.h>

extern void uart_init(unsigned long baud);
extern void uart_sendbyte(unsigned char date);
extern void uart_print(char * str);
extern void printf_init(void);

#endif // UART_H_INCLUDED
