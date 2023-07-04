#include "FreeRTOS.h"
#include "task.h"

#include "sysconfig.h"
#include "Drv_Bsp.h"
#include "Ano_FcData.h"

#include "Drv_Bsp.h"
#include "Drv_icm20602.h"
#include "Ano_LED.h"
#include "Ano_Sensor_Basic.h"

#include "Ano_DT.h"
#include "rc_update.h"
#include "Drv_led.h"
#include "Ano_FlightCtrl.h"
#include "Ano_AttCtrl.h"
#include "Ano_LocCtrl.h"
#include "Ano_AltCtrl.h"
#include "Ano_MotorCtrl.h"
#include "Ano_Parameter.h"
#include "Ano_MagProcess.h"
#include "Ano_OF.h"
#include "Drv_heating.h"
#include "Ano_FlyCtrl.h" 
#include "Ano_OF_DecoFusion.h"
#include "Drv_Uart.h"
#include "Ano_Imu.h"
#include "Ano_FlightDataCal.h"
#include "Ano_Sensor_Basic.h"
#include "ano_usb.h"
#include "Ano_ProgramCtrl_User.h"
#include "Drv_Timer.h"
#include "ano_usb.h"
#include "power_management.h"
#include "Drv_Uart.h"
#include "ring_buffer.h"
#include "nlink_utils.h"
#include "nlink_linktrack_tagframe0.h"
#include "hw_ints.h"

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


/* ���¿��ƽ��� ������Ϊ��׼���е����� ִ��Ƶ�ʾ�׼20Hz */
void temperature_loop(void *pvParameters)
{
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���

  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

    /*���µ�ѹֵ*/
    batteryUpdate();

    //���¿��ƣ�����ֱ��ע�͵������򿪻�������У׼��
    Thermostatic_Ctrl_Task(50);

    /*��ʱ�洢����*/
    Ano_Parame_Write_task(50);
		
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 20);
  }
}

/* ���Ź����� ������Ϊ��׼���е����� ִ��Ƶ�ʾ�׼2Hz */
void wdt0_loop(void *pvParameters)
{
	// Enable the peripherals used by this example.
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
	
	// Enable the watchdog interrupt.
	MAP_IntEnable(INT_WATCHDOG);

	// Set the period of the watchdog timer.
	MAP_WatchdogReloadSet(WATCHDOG0_BASE, MAP_SysCtlClockGet()); /* 1s�����ж� */

	// Enable reset generation from the watchdog timer.
	MAP_WatchdogResetEnable(WATCHDOG0_BASE);

	// Enable the watchdog timer.
	MAP_WatchdogEnable(WATCHDOG0_BASE);
	
	static TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���
	
	xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ
	
	while(1)
	{
		MAP_WatchdogIntClear(WATCHDOG0_BASE);	/* ι�� */
		printf("Feed wdt0\r\n");
		vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 2);
	}
}
void WDT0_Handler(void)
{
	printf("WDT0_Handler\r\n");
	printf("MCU Fuck Game Over\r\n");
}

/* �Զ������ */
void user_loop(void *pvParameters)
{
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���

  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

    //����UWB���� 
		//�����λ�����
		static int RingBufferDataLen = 0;
		static unsigned char pData[128 * 5];
		RingBufferDataLen = RingBuffer_GetCount(&U1rxring) ;

		//����uwb����
		if(RingBufferDataLen) {
			RingBuffer_PopMult(&U1rxring, pData, RingBufferDataLen);

			if (g_nlt_tagframe0.UnpackData(pData, RingBufferDataLen)) {
				return;
			}
		}
 
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 50);
  }
}
 
int main(void)
{
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

  /* ���¿��ƽ��� 20Hz*/
  xTaskCreate(temperature_loop, "temperature_loop", 128, NULL, 2, NULL);
	
	
//  /* �Զ������ 50Hz*/
//  xTaskCreate(user_loop, "user_loop", 128, NULL, 3, NULL); 
xTaskCreate(wdt0_loop, "wdt0_loop", 96 + 32, NULL, 1, NULL); 
  //�������������
  vTaskStartScheduler();

  //�������
  //fun();
}


