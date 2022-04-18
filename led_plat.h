
#ifndef _LED_PLAT_H_
#define _LED_PLAT_H_


// 自定义数据类型
struct led_platdata_t{
	int reg_confg_clear;
	int reg_confg_data;
	int reg_data;
	int shift;	
};




#endif




