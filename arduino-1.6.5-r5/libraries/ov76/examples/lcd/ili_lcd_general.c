/*

wangguanfu
2009-08-06

*/
#include "ili_lcd_general.h"
#include <util/delay.h>

//-----------------------------------------
#define DATA_LCD_DDR  DDRC
#define DATA_LCD_PORT PORTC
#define DATA_LCD_PIN  PINC

#define DATA_INPUT()   DATA_LCD_DDR = 0x00;
#define DATA_OUPUT()   DATA_LCD_DDR = 0xFF;

#define LCD_CS_DDR  DDRA
#define LCD_CS_PORT PORTA
#define LCD_CS_PIN  PINA
#define LCD_CS_BIT  5

#define LCD_RS_DDR  DDRA
#define LCD_RS_PORT PORTA
#define LCD_RS_PIN  PINA
#define LCD_RS_BIT  6

#define LCD_RD_DDR  DDRA
#define LCD_RD_PORT PORTA
#define LCD_RD_PIN  PINA
#define LCD_RD_BIT  7

#define LCD_WR_DDR  DDRB
#define LCD_WR_PORT PORTB
#define LCD_WR_PIN  PINB
#define LCD_WR_BIT  3

#define LCD_RST_DDR  DDRB
#define LCD_RST_PORT PORTB
#define LCD_RST_PIN  PINB
#define LCD_RST_BIT  1


#define LCD_BL_DDR  DDRD
#define LCD_BL_PORT PORTD
#define LCD_BL_PIN  PIND
#define LCD_BL_BIT  7

#define LCD_CS_H() LCD_CS_PORT |= 1<<LCD_CS_BIT
#define LCD_CS_L() LCD_CS_PORT &= ~(1<<LCD_CS_BIT)

#define LCD_RS_H() LCD_RS_PORT |= 1<<LCD_RS_BIT
#define LCD_RS_L() LCD_RS_PORT &= ~(1<<LCD_RS_BIT)

#define LCD_WR_L() LCD_WR_PORT |= 1<<LCD_WR_BIT//
#define LCD_WR_H() LCD_WR_PORT &= ~(1<<LCD_WR_BIT)//

#define LCD_RD_H() LCD_RD_PORT |= 1<<LCD_RD_BIT
#define LCD_RD_L() LCD_RD_PORT &= ~(1<<LCD_RD_BIT)

#define LCD_RST_H() LCD_RST_PORT |= 1<<LCD_RST_BIT
#define LCD_RST_L() LCD_RST_PORT &= ~(1<<LCD_RST_BIT)

#define LCD_BL_H() LCD_BL_PORT |= 1<<LCD_BL_BIT
#define LCD_BL_L() LCD_BL_PORT &= ~(1<<LCD_BL_BIT)

#define LCD_PORT_INI(){ LCD_BL_DDR |= 1<<LCD_BL_BIT;\
						LCD_CS_DDR |= 1<<LCD_CS_BIT;\
						LCD_RS_DDR |= 1<<LCD_RS_BIT;\
						LCD_WR_DDR |= 1<<LCD_WR_BIT;\
						LCD_RD_DDR |= 1<<LCD_RD_BIT;\
						LCD_RST_DDR|= 1<<LCD_RST_BIT;\
					   }
//------------------------------------------------

void LCD_WR_DATA16(unsigned int data)//写积存器数据
{
    LCD_RD_H();
    LCD_RS_H();
    LCD_CS_L();
    LCD_WR_L();

    DATA_LCD_PORT=data>>8;
    _delay_us(1);
    LCD_WR_H();
    LCD_WR_L();
    _delay_us(1);
    DATA_LCD_PORT=data&0XFF;
    _delay_us(1);
    LCD_WR_H();
    LCD_CS_H();

}
void LCD_WR_REG16(unsigned int index)//写积存器命令
{
    LCD_RD_H();
    LCD_RS_L();
    LCD_CS_L();
    LCD_WR_L();
    _delay_us(1);
    DATA_LCD_PORT=index>>8;
    _delay_us(1);
    LCD_WR_H();
    LCD_WR_L();
    _delay_us(1);
    DATA_LCD_PORT=index&0XFF;
    _delay_us(1);
    LCD_WR_H();
    LCD_CS_H();
}

void LCD_WR_REG(unsigned int index,unsigned int data)
{
    LCD_WR_REG16(index);
    LCD_WR_DATA16(data);
}

