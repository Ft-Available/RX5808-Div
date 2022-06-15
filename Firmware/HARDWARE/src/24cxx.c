#include "24cxx.h"
#include "soft_iic.h"


void eeprom_24cxx_init()
{
	
	soft_iic_io_init();
}

void eeprom_24cxx_read_write_addr(uint16_t addr)
{
#if EEPROM_ADDR_SIMPLE==0
	 	soft_iic_start(); 
	  soft_iic_send_byte(EEPROM_24CXX_ADDR); 
		soft_iic_wait_ack(); 
		soft_iic_send_byte(addr>>8);
	  soft_iic_wait_ack(); 
		soft_iic_send_byte(addr%256);   
		soft_iic_wait_ack();	 
#else
	  soft_iic_start(); 
	  soft_iic_send_byte(EEPROM_24CXX_ADDR+((addr>>8)<<1));  
	  soft_iic_wait_ack(); 
		soft_iic_send_byte(addr%256);   
		soft_iic_wait_ack();	 	
#endif

}

uint8_t eeprom_24cxx_read_one_byte(uint16_t addr)
{				  
		uint8_t temp=0;		  	    																 
	  eeprom_24cxx_read_write_addr(addr);
		soft_iic_start();  	 	   
		soft_iic_send_byte(EEPROM_24CXX_READ_ADDR);          
		soft_iic_wait_ack();	 
		temp=soft_iic_read_byte(0);		   
		soft_iic_stop();
		return temp;
}

void eeprom_24cxx_write_one_byte(uint16_t addr,uint8_t data)
{				   	  	    																 
		eeprom_24cxx_read_write_addr(addr); 	 										  		   
		soft_iic_send_byte(data);      
		soft_iic_wait_ack();  		    	   
		soft_iic_stop();
		SysTick_Delay_ms(10);	 
}



void eeprom_24cxx_read_byte_len(uint16_t addr,uint8_t *databuf,uint16_t len)
{
	soft_iic_start();  
	eeprom_24cxx_read_write_addr(addr);	    
	soft_iic_start(); 
  soft_iic_send_byte(EEPROM_24CXX_READ_ADDR);  
	soft_iic_wait_ack();	
	for(int i=0;i<len;i++)
	{	        	 
		if(i<len-1)		
		databuf[i]=soft_iic_read_byte(1);	
		else
		databuf[i]=soft_iic_read_byte(0);	
	}
	soft_iic_stop(); 
} 
void eeprom_24cxx_write_byte_len(uint16_t addr,uint8_t *databuf,uint16_t len)
{
	soft_iic_start(); 
	eeprom_24cxx_read_write_addr(addr);
	for(uint16_t i=0;i<len;++i)
	{
		soft_iic_send_byte(databuf[i]);    				   
		soft_iic_wait_ack();	
	}	 										  		   
   soft_iic_stop();
	 SysTick_Delay_ms(10);
}



uint16_t eeprom_24cxx_read_half_word(uint16_t addr)
{				   	  	    																 
  uint16_t temp=0;		  	    																 
  soft_iic_start();  
  eeprom_24cxx_read_write_addr(addr);	    
	soft_iic_start();  	 	   
	soft_iic_send_byte(EEPROM_24CXX_READ_ADDR);          
	soft_iic_wait_ack();	 
  temp=soft_iic_read_byte(1)<<8;	
  temp|=soft_iic_read_byte(0);		 	
  soft_iic_stop();
	return temp;	
	//return eeprom_24cxx_read_one_byte(addr)<<8|eeprom_24cxx_read_one_byte(addr+1);
}

void eeprom_24cxx_write_half_word(uint16_t addr,uint16_t data)
{				   	  	    																 
		soft_iic_start();  
	  eeprom_24cxx_read_write_addr(addr); 	 										  		   
		soft_iic_send_byte(data>>8);    				   
		soft_iic_wait_ack();
		soft_iic_send_byte(data);    				   
		soft_iic_wait_ack();  		
		soft_iic_stop();
		SysTick_Delay_ms(1);

//    eeprom_24cxx_write_one_byte(addr,data>>8);
//	  eeprom_24cxx_write_one_byte(addr+1,data&0xff);
	
}

void eeprom_24cxx_write_half_word_len(uint16_t addr,uint16_t *databuf,uint16_t len)
{

  soft_iic_start(); 
	eeprom_24cxx_read_write_addr(addr);
	for(uint16_t i=0;i<len;++i)
	{
		soft_iic_send_byte(databuf[i]>>8);    				   
		soft_iic_wait_ack();
		soft_iic_send_byte(databuf[i]&0xff);    				   
		soft_iic_wait_ack();  		
	}	 										  		   
  soft_iic_stop();
	SysTick_Delay_ms(1);

//	for(uint16_t i=0;i<len;i++)
//	  eeprom_24cxx_write_half_word(addr+2*i,databuf[i]);
}


void eeprom_24cxx_read_half_word_len(uint16_t addr,uint16_t *databuf,uint16_t len)
{
  	    																 
  soft_iic_start();  
	eeprom_24cxx_read_write_addr(addr);
	soft_iic_start(); 
  soft_iic_send_byte(EEPROM_24CXX_READ_ADDR);
	soft_iic_wait_ack();	   
	for(int i=0;i<len;i++)
	{	        
  databuf[i]=soft_iic_read_byte(1)<<8;
  if(i<len-1)		
	databuf[i]|=soft_iic_read_byte(1);	
  else
	databuf[i]|=soft_iic_read_byte(0);	
	}
	soft_iic_stop(); 
	
//	for(uint16_t i=0;i<len;i++)
//	  databuf[i]=eeprom_24cxx_read_half_word(addr+2*i);
}







