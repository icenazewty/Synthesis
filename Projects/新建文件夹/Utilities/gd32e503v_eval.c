/*!
    \file    gd32e503v_eval.c
    \brief   firmware functions to manage leds, keys, COM ports

    \version 2020-09-04, V1.0.0, demo for GD32E50x
    \version 2021-03-31, V1.1.0, demo for GD32E50x
*/

/*
    Copyright (c) 2021, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32e503v_eval.h"
#include "GlobalVar.h"
#include "systick.h"
#include "gprs.h"
#include "ch432t.h"
#include <stdio.h>
#include <string.h>

/* private variables */
static uint32_t GPIO_PORT[LEDn]           = {LED1_GPIO_PORT, 	LED2_GPIO_PORT};                                     
static uint32_t GPIO_PIN[LEDn]            = {LED1_PIN, 				LED2_PIN};
static rcu_periph_enum GPIO_CLK[LEDn]     = {LED1_GPIO_CLK, 	LED2_GPIO_CLK};


static rcu_periph_enum COM_CLK[COMn]      = {EVAL_COM0_CLK,				EVAL_COM1_CLK,				EVAL_COM2_CLK,				EVAL_COM3_CLK,				EVAL_COM4_CLK,			EVAL_COM5_CLK};
			 uint32_t COM_EVAL[COMn] 						= {EVAL_COM0,						EVAL_COM1,						EVAL_COM2,						EVAL_COM3,						EVAL_COM4,					EVAL_COM5};
static uint32_t COM_TX_PIN[COMn]          = {EVAL_COM0_TX_PIN,		EVAL_COM1_TX_PIN,			EVAL_COM2_TX_PIN,			EVAL_COM3_TX_PIN,			EVAL_COM4_TX_PIN,		EVAL_COM5_TX_PIN};
static uint32_t COM_RX_PIN[COMn]          = {EVAL_COM0_RX_PIN,		EVAL_COM1_RX_PIN,			EVAL_COM2_RX_PIN,			EVAL_COM3_RX_PIN,			EVAL_COM4_RX_PIN,		EVAL_COM5_RX_PIN};
static uint32_t COM_GPIO_PORT[COMn]       = {EVAL_COM0_GPIO_PORT,	EVAL_COM1_GPIO_PORT,	EVAL_COM2_GPIO_PORT,	EVAL_COM3_GPIO_PORT,	EVAL_COM4_GPIO_PORT,EVAL_COM5_GPIO_PORT};
static rcu_periph_enum COM_GPIO_CLK[COMn] = {EVAL_COM0_GPIO_CLK,	EVAL_COM1_GPIO_CLK,		EVAL_COM2_GPIO_CLK,		EVAL_COM3_GPIO_CLK,		EVAL_COM4_GPIO_CLK,	EVAL_COM5_GPIO_CLK};
static uint8_t UART_IRQn[COMn] 						= {EVAL_COM0_IRQn, 			EVAL_COM1_IRQn, 		 	EVAL_COM2_IRQn, 			EVAL_COM3_IRQn,				EVAL_COM4_IRQn,			EVAL_COM5_IRQn};

unsigned short CrcCheck(unsigned char *pData,unsigned short nLen)
{
	unsigned short crc=0xffff;						//ɨփCRC¼Ĵ憷£¬²¢¸øƤ¸³ֵFFFF(hex)
	int i,j;	
	for(i = 0; i < nLen; i++)
	{		
		crc^=(unsigned short)pData[i];			//½«ʽ¾ݵĵڒ»¸ö8-bitז·ûӫ16λCRC¼Ĵ憷µĵ͸λ½øАҬ»򣬲¢°ѽṻ´戫CRC¼Ĵ憷
		for(j = 0; j < 8; j++)
		{
			if(crc&1)
			{
				crc>>=1;												//CRC¼Ĵ憷ϲӒ҆һλ£¬MSB²¹Á㣬҆³ö²¢¼첩LSB
				crc^=0xA001;										//¶Ϯʽ«
			}
			else
			{
				crc>>=1;						//ȧ¹ûLSBΪ0£¬֘¸´µڈý²½£»ȴLSBΪ1£¬CRC¼Ĵ憷ӫ¶Ϯʽ«ϠҬ»򡣍	
			}
		}
	}
	return crc;
}

void Read_SW_Input_State(void)
{
	unsigned char i = 0;
	Sw_Sta[0]=gpio_input_bit_get(D_SWI0_PORT,D_SWI0);
	Sw_Sta[1]=gpio_input_bit_get(D_SWI1_PORT,D_SWI1);
	Sw_Sta[2]=gpio_input_bit_get(D_SWI2_PORT,D_SWI2);	
	Sw_Sta[3]=gpio_input_bit_get(D_SWI3_PORT,D_SWI3);
	Sw_Sta[4]=gpio_input_bit_get(D_SWI4_PORT,D_SWI4);
	Sw_Sta[5]=gpio_input_bit_get(D_SWI5_PORT,D_SWI5);
	Sw_Sta[6]=0;	//gpio_input_bit_get(D_SWI6_PORT,D_SWI6);
	Sw_Sta[7]=0;	//gpio_input_bit_get(D_SWI7_PORT,D_SWI7);	
	for(i=0;i<6;i++)
	{
		if(Sw_Sta[i])
		{
			Sw_Sta[i] = 0;
		}
		else
		{
			Sw_Sta[i] = 1;
		}
	}
}

