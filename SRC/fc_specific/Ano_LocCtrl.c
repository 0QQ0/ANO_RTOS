#include "Ano_LocCtrl.h"
#include "Ano_Imu.h"
#include "Ano_FlightCtrl.h"
#include "Ano_OF.h"
#include "Ano_OF_DecoFusion.h"
#include "Ano_Parameter.h"


//λ���ٶȻ����Ʋ���
_PID_arg_st loc_arg_1[2] ;

//λ���ٶȻ���������
_PID_val_st loc_val_1[2] ;

//λ���ٶȻ��������Ʋ���
_PID_arg_st loc_arg_1_fix[2] ;

//λ���ٶȻ�������������
_PID_val_st loc_val_1_fix[2] ;

/*�ǶȻ�PID������ʼ��*/
void Loc_1level_PID_Init()
{
  //normal
  loc_arg_1[X].kp = Ano_Parame.set.pid_loc_1level[KP];
  loc_arg_1[X].ki = 0.0f  ;
  loc_arg_1[X].kd_ex = 0.00f ;
  loc_arg_1[X].kd_fb = Ano_Parame.set.pid_loc_1level[KD];
  loc_arg_1[X].k_ff =  0.05f;

  loc_arg_1[Y] = loc_arg_1[X];
  //fix
  loc_arg_1_fix[X].kp = 0.0f  ;
  loc_arg_1_fix[X].ki = Ano_Parame.set.pid_loc_1level[KI] ;
  loc_arg_1_fix[X].kd_ex = 0.00f;
  loc_arg_1_fix[X].kd_fb = 0.00f;
  loc_arg_1_fix[X].k_ff = 0.0f;

  loc_arg_1_fix[Y] = loc_arg_1_fix[X];
}

//ˮƽλ��
_loc_ctrl_st loc_ctrl_1;

/*λ���ٶȻ�*/
void Loc_1level_Ctrl(u16 dT_ms)
{
  //���������ٶȷ���ֵ
  static float fb_speed_fix[2];

  //���ٶȼ��ٶȷ���
  static float vel_fb_d_lpf[2];

  //�����ٶȸ�ֵ
  loc_ctrl_1.exp[X] = fs.speed_set_h[X];
  loc_ctrl_1.exp[Y] = fs.speed_set_h[Y];

  //���ٶȼ��ٶȷ�����ͨ�˲�
  LPF_1_(5.0f,dT_ms*1e-3f,imu_data.h_acc[X],vel_fb_d_lpf[X]);
  LPF_1_(5.0f,dT_ms*1e-3f,imu_data.h_acc[Y],vel_fb_d_lpf[Y]);

  //��������
  if(switchs.of_flow_on) {

    if(sens_hd_check.of_ok) {
      loc_ctrl_1.fb[X] = OF_DX2 + 0.03f *vel_fb_d_lpf[X];
      loc_ctrl_1.fb[Y] = OF_DY2 + 0.03f *vel_fb_d_lpf[Y];

      fb_speed_fix[X] = OF_DX2FIX;
      fb_speed_fix[Y] = OF_DY2FIX;
    } else {
      loc_ctrl_1.fb[X] = of_rdf.gnd_vel_est_h[X] + 0.03f *vel_fb_d_lpf[X];
      loc_ctrl_1.fb[Y] = of_rdf.gnd_vel_est_h[Y] + 0.03f *vel_fb_d_lpf[Y];

      fb_speed_fix[X] = of_rdf.gnd_vel_est_h[X];
      fb_speed_fix[Y] = of_rdf.gnd_vel_est_h[Y];
    }

    for(u8 i =0; i<2; i++) {
      PID_calculate( dT_ms*1e-3f,            //���ڣ���λ���룩
                     loc_ctrl_1.exp[i],				//ǰ��ֵ
                     loc_ctrl_1.exp[i],				//����ֵ���趨ֵ��
                     loc_ctrl_1.fb[i],			//����ֵ����
                     &loc_arg_1[i], //PID�����ṹ��
                     &loc_val_1[i],	//PID���ݽṹ��
                     50,//��������޷�
                     10 *flag.taking_off			//integration limit�������޷�
                   )	;

      //fix
      PID_calculate( dT_ms*1e-3f,            //���ڣ���λ���룩
                     loc_ctrl_1.exp[i],				//ǰ��ֵ
                     loc_ctrl_1.exp[i],				//����ֵ���趨ֵ��
                     fb_speed_fix[i],			//����ֵ����
                     &loc_arg_1_fix[i], //PID�����ṹ��
                     &loc_val_1_fix[i],	//PID���ݽṹ��
                     50,//��������޷�
                     10 *flag.taking_off			//integration limit�������޷�
                   )	;

      loc_ctrl_1.out[i] = loc_val_1[i].out + loc_val_1_fix[i].out;	//(PD)+(I)
    }
  } else {
    loc_ctrl_1.out[X] = (float)MAX_ANGLE/MAX_SPEED *fs.speed_set_h[X] ;
    loc_ctrl_1.out[Y] = (float)MAX_ANGLE/MAX_SPEED *fs.speed_set_h[Y] ;
  }
}

_loc_ctrl_st loc_ctrl_2;

