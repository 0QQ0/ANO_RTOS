#ifndef _RC_UPDATE_H_
#define	_RC_UPDATE_H_ 

#include "sysconfig.h"
#include "Ano_Parameter.h"
#include "Drv_icm20602.h"
#include "Ano_MagProcess.h"
#include "Drv_RcIn.h"
#include "Ano_DT.h"
#include "Ano_LED.h"
#include "Ano_FcData.h"
#include "Ano_FlyCtrl.h"
#include "Ano_FlightCtrl.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"



//ҡ�˴���ֵ��ҡ��ֵ��ΧΪ+-500������300���ڴ�����Χ
#define UN_YAW_VALUE  300
#define UN_THR_VALUE  300
#define UN_PIT_VALUE  300
#define UN_ROL_VALUE  300


enum {
  CH1 = 0,
  CH2,
  CH3,
  CH4,
  CH5,
  CH6,
  CH7,
  CH8
};


extern int16_t CH_N[] ;

 
void receivingModeInit(void);
void receivingTask(void);



#endif