void Read_DI_Input_State(void)
{
//---------- DI0(L:ӐЧ) ------------------------------------------------------------------------------------
	if(gpio_input_bit_get(D_DI0_PORT,D_DI0)==0)
	{
		DI.ucJudge_Input_State[0]++;
		if(DI.ucJudge_Input_State[0]>=D_JUDGE_DI_INPUT_STATE_MAX_TIMES)
		{
			DI.ucJudge_Input_State[0]=0;
			DI.bInput_State[0]=true;
		}
	}
	else
	{
		DI.ucJudge_Input_State[0]=0;
		DI.bInput_State[0]=false;
	}
//---------- DI1(L:ӐЧ) ------------------------------------------------------------------------------------
	if(gpio_input_bit_get(D_DI1_PORT,D_DI1)==0){
		DI.ucJudge_Input_State[1]++;
		if(DI.ucJudge_Input_State[1]>=D_JUDGE_DI_INPUT_STATE_MAX_TIMES){
			DI.ucJudge_Input_State[1]=0;
			DI.bInput_State[1]=true;
		}
	}else{
		DI.ucJudge_Input_State[1]=0;
		DI.bInput_State[1]=false;
	}
//---------- DI2(L:ӐЧ) ------------------------------------------------------------------------------------
	if(gpio_input_bit_get(D_DI2_PORT,D_DI2)==0){
		DI.ucJudge_Input_State[2]++;
		if(DI.ucJudge_Input_State[2]>=D_JUDGE_DI_INPUT_STATE_MAX_TIMES){
			DI.ucJudge_Input_State[2]=0;
			DI.bInput_State[2]=true;
		}
	}else{
		DI.ucJudge_Input_State[2]=0;
		DI.bInput_State[2]=false;
	}
//---------- DI3(L:ӐЧ) ------------------------------------------------------------------------------------
	if(gpio_input_bit_get(D_DI3_PORT,D_DI3)==0){
		DI.ucJudge_Input_State[3]++;
		if(DI.ucJudge_Input_State[3]>=D_JUDGE_DI_INPUT_STATE_MAX_TIMES){
			DI.ucJudge_Input_State[3]=0;
			DI.bInput_State[3]=true;
		}
	}else{
		DI.ucJudge_Input_State[3]=0;
		DI.bInput_State[3]=false;
	}
//---------- DI4(L:ӐЧ) ------------------------------------------------------------------------------------
	if(gpio_input_bit_get(D_DI4_PORT,D_DI4)==0){
		DI.ucJudge_Input_State[4]++;
		if(DI.ucJudge_Input_State[4]>=D_JUDGE_DI_INPUT_STATE_MAX_TIMES){
			DI.ucJudge_Input_State[4]=0;
			DI.bInput_State[4]=true;
		}
	}else{
		DI.ucJudge_Input_State[4]=0;
		DI.bInput_State[4]=false;
	}
//---------- DI5(L:ӐЧ) ------------------------------------------------------------------------------------
	if(gpio_input_bit_get(D_DI5_PORT,D_DI5)==0){
		DI.ucJudge_Input_State[5]++;
		if(DI.ucJudge_Input_State[5]>=D_JUDGE_DI_INPUT_STATE_MAX_TIMES){
			DI.ucJudge_Input_State[5]=0;
			DI.bInput_State[5]=true;
		}
	}else{
		DI.ucJudge_Input_State[5]=0;
		DI.bInput_State[5]=false;
	}
//---------- DI6(L:ӐЧ) ------------------------------------------------------------------------------------
	if(gpio_input_bit_get(D_DI6_PORT,D_DI6)==0){
		DI.ucJudge_Input_State[6]++;
		if(DI.ucJudge_Input_State[6]>=D_JUDGE_DI_INPUT_STATE_MAX_TIMES){
			DI.ucJudge_Input_State[6]=0;
			DI.bInput_State[6]=true;
		}
	}else{
		DI.ucJudge_Input_State[6]=0;
		DI.bInput_State[6]=false;
	}
//---------- DI7(L:ӐЧ) ------------------------------------------------------------------------------------
	if(gpio_input_bit_get(D_DI7_PORT,D_DI7)==0)
	{
		DI.ucJudge_Input_State[7]++;
		if(DI.ucJudge_Input_State[7]>=D_JUDGE_DI_INPUT_STATE_MAX_TIMES)
		{
			DI.ucJudge_Input_State[7]=0;
			DI.bInput_State[7]=true;
		}
	}
	else
	{
		DI.ucJudge_Input_State[7]=0;
		DI.bInput_State[7]=false;
	}
}

void Led_Ctrl(unsigned char ch, unsigned char sta)
{
	if(ch==0)
	{
		if(sta==true)
		{
			gpio_bit_set(D_LED1_CTRL_PORT,D_LED1_CTRL);
		}
		else
		{
			gpio_bit_reset(D_LED1_CTRL_PORT,D_LED1_CTRL);	
		}
	}
	else if(ch==1)
	{
		if(sta==true)
		{
			gpio_bit_set(D_LED2_CTRL_PORT,D_LED2_CTRL);
		}
		else
		{
			gpio_bit_reset(D_LED2_CTRL_PORT,D_LED2_CTRL);	
		}
	}	
}

