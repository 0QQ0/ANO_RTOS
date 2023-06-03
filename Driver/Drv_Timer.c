
#include "Drv_Timer.h"

//Ϊ1ʱ��������ʱ��ͳ�ƹ���

#if(configGENERATE_RUN_TIME_STATS == 1)

#include "timer.h"
#include "hw_memmap.h"
#include "hw_ints.h"
#include "gpio.h"
#include "sysctl.h"
#include "interrupt.h"
#include "rom.h"

#include "FreeRTOS.h"
#include "task.h"

void TIMER_IRQHandler(void);
void TIMER_WID_IRQHandler(void);


/* ����ͳ������ʱ�� */
volatile uint32_t CPU_RunTime = 0UL;
//16/32bit��ʱ�����
void Timer_Config(void)
{
  //ʹ�ܶ�ʱ��TIMER0��16/32bit
  SysCtlPeripheralEnable( SYSCTL_PERIPH_TIMER0);
  //���ö�ʱ��������ʱ����֣������ò�ֺ�Ķ�ʱ��AΪ�����Լ���
  TimerConfigure( TIMER0_BASE,  TIMER_CFG_SPLIT_PAIR|TIMER_CFG_A_PERIODIC_UP);
  //���ö�ʱ��װ��ֵ��
  TimerLoadSet( TIMER0_BASE,  TIMER_A, ROM_SysCtlClockGet()/20000-1);
  //Ϊ��ʱ��Aע���жϺ���
  TimerIntRegister( TIMER0_BASE,  TIMER_A, TIMER_IRQHandler);
  //ʹ��time0�Ķ�ʱ��AΪ��ʱ�ж�
  TimerIntEnable( TIMER0_BASE,  TIMER_TIMA_TIMEOUT);
  //�����ж����ȼ�
  IntPrioritySet( INT_TIMER0A,  0);
  //ʹ���ж�
  IntEnable( INT_TIMER0A);
  IntMasterEnable();
  //ʹ�ܶ�ʱ��
  TimerEnable( TIMER0_BASE,  TIMER_A);
}

void TIMER_IRQHandler(void)
{ 
  //��ȡ��ʱ���ж�״̬
  uint32_t status=TimerIntStatus( TIMER0_BASE,  true);
	
  //��һ���ж�
  CPU_RunTime++; 
	
  //����жϱ�־λ
  TimerIntClear( TIMER0_BASE,  status);
} 



void task_census(void *pvParameters)
{
	#include "string.h"
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���
	uint8_t CPU_RunInfo[400];		//������������ʱ����Ϣ
  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

//		memset(CPU_RunInfo,0,400);				//��Ϣ����������

//		vTaskList((char *)&CPU_RunInfo);  //��ȡ��������ʱ����Ϣ

//		printf("---------------------------------------------\r\n");
//		printf("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
//		printf("%s", CPU_RunInfo);
//		printf("---------------------------------------------\r\n");

		memset(CPU_RunInfo,0,400);				//��Ϣ����������

		vTaskGetRunTimeStats((char *)&CPU_RunInfo);

		printf("������       ���м���         ������\r\n");
		printf("%s", CPU_RunInfo);
		printf("---------------------------------------------\r\n\n");
		
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 1);
  }
}
#endif
