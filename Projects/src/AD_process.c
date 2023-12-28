#include "AD_process.h"
#include "GlobalVar.h"
//#include "stm32f10x_conf.h"
//#include "hardware.h"
#include "string.h"
#include "systick.h"

void ADC_GPIO_Configuration(void)
{
		rcu_periph_clock_enable(RCU_GPIOA);
	//	rcu_periph_clock_enable(RCU_GPIOC);
		gpio_init(D_ADC_V_BAT_PORT, GPIO_MODE_AIN, GPIO_OSPEED_MAX, D_ADC_V_BAT );		//	gpio_init(D_ADC_V_BAT_PORT, GPIO_MODE_AIN, GPIO_OSPEED_MAX, D_ADC_V_BAT | D_ADC_V_P12);
	//	gpio_init(D_ADC_T_RTS_PORT, GPIO_MODE_AIN, GPIO_OSPEED_MAX, D_ADC_T_AMB | D_ADC_T_RTS);	
} 

void ADC_Inititile(void)
{
		ADC_GPIO_Configuration();
		
	 /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC0);
    /* configure ADC clock */
    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV12);
	
		/* reset ADC */
    adc_deinit(ADC0);
    /* enable ADC SCAN function */
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    /* disable ADC continuous function */
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);
    /* configure ADC data alignment */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    /* configure ADC mode */
    adc_mode_config(ADC_MODE_FREE);
    
    /* configure ADC channel length */
    adc_channel_length_config(ADC0, ADC_INSERTED_CHANNEL, 1);		//4->1
    /* configure ADC temperature sensor channel */
    //adc_inserted_channel_config(ADC0, 0, ADC_CHANNEL_16, ADC_SAMPLETIME_239POINT5);
		adc_inserted_channel_config(ADC0, 0, ADC_CHANNEL_0, ADC_SAMPLETIME_239POINT5);
		#if 0
    /* configure ADC internal reference voltage channel */
    //adc_inserted_channel_config(ADC0, 1, ADC_CHANNEL_17, ADC_SAMPLETIME_239POINT5);
		adc_inserted_channel_config(ADC0, 1, ADC_CHANNEL_1, ADC_SAMPLETIME_239POINT5);
    /* configure ADC trigger */
		adc_inserted_channel_config(ADC0, 2, ADC_CHANNEL_14 , ADC_SAMPLETIME_239POINT5);
		adc_inserted_channel_config(ADC0, 3, ADC_CHANNEL_10 , ADC_SAMPLETIME_239POINT5);
		#endif
    adc_external_trigger_source_config(ADC0, ADC_INSERTED_CHANNEL, ADC0_1_2_EXTTRIG_INSERTED_NONE);
    /* enable ADC external trigger */
    adc_external_trigger_config(ADC0, ADC_INSERTED_CHANNEL, ENABLE);
    /* enable ADC temperature and Vrefint */
    adc_tempsensor_vrefint_enable();

    /* enable ADC interface */
    adc_enable(ADC0);
   // delay_1ms(1);
		
		//adc_calibration_number(ADC0, ADC_CALIBRATION_NUM16);
		
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
		Ad_software_trigger();
} 



void V_Calc(void)
{
//---------- BAT --------------------------------------------------------------------------------------------
	VOLT.fBAT=(ADC_Value.BAT*3.3/4096.0)*(100.0+2.0)/2.0;
	Adc_Data[0] = (short int)(VOLT.fBAT*100);
	
//---------- P12	-------------------------------------------------------------------------------------------
	//VOLT.fP12=(ADC_Value.P12*3.3/4096.0)*(100.0+10.0)/10.0;
	//Adc_Data[1] = (short int)(VOLT.fP12*100);
	
}