void Get_Android_Sta(void)
{
//		Android_Sta = gpio_input_bit_get(D_ANDROID_POWER_PORT,D_ANDROID_POWER_CTRL); 		//低电平打开电源,默认上拉
//		if(Android_Sta)
//			Android_Sta = 0;
//		else
//			Android_Sta = 1;	
	Android_Sta = 1;
}

void Android_Ctrl(unsigned char sta)
{
//	if(sta)
//	{
//		gpio_bit_reset(D_ANDROID_POWER_PORT,D_ANDROID_POWER_CTRL); 
//	}
//	else
//	{
//		gpio_bit_set(D_ANDROID_POWER_PORT,D_ANDROID_POWER_CTRL);
//	}
}

void PowerWifiBle_Ctrl(unsigned char sta)
{
	if(sta)
	{
		gpio_bit_set(D_WIFI_BLE_POWER_PORT,D_WIFI_BLE_POWER_CTRL); 
	}
	else
	{
		gpio_bit_reset(D_WIFI_BLE_POWER_PORT,D_WIFI_BLE_POWER_CTRL);
	}
}

void wifi_reset(void)
{
		if(0==g_configRead.b_wifi_work)
		{
				return;
		}
		PowerWifiBle_Ctrl(false);	
	  wdt();
	  delay_1ms(300);	  wdt();		
		PowerWifiBle_Ctrl(true);			
		wifi_rev_cnt = 0;
		wifi_reset_sta = 0;	
		wifi_reset_tick = g_sysTick;
		wdt();	
		delay_1ms(100);	
}


void Power4G_Ctrl(unsigned char sta)
{
	if(sta)
	{
		gpio_bit_set(D_4G_POWER_PORT,D_4G_POWER); 
	}
	else
	{
		gpio_bit_reset(D_4G_POWER_PORT,D_4G_POWER);
	}
}

void GPRS_Reset(void)
{	
		wdt();
		Power4G_Ctrl(false);		//关机
		delay_1ms(500);					
		Power4G_Ctrl(true);		
		wdt();	
		Gprs_Para_Init();										
}


void Do_Get_Sta(void)
{	
//	unsigned char i = 0;
	Do_Sta[0]=gpio_input_bit_get(D_DO_CTRL0_PORT,D_DO_CTRL0);
	Do_Sta[1]=gpio_input_bit_get(D_DO_CTRL1_PORT,D_DO_CTRL1);
	Do_Sta[2]=gpio_input_bit_get(D_DO_CTRL2_PORT,D_DO_CTRL2);	
	Do_Sta[3]=gpio_input_bit_get(D_DO_CTRL3_PORT,D_DO_CTRL3);
	Do_Sta[4]=gpio_input_bit_get(D_DO_CTRL4_PORT,D_DO_CTRL4);
	Do_Sta[5]=gpio_input_bit_get(D_DO_CTRL5_PORT,D_DO_CTRL5);
	Do_Sta[6]=gpio_input_bit_get(D_DO_CTRL6_PORT,D_DO_CTRL6);
	Do_Sta[7]=gpio_input_bit_get(D_DO_CTRL7_PORT,D_DO_CTRL7);
	
	#if 0
	for(i=0;i<8;i++)
	{
		if(Do_Sta[i])
		{
			Do_Sta[i] = 0;
		}
		else
		{
			Do_Sta[i] = 1;
		}
	}	
	#endif
	
}


void AI_Channel_Select(unsigned char Para_ucCH)
{
	switch(Para_ucCH){
//---------- Channel 1 --------------------------------------------------------------------------------------
		case 0:
			gpio_bit_reset(D_AI_SELECT_A0_PORT,D_AI_SELECT_A0);
			gpio_bit_reset(D_AI_SELECT_A1_PORT,D_AI_SELECT_A1);
			gpio_bit_reset(D_AI_SELECT_A2_PORT,D_AI_SELECT_A2);
		break;
//---------- Channel 2 --------------------------------------------------------------------------------------
		case 1:
			gpio_bit_set(D_AI_SELECT_A0_PORT,D_AI_SELECT_A0);
			gpio_bit_reset(D_AI_SELECT_A1_PORT,D_AI_SELECT_A1);
			gpio_bit_reset(D_AI_SELECT_A2_PORT,D_AI_SELECT_A2);
		break;
//---------- Channel 3 --------------------------------------------------------------------------------------
		case 2:
			gpio_bit_reset(D_AI_SELECT_A0_PORT,D_AI_SELECT_A0);
			gpio_bit_set(D_AI_SELECT_A1_PORT,D_AI_SELECT_A1);
			gpio_bit_reset(D_AI_SELECT_A2_PORT,D_AI_SELECT_A2);
		break;
//---------- Channel 4 --------------------------------------------------------------------------------------
		case 3:
			gpio_bit_set(D_AI_SELECT_A0_PORT,D_AI_SELECT_A0);
			gpio_bit_set(D_AI_SELECT_A1_PORT,D_AI_SELECT_A1);
			gpio_bit_reset(D_AI_SELECT_A2_PORT,D_AI_SELECT_A2);
		break;
//---------- Channel 5 --------------------------------------------------------------------------------------
		case 4:
			gpio_bit_reset(D_AI_SELECT_A0_PORT,D_AI_SELECT_A0);
			gpio_bit_reset(D_AI_SELECT_A1_PORT,D_AI_SELECT_A1);
			gpio_bit_set(D_AI_SELECT_A2_PORT,D_AI_SELECT_A2);
		break;
//---------- Channel 6 --------------------------------------------------------------------------------------
		case 5:
			gpio_bit_set(D_AI_SELECT_A0_PORT,D_AI_SELECT_A0);
			gpio_bit_reset(D_AI_SELECT_A1_PORT,D_AI_SELECT_A1);
			gpio_bit_set(D_AI_SELECT_A2_PORT,D_AI_SELECT_A2);
		break;
//---------- Channel 7 --------------------------------------------------------------------------------------
		case 6:
			gpio_bit_reset(D_AI_SELECT_A0_PORT,D_AI_SELECT_A0);
			gpio_bit_set(D_AI_SELECT_A1_PORT,D_AI_SELECT_A1);
			gpio_bit_set(D_AI_SELECT_A2_PORT,D_AI_SELECT_A2);
		break;
//---------- Channel 8 --------------------------------------------------------------------------------------
		case 7:
			gpio_bit_set(D_AI_SELECT_A0_PORT,D_AI_SELECT_A0);
			gpio_bit_set(D_AI_SELECT_A1_PORT,D_AI_SELECT_A1);
			gpio_bit_set(D_AI_SELECT_A2_PORT,D_AI_SELECT_A2);
		break;
//-----------------------------------------------------------------------------------------------------------
		default:
		break;
	}
}


