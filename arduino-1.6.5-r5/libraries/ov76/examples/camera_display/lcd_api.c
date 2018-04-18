#include <avr/io.h>
#include <avr/pgmspace.h>

#include "ili_lcd_general.h"

extern const prog_char english[][16];

void LCD_write_english(uint16_t x,uint16_t y,uint8_t str,unsigned int color,unsigned int xcolor)//Ð´×Ö·û
{
    uint16_t xpos = x;
    uint16_t ypos = y;

    unsigned char avl,i,n;

    for(i=0; i<16; i++) //16ÐÐ
    {
        avl=pgm_read_byte( &english[str-32][i]);
        LCD_SetCursor(xpos,ypos);
        ypos++;
        rw_data_prepare();
        for(n=0; n<8; n++) //8ÁÐ
        {
            if(avl&0x80) LCD_WR_DATA16(color);
            else LCD_WR_DATA16(xcolor);
            avl<<=1;
        }
    }
}

void LCD_write_english_string(uint16_t x,uint16_t y,char *s,unsigned int color,unsigned int xcolor)//Ó¢ÎÄ×Ö·û´®ÏÔÊ¾
{
    while (*s)
    {
        LCD_write_english(x,y,*s,color,xcolor);
        s++;
        x += 8;
    }
}

