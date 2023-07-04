#include "FreeRTOS.h"
#include "task.h"
  
#include "Drv_Bsp.h"	
#include "Drv_Uart.h"
#include "Drv_Timer.h" 
#include "Drv_heating.h"

#include "Ano_Imu.h"
#include "Ano_Sensor_Basic.h" 
#include "Ano_FlightCtrl.h"
#include "Ano_AttCtrl.h"
#include "Ano_LocCtrl.h"
#include "Ano_AltCtrl.h"
#include "Ano_MotorCtrl.h" 
#include "Ano_OF.h"
#include "Ano_OF_DecoFusion.h"
#include "Ano_FlyCtrl.h" 
#include "Ano_FlightDataCal.h" 
#include "Ano_ProgramCtrl_User.h"
#include "nlink_linktrack_tagframe0.h"

#include "rc_update.h" 
#include "power_management.h" 
  
/* ��������������׼������ ������Ϊ��׼���е����� ִ��Ƶ�ʾ�׼1000Hz ���ȼ�ȫ�����*/
void basic_data_read(void *pvParameters)
{
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���

  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

    /*��ȡ�����Ǽ��ٶȼ�����*/
    Drv_Icm20602_Read();

    /*���Դ���������׼��*/
    Sensor_Data_Prepare(1);

    /*��̬�������*/
    IMU_Update_Task(1);

    /*��ȡWC_Z���ٶ�*/
    WCZ_Acc_Get_Task();

    /*����״̬����*/
    Flight_State_Task(1, CH_N);

    /*����״̬����*/
    Swtich_State_Task(1);

    /*�����ں�����׼������*/
    ANO_OF_Data_Prepare_Task(0.001f);
 
    //�ƹ�����
    LED_1ms_DRV();

    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 1000);
  }
}


/* ��̬���ٶȻ����ƽ��� ������Ϊ��׼���е����� ִ��Ƶ�ʾ�׼500Hz ���ȼ��ڶ�*/
void inner_loop(void *pvParameters)
{
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���

  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

    /*��̬���ٶȻ�����*/
    Att_1level_Ctrl(2 * 1e-3f);

    /*����������*/
    Motor_Ctrl_Task(2);


    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 500);
  }
}

/* ��̬�ǶȻ����ƽ��� ������Ϊ��׼���е����� ִ��Ƶ�ʾ�׼200Hz ���ȼ��ڶ�*/
void outer_loop(void *pvParameters)
{
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���

  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

    /*��ȡ��̬�ǣ�ROLL PITCH YAW��*/
    calculate_RPY();

    /*��̬�ǶȻ�����*/
    Att_2level_Ctrl(5e-3f, CH_N);


    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 200);
  }
}

/* �߶Ȼ����ƽ��� ������Ϊ��׼���е����� ִ��Ƶ�ʾ�׼100Hz ���ȼ����� */
void height_loop(void *pvParameters)
{
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���

  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

    /*ң�������ݴ�������*/
    receivingTask();

    /*�߶������ں�����*/
    WCZ_Fus_Task(10); 

    /*�߶��ٶȻ�����*/
    Alt_1level_Ctrl(10e-3f);

    /*�߶Ȼ�����*/
    Alt_2level_Ctrl(10e-3f);

    /*�������߼��*/
    AnoOF_Check(10);

    /*�ƹ����*/
    LED_Task2(10);

    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 100);
  }
}

/* λ�û����ƽ��� ������Ϊ��׼���е����� ִ��Ƶ�ʾ�׼50Hz ���ȼ�����*/
void position_loop(void *pvParameters)
{
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���

  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

    /*�������ݴ�������*/
    Mag_Update_Task(20); 

    /*λ���ٶȻ�����*/
    Loc_1level_Ctrl(20);
 
    /*�������ݽ���*/
    dtTask();
 
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 50);
  }
}


/* ����������� ������Ϊ��׼���е����� ִ��Ƶ�ʾ�׼20Hz */
void auxiliary_loop(void *pvParameters)
{
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���

  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

    /*���µ�ѹֵ*/
    batteryUpdate(); 

    /*��ʱ�洢����*/
    Ano_Parame_Write_task(50);
		
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 20);
  }
}


/* �Զ������ */
void user_loop(void *pvParameters)
{
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���

	static unsigned char pFrame[128];
	
  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

		//�����λ�����  
		if(RingBuffer_GetCount(&U1rxring) > 128) {
			RingBuffer_PopMult(&U1rxring, pFrame, 128);

			//����uwb����
			uint8_t res = g_nlt_tagframe0.UnpackData(pFrame, 128);
			//��������ˢ�»�����
			if (res == 0) { 
				RingBuffer_Flush(&U1rxring);
			}
		}
		   
		
		vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 10);
	}
}
 
int main(void)
{
	//�Ĵ���ֵ��Ĭ��ֵ�ͽ��������λ
	if(ROM_SysCtlClockGet() != 16000000 ) 
		ROM_SysCtlReset(); 
	
  Drv_BspInit(); 

  /* ��������������׼������ 1000Hz*/
  xTaskCreate(basic_data_read, "basic_data_read", 152, NULL, 4, NULL);

  /* ��̬���ٶȻ����ƽ��� 500Hz*/
  xTaskCreate(inner_loop, "inner_loop", 104, NULL, 3, NULL);

  /* ��̬�ǶȻ����ƽ��� 200Hz*/
  xTaskCreate(outer_loop, "outer_loop", 104, NULL, 3, NULL);

  /* �߶Ȼ����ƽ��� 100Hz*/
  xTaskCreate(height_loop, "height_loop", 248, NULL, 3, NULL);

  /* λ�û����ƽ��� 50Hz*/
  xTaskCreate(position_loop, "position_loop", 184, NULL, 2, NULL);

  /* ����������� 20Hz*/
  xTaskCreate(auxiliary_loop, "auxiliary_loop", 128, NULL, 2, NULL);
	
	
  /* �Զ������ 50Hz*/
  xTaskCreate(user_loop, "user_loop", 128, NULL, 3, NULL); 
 
  //�������������
  vTaskStartScheduler(); 
	
	//printf("Free_Heap_Size = %d \n",xPortGetFreeHeapSize());   
	//printf("MinimumEverFreeHeapSize = %d \n",xPortGetMinimumEverFreeHeapSize());   
	
  //�������
  //fun();
	
}
