#ifndef __soft_iic_H
#define __soft_iic_H

#include "include.h"


//#define iic_scl_out  PBout(8)
//#define iic_sda_out  PBout(9)
//#define iic_sda_in   PBin(9)

#define iic_scl_out  PAout(14)
#define iic_sda_out  PAout(13)
#define iic_sda_in   PAin(13)

void soft_iic_delay(uint32_t time);
void soft_iic_io_init(void);
void sda_io_set_in(void);
void sda_io_set_out(void);
void soft_iic_start(void);
void soft_iic_stop(void);
u8 soft_iic_wait_ack(void);
void soft_iic_send_ack(void);
void soft_iic_send_no_ack(void);
void soft_iic_send_byte(u8 txd);
u8 soft_iic_read_byte(u8 ack);





#endif


