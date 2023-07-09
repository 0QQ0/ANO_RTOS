#include "Ano_FlightDataCal.h"
#include "Ano_Imu.h"
#include "Drv_icm20602.h"
#include "Ano_MagProcess.h"
#include "Drv_spl06.h"
#include "Drv_ak8975.h"
#include "Ano_MotionCal.h"
#include "Ano_Sensor_Basic.h"
#include "Ano_FlightCtrl.h"
#include "Drv_led.h"
#include "Ano_OF.h"
#include "Drv_Laser.h"

#include "FreeRTOS.h"
#include "task.h"


void Aux_read(void *pvParameters)
{
  TickType_t xLastWakeTime;         //���ھ�׼��ʱ�ı���

  while (1) {
    xLastWakeTime = xTaskGetTickCount(); //��ȡ��ǰTick����,�Ը�����ʱ������ֵ

    /*��ȡ�������̴���������*/
    Drv_AK8975_Read();
    /*��ȡ��ѹ������*/
//    baro_height = (s32)Drv_Spl0601_Read();

    vTaskDelayUntil(&xLastWakeTime,configTICK_RATE_HZ/50);
  }
}



extern s32 sensor_val_ref[];

static u8 reset_imu_f;
void imu_update(u8 dT_ms)
{
  //���׼�����У���λ������λ��Ǻʹ����Ƹ�λ��� 
  if(flag.unlock_sta ) {
    imu_state.G_reset = imu_state.M_reset = 0;
    reset_imu_f = 0;
  } else { 

    if(reset_imu_f==0 ) {  
      imu_state.G_reset = 1;//�Զ���λ
      sensor.gyr_CALIBRATE = 2;//У׼�����ǣ�������
      reset_imu_f = 1;     //�Ѿ���λ��λ���
    }

  }

  /*�������������ں�����kpϵ��*/
  imu_state.gkp = 0.2f;//0.4f;
  /*�������������ں�����kiϵ��*/
  imu_state.gki = 0.01f;
  /*�������̻����ں�����kiϵ��*/
  //imu_state.mkp = 0.1f;
  imu_state.mkp = 0;

  /*����������ʹ��ѡ��*/
  imu_state.M_fix_en = sens_hd_check.mag_ok;


  /*��̬���㣬���£��ں�*/
  IMU_update(dT_ms *1e-3f, &imu_state,sensor.Gyro_rad, sensor.Acc_cmss, mag.val,&imu_data);

}

static s16 mag_val[3];
void Mag_Update_Task(u8 dT_ms)
{

  Mag_Get(mag_val);

  Mag_Data_Deal_Task(dT_ms,mag_val,imu_data.z_vec[Z],sensor.Gyro_deg[X],sensor.Gyro_deg[Z]);

}


 
float wcz_acc_use;

void wcz_acc_update(void)//��С����
{
  wcz_acc_use += 0.03f *(imu_data.w_acc[Z] - wcz_acc_use);
}



s32 baro_height;  
s16 ref_tof_height;  

void WCZ_Fus_Task(u8 dT_ms)
{   
	//TOF����OFӲ������
  if((sens_hd_check.of_df_ok || sens_hd_check.of_ok)) { 
		//ʹ�ù����ļ����� 
		if(switchs.of_tof_on)  
			ref_tof_height = jsdata.of_alt ; 
		//ʹ�ü�����ģ�� 	
    else if(switchs.tof_on)  
      ref_tof_height = -1;
		else
			ref_tof_height = -1;
   
		
		//����z����߶���Ϣ�ں�
		WCZ_Data_Calc(dT_ms,(s32)wcz_acc_use,(s32)(ref_tof_height));
  }  


}


