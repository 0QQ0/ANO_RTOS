#ifndef __DRV_UP_Flow_H
#define __DRV_UP_Flow_H

//==����
#include "sysconfig.h"
#include "Ano_FcData.h"

//==����

//==��������
extern u8 of_init_type;
extern uint8_t of_buf_update_cnt;
extern uint8_t OF_DATA[];
//==��������

//static


//public
u8 Drv_OFInit(void);
void OFGetByte(uint8_t data);

void up_flow_loop(void *pvParameters);
	

#endif

