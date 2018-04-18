/*

wangguanfu
2009-08-06

*/

#ifndef _OV7670_H
#define _OV7670_H

#include <avr/io.h>
#include "VARIABLE.h"
#include "font.h"
#include "SCCB.h"

#define CHANGE_REG_NUM 173



uchar wrOV7670Reg(uchar regID, uchar regDat);
uchar rdOV7670Reg(uchar regID, uchar *regDat);
void OV7670_config_window(uint startx,uint starty,uint width, uint height);
void my_delay_ms(uint time);//delay some time
uchar OV7670_init(void);


#endif /* _OV7660_H */



