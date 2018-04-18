/*

wangguanfu
2010-04-27

*/

#include <avr/io.h>
#include <util/delay.h>

#include "ili_lcd_general.h"
#include "lcd_api.h"

//CPU��ʼ��//
int main(void)
{
    DDRB |= (1<<PB0); // en_exclk ������� PB0
    DDRD |= (1<<PD6); // en_245   ������� PD6

    PORTB &= ~(1<<PB0); //DISEN_EXCLK;//��ֹ�ⲿCLK��ʹ��Ƭ��WR��TFT WR����
    PORTD |=  (1<<PD6); //DISEN_245;  //��ֹ�ⲿ�����ߣ�ʹ��Ƭ��������TFT����������

    //��ʼ��LCD
    LCD_Init();

    LCD_write_english_string(20,130,"Guanfu_Wang",RED,BLACK);
    LCD_write_english_string(20,150,"use avrgcc:",GREEN,BLACK);
    LCD_write_english_string(20+8*12,150,__AVR_LIBC_DATE_STRING__,GREEN,BLACK);
    LCD_write_english_string(20,170,__DATE__,GREEN,BLACK);
    LCD_write_english_string(20,190,"Atmega32 & OTM3225A FOR OV7670 REV4.0",GREEN,BLACK);

    while(1);
}

