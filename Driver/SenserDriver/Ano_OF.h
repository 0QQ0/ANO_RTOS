#ifndef __ANO_OF_H_
#define __ANO_OF_H_

#include "sysconfig.h"

//������Ϣ������QUA
//����ǿ�ȣ�LIGHT
extern uint8_t 	OF_STATE, OF_QUALITY;
//ԭʼ������Ϣ���������������ģ���ֲ�
extern int8_t	OF_DX, OF_DY;
//�ںϺ�Ĺ�����Ϣ���������������ģ���ֲ�
extern int16_t	OF_DX2, OF_DY2, OF_DX2FIX, OF_DY2FIX, OF_INTEG_X, OF_INTEG_Y;
//ԭʼ�߶���Ϣ���ںϺ�߶���Ϣ
extern uint32_t	OF_ALT, OF_ALT2;
//ԭʼ����������
extern int16_t	OF_GYR_X, OF_GYR_Y, OF_GYR_Z; 
//ԭʼ���ٶ�����
extern int16_t	OF_ACC_X, OF_ACC_Y, OF_ACC_Z; 
 
extern u8 of_init_type;

 
void AnoOF_GetOneByte(uint8_t data);
void AnoOF_DataAnl_Task(u8 dT_ms);
void AnoOF_Check(u8 dT_ms);
#endif
