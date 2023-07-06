#include "watch_dog.h" 
 
#include "FreeRTOS.h"
#include "task.h"

 
#include "TM4C123G.h"
#include "rom_map.h"
#include "rom.h" 
#include "sysctl.h" 
#include "pin_map.h" 
#include "hw_ints.h"
 
/* ���Ź����� ������Ϊ��׼���е����� ִ��Ƶ�ʾ�׼2Hz */
void wdt0_loop(void *pvParameters)
{
	// Enable the peripherals used by this example.
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
	
	// Enable the watchdog interrupt.
	ROM_IntEnable(INT_WATCHDOG);

	// Set the period of the watchdog timer.
	ROM_WatchdogReloadSet(WATCHDOG0_BASE, MAP_SysCtlClockGet()); /* 1s�����ж� */

	// Enable reset generation from the watchdog timer.
	ROM_WatchdogResetEnable(WATCHDOG0_BASE);

	// Enable the watchdog timer.
	ROM_WatchdogEnable(WATCHDOG0_BASE);
	
	static TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���
	
	xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ
	
	while(1)
	{
		ROM_WatchdogIntClear(WATCHDOG0_BASE);	/* ι�� */ 
		vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 2);
	}
}

void WDT0_Handler(void)
{
	//�����жϳ������ܷ� 
	ROM_WatchdogIntClear(WATCHDOG0_BASE);	 
//	MAP_IntDisable(INT_WATCHDOG);
//	MAP_WatchdogResetDisable(WATCHDOG0_BASE);
	
	//����
	#include "ano_usb.h"
	unsigned char theDogWantsToSay[] = "Hello World";
	AnoUsbCdcSend(theDogWantsToSay, sizeof(theDogWantsToSay));
	   
	for(unsigned int i = 0; i<10000; i++){ 
			__nop(); 
	}
	//��λ
	ROM_SysCtlReset();   
}


