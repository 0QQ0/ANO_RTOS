#include "light_flow_task.h"
 
 
static void unpack_data(void);
	
/* �������ݸ��¼��߶��ں� */
void light_flow_task(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

	//��ʼ������
	light_flow_init();
	
  while (1) { 
		
		unpack_data();
		
    /*λ�ô�����״̬���*/
    sensor_detection(5);
		
		/* �������ڻ��������ݸ�����������ݣ������߼�� */
		ANO_OF_Data_Get(5, OF_DATA);
		
    /* �������߼�� */
    AnoOF_Check(5); 
 
    /* �����ƴ�������������ں����� */
    ANO_OFDF_Task(5);
		
    /*�߶������ں�����*/
    wcz_fus_update(5);
		 
		
#if defined(USE_KS103)
    //����������
    Ultra_Duty();
#endif
		
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 200);
  }
}
 

static void unpack_data(void)
{ 
  uint8_t RingBufferDataLen = RingBuffer_GetCount(&lightFlowRing) ;
	 
  for(uint8_t cnt = 0; cnt <RingBufferDataLen ; cnt++) {
    uint8_t data = 0;
    RingBuffer_Pop(&lightFlowRing, &data); 
 	
    //������������
    if(of_init_type != 2)  
      AnoOF_GetOneByte(data);
		
		//�����������
		else if(of_init_type!=1) 
			OFGetByte(data); 
		
  }
} 
		