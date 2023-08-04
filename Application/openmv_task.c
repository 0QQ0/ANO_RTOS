#include "openmv_task.h"

//ֻ����X,Y��YAW���ٶ�
float openmvSpeedOut[3]; 

static int16_t firePos[2];

openmv_t mvValue; 
static uint32_t value;

static uint8_t unpack_data(void);
static void assign_value(uint8_t mId) ;
static float position_control_x(uint32_t exp ,uint32_t measureValue);
static float position_control_y(uint32_t exp ,uint32_t measureValue);
static uint8_t position_stability_judgment(uint16_t setTime);
static uint8_t throw_task(void);
static void send_fire_pos(void);

static uint8_t in_roi(void);

#define led_on() (ROM_GPIOPinWrite(GPIOF_BASE, GPIO_PIN_0, 0))
#define led_off() (ROM_GPIOPinWrite(GPIOF_BASE, GPIO_PIN_0, 1))

const uint16_t expPosX = 120 , expPosY = 160;
/* openmv���ݸ��� */
void openmv_update_task(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

  //openmv���ڳ�ʼ��
  Drv_Uart5Init(921600);
  debugOutput("openmv use uart5��rate:921600");

	static TickType_t waitTime = 0;
	static uint8_t  onlyOnce = 1;
  while (1) {
    uint8_t recId = unpack_data(); 
		
		//�ҵ�ɫ��
		if(recId && onlyOnce){ 
			//��ʱ�ȴ���ʱ����
			waitTime = xTaskGetTickCount();
			//��������
			assign_value(recId);
			//��������״ִ̬�ж���
			switch(taskStatus_2023){
				//Ѳ�����ҵ�ɫ��
				case cruise:
					//��ֹѲ��,��ʼ��λ��Դ
						if( in_roi() )
							taskStatus_2023 = fixPoint;
					break;
				case fixPoint:
					//λ���ȶ�,��¼��Դλ��,��ָʾ��
					if(position_stability_judgment(300) == 1){
						firePos[X] = getPosX();
						firePos[Y] = getPosY();
						
						led_on();
            
						taskStatus_2023 = throwObject; 
					}
					break;
				case throwObject:
					//Ͷ������ִ������һص�Ѳ���߶�,����Ѳ��
					if(throw_task() == 1){
						taskStatus_2023 = cruise; 
						onlyOnce = 0;
					}
					break;
			}
			
			//��λɫ��
			openmvSpeedOut[X] = position_control_x(expPosX ,mvValue.posX);
			openmvSpeedOut[Y] = position_control_y(expPosY ,mvValue.posY);
		}
		
		//һ��ʱ��������Ч���ݻ���Ѳ��״̬mv�����ٶ�����
		if((xTaskGetTickCount() - waitTime) > pdMS_TO_TICKS(500) || taskStatus_2023 == cruise) {
			taskStatus_2023 = cruise;
			memset(openmvSpeedOut,0,sizeof(openmvSpeedOut));
		}
		
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
	case X_ID :
		mvValue.posY = value;
		break;
	case Y_ID :
		mvValue.posX = value;
		break;
	default:
		break;
	} 
}

//����Ŀ����ͼ���е�λ�ã����ص��ٶȿ���yaw�����Ƹˣ�
static float position_control_x(uint32_t exp ,uint32_t measureValue)
{
  const float kp = 0.12f;
  const float ki = -0.06f; 

  //P
  int error = exp - measureValue  ;

  static int errorIntegral = 0;
	//I
	if(abs(error) < 10)
		errorIntegral += error;
	else
		errorIntegral = 0;
  
	errorIntegral = LIMIT(errorIntegral, -300,300); 
 
  float out = error*kp + errorIntegral*ki;
	return out ;
}

static float position_control_y(uint32_t exp ,uint32_t measureValue)
{
  const float kp = 0.12f;
  const float ki = -0.06f; 

  //P
  int error = exp - measureValue  ;

  static int errorIntegral = 0;
	//I
	if(abs(error) < 10)
		errorIntegral += error;
	else
		errorIntegral = 0;
 
	errorIntegral = LIMIT(errorIntegral, -300,300); 
 
  float out = error*kp + errorIntegral*ki;
	return out ;
}


//λ���ȶ��ж�
static uint8_t position_stability_judgment(uint16_t setTime)
{  	
	static TickType_t lastTime = 0;
	TickType_t thisTime = xTaskGetTickCount();
	
	uint8_t res = 0;
	res += (ABS(mvValue.posX-expPosX) < 40) ? 1 : 0 ;
	res += (ABS(mvValue.posY-expPosY) < 40) ? 1 : 0 ;
	
	if(lastTime == 0 || res != 2)
		lastTime = thisTime;
	
	if( (thisTime - lastTime) > pdMS_TO_TICKS(setTime)){
		lastTime = 0;
	  return 1;
	}
	
	return 0;
}

//Ͷ������(setTimeΪ��ͣʱ��)
static uint8_t throw_task(void)
{  	
	const uint16_t downWaitTime = 3200;
	const uint16_t upWaitTime = 1000;
	static uint16_t	setTime = downWaitTime;
	
	static uint16_t tarHight = 100;
	static TickType_t lastTime = 0;
	TickType_t thisTime = xTaskGetTickCount();
	
	float	hightError = tarHight - wcz_hei_fus.out;
	//��������ٶ�
	int16_t speedOut = 2.0 * hightError;
	Program_Ctrl_User_Set_Zcmps(speedOut);
	
	if( (lastTime == 0) || ( ABS((int)hightError) > 5) )
		lastTime = thisTime;
	
	if( (thisTime - lastTime) > pdMS_TO_TICKS(setTime)){
		lastTime = 0;
		//�½�������3s,Ͷ����Ʒ,�ҷ���������Ϣ������
		if(tarHight != Ano_Parame.set.auto_take_off_height){
			setTime = upWaitTime;
			tarHight = Ano_Parame.set.auto_take_off_height;
			//���ƶ��Ͷ��
			gear_protocol_set(6 , 0 );// perDegree_90(90.0f) );
			gear_protocol_set(7 , 0 );// perDegree_90(90.0f) );
			
			//��������
			send_fire_pos();
		}
		//������ɽ����������Ѳ��
		else
			return 1;
	}
	
	return 0;
}

//���͵�ǰλ����Ϣ��С����
static void send_fire_pos(void)
{  	
		char coordinateStr[50];
		
		memset(coordinateStr,0,sizeof(coordinateStr));
		sprintf((char*)coordinateStr,"firePos,%d,%d\r\n",firePos[X],firePos[Y]);
		
	//���Ͷ���ֹ��ʧ
	  Drv_Uart3SendBuf((uint8_t*)coordinateStr, sizeof(coordinateStr));
	  Drv_Uart3SendBuf((uint8_t*)coordinateStr, sizeof(coordinateStr));
	  Drv_Uart3SendBuf((uint8_t*)coordinateStr, sizeof(coordinateStr));
}

//�Ƿ��ڻ�Դ���ܳ��ֵ�����
static uint8_t in_roi(void)
{  	
	uint8_t res ;
	uint8_t x = dotPath[dotfIndex].x , y = dotPath[dotfIndex].y;
	if( x > 3 || y > 0)
		return 1;
	else
		return 0;

}