/*

wangguanfu
2010-04-27

*/

#include <avr/io.h>
#include <util/delay.h>

#include "ili_lcd_general.h"
#include "lcd_api.h"
#include "mmc_sd.h"
#include "uart.h"

#include "ff.h"

extern BYTE MMC_SD_ReadSingleBlock(DWORD sector, BYTE* buffer);

uint8_t tmp_buffer[512];

void buffer_print(uint8_t * buffer)
{
    uint16_t i = 0;

    for(i=0; i<512; i++)
    {
        if( (i%16) == 0 ) printf("\r\n");
        printf("%02X ",*buffer++);
    }
    printf("\r\n");
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

    //��ʼ�ļ�ϵͳ����
    {
        FATFS fs;
        FIL file1;
        int8_t str_buffer[31];
        uint16_t br;
        int result;

        for(result=0; result<31; result++)
        {
            str_buffer[result] = 0;
        }

        //�����ļ�ϵͳ,�ѷ���0����Ϣ���ص� FATFS fs;
        result = f_mount(0, &fs);
        printf("\r\nf_mount result:%d",result);
        if( result == FR_OK )
        {
            LCD_write_english_string(0,0,"f_mount OK!",GREEN,BLACK);
            //���ļ�
            result = f_open(&file1,"/test.txt",FA_OPEN_EXISTING|FA_READ);
            printf("\r\nf_open result:%d",result);
            if( result == FR_OK )
            {
                LCD_write_english_string(0,20,"f_open /test.txt OK!",GREEN,BLACK);
                //��ȡ�ļ�, ��ȡ30�ֽڵ�str_buffer��.
                result = f_read(&file1,str_buffer,30,&br);
                printf("\r\nf_read result:%d",result);
                if( result == FR_OK )
                {
                    LCD_write_english_string(0,40,"read /test.txt OK:",GREEN,BLACK);
                    LCD_write_english_string(0,60,(char*)str_buffer,BLUE,BLACK);
                }//f_read
                else
                {
                    LCD_write_english_string(0,40,"read /test.txt ERR:",GREEN,BLACK);
                    LCD_write_english_string(0,60,"ERR CODE: ",GREEN,BLACK);
                    LCD_write_english(8*9,60,'0'+result,RED,BLACK);
                }
            }//f_open
            else
            {
                LCD_write_english_string(0,20,"f_open /test.txt ERR!",GREEN,BLACK);
                LCD_write_english_string(0,40,"ERR CODE: ",GREEN,BLACK);
                LCD_write_english(8*9,40,'0'+result,RED,BLACK);
            }//f_open
        }//f_mount
        else
        {
            LCD_write_english_string(0,0,"f_mount ERR!",GREEN,BLACK);
            LCD_write_english_string(0,20,"ERR CODE: ",RED,BLACK);
            LCD_write_english(8*9,20,'0'+result,RED,BLACK);
        }//f_mount
    }

    while(1);
}

