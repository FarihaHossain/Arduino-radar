/*

wangguanfu
2009-10-15

*/

#include "int.h"
#include "ILI9331.h"
volatile unsigned char val1=0;

void Init_INT0(void)
{
    SREG |= 0x80 ;//使能全局中断
    MCUCR |= 0x02 ;//下降沿中断
    GICR=(1<<INT0) ;//

}


SIGNAL( SIG_INTERRUPT0 )
{
    cli();

    // if(val1)
    //{
    // val1=0;
    DISEN_EXCLK;//禁止外部CLK，使单片机WR与TFT WR相连
    DISEN_245;//禁止外部数据线，使单片机数据与TFT数据线相连
    DATA_OUPUT();//数据线输出，准备连数据线到TFT
    LCD_WR_REG(0x0020,0);//
    LCD_WR_REG(0x0021,0);//
    LCD_WR_REG(0x0050,0x00);//水平 GRAM起始位置
    LCD_WR_REG(0x0051,239);//水平GRAM终止位置
    LCD_WR_REG(0x0052,0x00);//垂直GRAM起始位置
    LCD_WR_REG(0x0053,319);//垂直GRAM终止位置
    LCD_WR_REG(0x0003,0x1028);
    LCD_WR_REG16(0x0022);

    LCD_CS_L();//数据使能
    DATA_INPUT();//数据线输入，准备连接外部数据线到TFT
    LCD_RS_H();//数据使能
    LCD_WR_L();//使能外部时钟
    EN_EXCLK;//使能外部CLK
    EN_245;//使能外部数据线，与TFT数据线相连
    /* }
     else
     {
     val1=1;
     DISEN_EXCLK;//禁止外部CLK，使单片机WR与TFT WR相连
     DISEN_245;//禁止外部数据线，使单片机数据与TFT数据线相连
     DATA_OUPUT();//数据线输出，准备连数据线到TFT
     LCD_WR_REG(0x0003,0x1018);
     //LCD_write_english_string(20,206,"Guanfu_Wang 2010-01-01",RED,BLACK);
     LCD_write_english_string(100,222,"OV7670 Demo Rev3.0",RED,BLACK);

     }
     */
    sei();
}
