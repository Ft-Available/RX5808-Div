#include "stmf4flash.h"
#include "rx5808_config.h"


#define FLASH_SAVE_START_ADDR   0x08004000 



#if RX5808_CONFIGT_FLASH_EEPROM==1
const uint8_t flash_map[16 * 1024] __attribute__((at(0x08004000)));
#endif	


static uint16_t STMFLASH_GetFlashSector(uint32_t addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}

uint32_t STMF4_FLASH_ReadWord(uint32_t addr)
{
	return *(vu32*)addr; 
}  

uint16_t STMF4_FLASH_ReadHalfWord(uint16_t addr)
{
	return *(vu16*)addr; 
}  

void STMF4_FLASH_Write_Word(uint32_t *buf,uint32_t count)	
{ 
	FLASH_Unlock();	
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);	
  FLASH_DataCacheCmd(DISABLE);
  FLASH_EraseSector(STMFLASH_GetFlashSector(FLASH_SAVE_START_ADDR),VoltageRange_3);
	
	for(int i=0;i<count;i++)
	{
	  if(FLASH_ProgramWord(FLASH_SAVE_START_ADDR+4*i,buf[i])!=FLASH_COMPLETE)
		{break;}	
	}
  FLASH_DataCacheCmd(ENABLE);	
	FLASH_Lock();
}

void STMF4_FLASH_Write_HalfWord(u16 *buf,uint32_t count)	
{ 
	FLASH_Unlock();	
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);	
  FLASH_DataCacheCmd(DISABLE);
  if(FLASH_EraseSector(STMFLASH_GetFlashSector(FLASH_SAVE_START_ADDR),VoltageRange_3)!=FLASH_COMPLETE){
	      FLASH_DataCacheCmd(ENABLE);	
	      FLASH_Lock();
				return;	
	}
	for(int i=0;i<count;i++)
	{
	  if(FLASH_ProgramWord(FLASH_SAVE_START_ADDR+2*i,buf[i])!=FLASH_COMPLETE)
		{break;}	
	}
  FLASH_DataCacheCmd(ENABLE);	
	FLASH_Lock();
} 

void STMF4_FLASH_Read_Word(uint32_t *buf,uint32_t count)	
{
	for(uint32_t i=0;i<count;i++)
	   buf[i]=STMF4_FLASH_ReadWord(FLASH_SAVE_START_ADDR+4*i);	
}

uint32_t STMF4_FLASH_Read_One_Word(uint32_t offset)
{
   return STMF4_FLASH_ReadWord(FLASH_SAVE_START_ADDR+4*offset);
}

void STMF4_FLASH_Read_HalfWord(uint16_t *buf,uint32_t count)	
{
	for(uint32_t i=0;i<count;i++)
		buf[i]=STMF4_FLASH_ReadHalfWord(FLASH_SAVE_START_ADDR+2*i);
}


uint32_t STMF4_FLASH_Read_One_HalfWord(uint32_t offset)
{
   return STMF4_FLASH_ReadHalfWord(FLASH_SAVE_START_ADDR+2*offset);
}