void DO_Output_Ctrl(unsigned char Para_CH,unsigned char Para_Output_State)
{
	switch(Para_CH)
	{
//---------- DO_0(L:ON£»H:OFF) ------------------------------------------------------------------------------
		case 0:
			if(Para_Output_State==D_OFF)
			{
				gpio_bit_reset(D_DO_CTRL0_PORT,D_DO_CTRL0);
			}
			else if(Para_Output_State==D_ON)
			{
				gpio_bit_set(D_DO_CTRL0_PORT,D_DO_CTRL0);
			}
		break;
//---------- DO_1(L:ON£»H:OFF) ------------------------------------------------------------------------------
		case 1:
			if(Para_Output_State==D_OFF)
			{
				gpio_bit_reset(D_DO_CTRL1_PORT,D_DO_CTRL1);
			}
			else if(Para_Output_State==D_ON)
			{
				gpio_bit_set(D_DO_CTRL1_PORT,D_DO_CTRL1);
			}
		break;
//---------- DO_2(L:ON£»H:OFF) ------------------------------------------------------------------------------
		case 2:
			if(Para_Output_State==D_OFF)
			{
				gpio_bit_reset(D_DO_CTRL2_PORT,D_DO_CTRL2);
			}
			else if(Para_Output_State==D_ON)
			{
				gpio_bit_set(D_DO_CTRL2_PORT,D_DO_CTRL2);
			}
		break;
//---------- DO_3(L:ON£»H:OFF) ------------------------------------------------------------------------------
		case 3:
			if(Para_Output_State==D_OFF)
			{
				gpio_bit_reset(D_DO_CTRL3_PORT,D_DO_CTRL3);
			}
			else if(Para_Output_State==D_ON)
			{
				gpio_bit_set(D_DO_CTRL3_PORT,D_DO_CTRL3);
			}
		break;
//---------- DO_4(L:ON£»H:OFF) ------------------------------------------------------------------------------
		case 4:
			if(Para_Output_State==D_OFF)
			{
				gpio_bit_reset(D_DO_CTRL4_PORT,D_DO_CTRL4);
			}
			else if(Para_Output_State==D_ON)
			{
				gpio_bit_set(D_DO_CTRL4_PORT,D_DO_CTRL4);
			}
		break;
//---------- DO_5(L:ON£»H:OFF) ------------------------------------------------------------------------------
		case 5:
			if(Para_Output_State==D_OFF)
			{
				gpio_bit_reset(D_DO_CTRL5_PORT,D_DO_CTRL5);
			}
			else if(Para_Output_State==D_ON)
			{
				gpio_bit_set(D_DO_CTRL5_PORT,D_DO_CTRL5);
			}
		break;
//---------- DO_6(L:ON£»H:OFF) ------------------------------------------------------------------------------
		case 6:
			if(Para_Output_State==D_OFF)
			{
				gpio_bit_reset(D_DO_CTRL6_PORT,D_DO_CTRL6);
			}
			else if(Para_Output_State==D_ON)
			{
				gpio_bit_set(D_DO_CTRL6_PORT,D_DO_CTRL6);
			}
		break;
//---------- DO_7(L:ON£»H:OFF) ------------------------------------------------------------------------------
		case 7:
			if(Para_Output_State==D_OFF)
			{
				gpio_bit_reset(D_DO_CTRL7_PORT,D_DO_CTRL7);
			}
			else if(Para_Output_State==D_ON)
			{
				gpio_bit_set(D_DO_CTRL7_PORT,D_DO_CTRL7);
			}
		break;
//---------- defaule ----------------------------------------------------------------------------------------		
		default:
		break;
	}
}

void Beep(unsigned char sta)
{
	if(sta==0)
	{
		gpio_bit_reset(D_BEEP_PORT,D_BEEP);					//beep 不响
	}
	else
	{
		gpio_bit_set(D_BEEP_PORT,D_BEEP);						//beep 响
	}
}


void wdt(void)
{
		gpio_bit_set(D_WDI_PORT , D_WDI);
		gpio_bit_reset(D_WDI_PORT,D_WDI);			//低有效
}

