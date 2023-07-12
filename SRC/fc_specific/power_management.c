#include "power_management.h"


float singleBatteryVoltage = 0;

void battery_update(void)
{
	static float voltage_f = 0;  
	
  //����ADC����
  Drv_Adc0Trigger();
	 
  //��ͨ�˲� 
  S_LPF_1(0.04f,Voltage,voltage_f); 
	
	//������ڵ�ص�ѹ
  singleBatteryVoltage = voltage_f / Ano_Parame.set.bat_cell;

 

  if(singleBatteryVoltage<2 ) {
    flag.power_state = 4;  //USB����
    LED_STA.lowVt = 1;
  }else if(singleBatteryVoltage<Ano_Parame.set.lowest_power_voltage ) {
    flag.power_state = 3;//����ֹ����
    LED_STA.lowVt = 1;
  }else if(singleBatteryVoltage<Ano_Parame.set.warn_power_voltage) { 
		flag.power_state = 2;
    LED_STA.lowVt = 1;
  }else if(singleBatteryVoltage<Ano_Parame.set.return_home_power_voltage) {  
		flag.power_state = 1;
  }else{
		LED_STA.lowVt = 0;
		flag.power_state = 1;
	} 
}

//���ص���ܵ�ѹ
float get_battery_voltage(void){
	
	return singleBatteryVoltage*Ano_Parame.set.bat_cell ;
}



