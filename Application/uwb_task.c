#include "uwb_task.h"


uint8_t useUwb = 0;
uint32_t totalFrameCount = 0, errorFrameCount = 0;

#define X 0
#define Y 1

int32_t satrtPos[2];
static int32_t pos[2];

static uint8_t unpack_data(void);
static uint8_t validate_data(void);
static uint8_t set_start_point(void); 
static void position_control(const int tarX,const int tarY);
static void drawing_circle(void);

void uwb_update_task(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  //uwb���ڳ�ʼ��
  Drv_Uart2Init(921600);
  debugOutput("uwb use uart2��rate:921600");

  while (1) {

    //�����ɹ�,У��ɹ�����ʹ������
    uint8_t dataValidity = unpack_data();

    //��¼��ɵ�
		static uint8_t setPointOk;
    if(useUwb == 0)
      Program_Ctrl_User_Set_HXYcmps(0, 0);
    else if(dataValidity==1 && setPointOk==0)
      setPointOk = set_start_point();

		if(!flag.taking_off)
			setPointOk = 0;
		
    //����ִ������
    if(dataValidity==1 && setPointOk==1) {
      switch(useUwb) {
      case 1:
				//������ɵ� 
        position_control(satrtPos[X],satrtPos[Y]);
        break;
      case 2:
        position_control(2000,2000);
        break;
      }
    }
		

    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 120);
  }
}

//�����������е�����,����1������ɹ�
static uint8_t unpack_data(void)
{
  static uint8_t pData[128];
  static uint16_t RingBufferDataLen;

  uint8_t analyticResult = 0;
  while(1) {
    RingBufferDataLen = RingBuffer_GetCount(&uwbRingBuff) ;
    //�����������ݲ���һ֡�ĳ���
    if(RingBufferDataLen<128)
      break;

    //����֡ͷ
    RingBuffer_Pop(&uwbRingBuff, &pData[0]);

    if(pData[0] == 0x55) {
      totalFrameCount++;

      //����ʣ������
      for(uint16_t cnt = 1; cnt <128 ; cnt++)
        RingBuffer_Pop(&uwbRingBuff, &pData[cnt]);

      //��������
      analyticResult = g_nlt_tagframe0.UnpackData(pData, 128);

      //�����д�
      if(analyticResult == 0)
        errorFrameCount++;
      else {
        pos[X] = g_nlt_tagframe0.result.pos_3d[X];
        pos[Y] = g_nlt_tagframe0.result.pos_3d[Y];

				//���ݼ���
				analyticResult = validate_data();
      }

    }
  }
  return analyticResult;
}

//�ж������Ƿ���Ч
static uint8_t validate_data(void)
{
  static float lastPos[2];
  //���Ϊ1������Ч
  uint8_t res = 0;
  //uwb�����ڱ仯˵��������Ч
  if(lastPos[X] != pos[X])
    res=1;
  //�����ϴ�����
  lastPos[X] = pos[X];

  //Χ�����������Ч(���ĸ�ê��Χ�ɵľ�����Χ��)
  const int16_t maxX = 5000, maxY = 4000;  ;
  if(pos[X] < 0 || pos[Y] < 0)
    res = 0;
  if(pos[X] > maxX || pos[Y] > maxY)
    res = 0;

  return res;
}

//�����ɺ���ݿ��Ŷ�������ɵ�
static uint8_t set_start_point(void)
{
  if(g_nlt_tagframe0.result.eop_3d[X]<10 &&
     g_nlt_tagframe0.result.eop_3d[Y]<10 ){

    satrtPos[X] = pos[X];
    satrtPos[Y] = pos[Y];
			 
		return 1;
  }
 
  return 0;
}

//λ�ÿ���(��λ:cm)
static void position_control(const int tarX,const int tarY)
{
  const float kp = 0.04f;
  const float ki = 0.0f;
  const float kd = 0.0f;

	//����Ͳ����п���
  const uint8_t permitError = 8;
	if(abs(pos[X]-tarX)<permitError &&
		 abs(pos[Y]-tarY)<permitError ){
		Program_Ctrl_User_Set_HXYcmps(0, 0);  
		return;	 
  }

	
  float out[2] = {0};
  int exp[2] = {tarX, tarY};

  for(uint8_t i=0; i<2; i++) {
		//P
    int error = exp[i] - pos[i] ;

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

    out[i] = error*kp + errorIntegral*ki + differential*kd;

  }

	Program_Ctrl_User_Set_HXYcmps(out[0], out[1]);
}


static void drawing_circle(void)
{
  //Բ����������(��λ:cm)
  const int16_t centerCoordinates[2] = {2500,2500};
  //Բ�İ뾶(��λ:cm)
  const int16_t radius = 500;
  //�켣ϸ�ֳɶ��ٸ���
  const uint16_t trackDivNumber = 40;
  //����
  const uint16_t step = 2*radius/trackDivNumber;

  static int8_t currentPoint = 0, direction= 1;

  //x2+y2=r2
  int16_t targetX = (centerCoordinates[0] - radius)+ (currentPoint * step);
  int16_t targetY = direction*(int16_t)sqrt((double)abs(radius*radius - targetX*targetX));


  position_control(targetX,targetY);


  int errorX = targetX - pos[X] ;
  int errorY = targetY - pos[Y] ;

  if(abs(errorX)<15 && abs(errorY)<15)
    currentPoint += direction;

  if(currentPoint == trackDivNumber)
    direction *= -1;
}

