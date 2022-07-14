#ifndef __STMF4FLASH_H__
#define __STMF4FLASH_H__

#include "include.h"   

#define STM32_FLASH_BASE        0x08000000 	//STM32 FLASH的起始地址
 
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) 	//扇区0起始地址, 16 Kbytes       //4     16K
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) 	//扇区1起始地址, 16 Kbytes       //6     32K
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) 	//扇区2起始地址, 16 Kbytes       
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) 	//扇区3起始地址, 16 Kbytes       //8     64K
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) 	//扇区4起始地址, 64 Kbytes       //B    128K
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) 	//扇区5起始地址, 128 Kbytes      //C    256K
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) 	//扇区6起始地址, 128 Kbytes      //D    384K
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) 	//扇区7起始地址, 128 Kbytes      //E    512K
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) 	//扇区8起始地址, 128 Kbytes      
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) 	//扇区9起始地址, 128 Kbytes      //F    768K
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) 	//扇区10起始地址,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) 	//扇区11起始地址,128 Kbytes      //G    1024K

uint32_t STMF4_FLASH_ReadWord(uint32_t addr);
uint16_t STMF4_FLASH_ReadHalfWord(uint16_t addr);
void STMF4_FLASH_Write_Word(uint32_t *buf,uint32_t count);
void STMF4_FLASH_Write_HalfWord(uint16_t *buf,uint32_t count)	;
void STMF4_FLASH_Read_Word(uint32_t *buf,uint32_t count);
uint32_t STMF4_FLASH_Read_One_Word(uint32_t offset);
void STMF4_FLASH_Read_HalfWord(uint16_t *buf,uint32_t count);
uint32_t STMF4_FLASH_Read_One_HalfWord(uint32_t offset);

						   
#endif

















