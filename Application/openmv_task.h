#ifndef _OPENMV_TASK_H_
#define	_OPENMV_TASK_H_


#include "FreeRTOS.h"
#include "task.h"

#include "Ano_DT.h"
#include "Ano_ProgramCtrl_User.h"
 
#include "Drv_Uart.h"  

//����ƫ��
#define ANGLE_ID (0x01)
//���
#define AREA_ID (0x02)
//���
#define WIDTH_ID (0x03)
//ͼ��λ��
#define POS_ID (0x04)
//��ά��ʶ����
#define RES_ID (0x05)

typedef struct {
	uint32_t angle ;
	uint32_t area ;
	uint32_t width ;
	uint32_t pos ;
	uint32_t res ;
}openmv_t ;
 
extern uint8_t useOpenmv;
extern openmv_t mvValue;
extern float openmvSpeedOut[];

void openmv_update_task(void *pvParameters);
 

#endif