/*!
    \brief      configure led GPIO
    \param[in]  lednum: specify the led to be configured
      \arg        LED1
      \arg        LED2
      \arg        LED3
      \arg        LED4
    \param[out] none
    \retval     none
*/
void  gd_eval_led_init (led_typedef_enum lednum)
{
    /* enable the led clock */
    rcu_periph_clock_enable(GPIO_CLK[lednum]);
    /* configure led GPIO port */ 
    gpio_init(GPIO_PORT[lednum], GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN[lednum]);

    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}


/*!
    \brief      turn on selected led
    \param[in]  lednum: specify the led to be turned on
      \arg        LED1
      \arg        LED2
      \arg        LED3
      \arg        LED4
    \param[out] none
    \retval     none
*/
void gd_eval_led_on(led_typedef_enum lednum)
{
    GPIO_BOP(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief      turn off selected led
    \param[in]  lednum: specify the led to be turned off
      \arg        LED1
      \arg        LED2
      \arg        LED3
      \arg        LED4
    \param[out] none
    \retval     none
*/
void gd_eval_led_off(led_typedef_enum lednum)
{
    GPIO_BC(GPIO_PORT[lednum]) = GPIO_PIN[lednum];
}

/*!
    \brief      toggle selected led
    \param[in]  lednum: specify the led to be toggled
      \arg        LED1
      \arg        LED2
      \arg        LED3
      \arg        LED4
    \param[out] none
    \retval     none
*/
void gd_eval_led_toggle(led_typedef_enum lednum)
{
    gpio_bit_write(GPIO_PORT[lednum], GPIO_PIN[lednum], 
                    (bit_status)(1-gpio_input_bit_get(GPIO_PORT[lednum], GPIO_PIN[lednum])));
}


/*!
    \brief      configure GPIO
    \param[in]  none
    \param[out] none
    \retval     none
*/
void can_gpio_config(void)
{
    /* enable CAN clock */
//	unsigned int freq   = rcu_clock_freq_get(CK_APB1);
//	unsigned int freq2  = rcu_clock_freq_get(CK_APB2); 	
//	unsigned int freq2  = rcu_clock_freq_get(CK_AHB);
//	unsigned int freq2  = rcu_clock_freq_get(CK_SYS); 
//	unsigned int freq2  = rcu_clock_freq_get(CK_USART); 	
	
    rcu_periph_clock_enable(RCU_CAN0);  		
    rcu_periph_clock_enable(RCU_AF);
    
    /* configure CAN0 GPIO */
	  //rcu_periph_clock_enable(RCU_GPIOB);															//PB8 9
		//gpio_init(GPIOB,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,GPIO_PIN_8);		//can0_rx
		//gpio_init(GPIOB,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_9);	//can0_tx
		//gpio_pin_remap_config(GPIO_CAN0_PARTIAL_REMAP,ENABLE);					//PB8 PB9	2
	
		rcu_periph_clock_enable(RCU_GPIOD);															//PD0 1
		gpio_init(GPIOD,GPIO_MODE_IPU,GPIO_OSPEED_50MHZ,GPIO_PIN_0);		//can0_rx
		gpio_init(GPIOD,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_1);	//can0_tx    
		gpio_pin_remap_config(GPIO_CAN0_FULL_REMAP,ENABLE);							//PD0 PD1	3				
}

void lora_config(void)
{
		spi_parameter_struct spi_init_struct;
		rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_SPI0);
		rcu_periph_clock_enable(RCU_AF);

    /* SPI0_SCK(PA5), SPI0_MISO(PA6) and SPI0_MOSI(PA7) GPIO pin configuration */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
	
    /* SPI0_CS(PC5)lora0   CS(PA4)lora1 GPIO pin configuration */
    gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
		gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
		gpio_bit_set(GPIOC, GPIO_PIN_5);			//cs0	high
		gpio_bit_set(GPIOA, GPIO_PIN_4);			//cs1	high
	
		
		//lora0(PC8) 	lora1(PB0) RST Low active
		gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);	
		gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);	
		gpio_bit_set(GPIOC, GPIO_PIN_8);			//rst0	high
		gpio_bit_set(GPIOB, GPIO_PIN_0);			//rst1	high
			
		//lora0(PB1) lora1(PC4) int  Low active    必须上拉
		gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_1);		
		gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_4);		
		gpio_bit_set(GPIOB, GPIO_PIN_1);			//int0	high
		gpio_bit_set(GPIOC, GPIO_PIN_4);			//int1	high
		
		//lora0(PA0) lora1(PA1)  busy Low active    必须上拉
		gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_0);		
		gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_1);		
		gpio_bit_set(GPIOA, GPIO_PIN_0);			//busy0	high
		gpio_bit_set(GPIOA, GPIO_PIN_1);			//busy1	high
		
   	
    /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_8 ;
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI0, &spi_init_struct);

    /* set crc polynomial */
    spi_crc_polynomial_set(SPI0,7);
    /* enable SPI0 */
    spi_enable(SPI0);
		
		/* enable and set EXTI interrupt priority PC6 */
		nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);		// NVIC_PRIGROUP_PRE0_SUB4  0 0  则最高级中断；   NVIC_PRIGROUP_PRE4_SUB0  15  0 则为最低中断
		nvic_irq_enable(EXTI0_IRQn,2U,0U);
		nvic_irq_enable(EXTI1_IRQn,2U,1U);
		nvic_irq_enable(EXTI4_IRQn,2U,2U);		
    
    /* connect key EXTI line to key GPIO pin */
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_0);		//PA0 	busy0
		gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_1);		//PA1 	busy1
		gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_1);		//PB1		INT0 	
		gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOC, GPIO_PIN_SOURCE_4);		//PC4		INT1	
		
    /* configure key EXTI line */
    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
		exti_init(EXTI_1, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
		exti_init(EXTI_4, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
		
    exti_interrupt_flag_clear(EXTI_0);
		exti_interrupt_flag_clear(EXTI_1);
		exti_interrupt_flag_clear(EXTI_4);
}

void delay_us(int _us)
{
	//volatile static int a=0;
//	for(;_us>0;_us--)
//		for(a=0;a<1;a++);
	
	volatile unsigned int _dcnt;
	_dcnt=(_us*100);
	while(_dcnt-->0)
	{
		continue;
	}
	
//	while(_us--){
//		a=10;
//		while(a--);
//	}
}

//g_Mttp
void Com_Send(com_typedef_enum com_num,uint8_t *send_buff,uint32_t length)
{
	uint32_t i = 0;		
	if(0xff == com_num)	//将rj45中8888 upd 接收到的命令进行接解析Local_RTU(0xff) 并将结果转发到上位机。
	{
			Write_SOCK_Data_Buffer(2, Com.RX_Analy_Buf, Com.usRX_Analy_Buf_Len);		//upd 8888 数据返回
	}
	else if(USART5==COM_EVAL[com_num])
	{		
			if(G4==com_num && m_gprsinfo.g_bGPRSConnected)			//4g com5  并且4g已连接
			{
				for(i=0;i<length;i++)
				{				
					while(RESET == usart5_flag_get(COM_EVAL[com_num], USART5_FLAG_TBE) || RESET == usart5_flag_get(COM_EVAL[com_num],USART5_FLAG_TC));		
					usart_data_transmit(COM_EVAL[com_num], send_buff[i]);
					//while(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_TBE));		
					//usart_data_transmit(USART0,send_buff[i]);     		
				}			
			}
	}
	else if(USB==com_num)				//usb通道
	{
		if(0==g_can_bus && USBD_CONFIGURED == usbd_cdc.cur_status)			//usb已连接
		{				
				memcpy(g_usbbuf,send_buff,length);	
				g_usblen = length;
				g_usbFlag = 1; 				
				if (0U == cdc_acm_check_ready(&usbd_cdc)) 
				{
						cdc_acm_data_receive(&usbd_cdc);								
				}  
				else 
				{
						cdc_acm_data_send(&usbd_cdc);								//数据收发实现函数			
				}
				if (0U == cdc_acm_check_ready(&usbd_cdc)) 
				{
						cdc_acm_data_receive(&usbd_cdc);								
				}  
				else 
				{
						cdc_acm_data_send(&usbd_cdc);									//数据收发实现函数			
				}
		}
	}
	else if(CH432T_1==com_num || CH432T_2==com_num)					//ch432转换通道
	{
		//向ch432通道中写入数据
			CH432UARTSend(com_num-CH432T_1,send_buff,length);		//将数据通过ch432t发送出去
	}
	else		
	{
		if(RS485 == com_num)
		{
				com1_sended = 0;		
		}
		else if(RS485_2 == com_num)
		{
				com2_sended = 0;		
		}
		else if(RS485_4 == com_num)
		{
				com4_sended = 0;		
		}
		else if(METER == com_num)
		{			
				com5_sended = 0;				
		}
				
		for(i=0;i<length;i++)
		{			
			
			while(RESET == usart_flag_get(COM_EVAL[com_num], USART_FLAG_TBE) || RESET == usart_flag_get(COM_EVAL[com_num], USART_FLAG_TC));		
			delay_us(10);			//确保发送完成，稍微等待
			usart_data_transmit(COM_EVAL[com_num], send_buff[i]);
			//while(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_TBE));		
			//usart_data_transmit(USART0,send_buff[i]);     		
		}
		while(RESET == usart_flag_get(COM_EVAL[com_num], USART_FLAG_TC));	
		
		if(RS485 == com_num)
		{
				delay_1ms(2);					//9600=960字节/秒，即1字节/ms
				com1_sended = 1;		
		}		
		else if(RS485_2 == com_num)
		{
				//delay_1ms(1);				//9600=960字节/秒，即1字节/ms
			  //delay_us(105);				//240 230 220 210     190 180 170 160        50 100 150 200 250 300 350 400 450 500
				com2_sended = 1;		
		}		
		else if(RS485_4 == com_num)
		{
				//delay_1ms(1);				//9600=960字节/秒，即1字节/ms
				//delay_us(105);				//205为最佳值		
				com4_sended = 1;		
		}		
		else if(METER == com_num)
		{
				delay_1ms(3);					//9600=960字节/秒，即1字节/ms
				com5_sended = 1;				
		}			
	}	
}

/*!
    \brief      configure COM port
    \param[in]  com: COM on the board
      \arg        EVAL_COM0: COM0 on the board
    \param[out] none
    \retval     none
*/
void gd_eval_com_init(com_typedef_enum com_num,uint32_t baudval,uint8_t nvic_irq_pre_priority,uint8_t nvic_irq_sub_priority)
{    	
    /* enable GPIO clock */
    rcu_periph_clock_enable(COM_GPIO_CLK[com_num]);

    /* enable USART clock */
    rcu_periph_clock_enable(COM_CLK[com_num]);		
		
		gpio_init(COM_GPIO_PORT[com_num], GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, COM_TX_PIN[com_num]);
	
    /* connect port to USARTx_Rx */		
		if(COM_EVAL[com_num]==UART4)
		{			
			/* enable GPIO clock */
			rcu_periph_clock_enable(RCU_GPIOD);			
			/* connect port to USARTx_Tx */
			gpio_init(GPIOD , GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, COM_RX_PIN[com_num]);			
		}		
		else
		{
			gpio_init(COM_GPIO_PORT[com_num], GPIO_MODE_IPU , GPIO_OSPEED_50MHZ, COM_RX_PIN[com_num]);		//GPIO_MODE_IN_FLOATING			
		}
				
		
		if(COM_EVAL[com_num]==USART5)
		{
			gpio_afio_port_config(AFIO_PC6_USART5_CFG,ENABLE);
			gpio_afio_port_config(AFIO_PC7_USART5_CFG,ENABLE);
			rcu_usart5_clock_config(RCU_USART5SRC_CKAPB2);
			usart5_overrun_disable (USART5);											//usart5 可以fifo  ovrd位默认使能，则可能产生USART5_INT_FLAG_RBNE_ORERR  orerr中断，而在中断处理中不清除，则一直进入中断。
		}		
		
		if(COM_EVAL[com_num]==USART1)
		{
			gpio_pin_remap_config(GPIO_USART1_REMAP,ENABLE);			//PBD5 6
		}		
		
		
    /* USART configure */
    usart_deinit(COM_EVAL[com_num]);
    usart_baudrate_set(COM_EVAL[com_num], 		baudval);
		usart_word_length_set(COM_EVAL[com_num], 	USART_WL_8BIT);
    usart_stop_bit_set(COM_EVAL[com_num], 		USART_STB_1BIT);
    usart_parity_config(COM_EVAL[com_num], 		USART_PM_NONE);
    usart_receive_config(COM_EVAL[com_num], 	USART_RECEIVE_ENABLE);
    usart_transmit_config(COM_EVAL[com_num], 	USART_TRANSMIT_ENABLE);
    usart_enable(COM_EVAL[com_num]);
	
		nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);		// NVIC_PRIGROUP_PRE0_SUB4  0 0  则最高级中断；   NVIC_PRIGROUP_PRE4_SUB0  15  0 则为最低中断		
		//抢占优先级0位 ，响应优先级4位   数字越小级别越高  NVIC_PRIGROUP_PRE0_SUB4:0  [0,15] ; NVIC_PRIGROUP_PRE1_SUB3: [0,1]  [0,7]; ...  NVIC_PRIGROUP_PRE4_SUB0: [0,15]  0
		nvic_irq_enable(UART_IRQn[com_num], nvic_irq_pre_priority, nvic_irq_sub_priority);		
		/* enable USART RBNE interrupt */
		if(COM_EVAL[com_num]==USART5)
		{
			usart5_interrupt_enable(COM_EVAL[com_num], USART5_INT_RBNE);			 //usart_interrupt_enable(USART0, USART_INT_TBE);		
		}
		else
		{
			usart_interrupt_enable(COM_EVAL[com_num], USART_INT_RBNE);				 //usart_interrupt_enable(USART0, USART_INT_TBE);		
		}
}


