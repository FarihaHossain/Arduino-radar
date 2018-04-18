/*

wangguanfu
2010-04-27

*/



#include <avr/io.h>
#include <avr/interrupt.h>
#include "ILI9331.h"
#include "ov7670.h"
#include "int.h"
#include <util/delay.h>
#include "FAT.h"
#include "config.h"
#include "mmc_sd.h"

unsigned int image_count = 0 ,image_all = 0 ;



extern uint16 SectorsPerClust;//每簇扇区数

extern uint8  FAT32_Enable;

void disp_image(void)
{
    WORD count= 1 ,i =0   ;
    uint8 *buffer ;
    uint8 type = 0 ,x = 0 , rgb = 0,first = 0 ,color_byte = 0,byte1 = 0,byte2 = 0,byte3 = 0 ;
    uint16 y = 0 ,color = 0,tmp_color = 0 ;
    uint32 p;				//簇指示值			//cluster
    type = 1 ;
//    struct FileInfoStruct FileInfo;//文件信息

    struct direntry BMPInfo;	//要播放的mp3文件信息		//the mp3 file item whichi will be played

    LCD_WR_REG(0x0050,0);//水平 GRAM起始位置
    LCD_WR_REG(0x0051,239);//水平GRAM终止位置
    LCD_WR_REG(0x0052,0);//垂直GRAM起始位置
    LCD_WR_REG(0x0053,319);//垂直GRAM终止位置

    if( image_all == 0 )    //读取总图片数
    {
        image_count =  0 ;
        byte1 = Search(&BMPInfo,&image_count,&type) ;
        image_all = image_count ;
        image_count = 1 ;

        /*
        image_count = 0 ;
        Search(&BMPInfo,&image_count,&type) ;
        image_all = image_count ;
        GUI_sprintf_chartohex(10,300,0x00,0xffff,0x0000) ;
        GUI_sprintf_chartohex(30,300,image_count,0xffff,0x0000) ;
        GUI_sprintf_chartohex(60,300,image_all,0xffff,0x0000) ;
        */
    }
    else
    {
        byte1 = Search(&BMPInfo,&image_count,&type) ;
    }



    p = BMPInfo.deStartCluster+(((uint32)BMPInfo.deHighClust)<<16);//读文件首簇	//the first cluster of the file

    x = 0 ;
    y = 319 ;
    rgb = 0 ;
    count = 0 ;
    first = 0 ;
    buffer=malloc(512);
    while(1)
    {
        i = 0 ;

        for( ; i < SectorsPerClust ; i++ )      //簇
        {

            FAT_LoadPartCluster(p,i,buffer);//读一个扇区	//read a sector

            if(i==0 && first==0)
            {
                count= buffer[0x0a] ;
                color_byte = buffer[0x1c] / 8  ;
                first = 1 ;
            }
            else
            {
                count=0 ;
            }

            while(count<512)  //读取一簇512扇区 (SectorsPerClust 每簇扇区数)
            {

                if(color_byte == 3)   //24位颜色图
                {
                    switch ( rgb )
                    {
                    case 0 :
                        tmp_color = buffer[count]>>3 ;
                        color |= tmp_color;
                        break ;

                    case 1 :
                        tmp_color = buffer[count]>>2 ;
                        tmp_color <<= 5 ;
                        color |= tmp_color ;
                        break ;

                    case 2 :
                        tmp_color = buffer[count]>>3 ;
                        tmp_color <<= 11 ;
                        color |= tmp_color ;
                        break ;
                    }

                    rgb ++ ;
                }
                else
                {
                    if(color_byte==2)  //16位颜色图
                    {
                        switch ( rgb )
                        {
                        case 0 :
                            byte1 = buffer[count] ;
                            break ;

                        case 1 :
                            color = buffer[count] ;
                            color<<=8 ;
                            color |= byte1 ;
                            break ;

                        }
                        rgb ++ ;

                    }
                    else
                    {
                        if(color_byte==4) //32位颜色图
                        {
                            switch ( rgb )
                            {
                            case 0 :
                                byte1 = buffer[count] ;
                                break ;

                            case 1 :
                                byte2 = buffer[count] ;
                                break ;

                            case 2 :
                                byte3 = buffer[count] ;
                                break ;

                            case 3 :
                                tmp_color = byte1 >> 3 ;
                                color |= tmp_color;
                                tmp_color = byte2 >>2 ;
                                tmp_color <<= 5 ;
                                color |= tmp_color ;
                                tmp_color = byte3 >>3 ;
                                tmp_color <<= 11 ;
                                color |= tmp_color ;
                                break ;

                            }
                            rgb ++ ;

                        }

                    }

                }

                count ++ ;

                if(rgb == color_byte)        //读取1像素数数据后显示
                {
                    // GUI_Point(x, y,color) ;

                    LCD_WR_REG(0x0020,239-x);//Lcd光标GRAM水平起始位置
                    LCD_WR_REG(0x0021,319-y); //Lcd光标垂直GRAM起始位置

                    LCD_WR_REG16(0x0022);
                    LCD_WR_DATA16(color);

                    color = 0x00 ;
                    x++ ;
                    if(x>=240)
                    {
                        y-- ;

                        if( y<=0 )
                        {

                            free(buffer) ;
                            return ;
                        }
                        x = 0 ;
                    }

                    rgb = 0 ;
                }


            }

        }  // 读取完一簇数据

        free(buffer) ;
        p=FAT_NextCluster(p);//读下一簇数据			//read next cluster
        buffer=malloc(512);

        if(p == 0x0fffffff || p == 0x0ffffff8 || (FAT32_Enable == 0 && p == 0xffff))//如果无后续簇则结束，	//no more cluster
        {
            free(buffer) ;
            return ;
        }

    }

    free(buffer) ;

}


