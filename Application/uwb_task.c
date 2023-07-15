#include "uwb_task.h"
 
uint32_t totalFrameCount = 0 , errorFrameCount = 0; 

static uint8_t validate_data(void);
static void position_control(const int tarX ,const int tarY);
static void drawing_circle(void);

uint8_t useUwb = 0; 
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
		uint8_t analyticResult = 0;
		for(; ;){ 
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
			}
		} 
		
		if(useUwb == 0)
			Program_Ctrl_User_Set_HXYcmps(0, 0);
		//�����ɹ�У��ɹ�����ʹ������
		if(analyticResult == 1 && validate_data() == 1){
			 
			if(useUwb == 1)
				drawing_circle();
			else if(useUwb == 2)
				position_control(2000 ,2000);
		} 
		
    vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / 100);
  }
} 

//�ж������Ƿ���Ч
static uint8_t validate_data(void)
{ 
  static float lastPos[3]; 
	//���Ϊ1������Ч
	uint8_t res = 0;
	//uwb�����ڱ仯˵��������Ч 
	if(lastPos[0] != g_nlt_tagframe0.result.pos_3d[0])
		res=1; 
	//�����ϴ�����
	lastPos[0] = g_nlt_tagframe0.result.pos_3d[0]; 
	
	return res; 
}

//λ�ÿ���(��λ:cm)
static void position_control(const int tarX ,const int tarY)
{
	const float kp = 0.1f;
	const float ki = 0.0f;
	const float kd = 0.0f;
	
	static int errorIntegral = 0;
	static int lastError = 0;
	 
	float out[2] = {0};
	int exp[2] = {tarX , tarY};
	 
	for(uint8_t i=0;i<2;i++){
		int error = exp[i] - g_nlt_tagframe0.result.pos_3d[i] ; 
		
		if(abs(error) < 10) 
			errorIntegral += error;  
		else
			errorIntegral = 0;
		 
		if(errorIntegral > 100)
			errorIntegral = 100;
		if(errorIntegral < -100)
			errorIntegral = -100;
		
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
	
	 
	position_control(targetX ,targetY);
	
	
  int errorX = targetX - g_nlt_tagframe0.result.pos_3d[0] ; 
  int errorY = targetY - g_nlt_tagframe0.result.pos_3d[1] ; 
	
	if(abs(errorX)<5 && abs(errorY)<5)
		currentPoint += direction;
	 
	if(currentPoint == trackDivNumber) 
		direction *= -1; 
}
