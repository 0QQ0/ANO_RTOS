#include "FreeRTOS.h"
#include "task.h"

#include "Drv_Bsp.h"
#include "Drv_Uart.h"
#include "Drv_laser.h"
#include "Drv_Timer.h"
#include "Drv_UP_flow.h"
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

#include "watch_dog.h"
#include "rc_update.h"
#include "power_management.h"

#include "uwb_task.h"
#include "light_flow_task.h"


/* ��������������׼������ */
void basic_data_read(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  while (1) {
    /*��ȡ�����Ǽ��ٶȼ�����*/
    Drv_Icm20602_Read();

    /*���Դ���������׼��*/
    Sensor_Data_Prepare(1);

    /*��̬�������*/
    imu_update(1);

    /*������������z����ٶȽ��е�ͨ�˲�*/
    wcz_acc_update();

    /*����״̬����*/
    Flight_State_Task(1, CH_N);

    /*����״̬����*/
    Swtich_State_Task(1);
		
		/*���ݷɻ����������ݽ������Ӧ��̬�Խ����������*/
		OF_INS_Get(0.001f);
		
    //�ƹ�����
    LED_1ms_DRV();

    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 1000);
  }
}


/* ��̬���ٶȻ����ƽ��� */
void inner_loop(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  while (1) {
    /*��̬���ٶȻ�����*/
    Att_1level_Ctrl(2 * 1e-3f);

    /*����������*/
    Motor_Ctrl_Task(2);


    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 500);
  }
}

/* ��̬�ǶȻ����ƽ��� */
void outer_loop(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  while (1) {
    /* ��Ԫ��ת����ŷ���� */
    calculate_RPY();

    /* ��̬�ǶȻ����� */
    Att_2level_Ctrl(5e-3f, CH_N);
 
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 200);
  }
}

/* �߶Ȼ����ƽ��� */
void height_loop(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  while (1) { 
    /*��ֱ�ٶȻ�����*/
    Alt_1level_Ctrl(0.005f);

    /*�߶Ȼ�����*/
    Alt_2level_Ctrl(0.005f);
 
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 200);
  }
}

/* λ�û����ƽ��� */
void position_loop(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  while (1) {
    /*�������ݴ�������*/
//    Mag_Update_Task(20);

    /*λ���ٶȻ�����*/
    Loc_1level_Ctrl(20);

    /*�������ݽ�������*/
    dt_handle(); 
		 
    /*�ƹ����*/
    LED_Task2(20);
		
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 50);
  }
}


/* ����������� */
void auxiliary_loop(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  while (1) {
    /*���µ�ѹֵ*/
    battery_update();

    /*��ʱ�洢����*/
    Ano_Parame_Write_task(50); //��ʱ�ϳ� ע�⿴�Ź���λ
  
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 20);
  }
}



int main(void)
{ 
  /*--------------------------------------------------------
  									����������ʼ��
  --------------------------------------------------------*/ 
  /* ������ʼ�� */
	Drv_BspInit();
	
  /*--------------------------------------------------------
  									�ɿذ��������
  --------------------------------------------------------*/
  /* ��������������׼������ */
  xTaskCreate(basic_data_read, "basic_data_read", 116, NULL, 4, NULL);

  /* ��̬���ٶȻ����ƽ��� */
  xTaskCreate(inner_loop, "inner_loop", 116, NULL, 3, NULL);

  /* ��̬�ǶȻ����ƽ��� */
  xTaskCreate(outer_loop, "outer_loop", 116, NULL, 3, NULL);

  /* �߶Ȼ����ƽ��� */
  xTaskCreate(height_loop, "height_loop", 116, NULL, 3, NULL);

  /* λ�û����ƽ��� */
  xTaskCreate(position_loop, "position_loop", 172, NULL, 2, NULL);

  /* ����������� */
  xTaskCreate(auxiliary_loop, "auxiliary_loop", 116, NULL, 1, NULL);

  /* ����Ӳ�����Ź� */
  xTaskCreate(wdt0_loop, "wdt0_loop", 112, NULL, 1, NULL);

  /*--------------------------------------------------------
  									������չ��������
  --------------------------------------------------------*/
  /* ����ң�������ݴ������� */
  xTaskCreate(receiving_task, "receiving_task", 116, NULL, 4, NULL);
 
  /* ������������ */
  xTaskCreate(light_flow_task, "light_flow_task", 116, NULL, 3, NULL);
	
  /* uwb���ݸ��� */
//  xTaskCreate(uwb_update_task, "uwb_update_task", 116, NULL, 3, NULL);

  /*--------------------------------------------------------
  									�ϲ���չ����
  --------------------------------------------------------*/



  /*--------------------------------------------------------
  									�������������
  --------------------------------------------------------*/
  vTaskStartScheduler();



  /*--------------------------------------------------------
  									�������
  --------------------------------------------------------*/
  //printf("Free_Heap_Size = %d \n",xPortGetFreeHeapSize());
  //printf("MinimumEverFreeHeapSize = %d \n",xPortGetMinimumEverFreeHeapSize());
}
