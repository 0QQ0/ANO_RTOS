#include "openmv_task.h"
#include "Ano_OPMV_Ctrl.h"
#include "Ano_OPMV_CBTracking_Ctrl.h"

#define OPENMV_CBTRACK_CTR (1)

//ֻ����X,Y��YAW���ٶ�
float openmvSpeedOut[3]; 

_openmv_data_st opmv;

uint8_t useOpenmv = 0;
openmv_t mvValue; 
static uint32_t value;

static uint8_t unpack_data(void);
static void assign_value(uint8_t mId) ;
static float position_control(uint32_t exp ,uint32_t measureValue);
static float area_control(uint32_t exp ,uint32_t measureValue);


/* openmv���ݸ��� */
void openmv_update_task(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  //openmv���ڳ�ʼ��
  Drv_Uart5Init(921600);
  debugOutput("openmv use uart5��rate:921600");

	static TickType_t waitTime = 0;
  while (1) {
    
		
#if (OPENMV_CBTRACK_CTR == 1)
    uint8_t recId = unpack_data(); /* ��֡ */
		assign_value(recId);					 /* ��ֵ */
		
		if((recId == POS_X_ERROR) || (recId == POS_Y_ERROR))
		{
			if(recId == POS_X_ERROR)
			{
				opmv.cb.pos_x = value;/* ԭʼ���� */
				opmv.cb.pos_x = opmv.cb.pos_x - 160;/* ���ƫ����ͼ������ */
				opmv.cb.pos_x = -opmv.cb.pos_x;
			}
			else if(recId == POS_Y_ERROR)
			{
				opmv.cb.pos_y = value;/* ԭʼ���� */
				opmv.cb.pos_y = opmv.cb.pos_y - 120;/* ���ƫ����ͼ������ */
				opmv.cb.pos_y = -opmv.cb.pos_y;
			}
			
			opmv.cb.sta = 1;					/* ʶ����Ч */
			opmv.mode_sta = 1;				/* openmv׷Ѱɫ��ģʽ */
			
			OpenMV_Check_Reset();	
		}

		/*OPMV����Ƿ����*/
		OpenMV_Offline_Check(20);
		/*OPMVɫ��׷�����ݴ�������*/
		ANO_CBTracking_Task(20);
		/*OPMV��������*/
		ANO_OPMV_Ctrl_Task(20);
#else
		uint8_t recId = unpack_data(); 
		if(recId){ 
			assign_value(recId);
			waitTime = xTaskGetTickCount();
		}
		
		//һ��ʱ��������Ч���ݻ��߲�ʹ��openmv������ٶ�����
		if((xTaskGetTickCount() - waitTime) > pdMS_TO_TICKS(500) || useOpenmv == 0) 
		memset(openmvSpeedOut,0,sizeof(openmvSpeedOut));
#endif
		
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 50);
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

//����id��ֵ,ʹ������
static void assign_value(uint8_t mId)
{
	switch (mId) {
	case ANGLE_ID :
		mvValue.angle = value;
		break;
	case AREA_ID :
		mvValue.area = value;
		break;
	case WIDTH_ID :
		mvValue.width = value;
		break;
	case POS_ID :
		mvValue.pos = value;
		openmvSpeedOut[YAW] = position_control(160 ,mvValue.pos);
		openmvSpeedOut[Y] = 20;
		break;
	case RES_ID :
		mvValue.res = value;
		break;
	default:
		break;
	} 
}

//����Ŀ����ͼ���е�λ�ã����ص��ٶȿ���yaw�����Ƹˣ�
static float position_control(uint32_t exp ,uint32_t measureValue)
{
  const float kp = -0.38f;
  const float ki = 0.0f; 

  //P
  int error = exp - measureValue  ;

  static int errorIntegral = 0;
		//I
    if(abs(error) < 5)
      errorIntegral += error;
    else
      errorIntegral = 0;
 
	errorIntegral = LIMIT(errorIntegral, -50,50); 
 
  float out = error*kp + errorIntegral*ki;
	return out ;
}

//����Ŀ����ͼ�������صĴ�С(���ƿ���openmv��Ŀ�����Ծ���)
static float area_control(uint32_t exp ,uint32_t measureValue)
{
  const float kp = 0.2f;
  const float ki = 0.0f; 

  //P
  int error = exp - measureValue  ;

  static int errorIntegral = 0;
		//I
    if(abs(error) < 5)
      errorIntegral += error;
    else
      errorIntegral = 0;
 
	errorIntegral = LIMIT(errorIntegral, -50,50); 
 
  float out = error*kp + errorIntegral*ki;
	return out ;
}

