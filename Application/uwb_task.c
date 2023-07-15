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
  Drv_Uart2Init(921600);
  debugOutput("uwb use uart2��rate:921600");

  while (1) {  
		
		for(; ;){ 
			RingBufferDataLen = RingBuffer_GetCount(&uwbRingBuff) ;
			//�����������ݲ���һ֡�ĳ���
			if(RingBufferDataLen<128)
				break;
			 
			//����֡ͷ
			RingBuffer_Pop(&uwbRingBuff, &pData[0]);
			//�ҵ�֡ͷ
			if(pData[0] == 0x55) {
				totalFrameCount++;
				//����ʣ������
				for(uint16_t cnt = 1; cnt <128 ; cnt++) 
					RingBuffer_Pop(&uwbRingBuff, &pData[cnt]); 
				//��������
				if(!g_nlt_tagframe0.UnpackData(pData, 128))  
					//�����д�
					errorFrameCount++;  
			}
		} 
		
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 100);
  }
} 