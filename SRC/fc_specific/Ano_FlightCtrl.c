#include "Ano_FlightCtrl.h"
#include "Ano_Imu.h"
#include "Drv_icm20602.h"
#include "Ano_MagProcess.h"
#include "Drv_spl06.h"
#include "Ano_MotionCal.h"
#include "Ano_AttCtrl.h"
#include "Ano_LocCtrl.h"
#include "Ano_AltCtrl.h"
#include "Ano_MotorCtrl.h"
#include "Drv_led.h"
#include "rc_update.h"
#include "Drv_laser.h"
#include "Ano_OF.h"
#include "Ano_OF_DecoFusion.h"
#include "Ano_FlyCtrl.h"
#include "Ano_Sensor_Basic.h"
#include "Ano_DT.h"
#include "Ano_LED.h"
#include "Ano_ProgramCtrl_User.h"


/*============================================================================
更新：
201908012059-Jyoun：修正因高度失效判定光流失效的条件bug，以更好兼容超声波。



===========================================================================*/

/*PID参数初始化*/
void All_PID_Init(void)
{
  /*姿态控制，角速度PID初始化*/
  Att_1level_PID_Init();

  /*姿态控制，角度PID初始化*/
  Att_2level_PID_Init();

  /*高度控制，高度速度PID初始化*/
  Alt_1level_PID_Init();

  /*高度控制，高度PID初始化*/
  Alt_2level_PID_Init();

  /*位置速度控制PID初始化*/
  Loc_1level_PID_Init();
}

static u16 one_key_taof_start;
/*一键起飞任务（主要功能为延迟）*/
void one_key_take_off_task(u16 dt_ms)
{
  if(one_key_taof_start != 0) {
    one_key_taof_start += dt_ms;


    if(one_key_taof_start > 1400 && flag.motor_preparation == 1) {
      one_key_taof_start = 0;
      if(flag.auto_take_off_land == AUTO_TAKE_OFF_NULL) {
        flag.auto_take_off_land = AUTO_TAKE_OFF;
        //解锁、起飞

        flag.taking_off = 1;
      }

    }
  }
  //reset
  if(flag.unlock_sta == 0) {
    one_key_taof_start = 0;
  }

}
/*一键起飞*/
void one_key_take_off()
{
  if(flag.unlock_err == 0) {
    if(flag.auto_take_off_land == AUTO_TAKE_OFF_NULL && one_key_taof_start == 0) {
      one_key_taof_start = 1;
      flag.unlock_cmd = 1;
    }
  }
}
/*一键降落*/
void one_key_land()
{
  flag.auto_take_off_land = AUTO_LAND;
}

//////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////
_flight_state_st fs;

s16 flying_cnt,landing_cnt;

extern s32 ref_height_get;

float stop_baro_hpf;

/*降落检测*/

static s16 ld_delay_cnt ;
void land_discriminat(s16 dT_ms)
{
  /*油门归一值小于0.1  或者启动自动降落*/
  if((fs.speed_set_h_norm[Z] < 0.1f) || flag.auto_take_off_land == AUTO_LAND) {
    if(ld_delay_cnt>0) {
      ld_delay_cnt -= dT_ms;
    }
  } else {
    ld_delay_cnt = 200;
  }

  /*意义是：如果向上推了油门，就需要等垂直方向加速度小于200cm/s2 保持200ms才开始检测*/
  if(ld_delay_cnt <= 0 && (flag.thr_low || flag.auto_take_off_land == AUTO_LAND) ) {
    /*油门最终输出量小于250持续1秒，认为着陆，然后上锁*/
    if(mc.ct_val_thr<250 && flag.unlock_sta == 1) { //还应当 与上速度条件，速度小于正20厘米每秒。
      if(landing_cnt<1500) {
        landing_cnt += dT_ms;
      } else {

        flying_cnt = 0;
        flag.taking_off = 0;
        landing_cnt =0;
        flag.unlock_cmd =0;

				debugOutput("Landing lock");
        flag.flying = 0;

      }
    } else {
      landing_cnt = 0;
    }


  } else {
    landing_cnt  = 0;
  }

}