#define _ILI_REVERSE_DIRECTION_
// 配置成横屏
//      0 ----- X ---- 319
//      |               |
//      |               |
//      Y               |
//      |               |
//      |               |
//     239--------------|
//
void LCD_Init(void)
{
    DATA_OUPUT(); //数据线输出，准备连数据线到TFT
    LCD_PORT_INI();

    LCD_RST_L();//硬件复位
    _delay_ms(2);
    LCD_RST_H();
    _delay_ms(2);

    //initializing funciton 1
    LCD_WR_REG(0x00e7,0x0010);
    LCD_WR_REG(0x0000,0x0001);  			        //start internal osc
#if defined(_ILI_REVERSE_DIRECTION_)
    LCD_WR_REG(0x0001,0x0000);                    //Reverse Display
#else
    LCD_WR_REG(0x0001,0x0100);                    //
#endif
    LCD_WR_REG(0x0002,0x0700); 				    //power on sequence
    /* [5:4]-ID1~ID0 [3]-AM-0垂直-1水平 */
    LCD_WR_REG(0x0003,(1<<12)|(1<<5)|(1<<4) | (1<<3) );
    LCD_WR_REG(0x0004,0x0000);
    LCD_WR_REG(0x0008,0x0207);
    LCD_WR_REG(0x0009,0x0000);
    LCD_WR_REG(0x000a,0x0000); 				//display setting
    LCD_WR_REG(0x000c,0x0001);				//display setting
    LCD_WR_REG(0x000d,0x0000); 				//0f3c
    LCD_WR_REG(0x000f,0x0000);
    //Power On sequence //
    LCD_WR_REG(0x0010,0x0000);
    LCD_WR_REG(0x0011,0x0007);
    LCD_WR_REG(0x0012,0x0000);
    LCD_WR_REG(0x0013,0x0000);
    _delay_ms(15);
    LCD_WR_REG(0x0010,0x1590);
    LCD_WR_REG(0x0011,0x0227);
    _delay_ms(15);
    LCD_WR_REG(0x0012,0x009c);
    _delay_ms(15);
    LCD_WR_REG(0x0013,0x1900);
    LCD_WR_REG(0x0029,0x0023);
    LCD_WR_REG(0x002b,0x000e);
    _delay_ms(15);
    LCD_WR_REG(0x0020,0x0000);
    LCD_WR_REG(0x0021,0x0000);
    _delay_ms(15);
    LCD_WR_REG(0x0030,0x0007);
    LCD_WR_REG(0x0031,0x0707);
    LCD_WR_REG(0x0032,0x0006);
    LCD_WR_REG(0x0035,0x0704);
    LCD_WR_REG(0x0036,0x1f04);
    LCD_WR_REG(0x0037,0x0004);
    LCD_WR_REG(0x0038,0x0000);
    LCD_WR_REG(0x0039,0x0706);
    LCD_WR_REG(0x003c,0x0701);
    LCD_WR_REG(0x003d,0x000f);
    _delay_ms(15);
    LCD_WR_REG(0x0050,0x0000);
    LCD_WR_REG(0x0051,0x00ef);
    LCD_WR_REG(0x0052,0x0000);
    LCD_WR_REG(0x0053,0x013f);
#if defined(_ILI_REVERSE_DIRECTION_)
    LCD_WR_REG(0x0060,0xA700);
#else
    LCD_WR_REG(0x0060,0x2700);
#endif
    LCD_WR_REG(0x0061,0x0001);
    LCD_WR_REG(0x006a,0x0000);
    LCD_WR_REG(0x0080,0x0000);
    LCD_WR_REG(0x0081,0x0000);
    LCD_WR_REG(0x0082,0x0000);
    LCD_WR_REG(0x0083,0x0000);
    LCD_WR_REG(0x0084,0x0000);
    LCD_WR_REG(0x0085,0x0000);
    LCD_WR_REG(0x0090,0x0010);
    LCD_WR_REG(0x0092,0x0000);
    LCD_WR_REG(0x0093,0x0003);
    LCD_WR_REG(0x0095,0x0110);
    LCD_WR_REG(0x0097,0x0000);
    LCD_WR_REG(0x0098,0x0000);
    //display on sequence
    LCD_WR_REG(0x0007,0x0133);
    LCD_WR_REG(0x0020,0x0000);
    LCD_WR_REG(0x0021,0x0000);

    LCD_Clear( BLACK );
    _delay_ms(5); // Delay 5ms

    LCD_BL_H();//打开背光
}