void gd_eval_io_init(void)
{
		rcu_periph_clock_enable(RCU_GPIOA);
		rcu_periph_clock_enable(RCU_GPIOB);
		rcu_periph_clock_enable(RCU_GPIOC);
		rcu_periph_clock_enable(RCU_GPIOD);
    rcu_periph_clock_enable(RCU_GPIOE);		   
		rcu_periph_clock_enable(RCU_AF);	   		
		
		//---------- GPIO input ------------------------------------------------------------------------------------------------------	
		//gpio_init(D_AD_DOUT_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, D_AD_DOUT );
		gpio_init(GPIOE, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ,D_DI0|D_DI1|D_DI2|D_DI3|D_DI4|D_DI5|D_DI6|D_DI7|D_SWI0|D_SWI1|D_SWI2|D_SWI3|D_SWI4|D_SWI5 );	//GPIO_InitStructure.GPIO_Pin = D_DI0|D_DI1|D_DI2|D_DI3|D_DI4|D_DI5|D_DI6|D_DI7|D_SWI0|D_SWI1|D_SWI2|D_SWI3|D_SWI4|D_SWI5|D_SWI6|D_SWI7;
	
		//---------- GPIO output ------------------------------------------------------------------------------------------------------			
		gpio_init(D_WIFI_BLE_POWER_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, D_WIFI_BLE_POWER_CTRL);		//wifi power
	
		if(1==g_configRead.b_wifi_work)		
		{
			PowerWifiBle_Ctrl(true);	
			wifi_rev_cnt = 0;
			wifi_reset_sta = 0;	
			wifi_reset_tick = g_sysTick;
		}
		else
		{
			PowerWifiBle_Ctrl(false);		
		}
		
		gpio_init(D_4G_POWER_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, D_4G_POWER);	//4g
		if(1==g_configRead.b_gprs_work)		
		{
			Power4G_Ctrl(true);	
		}
		else
		{
			Power4G_Ctrl(false);	
		}
		
	
	
		//gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, D_AD_DIN|D_AD_SCK  );	
  
		gpio_bit_set(D_AD_RST_PORT,D_AD_RST);			
		gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,D_AD_CS|D_AD_RST|D_WDI);													//PB3:RF_MISO		D_AD_START		GPIO_Pin_3   D_SPI2_CS		
		gpio_bit_set(D_AD_CS_PORT,D_AD_CS);
		gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,D_AI_SELECT_A0|D_AI_SELECT_A1|D_AI_SELECT_A2);		
		
		DO_Output_Ctrl(0,D_OFF);
		DO_Output_Ctrl(1,D_OFF);
		DO_Output_Ctrl(2,D_OFF);
		DO_Output_Ctrl(3,D_OFF);
		DO_Output_Ctrl(4,D_OFF);
		DO_Output_Ctrl(5,D_OFF);
		DO_Output_Ctrl(6,D_OFF);
		DO_Output_Ctrl(7,D_OFF);		
		gpio_init(GPIOD, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,D_DO_CTRL0|D_DO_CTRL1|D_DO_CTRL2|D_DO_CTRL3|D_DO_CTRL4|D_DO_CTRL5|D_DO_CTRL6|D_DO_CTRL7);		
	
		gpio_init(D_LED1_CTRL_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,D_LED1_CTRL|D_LED2_CTRL);		
		gpio_init(D_BEEP_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,D_BEEP);		
		gpio_bit_reset(D_BEEP_PORT, D_BEEP);
		//gpio_bit_set(D_BEEP_PORT, D_BEEP);							//busy1	high	
		
		gpio_init(D_AD_DRDY_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, D_AD_DRDY );
		gpio_init(D_ANDROID_POWER_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, D_ANDROID_POWER_CTRL);		//android-->ch432t int#  PE9
				
		gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);				//ch432t  cs high
		//gpio_bit_reset(GPIOC, GPIO_PIN_15);
		gpio_bit_set(GPIOC,GPIO_PIN_15); 
	
		
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
		/* enable and set EXTI interrupt priority PB8   AD_DRDY  */
		nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);		// NVIC_PRIGROUP_PRE0_SUB4  0 0  则最高级中断；   NVIC_PRIGROUP_PRE4_SUB0  15  0 则为最低中断
    nvic_irq_enable(EXTI5_9_IRQn, 1U, 0U);
    /* connect key EXTI line to key GPIO pin */
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_8);		
    /* configure key EXTI line */
    exti_init(EXTI_8, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_8);			
	
		gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOE, GPIO_PIN_SOURCE_9);
		exti_init(EXTI_9, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_9);				
}


