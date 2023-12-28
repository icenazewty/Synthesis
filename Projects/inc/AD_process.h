#ifndef __AD_process_H
#define __AD_process_H
#include "gd32e50x.h"
															 
//---------- ADC ------------------------------------------------------------


#define D_ADC_V_BAT				  						GPIO_PIN_0	//ADC_Channel_0 	PA0
#define D_ADC_V_P12											GPIO_PIN_1	//ADC_Channel_1 	PA1	-->w5500 CS2
#define D_ADC_V_BAT_PORT								GPIOA				//
#define D_ADC_V_P12_PORT								GPIOA				//										-->w5500 CS2

#define D_ADC_T_RTS											GPIO_PIN_4	//ADC_Channel_14	PC4	-->w5500 rst2
#define D_ADC_T_AMB											GPIO_PIN_0	//ADC_Channel_10	PC0 -->w5500 int2
#define D_ADC_T_RTS_PORT								GPIOC				//PC4 -->w5500 rst2
#define D_ADC_T_AMB_PORT								GPIOC				//PC0 -->w5500 int2

void ADC_GPIO_Configuration(void);
void ADC_Inititile(void);
void ADC_Cycle_Detect_Process(void);
void Ad_software_trigger(void);
void V_Calc(void);
void T_Calc(void);

#endif

