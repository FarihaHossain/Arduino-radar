/*

wangguanfu
2010-04-27

*/

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#include "ili_lcd_general.h"
#include "lcd_api.h"
#include "mmc_sd.h"
#include "uart.h"

#include "ff.h"

extern BYTE MMC_SD_ReadSingleBlock(DWORD sector, BYTE* buffer);

static uint8_t tmp_buffer[ 768 ]; //512太小,1024太大,容易造成栈溢出
static FATFS fs;

uint16_t X1R5G5B5_R5G6B5(uint16_t input)
{
    uint16_t  r, g, b, rgb;
    input &= 0x7FFF;//去掉X1
    r = (input>>0)  & 0x1F;
    g = (input>>5)  & 0x1F;
    b = (input>>10) & 0x1F;
    rgb = (b<<11) + (g<<6) + (r<<0);

    return rgb;
}

//图片显示函数
void show_image(const char * file_name)
{
    FIL file1;
    FRESULT result;
    uint16_t br;
    uint16_t i;
    uint16_t color = 0;
    unsigned long offset = 0;
    uint16_t biBitCount = 0 ;//用来保存像素比特数
    uint16_t biwidth = 0;
    uint16_t biHeight = 0;

    //打开文件
    result = f_open(&file1,file_name,FA_OPEN_EXISTING|FA_READ);
    if( result == FR_OK )//打开成功
    {

        result = f_read(&file1,tmp_buffer,54,&br);//读取54字节文件头

        biwidth    = *(uint32_t *)(tmp_buffer+0x12);//得到图像宽度
        biHeight   = *(uint32_t *)(tmp_buffer+0x16);//得到图像宽度
        biBitCount = *(uint16_t *)(tmp_buffer+0x1C);//获得像素比特数
        offset = *(uint32_t *)(tmp_buffer+0x0A);//得到图像数据的偏移量
        result = f_lseek(&file1,offset);

        //判断宽度和调试,再设置合适的扫描方式
        if( (biwidth == 240) && (biHeight==320) )
        {
            /* [5:4]-ID1 H ID0 V : 0 decrement 1 increment [3]-AM-1垂直-0水平 */
            LCD_WR_REG(0x0003,(1<<12)|(0<<5)|(0<<4) | (0<<3) );
            //设置光标为 右下角
            LCD_SetCursor(0,0);
        }
        else if( (biwidth == 320) && (biHeight==240) )
        {
            /* [5:4]-ID1 H ID0 V : 0 decrement 1 increment [3]-AM-1垂直-0水平 */
            LCD_WR_REG(0x0003,(1<<12)|(1<<5)|(0<<4) | (1<<3) );
            //设置光标为 左下角
            LCD_SetCursor(0,239);
        }
        else
        {
            printf("\r\nskip size: width:%d high:%d",biwidth,biHeight);
            return;
        }//判断宽度和调试,再设置合适的扫描方式

        rw_data_prepare();

        //根据像素比特数使用不同的算法
        switch( biBitCount )
        {
        case 16: // XRGB 1555
        {
            result = f_lseek(&file1,offset+2);
            //读取并发送图像数据
            for(i=0; i<(320*240UL)/384 ; i++)//每次处理384像素 384*2=768 200次可以处理完
            {
                unsigned char * buffer_point = NULL;
                unsigned int  n;

                result = f_read(&file1,tmp_buffer,384*2,&br);//读取 384 像素数据,每像素2字节
                buffer_point = tmp_buffer;
                for(n=0; n<384; n++)//转换并发送 384 像素
                {
                    color  = *(uint16_t *)buffer_point;
                    buffer_point += 2;
                    LCD_WR_DATA16( X1R5G5B5_R5G6B5(color) );//BGR2RGB(color)
                }

            }//读取并发送图像数据
        }
        break;
        case 24: // RGB 888
        {
            //读取并发送图像数据
            for(i=0; i<(320*240UL)/256; i++)//每次处理 256 像素 256*3=768  300次可以处理完
            {
                unsigned char * buffer_point = NULL;
                unsigned int  n;

                result = f_read(&file1,tmp_buffer,256*3,&br);//读取 256 像素数据,每像素3字节
                buffer_point = tmp_buffer;
                for(n=0; n<256; n++)//转换并发送320像素
                {
                    color = 0;
                    color |= ( *buffer_point++ >> 3 );
                    color |= ( *buffer_point++ >> 2 ) << 5;
                    color |= ( *buffer_point++ >> 3 ) << 11;
                    LCD_WR_DATA16(color);
                }

            }//读取并发送图像数据
        }
        break;
        case 32: // RGBA 888A
        {
            //读取并发送图像数据
            for(i=0; i<(320*240UL)/192; i++)//每次处理 192 像素,192*4=768字节 400次可以处理完
            {
                unsigned char * buffer_point = NULL;
                unsigned int  n;

                result = f_read(&file1,tmp_buffer,192*4,&br);//读取320像素数据,每像素4字节
                buffer_point = tmp_buffer;
                for(n=0; n<192; n++)//转换并发送256像素
                {
                    color = 0;
                    color |= ( *buffer_point++ >> 3 );
                    color |= ( *buffer_point++ >> 2 ) << 5;
                    color |= ( *buffer_point++ >> 3 ) << 11;
                    buffer_point++;
                    LCD_WR_DATA16(color);
                }

            }//读取并发送图像数据
        }
        break;
        default:  //不支持的图像格式
            printf("\r\nnot supported biBitCount:%d",biBitCount);
            return;
        }//switch( biBitCount ) 根据像素比特数使用不同的算法
    }//打开成功

}

