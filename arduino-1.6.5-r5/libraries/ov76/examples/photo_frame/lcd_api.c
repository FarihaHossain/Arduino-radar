#include <avr/io.h>
#include <avr/pgmspace.h>

#include "ili_lcd_general.h"

extern const prog_char english[][16];

void LCD_write_english(uint16_t x,uint16_t y,uint8_t str,unsigned int color,unsigned int xcolor)//写字符
{
    uint16_t xpos = x;
    uint16_t ypos = y;

    unsigned char avl,i,n;

            /* [5:4]-ID1 H ID0 V : 0 decrement 1 increment [3]-AM-1垂直-0水平 */
    LCD_WR_REG(0x0003,(1<<12)|(1<<5)|(1<<4) | (1<<3) );

    for(i=0; i<16; i++) //16行
    {
        avl=pgm_read_byte( &english[str-32][i]);
        LCD_SetCursor(xpos,ypos);
        ypos++;
        rw_data_prepare();
        for(n=0; n<8; n++) //8列
        {
            if(avl&0x80) LCD_WR_DATA16(color);
            else LCD_WR_DATA16(xcolor);
            avl<<=1;
        }
    }
}

void LCD_write_english_string(uint16_t x,uint16_t y,char *s,unsigned int color,unsigned int xcolor)//英文字符串显示
{
    while (*s)
    {
        LCD_write_english(x,y,*s,color,xcolor);
        s++;
        x += 8;
    }
}

