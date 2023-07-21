#include "openmv_task.h"

static uint8_t unpack_data(void);

//����ƫ��
#define ANGLE_ID (0x01)
//���
#define AREA_ID (0x02)
//���
#define WIDTH_ID (0x03)
//ͼ��λ��
#define POS_ID (0x04)
//��ά��ʶ����
#define RES_ID (0x05)

uint32_t angleValue, areaValue, widthValue, posValue, resValue;
static uint32_t value;

static uint8_t unpack_data(void);
static void position_control(uint32_t measureValue);

/* openmv���ݸ��� */
void openmv_update_task(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  //openmv���ڳ�ʼ��
  Drv_Uart5Init(921600);
  debugOutput("openmv use uart5��rate:921600");

  while (1) {
    uint8_t recId = unpack_data();
    switch (recId) {
    case 0:
      break;
    case ANGLE_ID :
      angleValue = value;
      break;
    case AREA_ID :
      areaValue = value;
      break;
    case WIDTH_ID :
      widthValue = value;
      break;
    case POS_ID :
      posValue = value;
      static uint32_t lastPos = 0; 
			//λ�������ڱ仯��δʶ�𵽶�ά��
      if(posValue!=lastPos && resValue == 0)
				//���Ƹ˵�λ����ͼ�����ı��ֵ���ǰ��
        position_control(posValue);
      else
        Program_Ctrl_User_Set_HXYcmps(0, 0);
      break;
    case RES_ID :
      resValue = value;
      break;
    }

    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 100);
  }
}


//�����������е�����,����id������ɹ�
static uint8_t unpack_data(void)
{
  static uint8_t analyticCnt = 0;
  static uint8_t numCount = 0;
  static uint8_t numBuf[5] = {0};

  uint8_t RingBufferDataLen = RingBuffer_GetCount(&openmvRing) ;

  uint8_t id = 0;

  for(uint8_t cnt = 0; cnt <RingBufferDataLen ; cnt++) {
    uint8_t data = 0;
    RingBuffer_Pop(&openmvRing, &data);
    //����֡ͷ
    switch(analyticCnt) {
    case 0:
      if(data == 0xAA )
        analyticCnt++;
      break;
    case 1:
      id = data;
      analyticCnt++;
      break;
    case 2:
      if(data == 0xFF)
        analyticCnt++;
      else {
        numBuf[numCount] = data;
        numCount++;
      }
      //����λ
      if(numCount>5) {
        analyticCnt=0;
        numCount = 0;
        id = 0;
      }
      break;
    }

    if(analyticCnt == 3)
      break;
  }

  //��ϳ����� fifo ���ģʽ
  if(id != 0 && analyticCnt == 3) {
    uint32_t numTemp = 0;
    for(uint8_t i=0; i<numCount; i++) {
      if(i != 0)
        numTemp *= 10;
      numTemp += numBuf[i];
    }
    numCount = 0;
    analyticCnt = 0;
    value = numTemp;
  }
  return id;
}


//λ�ÿ���(��λ:cm)
static void position_control(uint32_t measureValue)
{
  const float kp = 0.08f;
  const float ki = 0.0f;
  const float kd = 0.0f;


  float out  =  0 ;
  uint32_t exp  = 160;

  //P
  int error = exp - measureValue  ;

  static int errorIntegral = 0;
//		//I
//    if(abs(error) < 10)
//      errorIntegral += error;
//    else
//      errorIntegral = 0;

//    if(errorIntegral > 100)
//      errorIntegral = 100;
//    if(errorIntegral < -100)
//      errorIntegral = -100;

  //D
  static int lastError = 0;
  int differential = lastError - error;
  lastError = error;

  out = error*kp + errorIntegral*ki + differential*kd;


  Program_Ctrl_User_Set_HXYcmps(0, out);
}

