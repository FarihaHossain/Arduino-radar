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

static uint8_t tmp_buffer[ 768 ]; //512̫С,1024̫��,�������ջ���
static FATFS fs;

uint16_t X1R5G5B5_R5G6B5(uint16_t input)
{
    uint16_t  r, g, b, rgb;
    input &= 0x7FFF;//ȥ��X1
    r = (input>>0)  & 0x1F;
    g = (input>>5)  & 0x1F;
    b = (input>>10) & 0x1F;
    rgb = (b<<11) + (g<<6) + (r<<0);

    return rgb;
}

//ͼƬ��ʾ����
void show_image(const char * file_name)
{
    FIL file1;
    FRESULT result;
    uint16_t br;
    uint16_t i;
    uint16_t color = 0;
    unsigned long offset = 0;
    uint16_t biBitCount = 0 ;//�����������ر�����
    uint16_t biwidth = 0;
    uint16_t biHeight = 0;

    //���ļ�
    result = f_open(&file1,file_name,FA_OPEN_EXISTING|FA_READ);
    if( result == FR_OK )//�򿪳ɹ�
    {

        result = f_read(&file1,tmp_buffer,54,&br);//��ȡ54�ֽ��ļ�ͷ

        biwidth    = *(uint32_t *)(tmp_buffer+0x12);//�õ�ͼ����
        biHeight   = *(uint32_t *)(tmp_buffer+0x16);//�õ�ͼ����
        biBitCount = *(uint16_t *)(tmp_buffer+0x1C);//������ر�����
        offset = *(uint32_t *)(tmp_buffer+0x0A);//�õ�ͼ�����ݵ�ƫ����
        result = f_lseek(&file1,offset);

        //�жϿ�Ⱥ͵���,�����ú��ʵ�ɨ�跽ʽ
        if( (biwidth == 240) && (biHeight==320) )
        {
            /* [5:4]-ID1 H ID0 V : 0 decrement 1 increment [3]-AM-1��ֱ-0ˮƽ */
            LCD_WR_REG(0x0003,(1<<12)|(0<<5)|(0<<4) | (0<<3) );
            //���ù��Ϊ ���½�
            LCD_SetCursor(0,0);
        }
        else if( (biwidth == 320) && (biHeight==240) )
        {
            /* [5:4]-ID1 H ID0 V : 0 decrement 1 increment [3]-AM-1��ֱ-0ˮƽ */
            LCD_WR_REG(0x0003,(1<<12)|(1<<5)|(0<<4) | (1<<3) );
            //���ù��Ϊ ���½�
            LCD_SetCursor(0,239);
        }
        else
        {
            printf("\r\nskip size: width:%d high:%d",biwidth,biHeight);
            return;
        }//�жϿ�Ⱥ͵���,�����ú��ʵ�ɨ�跽ʽ

        rw_data_prepare();

        //�������ر�����ʹ�ò�ͬ���㷨
        switch( biBitCount )
        {
        case 16: // XRGB 1555
        {
            result = f_lseek(&file1,offset+2);
            //��ȡ������ͼ������
            for(i=0; i<(320*240UL)/384 ; i++)//ÿ�δ���384���� 384*2=768 200�ο��Դ�����
            {
                unsigned char * buffer_point = NULL;
                unsigned int  n;

                result = f_read(&file1,tmp_buffer,384*2,&br);//��ȡ 384 ��������,ÿ����2�ֽ�
                buffer_point = tmp_buffer;
                for(n=0; n<384; n++)//ת�������� 384 ����
                {
                    color  = *(uint16_t *)buffer_point;
                    buffer_point += 2;
                    LCD_WR_DATA16( X1R5G5B5_R5G6B5(color) );//BGR2RGB(color)
                }

            }//��ȡ������ͼ������
        }
        break;
        case 24: // RGB 888
        {
            //��ȡ������ͼ������
            for(i=0; i<(320*240UL)/256; i++)//ÿ�δ��� 256 ���� 256*3=768  300�ο��Դ�����
            {
                unsigned char * buffer_point = NULL;
                unsigned int  n;

                result = f_read(&file1,tmp_buffer,256*3,&br);//��ȡ 256 ��������,ÿ����3�ֽ�
                buffer_point = tmp_buffer;
                for(n=0; n<256; n++)//ת��������320����
                {
                    color = 0;
                    color |= ( *buffer_point++ >> 3 );
                    color |= ( *buffer_point++ >> 2 ) << 5;
                    color |= ( *buffer_point++ >> 3 ) << 11;
                    LCD_WR_DATA16(color);
                }

            }//��ȡ������ͼ������
        }
        break;
        case 32: // RGBA 888A
        {
            //��ȡ������ͼ������
            for(i=0; i<(320*240UL)/192; i++)//ÿ�δ��� 192 ����,192*4=768�ֽ� 400�ο��Դ�����
            {
                unsigned char * buffer_point = NULL;
                unsigned int  n;

                result = f_read(&file1,tmp_buffer,192*4,&br);//��ȡ320��������,ÿ����4�ֽ�
                buffer_point = tmp_buffer;
                for(n=0; n<192; n++)//ת��������256����
                {
                    color = 0;
                    color |= ( *buffer_point++ >> 3 );
                    color |= ( *buffer_point++ >> 2 ) << 5;
                    color |= ( *buffer_point++ >> 3 ) << 11;
                    buffer_point++;
                    LCD_WR_DATA16(color);
                }

            }//��ȡ������ͼ������
        }
        break;
        default:  //��֧�ֵ�ͼ���ʽ
            printf("\r\nnot supported biBitCount:%d",biBitCount);
            return;
        }//switch( biBitCount ) �������ر�����ʹ�ò�ͬ���㷨
    }//�򿪳ɹ�

}

//����ͼƬ
void play(void)
{
    FILINFO fno;
    DIR dir;
    char *fn;
    FRESULT result;
    uint8_t total = 0; //���������ҵ����ļ�
    char path[32];

    //�򿪸�Ŀ¼
    result = f_opendir(&dir, "/");
    if( result != FR_OK )
    {
        LCD_write_english_string(0,0,"f_opendir ERR!",GREEN,BLACK);
        LCD_write_english_string(0,20,"ERR CODE: ",RED,BLACK);
        LCD_write_english(8*9,20,'0'+result,RED,BLACK);
        return;//�д��ֱ�ӷ���
    }

    //��ʼ����Ŀ¼
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
    //�жϱ�����û�ҵ��ļ�.
    if( total == 0)
    {
        LCD_write_english_string(0,100,"no find bmp file",RED,BLACK);
    }
}

//CPU��ʼ��//
int main(void)
{
    DDRB |= (1<<PB0); // en_exclk ������� PB0
    DDRD |= (1<<PD6); // en_245   ������� PD6

    PORTB &= ~(1<<PB0); //DISEN_EXCLK;//��ֹ�ⲿCLK��ʹ��Ƭ��WR��TFT WR����
    PORTD |=  (1<<PD6); //DISEN_245;  //��ֹ�ⲿ�����ߣ�ʹ��Ƭ��������TFT����������

    //��ʼ��LCD
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
        //����Ƿ��д�.
        if( result != FR_OK )
        {
            LCD_write_english_string(0,0,"f_mount ERR!",GREEN,BLACK);
            LCD_write_english_string(0,20,"ERR CODE: ",RED,BLACK);
            LCD_write_english(8*9,20,'0'+result,RED,BLACK);
            while(1);//�д��ֱ�ӷ���
        }

    }

    while(1)
    {
        play();
    }
}

