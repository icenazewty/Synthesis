#ifndef __CAN_BUS_H
#define __CAN_BUS_H
#include "gd32e50x.h"
															 
//---------- ADC ------------------------------------------------------------
#define DEV_CAN0_ID          0x0001	//0xaabb
#define DEV_CAN0_MASK        0x0000


void can_config();
void communication_check(void);
void can_send(uint32_t can_periph,uint16_t sfid,uint8_t *str,uint8_t len);
#endif