/**********************************************
函数名：Lcd全屏擦除函数
功能：将Lcd整屏擦为指定颜色
入口参数：color 指定Lcd全屏颜色 RGB(5-6-5)
返回值：无
***********************************************/
void LCD_Clear(unsigned int Color)
{
    unsigned long index = 0;

    LCD_WR_REG(0x0020,0x0000);//GRAM水平起始位置
    LCD_WR_REG(0x0021,00000);

    LCD_WR_REG(0x0050,0x00);//水平 GRAM起始位置
    LCD_WR_REG(0x0051,239);//水平GRAM终止位置
    LCD_WR_REG(0x0052,0x00);//垂直GRAM起始位置
    LCD_WR_REG(0x0053,319);//垂直GRAM终止位置
    LCD_WR_REG16(0x0022);

    LCD_RD_H();
    LCD_RS_H();
    LCD_CS_L();
    for(index = 0; index < 76800; index++)
    {
        // LCD_WR_DATA16(Color);

        LCD_WR_L();
        DATA_LCD_PORT=Color>>8;
        LCD_WR_H();
        LCD_WR_L();
        DATA_LCD_PORT=Color&0XFF;
        LCD_WR_H();


    }
    LCD_CS_H();
}

//void LCD_write_SUM(unsigned int x,unsigned int y,unsigned char SUM,unsigned int color,unsigned int xcolor)//写数字
//{
//
//    unsigned char avl,i,n;
//    Lcd_SetBox(y,x,15,8);
//    LCD_WR_REG16(0x0022);
//    for (i=0; i<16; i++)
//    {
//        avl=pgm_read_byte( &english[SUM+16][i]);
//        for (n=0; n<8; n++)
//        {
//            if(avl&0x80) LCD_WR_DATA16(color);
//            else LCD_WR_DATA16(xcolor);
//
//            avl<<=1;
//        }
//    }
//}
/**********************************************
函数名：Lcd块选函数
功能：选定Lcd上指定的矩形区域

注意：xStart和 yStart随着屏幕的旋转而改变，位置是矩形框的四个角

入口参数：xStart x方向的起始点
          ySrart y方向的终止点
          xLong 要选定矩形的x方向长度
          yLong  要选定矩形的y方向长度
返回值：无
***********************************************/
void Lcd_SetBox(unsigned int xStart,unsigned int yStart,unsigned int xLong,unsigned int yLong)
{

#if ID_AM==000
    LCD_SetCursor(xStart+xLong-1,312-yStart+yLong-1);

#elif ID_AM==001
    LCD_SetCursor(xStart+xLong-1,312-yStart+yLong-1);

#elif ID_AM==010
    LCD_SetCursor(xStart,312-yStart+yLong-1);

#elif ID_AM==011
    LCD_SetCursor(xStart,312-yStart+yLong-1);

#elif ID_AM==100
    LCD_SetCursor(xStart+xLong-1,312-yStart);

#elif ID_AM==101
    LCD_SetCursor(xStart+xLong-1,312-yStart);

#elif ID_AM==110
    LCD_SetCursor(xStart,312-yStart);

#elif ID_AM==111
    LCD_SetCursor(xStart,312-yStart);

#endif

    LCD_WR_REG(0x0050,xStart);//水平 GRAM起始位置
    LCD_WR_REG(0x0051,xStart+xLong-1);//水平GRAM终止位置
    LCD_WR_REG(0x0052,312-yStart);//垂直GRAM起始位置
    LCD_WR_REG(0x0053,312-yStart+yLong-1);//垂直GRAM终止位置
}


/*************************************************
函数名：Lcd光标起点定位函数
功能：指定320240液晶上的一点作为写数据的起始点
入口参数：x 坐标 0~319
           y 坐标 0~249
返回值：无
*************************************************/
void LCD_SetCursor(unsigned int Xpos, unsigned int Ypos)
{
    LCD_WR_REG(0x20, Ypos);
    LCD_WR_REG(0x21, Xpos);
}

void  GUI_Point(unsigned int x, unsigned int y,unsigned int color)
{
    LCD_WR_REG(0x0020,x);//Lcd光标GRAM水平起始位置
    LCD_WR_REG(0x0021,y); //Lcd光标垂直GRAM起始位置
    LCD_WR_REG16(0x0022);
    LCD_WR_DATA16(color);
}

//void darw_str_test(void)
//{
//    LCD_write_english(100,100,'!',RED,GREEN);
//    LCD_write_english_string(200,200,"test",GREEN,BLACK);
//}
