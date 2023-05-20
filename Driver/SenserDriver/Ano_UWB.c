//==����
#include "Ano_UWB.h"
#include "Drv_Uart.h"
#include "ring_buffer.h"
#include "nlink_utils.h"
#include "nlink_linktrack_tagframe0.h"




/**********************************************************************************************************
*�� �� ��: UWB_Get_Data_Task
*����˵��: UWB���ݻ�ȡ����
*��    ��: ���ڣ����룩
*�� �� ֵ: ��
**********************************************************************************************************/
static u16 uwb_check_time;
void UWB_Get_Data_Task(void)
{
  //�����λ�����
  static  int RingBufferDataLen = 0;
  static unsigned char pData[128 * 5];
  RingBufferDataLen = RingBuffer_GetCount(&U1rxring) ;

  //����uwb����
  if(RingBufferDataLen) {
    RingBuffer_PopMult(&U1rxring, pData, RingBufferDataLen);

    if (g_nlt_tagframe0.UnpackData(pData, RingBufferDataLen)) {
      return;
    }
  }

}