/*飞行状态任务*/
void Flight_State_Task(u8 dT_ms,const s16 *CH_N)
{ 
  static float max_speed_lim,vel_z_tmp[2];
  /*设置油门摇杆量*/ 
  fs.speed_set_h_norm[Z] = CH_N[CH_THR] * 0.0023f;
  fs.speed_set_h_norm_lpf[Z] += 0.5f *(fs.speed_set_h_norm[Z] - fs.speed_set_h_norm_lpf[Z]);

  /*推油门起飞*/
  if(flag.unlock_sta) {
    if(fs.speed_set_h_norm[Z]>0.01f && flag.motor_preparation == 1) { // 0-1
      flag.taking_off = 1;
    }
  }
  //
  fc_stv.vel_limit_z_p = MAX_Z_SPEED_UP;
  fc_stv.vel_limit_z_n = -MAX_Z_SPEED_DW;
  //
  if( flag.taking_off ) {

    if(flying_cnt<1000) { //800ms
      flying_cnt += dT_ms;
    } else {
      /*起飞后1秒，认为已经在飞行*/
      flag.flying = 1;
    }

    if(fs.speed_set_h_norm[Z]>0) {
      /*设置上升速度*/
      vel_z_tmp[0] = (fs.speed_set_h_norm_lpf[Z] *MAX_Z_SPEED_UP);
    } else {
      /*设置下降速度*/
      vel_z_tmp[0] = (fs.speed_set_h_norm_lpf[Z] *MAX_Z_SPEED_DW);
    }

    //飞控系统Z速度目标量综合设定
    vel_z_tmp[1] = vel_z_tmp[0] + pc_user.vel_cmps_set_z;
    //
    vel_z_tmp[1] = LIMIT(vel_z_tmp[1],fc_stv.vel_limit_z_n,fc_stv.vel_limit_z_p);
    //
    fs.speed_set_h[Z] += LIMIT((vel_z_tmp[1] - fs.speed_set_h[Z]),-0.8f,0.8f);//限制增量幅度
  } else {
    fs.speed_set_h[Z] = 0 ;
  }
  float speed_set_tmp[2];
  /*速度设定量，正负参考ANO坐标参考方向*/
  fs.speed_set_h_norm[X] = (my_deadzone(+CH_N[CH_PIT],0,50) *0.0022f);
  fs.speed_set_h_norm[Y] = (my_deadzone(-CH_N[CH_ROL],0,50) *0.0022f);

  LPF_1_(3.0f,dT_ms*1e-3f,fs.speed_set_h_norm[X],fs.speed_set_h_norm_lpf[X]);
  LPF_1_(3.0f,dT_ms*1e-3f,fs.speed_set_h_norm[Y],fs.speed_set_h_norm_lpf[Y]);

  max_speed_lim = MAX_SPEED;

  if(switchs.of_flow_on && !switchs.gps_on ) {
    max_speed_lim = 1.5f *wcz_hei_fus.out;
    max_speed_lim = LIMIT(max_speed_lim,50,150);
  }

  fc_stv.vel_limit_xy = max_speed_lim;

  //飞控系统XY速度目标量综合设定
  speed_set_tmp[X] = fc_stv.vel_limit_xy *fs.speed_set_h_norm_lpf[X] + pc_user.vel_cmps_set_h[X];
  speed_set_tmp[Y] = fc_stv.vel_limit_xy *fs.speed_set_h_norm_lpf[Y] + pc_user.vel_cmps_set_h[Y];

  length_limit(&speed_set_tmp[X],&speed_set_tmp[Y],fc_stv.vel_limit_xy,fs.speed_set_h_cms);

  fs.speed_set_h[X] = fs.speed_set_h_cms[X];
  fs.speed_set_h[Y] = fs.speed_set_h_cms[Y];

  /*调用检测着陆的函数*/
  land_discriminat(dT_ms); 
  
  /*倾斜过大上锁*/ 
	if(imu_data.z_vec[Z]<0.25f && flag.unlock_cmd != 0) {  
		//
		if(mag.mag_CALIBRATE==0) {
			imu_state.G_reset = 1;
		}
		flag.unlock_cmd = 0;
		
		debugOutput("Rollover locks");
	} 
  
  //////////////////////////////////////////////////////////
  /*校准中，复位重力方向*/
  if(sensor.gyr_CALIBRATE != 0 || sensor.acc_CALIBRATE != 0 ||sensor.acc_z_auto_CALIBRATE) {
    imu_state.G_reset = 1;
  }

  /*复位重力方向时，认为传感器失效*/
  if(imu_state.G_reset == 1) { 
    flag.sensor_imu_ok = 0;
    LED_STA.rst_imu = 1;
    WCZ_Data_Reset(); //复位高度数据融合 
  } else if(imu_state.G_reset == 0) {
    if(flag.sensor_imu_ok == 0) {
      flag.sensor_imu_ok = 1;
      LED_STA.rst_imu = 0;
      debugOutput("IMU OK");
    }
  }

  /*飞行状态复位*/
  if(flag.unlock_sta == 0) {
    flag.flying = 0;
    landing_cnt = 0;
    flag.taking_off = 0;
    flying_cnt = 0;
 
    flag.rc_loss_back_home = 0;
 
  }


}

