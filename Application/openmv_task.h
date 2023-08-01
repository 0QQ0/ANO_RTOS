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
//Ѱ��ɫ���x��ƫ��
#define POS_X_ERROR (0x06)
//Ѱ��ɫ���y��ƫ��
#define POS_Y_ERROR (0x07)

typedef struct {
	uint32_t angle ;
	uint32_t area ;
	uint32_t width ;
	uint32_t pos ;
	uint32_t res ;
	uint32_t pos_x_error;
	uint32_t pos_y_error;
}openmv_t ;
 
typedef struct
{
	//
	u8 color_flag;
	u8 sta;
	s16 pos_x;
	s16 pos_y;
	u8 dT_ms;

}_openmv_color_block_st;

typedef struct
{
	//
	u8 sta;	
	s16 angle;
	s16 deviation;
	u8 p_flag;
	s16 pos_x;
	s16 pos_y;
	u8 dT_ms;

}_openmv_line_tracking_st;

typedef struct
{
	u8 offline;
	u8 mode_cmd;
	u8 mode_sta;
	//
	_openmv_color_block_st cb;
	_openmv_line_tracking_st lt;
}_openmv_data_st;
//==��������
extern _openmv_data_st opmv;

extern uint8_t useOpenmv;
extern openmv_t mvValue;
extern float openmvSpeedOut[];

void openmv_update_task(void *pvParameters);
 

#endif


