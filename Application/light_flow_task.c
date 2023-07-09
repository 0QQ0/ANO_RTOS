#include "light_flow_task.h"

 
/* �������ݸ��¼��߶��ں� */
void light_flow_task(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

	//��ʼ������
	light_flow_init();
	
  while (1) {
		/* �������ڻ��������ݸ�����������ݣ������߼�� */
		ANO_OF_Data_Get(5, OF_DATA);
 
    /* �����ƴ�������������ں����� */
    ANO_OFDF_Task(5);
		
    /* �������߼�� */
    AnoOF_Check(5);
		
    /*�߶������ں�����*/
    WCZ_Fus_Task(5);
		
#if defined(USE_KS103)
    //����������
    Ultra_Duty();
#endif
		
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 200);
  }
}