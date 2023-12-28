/*!
    \file    gd32e503v_eval.h
    \brief   definitions for GD32E503V_EVAL's leds, keys and COM ports hardware resources

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

#ifndef GD32E503V_EVAL_H
#define GD32E503V_EVAL_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32e50x.h"

#define D_JUDGE_DI_INPUT_STATE_MAX_TIMES	10
#define D_ON	1
#define D_OFF 0

/* exported types */
typedef enum 
{
    LED1 = 0,
    LED2 = 1  
} led_typedef_enum;

/* eval board low layer led */
#define LEDn                             2U

#define LED1_PIN                         GPIO_PIN_8
#define LED1_GPIO_PORT                   GPIOC
#define LED1_GPIO_CLK                    RCU_GPIOC
  
#define LED2_PIN                         GPIO_PIN_9
#define LED2_GPIO_PORT                   GPIOC
#define LED2_GPIO_CLK                    RCU_GPIOC

typedef enum 
{
    RS485 		= 0,		//rs485 master
    RS485_2 	= 1,  	//rs485
		WIFI_BLE 	= 2,		//wifi & ble
		RS485_4 	= 3,  	//rs485
	  METER 		= 4,		//meter
		G4				= 5,		//4g
		USB				= 6,		//USB
	  CH432T_1  = 7,		//ch432T com1
	  CH432T_2  = 8			//ch432T com2  A	
	  //ETH1			= 9,	//rj45_1
	  //EHT2      = 10,  //rj45_2
	 
} com_typedef_enum;



#define COMn                             9U			//中间有一个
extern  uint32_t COM_EVAL[COMn];

#define EVAL_COM0                        USART0
#define EVAL_COM0_CLK                    RCU_USART0
#define EVAL_COM0_TX_PIN                 GPIO_PIN_9
#define EVAL_COM0_RX_PIN                 GPIO_PIN_10
#define EVAL_COM0_GPIO_PORT              GPIOA
#define EVAL_COM0_GPIO_CLK               RCU_GPIOA
#define EVAL_COM0_IRQn               		 USART0_IRQn

#define EVAL_COM1                        USART1
#define EVAL_COM1_CLK                    RCU_USART1
//#define EVAL_COM1_TX_PIN               GPIO_PIN_2
//#define EVAL_COM1_RX_PIN               GPIO_PIN_3
//#define EVAL_COM1_GPIO_PORT            GPIOA
//#define EVAL_COM1_GPIO_CLK             RCU_GPIOA
#define EVAL_COM1_TX_PIN                 GPIO_PIN_5
#define EVAL_COM1_RX_PIN                 GPIO_PIN_6
#define EVAL_COM1_GPIO_PORT              GPIOD
#define EVAL_COM1_GPIO_CLK               RCU_GPIOD
#define EVAL_COM1_IRQn               		 USART1_IRQn

#define EVAL_COM2                        USART2
#define EVAL_COM2_CLK                    RCU_USART2
#define EVAL_COM2_TX_PIN                 GPIO_PIN_10
#define EVAL_COM2_RX_PIN                 GPIO_PIN_11
#define EVAL_COM2_GPIO_PORT              GPIOB
#define EVAL_COM2_GPIO_CLK               RCU_GPIOB
#define EVAL_COM2_IRQn               		 USART2_IRQn

#define EVAL_COM3                        UART3
#define EVAL_COM3_CLK                    RCU_UART3
#define EVAL_COM3_TX_PIN                 GPIO_PIN_10
#define EVAL_COM3_RX_PIN                 GPIO_PIN_11
#define EVAL_COM3_GPIO_PORT              GPIOC
#define EVAL_COM3_GPIO_CLK               RCU_GPIOC
#define EVAL_COM3_IRQn               		 UART3_IRQn

//pc12=tx  pd2=rx
#define EVAL_COM4                        UART4
#define EVAL_COM4_CLK                    RCU_UART4
#define EVAL_COM4_TX_PIN                 GPIO_PIN_12	//PC12
#define EVAL_COM4_RX_PIN                 GPIO_PIN_2		//PD2 RX
#define EVAL_COM4_GPIO_PORT              GPIOC
#define EVAL_COM4_GPIO_CLK               RCU_GPIOC
#define EVAL_COM4_IRQn               		 UART4_IRQn

#define EVAL_COM5                        USART5
#define EVAL_COM5_CLK                    RCU_USART5
#define EVAL_COM5_TX_PIN                 GPIO_PIN_6
#define EVAL_COM5_RX_PIN                 GPIO_PIN_7
#define EVAL_COM5_GPIO_PORT              GPIOC
#define EVAL_COM5_GPIO_CLK               RCU_GPIOC
#define EVAL_COM5_IRQn               		 USART5_IRQn


//---------------------------------wdt---------------------------------------
#define D_WDI                         	GPIO_PIN_6 		//PB6
#define D_WDI_PORT 											GPIOB

//--------------------------------2 leds-------------------------------------
#define D_LED1_CTRL											GPIO_PIN_8  	//PC8
#define D_LED1_CTRL_PORT								GPIOC
#define D_LED2_CTRL											GPIO_PIN_9 		//PC9
#define D_LED2_CTRL_PORT								GPIOC



//--------------------------------CAN----------------------------------------
#define D_CAN_RX_PIN										GPIO_PIN_0			//PD0
#define D_CAN_TX_PIN										GPIO_PIN_1			//PD1
#define D_CAN_PORT											GPIOD

