/*

wangguanfu
2009-08-06

*/

#include<avr/io.h>

///////////////////////////////////////////////////////
#define	BLACK	0x0000
#define	BLUE	0x001F
#define	RED 	0xF800
#define	GREEN 	0x07E0
#define CYAN	0x07FF
#define MAGENTA 0xF81F
#define YELLOW	0xFFE0
#define WHITE	0xFFFF

#define ID_AM         011

#define rw_data_prepare()               LCD_WR_REG16(0x0022)

void GUI_Point(unsigned int x, unsigned int y,unsigned int color) ;
void LCD_WR_REG(unsigned int index,unsigned int data);
void LCD_WR_REG16(unsigned int index);//写积存器命令
void LCD_WR_DATA16(unsigned int data);//西积存器数据
void LCD_Init(void);
void LCD_Clear(unsigned int Color);
void LCD_SetCursor(unsigned int Xpos, unsigned int Ypos);