//CPU初始化//
int main(void)
{
    unsigned char retry=0;

    EXCLK_DDR|=1<<EXCLK_BIT ;//en_exclk 引脚输出
    HC245_OE_DDR|=1<<HC245_OE_BIT;//en_245引脚输出

    DISEN_EXCLK;//禁止外部CLK，使单片机WR与TFT WR相连
    DISEN_245;//禁止外部数据线，使单片机数据与TFT数据线相连

    DATA_OUPUT(); //数据线输出，准备连数据线到TFT
    LCD_Init();
    //GUI_Point(0, 0,0xffff) ;
    LCD_write_english_string(20,130,"Guanfu_Wang",RED,BLACK);
    LCD_write_english_string(20,150,__DATE__,GREEN,BLACK);
    LCD_write_english_string(20,170,"Atmega32 & OTM3225A FOR OV7670 REV4.0",GREEN,BLACK);
    LCD_write_english_string(20,190,"TF Card Init......",BLUE,BLACK);

    MMC_SD_Init();//初始化spi口
    _delay_ms(500);

    //OSCCAL = 0x00;//最小RC振荡频率
    while(MMC_SD_Reset()&(retry<5))//初始化SD卡					//sd card initialize
    {
        retry++;
        if(retry>4)
        {
            LCD_write_english_string(20,190,"TF Card Init....NG",BLUE,BLACK);
            retry = 21;

        }

    }
    if(retry<20) LCD_write_english_string(20,190,"TF Card Init....OK",BLUE,BLACK);

    if(retry<20)LCD_write_english_string(20,210,"FAT Init......",BLUE,BLACK);
    while(FAT_Init()&(retry<20))//初始化文件系统 支持FAT16和FAT32	//initialize file system  FAT16 and FAT32 are supported
    {
    }

    if(retry<20)SearchInit();  //搜索文件初始化

    if(retry<20)LCD_write_english_string(20,210,"FAT Init....OK",BLUE,BLACK);


    if(retry>20)LCD_write_english_string(20,210,"OV7670 Init......",BLUE,BLACK);

    while((1!=OV7670_init())&(retry>20));//初始化ov7660
    //_delay_ms(1000);
    if(retry>20)LCD_write_english_string(20,210,"OV7670 Init  0K  ",RED,BLACK);
    _delay_ms(1000);
    if(retry>20)LCD_Clear(0);
    if(retry>20)DATA_INPUT();
    if(retry>20)Init_INT0();

    while(1)
    {
        if(retry<20)
        {
            disp_image() ;
            _delay_ms(2000);
            if(image_count<image_all)
            {
                image_count ++ ;
            }
            else
            {
                image_count = 1 ;
            }
        }

    }

}

