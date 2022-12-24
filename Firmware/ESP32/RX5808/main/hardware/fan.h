#ifndef __fan_H
#define __fan_H

#include <stdint.h>
#include "hardware/hwvers.h"




void fan_Init(void);
void fan_set_speed(uint8_t duty);
uint16_t fan_get_speed(void);


#endif

