#include "Drv_Bsp.h"
 
#include "FreeRTOS.h"
#include "task.h"

#include "sysconfig.h" 
#include "Drv_RcIn.h"
#include "Drv_Spi.h"
#include "Drv_Led.h"
#include "Drv_Paramter.h"
#include "Drv_PwmOut.h"
#include "Drv_Adc.h"
#include "Drv_Uart.h"
#include "Drv_laser.h"
#include "Drv_icm20602.h"
#include "drv_ak8975.h"
#include "drv_spl06.h"

#include "Ano_FlightCtrl.h"
#include "Ano_DT.h"
#include "Ano_Parameter.h"
#include "Ano_FcData.h"
#include "Ano_Sensor_Basic.h"
#include "ano_usb.h"

#include "rc_update.h"


static void SysTick_Init(void )
{
  ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 1000);
  ROM_SysTickIntEnable();
  ROM_SysTickEnable();
}

//����freeRTOS�δ�ص�
void xPortSysTickHandler( void );

void SysTick_Handler(void)
{
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    xPortSysTickHandler();
  } 
}
   

void Drv_BspInit(void)
{ 
	//�Ĵ���ֵ��Ĭ��ֵ�ͽ��������λ
  if(ROM_SysCtlClockGet() != 16000000 )
    ROM_SysCtlReset();
	
  /*����ϵͳ��ƵΪ80M*/
  ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

  /*�ж����ȼ��������*/
  NVIC_SetPriorityGrouping(0x03);
	
  /*�����������㵥Ԫ*/
  ROM_FPULazyStackingEnable();
  ROM_FPUEnable();
	
  //���ݳ�ʼ��
  Dvr_ParamterInit();
	
  //��ȡ��ʼ���� 
  Ano_Parame_Read();
	
  //�ƹ��ʼ��
  Dvr_LedInit(); 

  //spiͨ�ų�ʼ��
  Drv_Spi0Init(); 
	
  //��ʼ��ICM
  sens_hd_check.acc_ok = sens_hd_check.gyro_ok = Drv_Icm20602Init();
	
  //��ʼ����ѹ��
  sens_hd_check.baro_ok = Drv_Spl0601Init();
	
  //�������OK���������̲�������㣨ע���˴�û���������Ƿ������ļ�����
  sens_hd_check.mag_ok = 0;
 
	//��ʹ�ú��¹���
	flag.mems_temperature_ok = 1;
 
	//����ģʽ
  flag.flight_mode = LOC_HOLD;
 
  //��λ��ͨѶ���ó�ʼ��
  ANO_DT_Init();

  //ADC��ʼ��
  Drv_AdcInit(); 
	
  //�ɿش����������ʼ��
  Sensor_Basic_Init();
	
  //�ɿ�PID��ʼ��
  All_PID_Init();
	
  //��������ʼ��
  Drv_PwmOutInit();
	
  //�δ�ʱ�ӳ�ʼ��
  SysTick_Init(); 
	
			
  //���ֻ�Դָʾ�����ų�ʼ��
  ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
  ROM_GPIOPinTypeGPIOOutput(GPIOF_BASE, GPIO_PIN_0);
  ROM_GPIOPinWrite(GPIOF_BASE, GPIO_PIN_0, 0);
	
}




