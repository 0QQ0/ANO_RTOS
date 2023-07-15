#include "watch_dog.h" 
 
#include "FreeRTOS.h"
#include "task.h"

 
#include "TM4C123G.h"
#include "rom_map.h"
#include "rom.h" 
#include "sysctl.h" 
#include "pin_map.h" 
#include "hw_ints.h"
  
#include "Drv_Uart.h"
#include "ano_usb.h"
#include "Ano_DT.h"

/* ���Ź����� */
void wdt0_loop(void *pvParameters)
{
	// Enable the peripherals used by this example.
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
	
	// Enable the watchdog interrupt.
	MAP_IntEnable(INT_WATCHDOG);

	// Set the period of the watchdog timer.
	MAP_WatchdogReloadSet(WATCHDOG0_BASE, MAP_SysCtlClockGet()/10); /* 100ms�����ж� */

	// Enable reset generation from the watchdog timer.
	MAP_WatchdogResetEnable(WATCHDOG0_BASE);

	// Enable the watchdog timer.
	MAP_WatchdogEnable(WATCHDOG0_BASE);
	 
	debugOutput("Here's the dog");
	
	static TickType_t xLastWakeTime;    	//���ھ�׼��ʱ�ı��� 
	xLastWakeTime = xTaskGetTickCount(); 	//��ȡ��ǰTick����,�Ը�����ʱ������ֵ
	
	while(1)
	{
		/* ι�� */ 
		MAP_WatchdogIntClear(WATCHDOG0_BASE);	
		vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 20);
	}
}

void WDT0_Handler(void)
{
	//�����жϳ������ܷ� 
	MAP_WatchdogIntClear(WATCHDOG0_BASE);	 
	MAP_IntDisable(INT_WATCHDOG);
	MAP_WatchdogResetDisable(WATCHDOG0_BASE);
	
	//����
	unsigned char theDogWantsToSay[] = "66666666666"; 
	
	Drv_Uart3SendBuf(theDogWantsToSay,sizeof(theDogWantsToSay)); 
	
	//��ʱ��λ
	for(unsigned int i=0; ;i++) 
		if(i == 100000)
			ROM_SysCtlReset();    
}