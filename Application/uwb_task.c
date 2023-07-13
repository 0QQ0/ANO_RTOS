#include "uwb_task.h"


uint32_t totalFrameCount = 0;
uint32_t errorFrameCount = 0;
/* uwb���ݸ��� */
void uwb_update_task(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  static uint8_t pData[128];
	static uint16_t RingBufferDataLen;
  //uwb���ڳ�ʼ��
  Drv_Uart1Init(3000000);
  debugOutput("uwb use uart1��rate:3000000");

  while (1) { 
		
		RingBufferDataLen = RingBuffer_GetCount(&U1rxring) ;
		
		for(uint16_t i=0; ;i++){
			//�����������ݲ���һ֡�ĳ���
			if(RingBufferDataLen-i<128)
				break;
			 
			//����֡ͷ
			RingBuffer_Pop(&U1rxring, &pData[0]);
			if(pData[0] == 0x55) {
				//�ҵ�֡ͷ
				totalFrameCount++;
				//����ʣ������
				for(uint16_t cnt = 1; cnt <128 ; cnt++){ 
					RingBuffer_Pop(&U1rxring, &pData[cnt]);
				} 
				//��������
				if(!g_nlt_tagframe0.UnpackData(pData, RingBufferDataLen))  
					//�����д�
					errorFrameCount++; 
			}
		} 
		
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 200);
  }
} 