#ifndef __CONFIG_H
#define __CONFIG_H
 
#include "sysconfig.h" 

/***************����******************/
#define ANGLE_TO_RADIAN 0.01745329f //*0.01745 = /57.3	�Ƕ�ת����  

//GYR_ACC_FILTER �������·�Χ�ο�
//500KV���� 0.15f~0.2f
//500~2000KV 0.2f~0.3f
//2000kv���� 0.3f-0.5f 

#define GYR_ACC_FILTER 0.22f //�����Ǽ��ٶȼ��˲�ϵ��

//FINAL_P �������·�Χ�ο�
//500KV���� 0.4f����
//500~2000KV 0.4f~0.3f
//2000kv���� 0.3f-0.2f

#define FINAL_P 			 0.33f  //������������ϵ��

#define MOTOR_ESC_TYPE 1  //2����ˢ�����ɲ���ĵ����1����ˢ�������ɲ���ĵ����
#define MOTORSNUM 4



#define MAX_ANGLE     25.0f 
 
#define MAX_ROLLING_SPEED 1600  //�Ƕ�ÿ��

#define MAX_SPEED 500 //���ˮƽ�ٶȣ�����ÿ�� cm/s

#define MAX_Z_SPEED_UP 350 //����ÿ�� cm/s
#define MAX_Z_SPEED_DW 250 //����ÿ�� cm/s
 

#define CTRL_1_INTE_LIM 250 //���ٶȻ������޷� �����
   

#define MAX_THR_SET    90  //������Űٷֱ� %
#define THR_INTE_LIM_SET   70  //���Ż��ְٷֱ� % 
 
#define THR_INTE_LIM   THR_INTE_LIM_SET/FINAL_P

#define THR_START      35  //����������ٷֱ� % 


#define BARO_FIX -0                          //��ѹ�ٶȻ����������ֵ/CM����  


#endif


