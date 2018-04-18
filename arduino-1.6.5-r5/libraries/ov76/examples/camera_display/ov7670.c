/*

wangguanfu
2009-08-06

*/



#include "ov7670.h"
#include <util/delay.h>
#include <avr/pgmspace.h>

extern const prog_char change_reg[CHANGE_REG_NUM][2];

////////////////////////////
//功能：写OV7660寄存器
//返回：1-成功	0-失败
uchar wrOV7670Reg(uchar regID, uchar regDat)
{
    startSCCB();
    if(0==SCCBwriteByte(0x42))
    {
        stopSCCB();
        return(0);
    }
    _delay_us(100);
    if(0==SCCBwriteByte(regID))
    {
        stopSCCB();
        return(0);
    }
    _delay_us(100);
    if(0==SCCBwriteByte(regDat))
    {
        stopSCCB();
        return(0);
    }
    stopSCCB();

    return(1);
}
////////////////////////////
//功能：读OV7660寄存器
//返回：1-成功	0-失败
uchar rdOV7670Reg(uchar regID, uchar *regDat)
{
    //通过写操作设置寄存器地址
    startSCCB();
    if(0==SCCBwriteByte(0x42))
    {
        stopSCCB();
        return(0);
    }
    _delay_us(100);
    if(0==SCCBwriteByte(regID))
    {
        stopSCCB();
        return(0);
    }
    stopSCCB();

    _delay_us(100);

    //设置寄存器地址后，才是读
    startSCCB();
    if(0==SCCBwriteByte(0x43))
    {
        stopSCCB();
        return(0);
    }
    _delay_us(100);
    *regDat=SCCBreadByte();
    noAck();
    stopSCCB();
    return(1);
}




/* OV7670_init() */
//返回1成功，返回0失败
uchar OV7670_init(void)
{
    uchar temp;

    uint i=0;


    InitSCCB();//io init..

    temp=0x80;
    if(0==wrOV7670Reg(0x12, temp)) //Reset SCCB
    {
        return 0 ;
    }
    _delay_ms(10);

    for(i=0; i<CHANGE_REG_NUM; i++)
    {
        if( 0==wrOV7670Reg(pgm_read_byte( &change_reg[i][0]),pgm_read_byte( &change_reg[i][1]) ))
        {
            return 0;
        }
    }

    return 0x01; //ok


}

