#ifndef __24cxx_H
#define __24cxx_H

#include <stdint.h>


#define EEPROM_ADDR_SIMPLE 0


#define EEPROM_24CXX_A0    (0)
#define EEPROM_24CXX_A1    (0<<1)
#define EEPROM_24CXX_A2    (0<<2)

#define EEPROM_24CXX_READ    0x01
#define EEPROM_24CXX_WRITE   0x00

#define EEPROM_24CXX_ADDR    (0xA0|EEPROM_24CXX_A2|EEPROM_24CXX_A1|EEPROM_24CXX_A0)

#define EEPROM_24CXX_READ_ADDR (EEPROM_24CXX_ADDR|EEPROM_24CXX_READ)
#define EEPROM_24CXX_WRITE_ADDR (EEPROM_24CXX_ADDR|EEPROM_24CXX_WRITE)



void eeprom_24cxx_init(void);
uint8_t eeprom_24cxx_read_one_byte(uint16_t addr);
void eeprom_24cxx_write_one_byte(uint16_t addr,uint8_t data);
void eeprom_24cxx_read_byte_len(uint16_t addr,uint8_t *databuf,uint16_t len);
void eeprom_24cxx_write_byte_len(uint16_t addr,uint8_t *databuf,uint16_t len);
uint16_t eeprom_24cxx_read_half_word(uint16_t addr);
void eeprom_24cxx_write_half_word(uint16_t addr,uint16_t data);
void eeprom_24cxx_write_half_word_len(uint16_t addr,uint16_t *databuf,uint16_t len);
void eeprom_24cxx_read_half_word_len(uint16_t addr,uint16_t *databuf,uint16_t len);
#endif