//---------- input -------------------------------------------------------
#define D_DI0														GPIO_PIN_0   //PE0
#define D_DI0_PORT											GPIOE
#define D_DI1														GPIO_PIN_1   //PE1
#define D_DI1_PORT											GPIOE
#define D_DI2														GPIO_PIN_2   //PE2
#define D_DI2_PORT											GPIOE
#define D_DI3														GPIO_PIN_3   //PE3
#define D_DI3_PORT											GPIOE
#define D_DI4														GPIO_PIN_4   //PE4
#define D_DI4_PORT											GPIOE
#define D_DI5														GPIO_PIN_5   //PE5
#define D_DI5_PORT											GPIOE
#define D_DI6														GPIO_PIN_6   //PE6
#define D_DI6_PORT											GPIOE
#define D_DI7														GPIO_PIN_7   //PE7
#define D_DI7_PORT											GPIOE

#define D_SWI0													GPIO_PIN_15   //PE15
#define D_SWI0_PORT											GPIOE
#define D_SWI1													GPIO_PIN_14   //PE14
#define D_SWI1_PORT											GPIOE
#define D_SWI2													GPIO_PIN_13   //PE13
#define D_SWI2_PORT											GPIOE
#define D_SWI3													GPIO_PIN_12   //PE12
#define D_SWI3_PORT											GPIOE
#define D_SWI4													GPIO_PIN_11   //PE11
#define D_SWI4_PORT											GPIOE
#define D_SWI5													GPIO_PIN_10   //PE10
#define D_SWI5_PORT											GPIOE
//#define D_SWI6													GPIO_PIN_9   //PE9
//#define D_SWI6_PORT											GPIOE
//#define D_SWI7													GPIO_PIN_8   //PE8
//#define D_SWI7_PORT											GPIOE

//---------- output --------------------------------------------------------------
#define D_WIFI_BLE_POWER_CTRL						GPIO_PIN_8   	//PE8
#define D_WIFI_BLE_POWER_PORT						GPIOE

#define D_ANDROID_POWER_CTRL						GPIO_PIN_9	  //PE9
#define D_ANDROID_POWER_PORT						GPIOE

#define D_4G_POWER											GPIO_PIN_13    //PC13	4G power control high=active
#define D_4G_POWER_PORT                	GPIOC

#define D_W5500_SCS											GPIO_PIN_4    //PA4
#define D_W5500_SCS_PORT                GPIOA

#define D_W5500_RST                     GPIO_PIN_1    //PB1
#define D_W5500_RST_PORT                GPIOB

#define D_BEEP                     			GPIO_PIN_8    //PA8
#define D_BEEP_PORT                			GPIOA

#define D_RF_RST                        GPIO_PIN_6    //PB6
#define D_RF_RST_PORT                   GPIOB

#define D_SPI2_CS                       GPIO_PIN_12   //PB12
#define D_SPI2_CS_PORT                  GPIOB

#define D_DO_CTRL0 											GPIO_PIN_8    //PD8
#define D_DO_CTRL0_PORT                 GPIOD
#define D_DO_CTRL1 											GPIO_PIN_9    //PD9
#define D_DO_CTRL1_PORT                 GPIOD
#define D_DO_CTRL2 											GPIO_PIN_10   //PD10
#define D_DO_CTRL2_PORT                 GPIOD
#define D_DO_CTRL3 											GPIO_PIN_11   //PD11
#define D_DO_CTRL3_PORT                 GPIOD
#define D_DO_CTRL4 											GPIO_PIN_12   //PD12
#define D_DO_CTRL4_PORT                 GPIOD
#define D_DO_CTRL5 											GPIO_PIN_13   //PD13
#define D_DO_CTRL5_PORT                 GPIOD
#define D_DO_CTRL6 											GPIO_PIN_14   //PD14
#define D_DO_CTRL6_PORT                 GPIOD
#define D_DO_CTRL7 											GPIO_PIN_15   //PD15
#define D_DO_CTRL7_PORT                 GPIOD


/* function declarations */

void gd_eval_io_init(void);
void can_gpio_config(void);

/* configure COM port */
void gd_eval_com_init(com_typedef_enum com_num,uint32_t baudval,uint8_t nvic_irq_pre_priority,uint8_t nvic_irq_sub_priority);

void DO_Output_Ctrl(unsigned char Para_CH,unsigned char Para_Output_State);
void Do_Get_Sta(void);
void PowerWifiBle_Ctrl(unsigned char sta);
void Power4G_Ctrl(unsigned char sta);
void Android_Ctrl(unsigned char sta);
void Get_Android_Sta(void);	
void Led_Ctrl(unsigned char ch, unsigned char sta);
void Read_SW_Input_State(void);
void Read_DI_Input_State(void);
void wdt(void);
void Beep(unsigned char sta);
void Com_Send(com_typedef_enum com_num,uint8_t *send_buff,uint32_t length);	
void AI_Channel_Select(unsigned char Para_ucCH);
unsigned short CrcCheck(unsigned char *pData,unsigned short nLen);
void print_base_info(void);
void GPRS_Reset(void);
void wifi_reset(void);
	
#ifdef __cplusplus
}
#endif

#endif /* GD32E503V_EVAL_H */