void print_base_info(void)
{
	uint32_t  freq_apb1   	= rcu_clock_freq_get(CK_APB1);
	uint32_t  freq_apb2  		= rcu_clock_freq_get(CK_APB2); 	
	uint32_t  freq_ahb  		= rcu_clock_freq_get(CK_AHB);
	uint32_t  freq_sys  		= rcu_clock_freq_get(CK_SYS); 
	uint32_t  freq_usart5  	= rcu_clock_freq_get(CK_USART); 
	printf("[clock] sys=%d,ahb=%d,apb1=%d,apb2=%d,usart5=%d\r\n",freq_sys,freq_ahb,freq_apb1,freq_apb2,freq_usart5);
	//[clock] sys=168000000,ahb=168000000,apb1=84000000,apb2=168000000,usrat5=168000000
	

	uint8_t   user_ob = ob_user_get();	//0xff default
	uint16_t  data_ob = ob_data_get();
	uint32_t  write_protection = ob_write_protection_get();
	FlagStatus security_protection = ob_security_protection_flag_get();
	
	//FlagStatus fmc_flag_get(uint32_t flag);
	//FlagStatus fmc_interrupt_flag_get(fmc_interrupt_flag_enum flag);
	
	printf("[fmc1] user_ob=0x%02x, data_ob=0x%04x, write_protection=0x%08x, security_protection=%d \r\n",user_ob,data_ob,write_protection,security_protection);	
	printf("[fmc2] FMC_WS=0x%04x, FMC_STAT=0x%x, FMC_CTL=0x%04x,  FMC_OBSTAT=0x%08x, FMC_WP=0x%08x, FMC_PID=0x%08x, \r\n",FMC_WS&0xffff,FMC_STAT&0xff,FMC_CTL&0xffff,FMC_OBSTAT,FMC_WP,FMC_PID);
	
	//uint32_t storage = 0x1FFFF7E0;
	printf("store capacity=0x%08x \r\n",REG32(0x1ffff7e0));																				//0x0060(96K sram) 0100(256K Flash)
	printf("cpu id=0x%08x%08x%08x \r\n",REG32(0x1ffff7f0),REG32(0x1ffff7ec),REG32(0x1ffff7e8));		//从左到右依次为   从高位到低位 0x413848541337343155315a18	
	
	//启动看门狗
	//将秘钥1次性写入 opt位置，采用加密算法实现。	    	
	printf("[WDGT] FWDGT_PSC=%d, FWDGT_RLD=%d, FWDGT_STAT=%d\r\n",FWDGT_PSC&0xf,FWDGT_RLD&0xffff,FWDGT_STAT&0x3);	
}
