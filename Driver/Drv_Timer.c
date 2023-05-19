#include "Drv_Timer.h" 
 

#include "Timer.h" 

#include "tm4c123gh6pm.h"
#include "TM4C123G.h"
#include "rom.h"
#include "rom_map.h"
#include "sysctl.h" 
#include "hw_ints.h"

#include "Ano_DT.h"

unsigned long long int timer_count = 0;
void IntHandle_TIMER3A(void);
void vConfigureTimerForRunTimeStats(void)
{
		// ʹ�� Timer 1 ��ʱ��
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);

    // �ȴ� Timer 1 ʱ��ʹ�����
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3))
    {
        // �ȴ�ʱ��ʹ��
    }

    // �ر� Timer 1
    ROM_TimerDisable(TIMER3_BASE, TIMER_BOTH);

    // ���� Timer 1 Ϊ���ڼ�ʱ��ģʽ
    ROM_TimerConfigure(TIMER3_BASE, TIMER_CFG_PERIODIC);

    // ���� Timer 1 �ļ�ʱ�����ڣ�����ϵͳʱ��Ƶ�ʺ������ʱ��ֱ��ʽ��м��㣩
    uint32_t timerPeriod = ROM_SysCtlClockGet() / 1000;
    ROM_TimerLoadSet(TIMER3_BASE, TIMER_BOTH, timerPeriod - 1);
	
		//ע���жϷ�����������Ǹ����жϷ������������ʵ���濴�⺯���ֲ���һ��Ҳ���Բ���������ʱ���ж���ں������־��ǹٷ�Ĭ�ϵģ�������ʲô����...��  
		TimerIntRegister(TIMER3_BASE,TIMER_A,IntHandle_TIMER3A);
       	
		//ʹ�ܶ�ʱ��ģ��Timer0�Ķ�ʱ��A���жϡ�
		ROM_IntEnable(INT_TIMER3A);
		//ʹ�ܵ����Ķ�ʱ���ж�Դ����һ��TIMER0_BASEΪTimer0�Ļ���ַ���ڶ������ж�Դ���õ��ж��¼��ı�ʶ�룬TIMER_TIMA_TIMEOUT����˼�Ƕ�ʱ��A(TimerA)���(��װ��)���Դ�Ϊ��־λ����TimerA��װ��ʱ�ͻᴥ���жϡ�
		TimerIntEnable(TIMER3_BASE,TIMER_TIMA_TIMEOUT);
		
		//ʹ�ܴ������жϣ�ʹ�������ܹ���Ӧ�жϡ�
		ROM_IntMasterEnable();
    // ���� Timer 1
    ROM_TimerEnable(TIMER3_BASE, TIMER_A);
}

void IntHandle_TIMER3A(void)//����ע���жϾ��ʱ�����Ʊ���һ�� 
{
	//�����־λ���ڶ������ж����ͣ��������Ƕ�ʱ��A����ж�
	TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
      
	timer_count++;
	ANO_DT_SendString("TIMER_TIMA_TIMEOUT"); 
 }