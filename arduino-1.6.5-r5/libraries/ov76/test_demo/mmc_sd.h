#ifndef __MMC_SD_h__
#define __MMC_SD_h__

#include "config.h"

#define MMC_SD_PORT       PORTA                    //Òý½Å¶¨Òå
#define MMC_SD_CS_PIN     4                        //mega16
#define DDR_INI() DDRB |= _BV(5)|_BV(7)     //mega16
#define SPI_CS_Assert()   MMC_SD_PORT &= ~(1<<MMC_SD_CS_PIN)
#define SPI_CS_Deassert() MMC_SD_PORT |= 1<<MMC_SD_CS_PIN
#define SPI_CS_int()      DDRA |=1<<MMC_SD_CS_PIN


extern void MMC_SD_Init(void);
extern uint8 MMC_SD_Reset(void);
extern uint8 MMC_SD_ReadSingleBlock(uint32 sector, uint8* buffer);
extern uint8 MMC_SD_WriteSingleBlock(uint32 sector, uint8* buffer);
extern uint32 MMC_SD_ReadCapacity(void);
extern void SPI_High(void);
uint8 SPI_WriteByte(uint8 val);


#endif