//播放图片
void play(void)
{
    FILINFO fno;
    DIR dir;
    char *fn;
    FRESULT result;
    uint8_t total = 0; //用来计数找到的文件
    char path[32];

    //打开根目录
    result = f_opendir(&dir, "/");
    if( result != FR_OK )
    {
        LCD_write_english_string(0,0,"f_opendir ERR!",GREEN,BLACK);
        LCD_write_english_string(0,20,"ERR CODE: ",RED,BLACK);
        LCD_write_english(8*9,20,'0'+result,RED,BLACK);
        return;//有错就直接返回
    }

    //开始遍列目录
    for(;;)
    {
        result = f_readdir(&dir, &fno);
        if ( result != FR_OK || fno.fname[0] == 0)
        {
            break;
        }
        if (fno.fname[0] == '.') continue;
        fn = fno.fname;

        if( (strstr(fn,".bmp") != NULL)
                || (strstr(fn,".BMP") != NULL)
          )
        {
            sprintf( path , "/%s", fn );
            printf("\r\nshow : %s",path);
            total ++;
            show_image( fn );
            _delay_ms(500);
            _delay_ms(500);
            _delay_ms(500);
            _delay_ms(500);
        }
        else
        {
            printf("\r\nskip : %s",fn);
        }
    }
    //判断本次有没找到文件.
    if( total == 0)
    {
        LCD_write_english_string(0,100,"no find bmp file",RED,BLACK);
    }
}

//CPU初始化//
int main(void)
{
    DDRB |= (1<<PB0); // en_exclk 引脚输出 PB0
    DDRD |= (1<<PD6); // en_245   引脚输出 PD6

    PORTB &= ~(1<<PB0); //DISEN_EXCLK;//禁止外部CLK，使单片机WR与TFT WR相连
    PORTD |=  (1<<PD6); //DISEN_245;  //禁止外部数据线，使单片机数据与TFT数据线相连

    //初始化LCD
    LCD_Init();

    LCD_write_english_string(20,130,"Guanfu_Wang",RED,BLACK);
    LCD_write_english_string(20,150,"use avrgcc:",GREEN,BLACK);
    LCD_write_english_string(20+8*12,150,__AVR_LIBC_DATE_STRING__,GREEN,BLACK);
    LCD_write_english_string(20,170,__DATE__,GREEN,BLACK);
    LCD_write_english_string(20,190,"Atmega32 & OTM3225A FOR OV7670 REV4.0",GREEN,BLACK);

    uart_init(115200);
    printf_init();
    printf("\r\nsystem startup");

    {
        FRESULT result;

        result = f_mount(0, &fs);
        printf("\r\nf_mount result:%d",result);
        //检查是否有错.
        if( result != FR_OK )
        {
            LCD_write_english_string(0,0,"f_mount ERR!",GREEN,BLACK);
            LCD_write_english_string(0,20,"ERR CODE: ",RED,BLACK);
            LCD_write_english(8*9,20,'0'+result,RED,BLACK);
            while(1);//有错就直接返回
        }

    }

    while(1)
    {
        play();
    }
}

