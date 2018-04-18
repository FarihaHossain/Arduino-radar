/*

wangguanfu
2009-10-15

*/


#ifndef _INT_H_
#define _INT_H_

#include <avr/interrupt.h>
#include <avr/io.h>

////////////////////////////////////


#define HC245_OE_DDR  DDRD
#define HC245_OE_PORT PORTD
#define HC245_OE_PIN  PIND
#define HC245_OE_BIT  6

#define EXCLK_DDR  DDRB
#define EXCLK_PORT PORTB
#define EXCLK_PIN  PINB
#define EXCLK_BIT  0



#define EN_245			HC245_OE_PORT&=~(1<<HC245_OE_BIT)
#define DISEN_245		HC245_OE_PORT|=1<<HC245_OE_BIT
#define EN_EXCLK		EXCLK_PORT|=1<<EXCLK_BIT 
#define DISEN_EXCLK	    EXCLK_PORT&=~(1<<EXCLK_BIT )


//////////////////////////////////
void Init_INT0(void);

#endif