//3978,3970,3962,3953,3944,3934,3924,3913,3902,3890,3878,	//-40 - -30
const uint16_t TEMPERATURE_CHANGE_TB[]={3878,3864,3851,3836,3821,3805,3789,3771,3753,3734,3715,			//-30 - -20
															3694,3673,3651,3628,3604,3579,3553,3527,3499,3471,			//-19 - -10
															3442,3411,3380,3348,3315,3282,3247,3211,3175,3138,			//-9 - 0
															3100,3061,3021,2981,2941,2899,2857,2814,2771,2728,			//1-10
															2684,2639,2595,2549,2504,2458,2412,2367,2321,2276,			//11-20
															2230,2184,2138,2093,2048,2003,1959,1914,1871,1827,			//21-30
															1784,1742,1701,1659,1618,1577,1538,1499,1460,1422,			//31-40
															1385,1349,1314,1279,1245,1211,1179,1147,1115,1085,			//41-50
															1055,1025,997,969,942,915,889,864,840,816,		//51-60
															793,770,748,727,706,686,666,647,629,611,			//61-70
															593,576,560,544,528,513,498,484,470,457,			//71-80
															444,431,419,407,396,385,374,364,354,344, 			//81-90
															334,325,316,307,299,291,283,275,268,261, 			//91-100
															254,247,240,234,228,222,216,210,205,199, 			//101-110
															194,189,184,180,175,171,166,162,158,154  			//111-120
															};
//2014-06-20
char Temperature_adc_cal(unsigned short int adc_temp1)
{
	char						temp1;
	unsigned 	char 	i;
	
	if(adc_temp1>3900){	//²âÎÂµç×èÎ´½Ó
		temp1=0x80;
	}
	else if(adc_temp1<100){	//²âÎÂµç×è¶ÌÂ·
		temp1=0x7F;
	}
	else if(adc_temp1>=TEMPERATURE_CHANGE_TB[0]){	//ÎÂ¶È<-30¡æ
		temp1=0xE2;	//-30
	}
	else if(adc_temp1<=TEMPERATURE_CHANGE_TB[150]){//ÎÂ¶È>120¡æ
		temp1=0x78;	//120
	}else{
		temp1=0xE2;
		for(i=1;i<151;i++){
			temp1+=1;
			if(adc_temp1>TEMPERATURE_CHANGE_TB[i]){
				i=151;
			}
		}
	}
	return(temp1);
}
	 
void T_Calc(void)
{
//	s8 sc1=0x7F,sc2=0x80,sc3=0x81;
	Temp.scRTS=Temperature_adc_cal(ADC_Value.Trts);		//RTSÎÂ¶È
	Temp.scAmb=Temperature_adc_cal(ADC_Value.Tamb);		//»·¾³ÎÂ¶È
	Adc_Data[2] = Temp.scAmb;
	Adc_Data[3] = Temp.scRTS;
}

void Ad_software_trigger(void)
{	
	adc_software_trigger_enable(ADC0, ADC_INSERTED_CHANNEL);
}

void ADC_Cycle_Detect_Process(void)
{	
//---------- BATÂµÄDC ---------------------------------------------------------------------------------------
	ADC_Value.BAT=ADC_IDATA0(ADC0);					//ADC_IN0		è“„ç”µæ± ç”µå‹					100K	2K
//---------- 12VÂµÄDC ---------------------------------------------------------------------------------------
	#if 0
	ADC_Value.P12=ADC_IDATA1(ADC0);					//ADC_IN1			android dc 				12Vç”µæºç”µå‹é‡‡é›†	100K  10K
//---------- RSTÂµÄDC ---------------------------------------------------------------------------------------	

	ADC_Value.Trts=ADC_IDATA2(ADC0);				//ADC_IN14		rts				10K  10K  10K		rts	
//---------- AmbÂµÄDC ---------------------------------------------------------------------------------------	
	ADC_Value.Tamb=ADC_IDATA3(ADC0);				//ADC_IN15		ADC_AmbientTemp   ç¯å¢ƒæ¸©åº¦ntcç”µé˜»		10K  10K
	#endif
	V_Calc();
	//T_Calc();
}
