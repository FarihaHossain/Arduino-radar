#include <avr/io.h>

// 8,N,1
void uart_init(unsigned long baud)
{
    UBRRH = ( ( F_CPU/8/baud )-1 )>>8;
    UBRRL = ( ( F_CPU/8/baud )-1 ) ;

    UCSRA |=  (1<<U2X); // ±¶ËÙ
    UCSRB  = (1<<RXEN)|(1<<TXEN);

    DDRD &= ~(1<<PD0); // RXD PD0 input
    DDRD |=  (1<<PD1); // TXD PD1 output
}

void uart_sendbyte(unsigned char date)
{
    while( !(UCSRA & (1<<UDRE)) );
    UDR = date;
}

void uart_print(char * str)
{
    while(*str)
    {
        uart_sendbyte(*str++);
    }
}

#include <stdio.h>

static int uart_putchar(char c, FILE *stream);
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
                       _FDEV_SETUP_WRITE);
static int
uart_putchar(char c, FILE *stream)
{
    if (c == '\n')
        uart_putchar('\r', stream);
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
    return 0;
}

void printf_init(void)
{
    stdout = &mystdout;
}