//
static u8 of_quality_ok;
static u16 of_quality_delay;
//
static u8 of_alt_ok;
static s16 of_alt_delay;
//
static u8 of_tof_on_tmp;
//

_judge_sync_data_st jsdata;
void Swtich_State_Task(u8 dT_ms)
{
  switchs.baro_on = 1;

  //光流模块
  if(sens_hd_check.of_ok || sens_hd_check.of_df_ok) {
    //
    if(sens_hd_check.of_ok) {
      jsdata.of_qua = OF_QUALITY;
      jsdata.of_alt = (u16)OF_ALT;
    } else if(sens_hd_check.of_df_ok) {
      jsdata.of_qua = of_rdf.quality;
      jsdata.of_alt = Laser_height_cm;
    }
		
		//光流质量大于50，认为光流可用，判定可用延迟时间为500ms
    if(jsdata.of_qua>50 ) {
      if(of_quality_delay<500) {
        of_quality_delay += dT_ms;
      } else {
        of_quality_ok = 1;
      }
    } else {
      of_quality_delay =0;
      of_quality_ok = 0;
    }

    //光流高度600cm内有效
    if(jsdata.of_alt<600) {
      //
      of_tof_on_tmp = 1;
      jsdata.valid_of_alt_cm = jsdata.of_alt;
      //延时1.5秒判断激光高度是否有效
      if(of_alt_delay<1500) {
        of_alt_delay += dT_ms;
      } else {
        //判定高度有效
        of_alt_ok = 1;
      }
    } else {
      //
      of_tof_on_tmp = 0;
      //
      if(of_alt_delay>0) {
        of_alt_delay -= dT_ms;
      } else {
        //判定高度无效
        of_alt_ok = 0;
      }
    }


    if(flag.flight_mode == LOC_HOLD) {
      if(of_alt_ok && of_quality_ok) {
        switchs.of_flow_on = 1;
      } else {
        switchs.of_flow_on = 0;
      }

    } else {
      of_tof_on_tmp = 0;
      switchs.of_flow_on = 0;
    }
    //
    switchs.of_tof_on = of_tof_on_tmp;
  } else {
    switchs.of_flow_on = switchs.of_tof_on = 0;
  }

  //激光模块
  if(sens_hd_check.tof_ok) {
    switchs.tof_on = 1;
  } else {
    switchs.tof_on = 0;
  }


}
