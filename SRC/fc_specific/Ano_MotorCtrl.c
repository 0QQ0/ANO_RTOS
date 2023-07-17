#include "Ano_MotorCtrl.h"
#include "Drv_PwmOut.h"

#include "rc_update.h"

/*
���᣺
      ��ͷ
   m2     m1
     \   /
      \ /
      / \
     /   \
   m3     m4
      ƨ��
*/
int16_t motor[MOTORSNUM];

_mc_st mc;


void power_distribution(uint8_t dT_ms)
{
  //���У׼ģʽ,��·PWM�����Ϊ����ֵ
  if(escCalibrationMode) {
    //ң����������Ӧ
    uint16_t out = CH_N[CH_THR] + 500 ;
    out = LIMIT(out,0,999);

    //���䶯��
    for(uint8_t i =0; i<4; i++)
      Drv_MotorPWMSet(i,out);

    return;
  }

  //����״̬һ��ͣת���
  if(!flag.unlock_sta) {
    //���ͣת
    for(uint8_t i =0; i<4; i++)
      Drv_MotorPWMSet(i,0);

    flag.motor_preparation = 0;

    return;
  }

  uint16_t idleOut = 10*LIMIT(Ano_Parame.set.idle_speed_pwm,0,30);

  //��˳�����������������
  if(!flag.motor_preparation) {
    static uint16_t timerCount = 0;
    timerCount += dT_ms;
    static uint16_t initialValue = 10* 8;
    uint16_t step = 10* 1;

    //��˳������������
    if(timerCount<400) {
      motor[m1] = initialValue;
    } else if(timerCount<800) {
      motor[m2] = initialValue;
    } else if(timerCount<1200) {
      motor[m3] = initialValue;
    } else if(timerCount<1600) {
      motor[m4] = initialValue;
    }

    //�ٶȽ������趨ֵ
    if(timerCount>1600) {
      static uint16_t lastCount = 1600;
      if(timerCount - lastCount > 100) {
        if(idleOut > initialValue)
          initialValue += step;
        else if(idleOut < initialValue)
          initialValue -= step;

        for(uint8_t i=0; i<MOTORSNUM; i++)
          motor[i] = initialValue;

        lastCount = timerCount;

        if(idleOut == initialValue) {
          flag.motor_preparation = 1;
          timerCount = 0;
          lastCount = 1600;
          initialValue = 10*8;
        }
      }
    }
  }

  //����״̬
  if(flag.taking_off && flag.motor_preparation) {
    motor[m1] = mc.ct_val_thr  +mc.ct_val_yaw -mc.ct_val_rol +mc.ct_val_pit;
    motor[m2] = mc.ct_val_thr  -mc.ct_val_yaw +mc.ct_val_rol +mc.ct_val_pit;
    motor[m3] = mc.ct_val_thr  +mc.ct_val_yaw +mc.ct_val_rol -mc.ct_val_pit;
    motor[m4] = mc.ct_val_thr  -mc.ct_val_yaw -mc.ct_val_rol -mc.ct_val_pit;

    //�޷�
    for(uint8_t i=0; i<MOTORSNUM; i++)
      motor[i] = LIMIT(motor[i],0,999);
  }

  //���䶯��
  for(uint8_t i=0; i<MOTORSNUM; i++)
    Drv_MotorPWMSet(i,motor[i]);
}



