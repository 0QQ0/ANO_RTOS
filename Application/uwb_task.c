#include "uwb_task.h"

 
/* uwb���ݸ��� */
void uwb_update_task(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  static unsigned char pFrame[128];

  while (1) {
    //�����λ�����
    if(RingBuffer_GetCount(&U1rxring) > 128) {

      memset(pFrame,0,128);
      RingBuffer_PopMult(&U1rxring, pFrame, 128);

      //����uwb����
      uint8_t res = g_nlt_tagframe0.UnpackData(pFrame, 128);
      switchs.uwb_on = res;
      //��������ˢ�»�����
      if (res == 0) {
        RingBuffer_Flush(&U1rxring);
      }
    }
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 50);
  }
}
