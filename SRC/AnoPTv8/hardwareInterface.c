#include "hardwareInterface.h"
#include "AnoPTv8Run.h"
#include "AnoPTv8FrameFactory.h"
//AptHwSendBytes�˺�����Ҫ�����û��Լ����豸������ʵ�֣�����ʹ�ô���������λ��������Ͷ�Ӧ�ô��ڵķ��ͺ���
//ע�⣺�����������ʹ���ж�+����������DMA+�������ķ�ʽ������ʽ���ͽ����Ӱ��ϵͳ����
//ע�⣺���ڻ�������Ӧ��С���Ƽ�256�ֽڻ�����
//���������ö�Ӧ�Ĵ���h�ļ�
#include "ano_usb.h"
void AnoPTv8HwSendBytes(uint8_t *buf, uint16_t len)
{
  AnoUsbCdcSend(buf, len);
}

//AptHwRecvByte�˺�������hardwareInterface.h���������û���Ҫ�ڶ�Ӧ���ڵĽ����¼��е��ô˺���
//ע�⣺�˺�����������ֽ����ݣ���������¼����յ������ݴ���1�ֽڣ���ε��ô˺�������
void AnoPTv8HwRecvByte(uint8_t dat)
{
	AnoPTv8RecvOneByte(dat);
}

//AptHwTrigger1ms�˺�������hardwareInterface.h���������û���Ҫ��1ms��ʱ�жϻ���ϵͳ�δ�����Լ���Ƶĵ�������
//��1ms��ʱ�������ô˺���
void AnoPTv8HwTrigger1ms(void)
{
	AnoPTv8TxRunThread1ms();
}

#include "FreeRTOS.h"
#include "task.h"

/* У׼ͨ��������� */
void ano_helper_task(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

	static u8 usbdatarxbuf[100];
	static u8 count = 0;
  while (1) {
		
		AnoPTv8TxRunThread1ms();

		u16 len = AnoUsbCdcRead(usbdatarxbuf,100);
		if(len) {
			for(u8 i=0; i<len; i++)
				AnoPTv8RecvOneByte(usbdatarxbuf[i]);
		}
		
		if(count++ == 25){
			count=0;
			AnoPTv8SendFCD01(0xFF);
			AnoPTv8SendFCD02(0xFF);
		}
	
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 1000);
  }
}