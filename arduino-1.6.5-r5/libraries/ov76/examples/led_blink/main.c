/*
 */

#include <avr/io.h>
#include <util/delay.h>
// LED BL_K   PD7

int main(void)
{
    DDRD |= (1<<PD7);

    while(1)
    {
        PORTD |=  (1<<PD7); // PD7 = 1
        _delay_ms(1000);
        PORTD &= ~(1<<PD7); // PD7 = 0
        _delay_ms(1000);
    }

    return 0;
}
