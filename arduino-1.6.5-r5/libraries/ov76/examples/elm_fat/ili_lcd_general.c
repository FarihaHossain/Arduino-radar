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

void LCD_WR_DATA16(unsigned int data)//д����������
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
void LCD_WR_REG16(unsigned int index)//д����������
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
// ���óɺ���
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
    DATA_OUPUT(); //�����������׼���������ߵ�TFT
    LCD_PORT_INI();

    LCD_RST_L();//Ӳ����λ
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
    /* [5:4]-ID1~ID0 [3]-AM-0��ֱ-1ˮƽ */
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

    LCD_BL_H();//�򿪱���
}


/**********************************************
��������Lcdȫ����������
���ܣ���Lcd������Ϊָ����ɫ
��ڲ�����color ָ��Lcdȫ����ɫ RGB(5-6-5)
����ֵ����
***********************************************/
void LCD_Clear(unsigned int Color)
{
    unsigned long index = 0;

    LCD_WR_REG(0x0020,0x0000);//GRAMˮƽ��ʼλ��
    LCD_WR_REG(0x0021,00000);

    LCD_WR_REG(0x0050,0x00);//ˮƽ GRAM��ʼλ��
    LCD_WR_REG(0x0051,239);//ˮƽGRAM��ֹλ��
    LCD_WR_REG(0x0052,0x00);//��ֱGRAM��ʼλ��
    LCD_WR_REG(0x0053,319);//��ֱGRAM��ֹλ��
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

//void LCD_write_SUM(unsigned int x,unsigned int y,unsigned char SUM,unsigned int color,unsigned int xcolor)//д����
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
��������Lcd��ѡ����
���ܣ�ѡ��Lcd��ָ���ľ�������

ע�⣺xStart�� yStart������Ļ����ת���ı䣬λ���Ǿ��ο���ĸ���

��ڲ�����xStart x�������ʼ��
          ySrart y�������ֹ��
          xLong Ҫѡ�����ε�x���򳤶�
          yLong  Ҫѡ�����ε�y���򳤶�
����ֵ����
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

    LCD_WR_REG(0x0050,xStart);//ˮƽ GRAM��ʼλ��
    LCD_WR_REG(0x0051,xStart+xLong-1);//ˮƽGRAM��ֹλ��
    LCD_WR_REG(0x0052,312-yStart);//��ֱGRAM��ʼλ��
    LCD_WR_REG(0x0053,312-yStart+yLong-1);//��ֱGRAM��ֹλ��
}


/*************************************************
��������Lcd�����㶨λ����
���ܣ�ָ��320240Һ���ϵ�һ����Ϊд���ݵ���ʼ��
��ڲ�����x ���� 0~319
           y ���� 0~249
����ֵ����
*************************************************/
void LCD_SetCursor(unsigned int Xpos, unsigned int Ypos)
{
    LCD_WR_REG(0x20, Ypos);
    LCD_WR_REG(0x21, Xpos);
}

void  GUI_Point(unsigned int x, unsigned int y,unsigned int color)
{
    LCD_WR_REG(0x0020,x);//Lcd���GRAMˮƽ��ʼλ��
    LCD_WR_REG(0x0021,y); //Lcd��괹ֱGRAM��ʼλ��
    LCD_WR_REG16(0x0022);
    LCD_WR_DATA16(color);
}

//void darw_str_test(void)
//{
//    LCD_write_english(100,100,'!',RED,GREEN);
//    LCD_write_english_string(200,200,"test",GREEN,BLACK);
//}
