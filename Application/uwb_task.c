#include "uwb_task.h"

float uwbSpeedOut[2];
uint8_t useUwb = 0;
uint32_t totalFrameCount = 0, errorFrameCount = 0;

static int16_t pos[2];

static uint8_t unpack_data(void);
static uint8_t validate_data(void);  
static uint8_t position_control(const int16_t tarX,const int16_t tarY,uint16_t checkTime);
static void fusion_parameter_init(void);
static void imu_fus_update(u8 dT_ms);
static void uwb_test_task(uint8_t direction);

void uwb_update_task(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  //uwb���ڳ�ʼ��
  Drv_Uart2Init(921600);
  debugOutput("uwb use uart2��rate:921600");

	fusion_parameter_init();
  while (1) {

    //�����ɹ�,У��ɹ�����ʹ������
    uint8_t dataValidity = unpack_data(); 
		 
    if(useUwb == 0)
			memset(uwbSpeedOut,0,sizeof(uwbSpeedOut)/sizeof(uwbSpeedOut[0]));
			
		if(flag.taking_off)
      switch(useUwb) {
      case 1:
				uwb_test_task(0);
        break;
      case 2:
				uwb_test_task(1);
        break;
      }
			
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 100);
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
  const int16_t maxX = 5000, maxY = 4000; 
  if(pos[X] < 0 || pos[Y] < 0)
    res = 0;
  if(pos[X] > maxX || pos[Y] > maxY)
    res = 0;

  return res;
} 

static _inte_fix_filter_st accFus[2]; 
_fix_inte_filter_st posFus[2], speedFus[2];
 
//uwb�����ںϼ��ٶȼ�
static void imu_fus_update(u8 dT_ms)
{
  s32 rawPos[2];
  rawPos[X] = pos[X];
  rawPos[Y] = pos[Y];

  static s32 lastRawPos[2],lastRawSpeed[2];
  static s32 rawSpeed[2],rawAcc[2];

	rawSpeed[X] = OF_DX2;
	rawSpeed[Y] = OF_DY2;
  for(uint8_t i=X; i<Z ; i++) { 
//		rawSpeed[i] = g_nlt_tagframe0.result.vel_3d[i];
		rawAcc[i] = (rawSpeed[i] - lastRawSpeed[i]);

		lastRawPos[i] = rawPos[i];
		lastRawSpeed[i] = rawSpeed[i]; 

    accFus[i].in_est = uwb_acc_use[i];
    accFus[i].in_obs = rawAcc[i];
    inte_fix_filter(dT_ms*1e-3f,&accFus[i]);
 
    speedFus[i].in_est_d = accFus[i].out;
    speedFus[i].in_obs = rawSpeed[i];
    fix_inte_filter(dT_ms*1e-3f,&speedFus[i]);
 
    posFus[i].in_est_d = speedFus[i].out;
    posFus[i].in_obs = rawPos[i];
    fix_inte_filter(dT_ms*1e-3f,&posFus[i]); 
  }

}

static void fusion_parameter_init(void)
{ 
  for(uint8_t i=X; i<Z ; i++) { 
		accFus[i].fix_ki = 0.1f;
		accFus[i].ei_limit = 100;

		speedFus[i].fix_kp = 0.6f;
		speedFus[i].e_limit = 100;

		posFus[i].fix_kp = 0.3f;
		//posFus[i].e_limit = 200;
		
		
		accFus[i].out = 0;
		accFus[i].ei = -uwb_acc_use[i];

		speedFus[i].out = 0;
		speedFus[i].e = 0;

		posFus[i].out = 0;
		posFus[i].e = 0;
  } 
}

//λ�ÿ���(��λ:cm)
static uint8_t position_control(const int16_t tarX,const int16_t tarY,uint16_t checkTime)
{
  const float kp = 1.05f;
  const float ki = -0.01f; 
 
	uint8_t isGetAround = 0;
	static TickType_t startTime = 0;
	
	int16_t exp[2] = {tarX, tarY};
  for(uint8_t i=0; i<2; i++) {
    //P
//    int error = exp[i] - posFus[i].out ;
    int16_t error = exp[i] - pos[i] ;

    static int16_t errorIntegral = 0;
		//I
    if(abs(error) < 3)
      errorIntegral += error;
    else
      errorIntegral = 0;

		errorIntegral = LIMIT(errorIntegral, -60,60); 

    uwbSpeedOut[i] = error*kp + errorIntegral*ki;

    if(abs(error) < 10)
			isGetAround++; 
  }   
	
	if(isGetAround != 2){
		startTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ
		return 0;
	}
	
	if( (xTaskGetTickCount() - startTime) > pdMS_TO_TICKS(checkTime))
		return 1;
	else
		return 0;
} 

static void uwb_test_task(uint8_t direction)
{ 
	static	uint8_t	tarPosBufIndex = 0;
	static	uint16_t	tarPosBuf[] = {210,230,250,270,290,310,330};
	
	uint8_t isArrive = position_control(tarPosBuf[tarPosBufIndex] , 200 , 500); 
	
	if(!isArrive)
		return;
	
	if(direction == 0 && tarPosBufIndex>0)
		tarPosBufIndex--;
	else if(direction == 1 && tarPosBufIndex<((sizeof(tarPosBuf)/sizeof(tarPosBuf[0]))-1))
		tarPosBufIndex++;
		
	
}