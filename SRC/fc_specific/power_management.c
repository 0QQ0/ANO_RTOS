#include "power_management.h"


float singleBatteryVoltage = 0;

void batteryUpdate()
{
	static float voltage_f = 0;  
	
  //����ADC����
  Drv_Adc0Trigger();
	 
  //��ͨ�˲� 
  S_LPF_1(0.05f,Voltage,voltage_f); 
	
	//������ڵ�ص�ѹ
  singleBatteryVoltage = voltage_f / Ano_Parame.set.bat_cell;

 

  if(singleBatteryVoltage<Ano_Parame.set.lowest_power_voltage ) {
    flag.power_state = 3;//����ֹ����
    LED_STA.lowVt = 1;
  }else if(singleBatteryVoltage<Ano_Parame.set.warn_power_voltage) {
    LED_STA.lowVt = 1;
  }else if(singleBatteryVoltage<Ano_Parame.set.return_home_power_voltage) { 
		
  }else{
		LED_STA.lowVt = 0;
		flag.power_state = 1;
	}
}

//���ص���ܵ�ѹ
float getBatteryVoltage(void){
	
	return singleBatteryVoltage*Ano_Parame.set.bat_cell ;
}



