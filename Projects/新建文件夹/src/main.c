/*!
    \file    main.c
    \brief   USB CDC ACM device

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

#include "sht3x.h"
#include "gd32e50x.h"
#include "cdc_acm_core.h"
#include "usbd_hw.h"
#include "systick.h"
#include "W5500.h"
#include "cfg_flash.h"
#include "GlobalVar.h"
#include <stdio.h>
#include "wifi_ble.h"
#include "spi_flash.h"
#include "ads1247.h"
#include "comm_process.h"
#include "AD_process.h"
#include "can_bus.h"
#include "string.h"
#include "gprs.h"
#include "usb_para.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ch432t.h"
#include "prj.h"

//uint8_t W5500_Interrupt = 0;
//static void w5500_config(void);

extern unsigned int 	read_ext_data_tick ;
extern int						g_GprsWaitResultFlag;			//需要gprs等待标志
extern int 						iNetReg;									//gprs拨号的时候，gprs网络注册不成功的次数 
extern unsigned char  g_com_port[4] ;

void print_base_info(void);

#define D_W25QXX_FLASH_DEVICEID_ADDR	  				0x001000
#define D_W25QXX_FLASH_DC_DC_OUTPUT_STATE_ADDR	0x002000

void Flash_DeviceID_WR(void)
{
	unsigned char buf[8];
	SPI_FLASH_SectorErase(D_W25QXX_FLASH_DEVICEID_ADDR);
	buf[0]=DevInfo.ucID;
	SPI_FLASH_BufferWrite(buf,D_W25QXX_FLASH_DEVICEID_ADDR,1);
}

void Flash_DeviceID_RD(void)
{
	unsigned char buf[8];
	SPI_FLASH_BufferRead(buf,D_W25QXX_FLASH_DEVICEID_ADDR,1);
	if(buf[0]==0xFF)
	{
		DevInfo.ucID=0x01;
	}
	else
	{
		DevInfo.ucID=buf[0];
	}
}

void FLASH_Data_Init(void)
{
	unsigned char buf[128];
	uint8_t 	i=1,j=0;
	uint32_t 	ln1;	
  SPI_FLASH_Init();	 
 // while(i) 
	{
  	ln1=SPI_FLASH_ReadID();
		printf("spi flash id is 0x%08x \r\n",ln1);
  	if(ln1==0x00EF4016||ln1==0x00EF6016||ln1==0x00EF4316)
		{	  //0x00EF6016
  		i=0;
			//break;
  	}		
		else
		{
  	//	SPI_FLASH_Init();
//  		DlymS(20);
  	}
  }
  
	SPI_FLASH_BufferRead(buf,0x000000,7);
	#if 0
  if(memcmp(buf,DevInfo.COMPANY_TB.ucStr,DevInfo.COMPANY_TB.ucLen))
	{
//	if(memcmp(buf,COMPANY_TB.ucStr,COMPANY_TB.ucLen)){
		j=1;
	}
//	j=1;
//	SPI_FLASH_BulkErase();//²Á³ýջƬFlash
	if(j>0)
	{	//δʹӃ¹ýµĆLASHROM
		SPI_FLASH_SectorErase(0x000000); 	//ɨ±¸ЅϢ
		SPI_FLASH_SectorErase(0x001000);	//ɨ±¸ID
		SPI_FLASH_SectorErase(0x002000);	//DC_DCʤ³ö״̬
//---------- ¹«˾û ---------------------------------------------------
		memcpy(buf,DevInfo.COMPANY_TB.ucStr,DevInfo.COMPANY_TB.ucLen);//¿ªʼ±ꖾ
		SPI_FLASH_BufferWrite(buf,0x000000,DevInfo.COMPANY_TB.ucLen);
		Flash_DeviceID_WR();
	}
	else
	{
		Flash_DeviceID_RD();
	}
	#endif
}

//com口测试，将com1 usb接收到的数据转发到指定(ch) com口中，指定(ch) com口收到的数据在转发到com1 usb口上。 可以将指定(ch) com的tx rx进行短接
//指定 (ch) com口中 收到数据，则打印到调试口
//ch = 1 2 3 4 5
void uart_test(unsigned char ch)
{	
	//com  usb固定通道 USB
	unsigned char buf[512];
	int		len = 0;	
	while(Com.Usart[RS485].usRec_WR != Com.Usart[RS485].usRec_RD)
	{
			buf[len++] = Com.Usart[RS485].RX_Buf[Com.Usart[RS485].usRec_RD];
			Com.Usart[RS485].usRec_RD = (Com.Usart[RS485].usRec_RD+1)% D_USART_REC_BUFF_SIZE;	
	}	
	if(len>0)
	{
			Com_Send(ch,buf,len);
	}	
	
	len = 0;
	while(Com.Usart[ch].usRec_WR != Com.Usart[ch].usRec_RD)
	{
			buf[len++] = Com.Usart[ch].RX_Buf[Com.Usart[ch].usRec_RD];
			Com.Usart[ch].usRec_RD = (Com.Usart[ch].usRec_RD+1)% D_USART_REC_BUFF_SIZE;	
	}
	if(len>0)
	{
			Com_Send(RS485,buf,len);
	}		
}

#if 0
void GetIPAndPort()
{
		int len = g_configRead.PortLen;
		int port = 0,i=0,j=0;
		for(i=0;i<len;i++)
		{
				port = port*10 + (g_configRead.remotePort[i] - 0x30);  
		}
		//g_remoteport = port;
		ucDestPort = port;
		
		//=====================ip=============================
		len = g_configRead.IPLen;		//remoteIP
		port = 0;
		ucDestIP[port] = 0;
		j = 0;
		for(i=0;i<len;i++)
		{
				if(g_configRead.remoteIP[i]=='.')
				{
						port++;
						ucDestIP[port] = 0;
						j = 0;
				}
				else
				{
						ucDestIP[port] = ucDestIP[port]*10 + (g_configRead.remoteIP[i] - 0x030);  
				}
		}
		//g_remoteip = (ucDestIP[0]<<24)|(ucDestIP[1]<<16)|(ucDestIP[2]<<8)|ucDestIP[3];
}
#endif

//设置wifi 2通道的ip 地址		ucDestIP		ucDestPort
void	SetCh2IP(unsigned char *uDestIP,int uDestPort,int delay_ms )
{
	int i = 0,j=0,k=0;
	
}

#define		BOOT_NO				1								//如果从启动代码启动，则该为设置为1
#define		VECT_TAB_APP	0xc000					//

//修改文件system_gd32e50x.c中的     #define VECT_TAB_OFFSET  (uint32_t)0x00         //#define VECT_TAB_OFFSET  (uint32_t)0xC000            


void BSP_Init(void)
{				
		uint8_t					i=0;		
		#if BOOT_NO	
		//nvic_vector_table_set(NVIC_VECTTAB_FLASH,VECT_TAB_APP);
		#endif	
	
		cpu_id[0] = *(unsigned int*)(0x1FFFF7E8);
    cpu_id[1] = *(unsigned int*)(0x1FFFF7EC);
    cpu_id[2] = *(unsigned int*)(0x1FFFF7F0);	

		//++++++++++++++++++++++++++++++++++++++++++ FMC 参数读取 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++  
		for(i=0;i<3;i++)									//读配置参数,只读不写，如果读不成功，则采用默认参数，但不写eeprom,目的防止电压低读写失败。
		{				
			if(init_config_net(2))									
			{
					break;
			}
		}		
		wdt();
		for(i=0;i<3;i++)									//读配置参数,只读不写，如果读不成功，则采用默认参数，但不写eeprom,目的防止电压低读写失败。
		{	
			wdt();		
			if(init_config_ads1247(2))									
			{
					break;
			}
		}
		
		wdt();	
		for(i=0;i<3;i++)										//读配置参数,只读不写，如果读不成功，则采用默认参数，但不写eeprom,目的防止电压低读写失败。
		{		
			if(init_config(2))									
			{
					break;
			}
		}
		
		ReadFlashWifi();
	//	GetIPAndPort();
		
		gd_eval_io_init();	wdt();																		//android power, 4g power, wifi&ble power	
		if(g_configRead.beep)
		{
			Beep(1);
		}
		ADC_Inititile();																							//ad init		
		
		//+++++++++++++++++++++++++++++++++++++++++	2 leds init	++++++++++++++++++++++++++++++++++++++++++++++++++++++
		Led_Ctrl(1,true);		 
		wdt();
		//++++++++++++++++++++++++++++++++++++++++++ SPI FLASH, w5500 cs init +++++++++++++++++++++++++++++++++++++++++++++++++++++++++   
		gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);		//w5500		cs1	
		gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);		//w5500		cs2	
		
		gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);		//w25q32  cs
		
		gpio_bit_set(GPIOA, GPIO_PIN_4);		//w5500_config cs1	
		gpio_bit_set(GPIOA, GPIO_PIN_1);		//w5500_config cs2	
		
		
		W5500_CS_HIGH();   									//w5500_config cs1
		
		SPI_FLASH_CS_HIGH();

		Read_SW_Input_State();	
		g_can_bus = Sw_Sta[4];									//dip0 表示usb模式或者can模式 =0 默认usb,开机前
		
		wdt();
		//Android_Ctrl(true);										//android 开机
			
		for(i=0;i<COMn;i++)
		{
				Com.Usart[i].usDelay_Rev = 16;				//20ms 两个字节接收时间>该值，表示该包数据接收完毕。9600bps 1s=960字节  1字节=1ms ; 4800  1字节=2ms ; 2400 1字节=4ms
		}		
		
		for(i=0;i<4;i++)
		{
				Sw_com[i].usDelay_Rev = 600;				//300ms如果还没有收到回应，则可以切换模式
				Sw_com[i].ucFlag = 0;								//空闲模式
				Sw_com[i].ucMode = 0;
		}		
		
		wdt();			
			
		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ CAN0 +++++++++++++++++++++++++++++++++++++++++++++++		  
			
			
		//if(Master_sta)
		//{
		//	COM_MASTER = G4;
		//}
		FLASH_Data_Init();							//本项目中SPI FLash无用，暂时注释掉 与w5500公用一个spi2
		//SPI_flash_CSH();
		
		AI_Channel_Select(D_AI_CH_0);	
		ADS1247_IO_Init();
		ADS1247_Init();				wdt();			//0x30  121601/4194304=0.0289919376373291  0.0297v    2.047		122509/4194304
		
		//delay_1ms(500);  wdt();
		//delay_1ms(500);  wdt();
		systick_config();	
		if(1==g_configRead.b_gprs_work)		
		{
			ResetGPRS(); 
		}	
		
		//wdt();
		//printf("\r\n USART0 test. \r\n");		
		wdt();
		
		#if 0
		//++++++++++++++++++++++++++++++++++++++++ usb device configure+++++++++++++++++++++++++++++++++++++++++++++++++++
		if(g_can_bus==1)
		{
			/* initialize CAN and filter */
			can_gpio_config();	
			can_config();                             		
		}
		else
		{
			/* system clocks configuration */
			rcu_config();

			/* GPIO configuration */
			gpio_config();

			/* USB device configuration */
			usbd_init(&usbd_cdc, &cdc_desc, &cdc_class);

			/* NVIC configuration */
			nvic_config();

			/* enabled USB pull-up */
			usbd_connect(&usbd_cdc);
		}
		
		//RS485 		= 0,	//rs485 master
    //RS485_2 	= 1,  //rs485
		//WIFI_BLE 	= 3,	//wifi & ble
		//RS485_4 	= 4,  //rs485
	  //METER 		= 5,	//meter
		//G4				= 6		//4g
		//+++++++++++++++++++++++++++++configure usart2 usb->ch340->usart2 +++++++++++++++++++++++++++++++++++++++++++++		
		rcu_periph_clock_enable(RCU_AF);		
		gd_eval_com_init(RS485,				9600,			0,1);		//rs485
		gd_eval_com_init(RS485_2,			9600,			0,2);		//rs485		2
		gd_eval_com_init(WIFI_BLE,		115200,		0,3);		//wifi&ble
    gd_eval_com_init(RS485_4,			9600,			0,4);		//rs485		4
		gd_eval_com_init(METER,				9600,			0,5);		//meter
		gd_eval_com_init(G4,					115200,		0,6);		//meter
		//Master_sta=Sw_Sta[1];				//dip1 默认rs485作为主口	=0;		
		#endif
		wdt();
		
		if(1==g_configRead.b_rj45_work)		
		{	
			w5500_config();							wdt();		
			W5500_Hardware_Reset();			wdt();						
			W5500_Initialization();			wdt();   // printf("56\r\n");
		}
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 调试信息打印 +++++++++++++++++++++++++++++++++++++++++		 	
					
		//Beep(0);				
		read_ext_data_tick = g_sysTick;
		g_GprsWaitResultFlag = 0;
		iNetReg = 0;										//gprs拨号的时候，gprs网络注册不成功的次数 			
		Beep(0);
}

static TaskHandle_t AppTaskCreate_Handle = NULL;
static TaskHandle_t WIFI_Task_Handle 		 = NULL;			/* WIFI任务句柄 */
static TaskHandle_t G4_Task_Handle   		 = NULL;			/* 4G任务句柄 */
static TaskHandle_t Main_Task_Handle 		 = NULL;			/* Main任务句柄 */

TaskHandle_t EtherNet_Task_Handle        = NULL;			/* w5500任务句柄 */
static void AppTaskCreate(void);											/* 用于创建任务 */
//static void WIFI_Task(void* pvParameters);					/* WIFI_Task任务实现 */
//static void G4_Task(void* pvParameters);						/* 4G_Task任务实现 */
static void Main_Task(void* pvParameters);						/* Main_Task任务实现 */

int main(void)
{	
	#if 0
	BaseType_t xReturn = pdPASS;																		/* 定义一个创建信息返回值，默认为pdPASS */		
	BSP_Init();	
	
	xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  					/* 任务入口函数 */
                        (const char*    )"AppTaskCreate",					/* 任务名字 */
                        (uint16_t       )256,  										/* 任务栈大小 */
                        (void*          )NULL,										/* 任务入口函数参数 */
                        (UBaseType_t    )1, 											/* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);	/* 任务控制块指针 */ 
												
	if(pdPASS == xReturn)
    vTaskStartScheduler();   /* 启动任务，开启调度 */
  else
    return -1;   
  while(1);   /* 正常不会执行到这里 */   
	#endif
		BSP_Init();
		Main_Task(NULL);
}

/***********************************************************************
  * @ 函数名  ： AppTaskCreate
  * @ 功能说明： 为了方便管理，所有的任务创建函数都放在这个函数里面
  * @ 参数    ： 无  
  * @ 返回值  ： 无
  **********************************************************************/
static void AppTaskCreate(void)
{
  BaseType_t xReturn = pdPASS;		/* 定义一个创建信息返回值，默认为pdPASS */
  taskENTER_CRITICAL();           //进入临界区
  
  /* 创建LED_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )G4_Task, 					/* 任务入口函数 */
                        (const char*    )"G4_Task",					/* 任务名字 */
                        (uint16_t       )2048,   						/* 任务栈大小 */
                        (void*          )NULL,							/* 任务入口函数参数 */
                        (UBaseType_t    )3,	    						/* 任务的优先级 */
                        (TaskHandle_t*  )&G4_Task_Handle);	/* 任务控制块指针 */
												
  if(pdPASS == xReturn)
    printf("Create 4g_Task success!\r\n");
  wdt();
	/* 创建LED_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )WIFI_Task, 					/* 任务入口函数 */
                        (const char*    )"WIFI_Task",					/* 任务名字 */
                        (uint16_t       )2048,   							/* 任务栈大小 */
                        (void*          )NULL,								/* 任务入口函数参数 */
                        (UBaseType_t    )3,	    							/* 任务的优先级 */
                        (TaskHandle_t*  )&WIFI_Task_Handle);	/* 任务控制块指针 */
 // if(pdPASS == xReturn)
//	  printf("Create wifi_Task success!\r\n");
	wdt();
	
	/* 创建Main_Task任务 */
  xReturn = xTaskCreate((TaskFunction_t )Main_Task, 					/* 任务入口函数 */
                        (const char*    )"MAIN_Task",					/* 任务名字 */
                        (uint16_t       )4096,   							/* 任务栈大小 */
                        (void*          )NULL,								/* 任务入口函数参数 */
                        (UBaseType_t    )3,	    							/* 任务的优先级 */
                        (TaskHandle_t*  )&Main_Task_Handle);	/* 任务控制块指针 */
  //if(pdPASS == xReturn)
  //  printf("Create Main_Task success!\r\n");
	wdt();
	
	/* 创建EtherNet_Task任务 */
//  xReturn = xTaskCreate((TaskFunction_t )EtherNet_Task, 					/* 任务入口函数 */
//                        (const char*    )"Rj45_Task",							/* 任务名字 */
//                        (uint16_t       )2048,   									/* 任务栈大小 */
//                        (void*          )NULL,										/* 任务入口函数参数 */
//                        (UBaseType_t    )3,	    									/* 任务的优先级 */
//                        (TaskHandle_t*  )&EtherNet_Task_Handle);	/* 任务控制块指针 */
  //if(pdPASS == xReturn)
  //  printf("Create EhterNet_Task success!\r\n");		
	
	wdt();
	
  vTaskDelete(AppTaskCreate_Handle); 	//删除AppTaskCreate任务  
  taskEXIT_CRITICAL();            		//退出临界区
	wdt();
}

//数据处理过程
//unsigned int  g_Modbus_Send_Tick      = 0;		//命令发送时刻
//extern unsigned char g_Mppt_Cnt	;		//mppt个数
unsigned char  Data_Tran()
{
	//g_Result
	//数据转换并将所有转后的结果数据写到一个结果结构体中。
	//static unsigned char muc_E_UIt_J_Sum_DayExist[4] = {0,0,0,0};
	static unsigned char   start					= 0;
	static unsigned char   start_soc      = 0;
	static unsigned int    load_time			= 0;
	static unsigned int		 free_time      = 0;
	static unsigned short  ls_data        = 0;
	static unsigned int    mul_Day_t0     = 0;								//天  计时单位	
	static unsigned int    Interval_Tick  = 0;								//最后一条命令离当前时间	
	unsigned char 			   batt_v_Exist   = 0;
	unsigned int 					 tim = systickCount - Interval_Tick;
	float tmp = 0.0,t=0.0,ff=0.0;
	int i = 0;
	if(tim>999)																														//1s转换1次
	{
		//数据进行转换
		//g_Mttp[i]  V_PU_Hi   I_PU_Hi		
		wdt();
		g_Result.lem1_load1 = Ads_1247_Fvalue[2] * 150 / 0.80	;							//负载1电流in2 
	  g_Result.lem2_load2 = Ads_1247_Fvalue[1] * 150 / 0.80	;	;						//负载2电流in1 
		g_Result.lem_load3  = Ads_1247_Fvalue[3] * 150 / 0.80	;							//负载3电流in3 
	  g_Result.lem_load4  = Ads_1247_Fvalue[4] * 150 / 0.80	;	;						//负载4电流in4 
		
		if(g_Result.lem1_load1<0)
			g_Result.lem1_load1 = 0;
		
		if(g_Result.lem2_load2<0)
			g_Result.lem2_load2 = 0;
		
		if(g_Result.lem_load3<0)
			g_Result.lem_load3 = 0;
		
		if(g_Result.lem_load4<0)
			g_Result.lem_load4 = 0;
		
	  g_Result.load_i     = g_Result.lem1_load1 + g_Result.lem2_load2 + g_Result.lem_load3 + g_Result.lem_load4;		//负载总电流
		
		g_Result.temp[0]   = Ads_1247_Fvalue[5] * 100;					//in5(内部温度)     单位℃   0.1℃/mv
		g_Result.temp[1]   = Ads_1247_Fvalue[7] * 100;					//in7(环境温度)			单位℃		
				
		g_Result.ir = Ads_1247_Fvalue[0]*517;										//太阳能光照强度 in0	 进行计算  单位  w/m2
		if(g_Result.ir<0)
			g_Result.ir = 0;
		
    g_Result.batt_status 					= DI.bInput_State[0];			//电池状态 电池空气开关是否都合闸 	 DI0    led亮为正常
	  g_Result.load_status 					= DI.bInput_State[1];			//负载状态 负载1 2空气开关是否都合闸 DI1		led亮为正常	  		
		g_Result.doormagnetic_status 	= DI.bInput_State[2];			//门磁状态 												   DI2		led亮为正常	 
	  g_Result.battlose_status     	= DI.bInput_State[3];			//电池状态  											   DI3		led亮为正常
	  g_Result.pvlose_status       	= DI.bInput_State[4];			//pv防盗状态          						   DI4		led亮为正常		
		g_Result.load34_status			  = DI.bInput_State[5];			//负载状态 负载3 4空气开关是否都合闸 DI5	  led亮为正常
		g_Result.smog_alarm       		= DI.bInput_State[6];			//烟雾报警           							   DI6		led亮为正常		
		g_Result.waterout_alarm       = DI.bInput_State[7];			//水浸报警         								   DI7		led亮为正常		
		
//		if(Ads_1247_Fvalue[6]<0.1 && Ads_1247_Fvalue[6]>-0.1)
//		{
//			g_Result.lem4_batt = 0;
//		}
//		else
		{
			//g_Result.lem4_batt =  (Ads_1247_Fvalue[6]-2.5) * 150 *4 / 0.80	;									//电池电流in4	  150a表示 800mv  in4-->in6
			g_Result.lem4_batt =  Ads_1247_Fvalue[6] * 750	;																		//电池电流in4	  150a表示 800mv  in4-->in6    *4    150*4/0.8=750
		}		
		
		for(i=0;i<g_configRead.alarmDelyMinute;i++)	 	//mppt个数
		{			  
		 	if(g_Mttp[i].Mask&1)
			{
					if(0==start)
							start          = 1;
					batt_v_Exist       = 1;
					g_Result.batt_v    =  g_Mttp[i].Reg[0] * (g_Mttp[i].V_PU_Hi+g_Mttp[i].V_PU_Lo/65536.0)/32768.0;			//电池电压	或者			g_Mttp[i].Reg[2]  
					g_Result.batt_p    =  g_Result.lem4_batt*g_Result.batt_v;																						//电池功率 充电，注意如果超过150A则采用多股线分流的办法来实现，需要 * 股数 					        
					g_Result.load_p    =  g_Result.load_i*g_Result.batt_v; 																							//负载总功率					
					break;
			}
		}
		
		tmp = 0.0;
		g_Result.pv_p = 0.0;
		g_Result.pv_i = 0.0;
		for(i=0;i<g_configRead.alarmDelyMinute;i++)	 						//增加pv总电流
		{			  
		 	if(g_Mttp[i].Mask&1)
			{		
					g_Result.pv_i += g_Mttp[i].Reg[5] * (g_Mttp[i].I_PU_Hi+g_Mttp[i].I_PU_Lo/65536.0)/32768.0;				//pv电流
				  t = g_Mttp[i].Reg[3] * (g_Mttp[i].V_PU_Hi+g_Mttp[i].V_PU_Lo/65536.0)/32768.0;											//光伏电压	 需要算法来实现。	找最大值
					g_Result.pv_p	+= t*(g_Mttp[i].Reg[5] * (g_Mttp[i].I_PU_Hi+g_Mttp[i].I_PU_Lo/65536.0)/32768.0);		//pv功率			   				  
				  
					if(t>tmp)																					
					{
						tmp = t;
					}
			}			
		}			
		g_Result.pv_v = tmp;																		//光伏最高电压	
		
		g_Result.mppt_p = 0.0;
		g_Result.mppt_i = 0.0;
		for(i=0;i<g_configRead.alarmDelyMinute;i++)	 						//增加mppt总电流
		{			  
		 	if(g_Mttp[i].Mask&1)
			{		
					tmp = g_Mttp[i].Reg[4] * (g_Mttp[i].I_PU_Hi + g_Mttp[i].I_PU_Lo/65536.0)/32768.0;					//mppt电流
					g_Result.mppt_i += tmp;																																		//mppt总电流
				  t = g_Mttp[i].Reg[0] * (g_Mttp[i].V_PU_Hi + g_Mttp[i].V_PU_Lo/65536.0)/32768.0;						//mppt电压
					g_Result.mppt_p	+= t*tmp;																																	//mppt总功率		
					g_Result2.f_E_TotalMPPT_KWH[0] += tim/1000.0;																							//时间统计,统计方法。 mppt充电量
					
			}			
		}			
		
		g_Result.mppt_temp = 0;																	//mppt温度
		for(i=0;i<g_configRead.alarmDelyMinute;i++)	 						//mppt 算热片最高温度
		{			  
		 	if(g_Mttp[i].Mask&1)
			{						
					char abc = g_Mttp[i].Reg[11]&0xff;
				  tmp = abc; 																				//算热片温度，需要测试负值
					if(g_Result.mppt_temp<tmp)												//找最大算热片温度
					{
						g_Result.mppt_temp = tmp;
					}
			}			
		}		
		
		if(g_configRead.bCH[0]&1)			
		{
			if(g_PYLON_BAT.Mask & 0x40)
			{		
					ls_data = (unsigned short)g_PYLON_BAT.Get_System_Analog[0].ucaverage_SOC;		//
				
					g_Result.ucaverage_SOC = ls_data;
					g_Result.ftotal_current = g_PYLON_BAT.Get_System_Analog[0].ftotal_current;
					g_Result.faverage_module_voltage = g_PYLON_BAT.Get_System_Analog[0].faverage_module_voltage;
				
					if(ls_data > g_Result2.i_MaxBatSoc[0])
					{
							g_Result2.i_MaxBatSoc[0] = ls_data;
					}
					if(0==start_soc)
					{
							g_Result2.i_MinBatSOC[0] = ls_data;
							start          = 1;
					}
					else
					{
						if(ls_data < g_Result2.i_MinBatSOC[0])
						{
							g_Result2.i_MinBatSOC[0] = ls_data;
						}					
					}
			}				
		}	
		
		if(g_Rectifiter.fV_AC_Distribution_A_Phase>100)			//整流器交流输入电压
		{
				g_Result2.i_DGRunTime[0] += tim/1000.0;					//时间统计,统计方法。
				ff = g_Rectifiter.fV_DC_Distribution_Output * g_Rectifiter.fI_DC_Distribution_Total_Load * (tim/1000.0);		//整流器输出电压 *  整流器输出总电流			=  功
				g_Result2.f_E_TotalDG_KWH[0] += ff;							//总能量
		}			
		
		//g_Result.load1_ctrl		g_Result.load2_ctrl		g_Result.load3_ctrl  	g_Result.load4_ctrl
		if(g_Result.load1_ctrl && g_Result.load2_ctrl && g_Result.load3_ctrl && g_Result.load4_ctrl)
		{
			g_Result2.i_PowerAvailability[0] += tim/1000.0;					//时间统计,统计方法。
		}
		else
		{
			free_time += tim/1000.0;		
		}
		g_Result2.i_PowerAvailabilityPer[0] = (1000*g_Result2.i_PowerAvailability[0])/(g_Result2.i_PowerAvailability[0]+free_time);
		
		
		
//wait 1st come v value  if rec v 1st ,then v exist ,then can compute UI   batt_v读取成功一次就是用此值积分 如果一直没有读取成功 就不积分
		if(batt_v_Exist)
		{	
			float abc[5];
			abc[0] = g_Result.lem1_load1;		abc[1] = g_Result.lem2_load2;		abc[2] = g_Result.lem_load3;		abc[3] = g_Result.lem_load4;
			abc[4] = 0;
			for(i=0;i<4;i++)
			{
				tmp = g_Result.batt_v * abc[i];
				abc[4]+=tmp;
				if(tmp>g_Result2.f_E_MaxLoadP[0][i])
				{
					g_Result2.f_E_MaxLoadP[0][i] = tmp;
				}
				
				if(1==start)
				{					
					g_Result2.f_E_MinLoadP[0][i] = tmp;
				}
				else
				{
					if(tmp<g_Result2.f_E_MinLoadP[0][i])
					{
						g_Result2.f_E_MinLoadP[0][i] = tmp;
					}				
				}
				
				ff = tmp * (tim/1000.0);					
				g_Result2.ff_E_UIt_J_Sum[0][i] += ff;		
				g_Result2.ff_E_UIt_J_Sum[0][4] += ff;				
			}
			
			if(abc[4]>g_Result2.f_E_MaxLoadP[0][4])
			{
					g_Result2.f_E_MaxLoadP[0][4] = abc[4];
			}
			if(1==start)
			{
					start = 2;
					g_Result2.f_E_MinLoadP[0][4] = abc[4];
			}
			else
			{	
				if(abc[4]<g_Result2.f_E_MinLoadP[0][4])
				{
					g_Result2.f_E_MinLoadP[0][4] = abc[4];
				}				
			}			
		}
		
		load_time += tim/1000.0;	
		for(i=0;i<5;i++)
		{
			if(load_time>0)
			{
				g_Result2.f_E_AverLoadP[0][i] = (float)(g_Result2.ff_E_UIt_J_Sum[0][i]/load_time);
			}
		}
		
		g_Result2.f_E_NetKWH[0] = g_Result2.f_E_TotalDG_KWH[0] + g_Result2.f_E_TotalMPPT_KWH[0] - g_Result2.ff_E_UIt_J_Sum[0][4];  
		
		if(Interval_Tick - mul_Day_t0 >= 600000)										//Interval_Tick单位ms  	4294967295ms=4294967s=1193h=49.7d		24UL*3600UL*1000UL=86400000
		{
				//达到1天时间
				mul_Day_t0 = Interval_Tick;
				for(i=0;i<5;i++)																											//负载能耗
				{
					g_Result2.ff_E_UIt_J_Sum[1][i] = g_Result2.ff_E_UIt_J_Sum[0][i];					
					g_Result2.ff_E_UIt_J_Sum[0][i] = 0;																	//从0重新积分
					//muc_E_UIt_J_Sum_DayExist[i] = 1;																	//for disp save use 显示存储收到这个标志 使用天积分值  当为1则可以显示前一天的数据					
					g_Result2.f_E_MaxLoadP[1][i] = g_Result2.f_E_MaxLoadP[0][i];					
					g_Result2.f_E_MaxLoadP[0][i] = 0;						

					g_Result2.f_E_MinLoadP[1][i]   = g_Result2.f_E_MinLoadP[0][i];					
					//g_Result2.f_E_MinLoadP[0][i] = 0;			
						
					g_Result2.f_E_AverLoadP[1][i] = g_Result2.f_E_AverLoadP[0][i];
					g_Result2.f_E_AverLoadP[0][i] = 0;
					load_time = 0;
				}				
				g_Result2.f_E_TotalDG_KWH[1] = g_Result2.f_E_TotalDG_KWH[0];					//发电机产生的总功
				g_Result2.f_E_TotalDG_KWH[0] = 0.0;																		//
				
				g_Result2.f_E_TotalMPPT_KWH[1] = g_Result2.f_E_TotalMPPT_KWH[0];			//mppt的总量	
				g_Result2.f_E_TotalMPPT_KWH[0] = 0;																		
				
				g_Result2.i_DGRunTime[1] = g_Result2.i_DGRunTime[0];									//发电机工作时间
				g_Result2.i_DGRunTime[0] = 0;
				
				g_Result2.f_E_NetKWH[1]  = g_Result2.f_E_NetKWH[0];										//净能量
				g_Result2.f_E_NetKWH[0]  = 0;
				
				g_Result2.i_PowerAvailability[1] = g_Result2.i_PowerAvailability[0];	//当天电源供电正常时长，单位min
				g_Result2.i_PowerAvailability[0] = 0;
				free_time = 0;
				g_Result2.i_PowerAvailabilityPer[1] = g_Result2.i_PowerAvailabilityPer[0];
				g_Result2.i_PowerAvailabilityPer[0] = 0; 
				
				g_Result2.i_MaxBatSoc[1] = g_Result2.i_MaxBatSoc[0];									// 				
				g_Result2.i_MinBatSOC[1] = g_Result2.i_MinBatSOC[0];									// 
				g_Result2.i_MaxBatSoc[0] = ls_data;																		
				g_Result2.i_MinBatSOC[0] = ls_data;																		
		}
		Interval_Tick = systickCount;//time t0 update		
		return 1;
	}	
	return 0;
}

	

unsigned int g_Load1_tim = 0;		//单位ms
unsigned int g_Load2_tim = 0;		//单位ms
unsigned int g_Load3_tim = 0;		//单位ms
unsigned int g_Load4_tim = 0;		//单位ms

void Load1_Ctrl_Relay()
{
		static unsigned char load1_flag = 0;
	
		//unsigned char	load1_ctrl;							//负载1控制    ka1  do0  do1
		//unsigned char	load2_ctrl;							//负载2控制    ka2  do2  do3
	
		//NonEssentialLoadRelayOpenVoltage  	 	备用负载继电器开路电压  (第2路)46
		//NonEssentialLoadRelayCloseVoltage    	备用负载继电器闭合电压  (第2路) 48
		
		//EssentialLoadRelayOpenVoltage   			主负载继电器开路电压  (第1路)		46	
		//EssentialLoadRelayCloseVoltage   			主负载继电器闭合电压  (第1路)		48		
	
		if(0==load1_flag && g_Result.batt_v > g_EssentialLoadRelayCloseVoltage)
		{
				if(0==g_Result.load1_ctrl)
				{
						g_Result.load1_ctrl = 1;		//继续对负载1的供电
						DO_Output_Ctrl(0,D_ON);
						load1_flag  = 1;
						//DO0闭合 延时500ms 然后DO0断开  则接通
						g_Load1_tim = systickCount;				
				}
		}
		
		if(0==load1_flag && g_Result.batt_v > 9.0 && g_Result.batt_v < g_EssentialLoadRelayOpenVoltage)
		{
			if(g_Result.load1_ctrl)
			{
				g_Result.load1_ctrl = 0;		//断开对负载1的供电
				//DO1导通  DO0导通  -->  延时500ms  --> DO0断开   DO1断开			
				DO_Output_Ctrl(0,D_ON);
				DO_Output_Ctrl(1,D_ON);
				load1_flag  = 1;	
				g_Load1_tim = systickCount;				
			}
		}
		
		if(1==load1_flag)
		{
				g_Load1_tim = systickCount - g_Load1_tim;
				if(g_Load1_tim>500)
				{
						DO_Output_Ctrl(0,D_OFF);
						DO_Output_Ctrl(1,D_OFF);
						load1_flag = 0;
				}
		}
}

void Load2_Ctrl_Relay()
{
		static unsigned char load2_flag = 0;
	
		//unsigned char	load1_ctrl;							//负载1控制    ka1  do0  do1
		//unsigned char	load2_ctrl;							//负载2控制    ka2  do2  do3
	
		//NonEssentialLoadRelayOpenVoltage  	 	备用负载继电器开路电压  (第2路)46
		//NonEssentialLoadRelayCloseVoltage    	备用负载继电器闭合电压  (第2路) 48
		
		//EssentialLoadRelayOpenVoltage   			主负载继电器开路电压  (第1路)		46	
		//EssentialLoadRelayCloseVoltage   			主负载继电器闭合电压  (第1路)		48		
	
		if(0==load2_flag && g_Result.batt_v > g_NonEssentialLoadRelayCloseVoltage)
		{
				if(0==g_Result.load2_ctrl)
				{
					g_Result.load2_ctrl = 1;		//继续对负载2的供电
					DO_Output_Ctrl(2,D_ON);
					load2_flag  = 1;
					//DO2闭合 延时500ms 然后DO2断开  则接通
					g_Load2_tim = systickCount;				
				}
		}
		
		if(0==load2_flag && g_Result.batt_v > 9.0 && g_Result.batt_v < g_NonEssentialLoadRelayOpenVoltage)
		{				
				//DO3导通  DO2导通  -->  延时500ms  --> DO2断开   DO3断开
				if(g_Result.load2_ctrl)
				{
					g_Result.load2_ctrl = 0;		//断开对负载2的供电				
					DO_Output_Ctrl(2,D_ON);
					DO_Output_Ctrl(3,D_ON);
					load2_flag  = 1;	
					g_Load2_tim = systickCount;				
				}
		}
		
		if(1==load2_flag)
		{
				g_Load2_tim = systickCount - g_Load2_tim;
				if(g_Load2_tim>500)
				{
						DO_Output_Ctrl(2,D_OFF);
						DO_Output_Ctrl(3,D_OFF);
						load2_flag = 0;
				}
		}
}

void Load3_Ctrl_Relay()
{
		static unsigned char load3_flag = 0;
	
		//unsigned char	load1_ctrl;							//负载1控制    ka1  do0  do1
		//unsigned char	load2_ctrl;							//负载2控制    ka2  do2  do3
	
		//NonEssentialLoadRelayOpenVoltage  	 	备用负载继电器开路电压  (第2路)46
		//NonEssentialLoadRelayCloseVoltage    	备用负载继电器闭合电压  (第2路) 48
		
		//EssentialLoadRelayOpenVoltage   			主负载继电器开路电压  (第1路)		46	
		//EssentialLoadRelayCloseVoltage   			主负载继电器闭合电压  (第1路)		48		
	
		if(0==load3_flag && g_Result.batt_v > g_EssentialLoadRelayCloseVoltage_Sec)
		{
				if(0==g_Result.load3_ctrl)
				{
					g_Result.load3_ctrl = 1;				//继续对负载2的供电
					DO_Output_Ctrl(4,D_ON);
					load3_flag  = 1;
					//DO2闭合 延时500ms 然后DO2断开  则接通
					g_Load3_tim = systickCount;				
				}
		}
		
		if(0==load3_flag && g_Result.batt_v > 9.0 && g_Result.batt_v < g_EssentialLoadRelayOpenVoltage_Sec)
		{				
				//DO3导通  DO2导通  -->  延时500ms  --> DO2断开   DO3断开
				if(g_Result.load3_ctrl)
				{
					g_Result.load3_ctrl = 0;		//断开对负载3的供电				
					DO_Output_Ctrl(4,D_ON);
					DO_Output_Ctrl(5,D_ON);
					load3_flag  = 1;	
					g_Load3_tim = systickCount;				
				}
		}
		
		if(1==load3_flag)
		{
				g_Load3_tim = systickCount - g_Load3_tim;
				if(g_Load3_tim>500)
				{
						DO_Output_Ctrl(4,D_OFF);
						DO_Output_Ctrl(5,D_OFF);
						load3_flag = 0;
				}
		}
}

void Load4_Ctrl_Relay()
{
		static unsigned char load4_flag = 0;
	
		//unsigned char	load1_ctrl;							//负载1控制    ka1  do0  do1
		//unsigned char	load2_ctrl;							//负载2控制    ka2  do2  do3
	
		//NonEssentialLoadRelayOpenVoltage  	 	备用负载继电器开路电压  (第2路)46
		//NonEssentialLoadRelayCloseVoltage    	备用负载继电器闭合电压  (第2路) 48
		
		//EssentialLoadRelayOpenVoltage   			主负载继电器开路电压  (第1路)		46	
		//EssentialLoadRelayCloseVoltage   			主负载继电器闭合电压  (第1路)		48		
	
		if(0==load4_flag && g_Result.batt_v > g_NonEssentialLoadRelayCloseVoltage_Sec)
		{
				if(0==g_Result.load4_ctrl)
				{
					g_Result.load4_ctrl = 1;		//继续对负载2的供电
					DO_Output_Ctrl(6,D_ON);
					load4_flag  = 1;
					//DO2闭合 延时500ms 然后DO2断开  则接通
					g_Load4_tim = systickCount;				
				}
		}
		
		if(0==load4_flag && g_Result.batt_v > 9.0 && g_Result.batt_v < g_NonEssentialLoadRelayOpenVoltage_Sec)
		{				
				//DO3导通  DO2导通  -->  延时500ms  --> DO2断开   DO3断开
				if(g_Result.load4_ctrl)
				{
					g_Result.load4_ctrl = 0;		//断开对负载2的供电				
					DO_Output_Ctrl(6,D_ON);
					DO_Output_Ctrl(7,D_ON);
					load4_flag  = 1;	
					g_Load4_tim = systickCount;				
				}
		}
		
		if(1==load4_flag)
		{
				g_Load4_tim = systickCount - g_Load4_tim;
				if(g_Load4_tim>500)
				{
						DO_Output_Ctrl(6,D_OFF);
						DO_Output_Ctrl(7,D_OFF);
						load4_flag = 0;
				}
		}
}


void Temp_Ctrl_Fan(void)
{
		//unsigned char  fan_ctrl;							//风扇控制		 	ka3  do4
		//EnclosureFanTurnOnTemperature	  			外壳风扇打开温度			40
		//EnclosureFanTurnOffTemperature				外壳风扇断开温度			35
		if(g_Result.mppt_temp > g_EnclosureFanTurnOnTemperature)
		{
				DO_Output_Ctrl(4,D_ON);
				g_Result.fan_ctrl = 1;
		}
		
		if(g_Result.mppt_temp < g_EnclosureFanTurnOffTemperature)
		{
				DO_Output_Ctrl(4,D_OFF);
				g_Result.fan_ctrl = 0;
		}	
}



//后备电机控制逻辑
void BackMotor_Ctrl(void)
{
		//unsigned char  reserve_motor_ctrl;		//后备电机控制 					ka4  do5
		//LowBatteryVoltageStartGenerator				电池电压低启动发电机		46.0
		//HighBatteryVoltageStopGenerator				电池电压高停止发电机		58.0		
		if(g_Result.batt_v >9.0 &&  g_Result.batt_v < g_LowBatteryVoltageStartGenerator)		//电池电压 过低 则停止
		{
				g_Result.reserve_motor_ctrl = 1;   		//表示发电机已经打开
			  DO_Output_Ctrl(5,D_ON);
		}		
		
		if(g_Result.batt_v > g_HighBatteryVoltageStopGenerator || g_Result.pv_v > g_HighPVVoltageStopGenerator)			//电池已经充电具备一定的电压  则关闭发电机，当pv的电压比较高的时候也停止后备电机
		{
				DO_Output_Ctrl(5,D_OFF);
				g_Result.reserve_motor_ctrl = 0;   		//表示发电机已经停止
		}
}


extern unsigned char g_Modbus_Send_Flag;		//命令是否已发送

/*!
    \brief      main routine
    \param[in]  none
    \param[out] none 
    \retval     none
*/
//  signed short int g_iwendu=0;
//unsigned short int g_ishidu=0;
int g_success = 0;
int g_fail = 0;
unsigned int sht_tick = 0;
int GetTH_SHT3X()
{
		etError   		error = NO_ERROR;       // error code 
		float        	temperature=0; 					// temperature [у]
		float        	humidity=0;    					// relative humidity [%RH]
		char data[32];	
		int						i = 0;
		if(g_sysTick - sht_tick > 4)					//5秒采集1次
		{
			sht_tick = g_sysTick;
			SHT3X_Init(0x44); 		// Address: 0x44 = Sensor on EvalBoard connector 初始化
			wdt();
			error = SHT3X_GetTempAndHumi(&temperature, &humidity, REPEATAB_MEDIUM, MODE_CLKSTRETCH, 100);		//REPEATAB_HIGH=15ms 	REPEATAB_MEDIUM=6ms   REPEATAB_LOW=4ms  60=3.3ms
			wdt();
			//当传感器不存在，则不成功读1次为1ms,合计2次，2ms
			if(error==NO_ERROR)	
			{
				g_success++;
			}
			else			//如果不成功，在读1次。
			{
				error = SHT3X_GetTempAndHumi(&temperature, &humidity, REPEATAB_MEDIUM, MODE_CLKSTRETCH, 100);		//REPEATAB_HIGH=15ms 	REPEATAB_MEDIUM=6ms   REPEATAB_LOW=4ms  延时参数1=100us.
				wdt();
				if(error==NO_ERROR)
				{
						g_success++;
				}
				else
				{		
						Adc_Data[4] = 0x8000;		
						Adc_Data[5] = 0xffff;
						g_fail++;
				}
			}		
			
			if(error==NO_ERROR)
			{
					Adc_Data[4] = (short int)(temperature*10.0);				
					Adc_Data[5] = (short int)(humidity*10.0);	  //[0 1000]
					if(Adc_Data[5]<0 || Adc_Data[5]>1000)
					{
						Adc_Data[5] = 0;
					}
			}
			g_Result.main_carbin_T = Adc_Data[4];
			g_Result.main_carbin_H = Adc_Data[5];
			
			//i = sprintf(data,"wd:%d sd:%d\r\n",Adc_Data[4],Adc_Data[5]);			//调试信息
		  //Com_Send(USB,(unsigned char*)data,i);	
		}		
		return Adc_Data[4];								//温度																						
}

unsigned char EtherNet_Rev_Timeout(unsigned char ch)
{
		unsigned int tmpTick = systickCount;
		unsigned int ComX_Tick = 0;
		unsigned int Res=0;
		//unsigned short TickDelay = 0;
		
		ComX_Tick = g_UDP_usTick[ch];							//最后收到数据的时间  中断
		//TickDelay = Com.Usart[ch].usDelay_Rev;		
		Res = tmpTick - ComX_Tick;								//最后1次读到的数据到目前的时间间隔
		if(Res>9)
		{
				return 1;															//超时，则认为一包完整的数据，可以转发
		}
		else
		{
				return 0;
		}		
}
//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲
//unsigned char get_sw_timeout(unsigned char ch,unsigned char mode)
//{
//	unsigned int tmpTick = 0;
//	if(0==Sw_com[ch].ucMode)
//	{
//			Sw_com[ch].ucMode = mode;	
//			return 1;
//	}
//	else
//	{
//			tmpTick = systickCount;
//			tmpTick = tmpTick - Sw_com[ch].uiTick[Sw_com[ch].ucMode-1];								//最后1次发送完成数据后到现在的时间间隔,ucMode表示最后一次发送数据源是谁。
//			if(tmpTick>Sw_com[ch].usDelay_Rev)				//超时
//			{
//				Sw_com[ch].ucMode = mode;	
//				return 1;
//			}
//			else
//			{
//				return 0;
//			}
//	}
//}

void EtherNet_Tran2Com(int i)
{
		//unsigned char i = 0;
	 		
		//for(i=0;i<4;i++)
		if(i>-1 && i<4)
		{				
			if(g_wr_UDP[i]!=g_rd_UDP[i])																							//有数据接收到了。
			{			
				unsigned char RX_Analy_Buf[D_USART_REC_BUFF_SIZE];
				unsigned short int usRX_Analy_Buf_Len = 0;
				//if(1==EtherNet_Rev_Timeout(i))																					//接收到新的数据并且接收超时，则考虑转发
				{
					wdt();
					usRX_Analy_Buf_Len = 0;
					while(g_wr_UDP[i]!=g_rd_UDP[i] && usRX_Analy_Buf_Len < D_USART_REC_BUFF_SIZE)
					{				
						RX_Analy_Buf[usRX_Analy_Buf_Len] = g_rxbuf_UDP[i][g_rd_UDP[i]];									
						g_rd_UDP[i] = (g_rd_UDP[i]+1)%D_USART_REC_BUFF_SIZE;
						usRX_Analy_Buf_Len++;
					}
					if(usRX_Analy_Buf_Len>0)
					{
						#if 0
						if(0==i)
						{	
							if(get_sw_timeout(i,3))										//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲     成功则进行新的转发类型
							{
								Com.Usart[g_com_port[i]].usRec_RD = Com.Usart[g_com_port[i]].usRec_WR;											//清空接受缓存 准备开始新的1包数据的解析
								Com_Send(RS485_2, RX_Analy_Buf, usRX_Analy_Buf_Len);		//RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7
								Sw_com[i].uiTick[2] = systickCount;		//以太网向串口转发时刻
								//记录发送时刻和发送类型，当收到数据进行转发的时候
							}
							//否则直接放弃本次数据转发
						}
						else if(1==i)
						{
							if(get_sw_timeout(i,3))									//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲		成功则进行新的转发类型
							{
								Com.Usart[g_com_port[i]].usRec_RD = Com.Usart[g_com_port[i]].usRec_WR;											//清空接受缓存 准备开始新的1包数据的解析
								Com_Send(CH432T_1, RX_Analy_Buf, usRX_Analy_Buf_Len);		//RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7
								Sw_com[i].uiTick[2] = systickCount;		//以太网向串口转发时刻
								//记录发送时刻和发送类型，当收到数据进行转发的时候
							}
							//否则直接放弃本次数据转发
						}
						else if(2==i)
						{
							if(get_sw_timeout(i,3))									//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲		成功则进行新的转发类型
							{
								Com.Usart[g_com_port[i]].usRec_RD = Com.Usart[g_com_port[i]].usRec_WR;											//清空接受缓存 准备开始新的1包数据的解析
								Com_Send(RS485_4, RX_Analy_Buf, usRX_Analy_Buf_Len);		//RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7
								Sw_com[i].uiTick[2] = systickCount;		//以太网向串口转发时刻
								//记录发送时刻和发送类型，当收到数据进行转发的时候
							}
							//否则直接放弃本次数据转发
						}
						else if(3==i)
						{
							if(get_sw_timeout(i,3))									//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲		成功则进行新的转发类型
							{
								Com.Usart[g_com_port[i]].usRec_RD = Com.Usart[g_com_port[i]].usRec_WR;											//清空接受缓存 准备开始新的1包数据的解析
								Com_Send(CH432T_2, RX_Analy_Buf, usRX_Analy_Buf_Len);								//RS485_2	1=4;	CH432T_1 7=5;		RS485_4	3=6	;	CH432T_2 8 =7
								Sw_com[i].uiTick[2] = systickCount;		//以太网向串口转发时刻
								//记录发送时刻和发送类型，当收到数据进行转发的时候
							}
							//否则直接放弃本次数据转发
						}	
						#else
						if(get_sw_timeout(i,3))																																					//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲		成功则进行新的转发类型
						{
								Com.Usart[g_com_port[i]].usRec_RD = Com.Usart[g_com_port[i]].usRec_WR;											//清空接受缓存 准备开始新的1包数据的解析
								Com_Send(g_com_port[i], RX_Analy_Buf, usRX_Analy_Buf_Len);																	//RS485_2	1=4;	CH432T_1 7=5;		RS485_4	3=6	;	CH432T_2 8 =7
								Sw_com[i].uiTick[2] = systickCount;		//以太网向串口转发时刻
								//记录发送时刻和发送类型，当收到数据进行转发的时候
						}
						#endif
					}			
				}	
			}
		}
}
/**********************************************************************
  * @ 函数名  ： LED_Task
  * @ 功能说明： LED_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
void EtherNet_P()
{	 			
			if(1==g_configRead.b_rj45_work)		
			{	
						int i = 0;						
						W5500_Socket_Set();		wdt();									//W5500׋ࠚԵʼۯƤ׃										
						if(W5500_Interrupt)														//中断发送
						{						
								W5500_Interrupt_Process();								//W5500א׏ԦmԌѲ࠲ݜ					
								//if(((S0_Data & S_RECEIVE) == S_RECEIVE) || ((S1_Data & S_RECEIVE) == S_RECEIVE) || ((S2_Data & S_RECEIVE) == S_RECEIVE))	//ɧڻSocket0ޓ˕ս˽ߝ
								if((S_Data[0] & S_RECEIVE) == S_RECEIVE)
								{				
										S_Data[0]&=~S_RECEIVE;
										Process_Socket_Data(0);		
								}							
								#if RJ45_TCPSERVER_S1			
								if((S_Data[1] & S_RECEIVE) == S_RECEIVE)
								{				
									S_Data[1]&=~S_RECEIVE;
									Process_Socket_Data(1);											//W5500ޓ˕Ңע̍ޓ˕սք˽ߝ
								}
								#endif			
								if((S_Data[2] & S_RECEIVE) == S_RECEIVE)			//8888端口出路  udp  本地 reg读写
								{				
									S_Data[2]&=~S_RECEIVE;
									Process_Socket_Data(2);		
									//g_rxbuf[g_wr]数据进行处理	
									if(g_wr>0)
									{
										for(i=0;i<g_wr;i++)
										{
											Com.RX_Analy_Buf[i] = g_rxbuf[i];				//#define D_USART_REC_BUFF_SIZE				1024        #define MAXRecvBuf 		1024    						//changed by wjj 30->128
										}
										Com.usRX_Analy_Buf_Len = g_wr;									
										Local_RTU(0xff);
									}
								}		
								
								if((S_Data[3] & S_RECEIVE) == S_RECEIVE)			//收到数据  snmp处理
								{				
										S_Data[3]&=~S_RECEIVE;
										SnmpXDaemon();		wdt();												//Process_Socket_Data(3);			
								}		
								
								if((S_Data[4] & S_RECEIVE) == S_RECEIVE)
								{				
									S_Data[4]&=~S_RECEIVE;
									Process_Socket_Data(4);			
									EtherNet_Tran2Com(0);													//8889  8890 8891 8892 4个通道的数据转发到四个rs485通道上。					
								}	
								
								if((S_Data[5] & S_RECEIVE) == S_RECEIVE)
								{				
									S_Data[5]&=~S_RECEIVE;
									Process_Socket_Data(5);			
									EtherNet_Tran2Com(1);													//8889  8890 8891 8892 4个通道的数据转发到四个rs485通道上。					
								}	
								
								if((S_Data[6] & S_RECEIVE) == S_RECEIVE)
								{				
									S_Data[6]&=~S_RECEIVE;
									Process_Socket_Data(6);			
									EtherNet_Tran2Com(2);													//8889  8890 8891 8892 4个通道的数据转发到四个rs485通道上。					
								}	
								
								if((S_Data[7] & S_RECEIVE) == S_RECEIVE)
								{				
									S_Data[7]&=~S_RECEIVE;
									Process_Socket_Data(7);			
									EtherNet_Tran2Com(3);													//8889  8890 8891 8892 4个通道的数据转发到四个rs485通道上。					
								}									
						}	
//						if(last_event&0x01)					//通道1待发数据
//						{
//								Chan_Send(0);
//								last_event &= ~(1<<0);
//						}
//						if(last_event&0x02)					//通道2待发数据
//						{
//								Chan_Send(1);
//								last_event &= ~(1<<1);
//						}
//						if(last_event&0x04)					//通道3待发数据
//						{
//								Chan_Send(2);
//								last_event &= ~(1<<2);
//						}
			}		
			
  
}

//ch通道mode模式下是否还在等待中，即没有操作完，正在操作中  =1； 0=表示空闲
unsigned char get_sw_timein(unsigned char ch,unsigned char mode)
{
	unsigned int tmpTick = 0;	
	#if 0
	if(0==Sw_com[ch].ucFlag || 0==Sw_com[ch].ucMode)															//空闲模式,直接返回
	{			
			return 0;
	}
	else if(Sw_com[ch].ucMode==mode)																							//非空闲，进一步查看是否在mode模式下工作，如果是，看是否已经超时
	{
			tmpTick = systickCount;
			tmpTick = tmpTick - Sw_com[ch].uiTick[Sw_com[ch].ucMode-1];								//最后1次发送完成数据后到现在的时间间隔,ucMode表示最后一次发送数据源是谁。
			if(tmpTick>Sw_com[ch].usDelay_Rev)																				//超时 但别人可以使用	 即空闲模式下
			{				
				return 0;
			}
			else
			{
				return 1;														//还在等待中
			}
	}
	else																			//不在mode模式下工作，
	{
			return 0;
	}
	#else
	if(Sw_com[ch].ucMode==mode)																										//某通道，某模式下，是否还需要等待操作
	{
			tmpTick = systickCount;
			tmpTick = tmpTick - Sw_com[ch].uiTick[mode-1];								
			if(tmpTick<Sw_com[ch].usDelay_Rev)																				//还在等待中,没有超时
			{				
				return 1;
			}			
	}
	return 0;
	#endif
}

static void Main_Task(void* parameter)
{					
		unsigned int  Ads1247_Tick=0,Ads1247_Cnt=0,Ads1247_Timeout=0;			//Ads1247_Timer=100,
		unsigned char Ads1247_Ch = 0,Ads1247_Ch_Pre=0;
		unsigned int  can_tick = 0;
		unsigned int  sec_tick = 0;
		unsigned char Master_sta = RS485;		
	  unsigned char Ch432t_int_sta = 0;
		ch432t_Init();	wdt();
		RTC_Init();			wdt();
			 //通过跳线来控制
			 g_Mttp[0].ComPort											 = 0x31;		//MPPT的转发通讯口
			 g_PYLON_BAT.ComPort[D_PYLON_BAT_SETS_1] = 0x39;		//电池组1的转发通讯口				
			 g_Rectifiter.ComPort 									 = 0x33;		//整流器的转发通讯口
			 g_Inverter.ComPort				               = 0x33;		//逆变器转发通讯口
			 g_Air.ComPort				                 	 = 0x33;		//空调的转发通讯口			 
			 g_Fan.ComPort				              		 = 0x33;		//风扇转发通讯口  
			 g_Diesel_Generator.ComPort              = 0x33;		//柴油发电机转发通讯口																															
			 g_PYLON_BAT.ComPort[D_PYLON_BAT_SETS_2] = 0x3a;		//电池组2的转发通讯口
			 
			 g_Mttp[0].modbus_id										 = 0x41;		//mppt的modbus id		//g_Mttp.modbus_id				 0x41;	或  1
			 g_Air.modbus_id				                 = 0x01;		//空调modbus id			 
			 g_Inverter.modbus_id				             = 0x02;		//逆变器modbus id			 
			 g_Fan.modbus_id				            		 = 0x03;		//风扇modbus id
			 g_Diesel_Generator.modbus_id            = 0x0A;    //柴油发电机modbus id			
			 
	#if 1
		//++++++++++++++++++++++++++++++++++++++++ usb device configure+++++++++++++++++++++++++++++++++++++++++++++++++++
		if(g_can_bus==1)
		{
			/* initialize CAN and filter */
			can_gpio_config();			wdt();
			can_config();     		 	wdt();                       		
		}
		else
		{
			/* system clocks configuration */
			rcu_config();			wdt();

			/* GPIO configuration */
			gpio_config();		wdt();

			/* USB device configuration */
			usbd_init(&usbd_cdc, &cdc_desc, &cdc_class);		wdt();

			/* NVIC configuration */
			nvic_config();			wdt();
	
			/* enabled USB pull-up */
			usbd_connect(&usbd_cdc);		wdt();
		}
		
		//RS485 		= 0,	//rs485 master
    //RS485_2 	= 1,  //rs485
		//WIFI_BLE 	= 3,	//wifi & ble
		//RS485_4 	= 4,  //rs485
	  //METER 		= 5,	//meter
		//G4				= 6		//4g
		//+++++++++++++++++++++++++++++configure usart2 usb->ch340->usart2 +++++++++++++++++++++++++++++++++++++++++++++		
		rcu_periph_clock_enable(RCU_AF);			wdt();
		gd_eval_com_init(RS485,				9600,			0,1);	wdt();	//rs485
		gd_eval_com_init(RS485_2,			9600,			0,2);	wdt();	//rs485		2
		gd_eval_com_init(WIFI_BLE,		115200,		0,3);	wdt();	//wifi&ble
    gd_eval_com_init(RS485_4,			9600,			0,4);	wdt();	//rs485		4
		gd_eval_com_init(METER,				9600,			0,5);	wdt();	//meter
		gd_eval_com_init(G4,					115200,		0,6);	wdt();	//meter
		//Master_sta=Sw_Sta[1];				//dip1 默认rs485作为主口	=0;		
		#endif
		if(0==g_configRead.wifi_mode)
		{
			COM_MASTER = RS485;
		}
		else if( g_configRead.wifi_mode<=COMn)
		{
			COM_MASTER = g_configRead.wifi_mode-1;
		}
		else
		{
			COM_MASTER = RS485;
		}
		wdt();
		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ W5500 +++++++++++++++++++++++++++++++++++++++++++++++		  
		printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
		printf("modbus id: %d\r\n",gs_SaveNetIPCfg.g_ModbusID);
		printf("dhcp    : %d\r\n", gs_SaveNetIPCfg.ucDhcpMode[0]);
		printf("mac			: %02x-%02x-%02x-%02x-%02x-%02x\r\n",gs_SaveNetIPCfg.ucMAC[0],gs_SaveNetIPCfg.ucMAC[1],gs_SaveNetIPCfg.ucMAC[2],gs_SaveNetIPCfg.ucMAC[3],gs_SaveNetIPCfg.ucMAC[4],gs_SaveNetIPCfg.ucMAC[5]);
		printf("ip 			: %d.%d.%d.%d\r\n",gs_SaveNetIPCfg.ucSelfIP[0],gs_SaveNetIPCfg.ucSelfIP[1],gs_SaveNetIPCfg.ucSelfIP[2],gs_SaveNetIPCfg.ucSelfIP[3]);
		printf("submask	: %d.%d.%d.%d\r\n",gs_SaveNetIPCfg.ucSubMASK[0],gs_SaveNetIPCfg.ucSubMASK[1],gs_SaveNetIPCfg.ucSubMASK[2],gs_SaveNetIPCfg.ucSubMASK[3]);
		printf("gateway	: %d.%d.%d.%d\r\n",gs_SaveNetIPCfg.ucGateWay[0],gs_SaveNetIPCfg.ucGateWay[1],gs_SaveNetIPCfg.ucGateWay[2],gs_SaveNetIPCfg.ucGateWay[3]);
		wdt();
		printf("tcp server port	: %d\r\n",gs_SaveNetIPCfg.ucMonitorPort[0]<<8|gs_SaveNetIPCfg.ucMonitorPort[1]);		
		printf("tcp client mode.	Dest ip: %d.%d.%d.%d ,  Dest port: %d , Local port:%d \r\n",gs_SaveNetIPCfg.ucDestIP[0],gs_SaveNetIPCfg.ucDestIP[1],gs_SaveNetIPCfg.ucDestIP[2],gs_SaveNetIPCfg.ucDestIP[3],gs_SaveNetIPCfg.ucDestPort[0]<<8|gs_SaveNetIPCfg.ucDestPort[1],gs_SaveNetIPCfg.ucSourcePort[0]<<8|gs_SaveNetIPCfg.ucSourcePort[1]);
		printf("udp mode.     	  Dest ip: %d.%d.%d.%d ,  Dest port: %d , Local port:%d \r\n",gs_SaveNetIPCfg.ucUdpDestIP[0],gs_SaveNetIPCfg.ucUdpDestIP[1],gs_SaveNetIPCfg.ucUdpDestIP[2],gs_SaveNetIPCfg.ucUdpDestIP[3],gs_SaveNetIPCfg.ucUdpDestPort[0]<<8|gs_SaveNetIPCfg.ucUdpDestPort[1],gs_SaveNetIPCfg.ucUdpSourcePort[0]<<8|gs_SaveNetIPCfg.ucUdpSourcePort[1]);
		printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");
		wdt();
		print_base_info();					wdt();	
		//RTC_Test();
		while (1)
		{				
			if(g_sysTick%2)
			{			
				Led_Ctrl(1,false);		//GPIO_ResetBits(D_LED2_CTRL_PORT,D_LED2_CTRL);						
			}
			else
			{			
				Led_Ctrl(1,true);			//GPIO_SetBits(D_LED2_CTRL_PORT,D_LED2_CTRL);				
			}
			wdt();
			if(sec_tick!=g_sysTick)
			{
				sec_tick = g_sysTick;
				ADC_Cycle_Detect_Process();		wdt();
				Ad_software_trigger();				wdt();
			}
						
      Read_DI_Input_State();			wdt();		//di  		0x00
			Do_Get_Sta();								wdt();		//do			0x10		
			Read_SW_Input_State();			wdt();		//switch	0x20
		//	Get_Android_Sta();					wdt();		//android	0x30
			
			Ads1247_Timeout = systickCount - Ads1247_Tick;
			if(Ads1247_Timeout>99)								//ms
			{
				wdt();
				if(Irq_Ads1247_Ready)  
				{				
					Irq_Ads1247_Ready = 0;						//等待下次
					Ads1247_Ch_Pre = Ads1247_Ch;
					Ads1247_Ch = (Ads1247_Ch+1)%8;		//切换下一个通道
					
					AI_Channel_Select(Ads1247_Ch);		//AI_Channel_Select(Ads1247_Ch);
					Read_AI_Data(Ads1247_Ch_Pre);			//读上次的数据 Ads1247_Ch_Pre
					wdt();
					Ads1247_Cnt++;										//合计采集的次数
					Read_Ads1247_Mode(D_RDATA);				//启动单次读				
				}
				else
				{
					Ads1247_Ch_Pre = Ads1247_Ch;
					AI_Channel_Select(Ads1247_Ch);		//Ads1247_Ch
					Read_Ads1247_Mode(D_RDATA);		
				}
				Ads1247_Tick = systickCount;
				
				Ch432t_int_sta = gpio_input_bit_get(D_ANDROID_POWER_PORT,D_ANDROID_POWER_CTRL); 		//低电平打开电源,默认上拉
				if(0==Ch432t_int_sta)
				{
					wdt();
					CH432_Int_Process();
					wdt();
				}
			}		
		
		if(g_configRead.save_frq&0x01)
		{
				GetTH_SHT3X();		wdt();				//5秒获取1次		Adc_Data[4]温度 掉线  0x8000  short int  Adc_Data[5]湿度 掉线 0xffff  short int			
		}
		else 												//测试用
		{
				g_Result.main_carbin_T = 250;
				g_Result.main_carbin_H = 500;
				g_Result.second_carbin_T = 251;
				g_Result.second_carbin_H = 501;
		}
			
		RTU_Process();													wdt();			//接收到的COM_MASTER数据进行分析实现转发或本地控制处理
		
		//if(0==Sw_Sta[5] && 0==g_configRead.b_light_work)		//当背光灯灭并且dip1=0则表示默认透传模式 否则主动工作模式
		{
			//Com2Com(RS485,   		COM_MASTER);			wdt();			//接收到的COM1数据 转发到主口的通讯处理		
							
			if(2==Sw_com[0].ucMode || 3==Sw_com[0].ucMode)		//if(0==get_sw_timein(0,1))													//本地模式处于空闲状态，则可以进行转发尝试
			{
				Com2Com(RS485_2,  COM_MASTER);			wdt();			//接收到的COM2数据 转发到主口的通讯处理			  mppt可能在本地进行数据读写		
			}
			
			Com2Com(WIFI_BLE,  	COM_MASTER);			wdt();			//接收到的COM3数据 转发到主口的通讯处理   	ble和wifi				
			
			if(2==Sw_com[2].ucMode || 3==Sw_com[2].ucMode)		//if(0==get_sw_timein(2,1))														//本地模式处于空闲状态，则可以进行转发尝试
			{
				Com2Com(RS485_4,  	COM_MASTER);			wdt();			//接收到的COM4数据 转发到主口的通讯处理  		实现逆变器数据的读取
			}
			
			Com2Com(METER,   		COM_MASTER);			wdt();			//接收到的COM5数据 转发到主口的通讯处理
			Com2Com(G4,   			COM_MASTER);			wdt();			//接收到的COM6数据 转发到主口的通讯处理
			Com2Com(USB,   			COM_MASTER);			wdt();			//接收到的usb数据 转发到主口的通讯处理			
			
			if(2==Sw_com[1].ucMode || 3==Sw_com[1].ucMode)		//if(0==get_sw_timein(1,1))														//本地模式处于空闲状态，则可以进行转发尝试
			{				
				Com2Com(CH432T_1,   COM_MASTER);			wdt();			//接收到的COM7数据 转发到主口的通讯处理						
			}
			
			if(2==Sw_com[3].ucMode || 3==Sw_com[3].ucMode)		//if(0==get_sw_timein(3,1))														//本地模式处于空闲状态，则可以进行转发尝试
			{				
				Com2Com(CH432T_2,   COM_MASTER);			wdt();			//接收到的COM8数据 转发到主口的通讯处理				
			}				
		}
//		else
//		{
//			if(2==g_Modbus_Send_Flag)
//			{					
//					Com2Com(RS485_2,   	COM_MASTER);			wdt();						//接收到的COM1数据 转发到主口的通讯处理					
//			}			
//			Get_Mppt_Data();			 wdt();  		//com2 	9600  rs485					数据接收 mppt数据读取
//			if(Data_Tran())					   				//数据处理过程并形成结果   	g_Result  
//			{
//				BackMotor_Ctrl();								//后备电机控制
//				Temp_Ctrl_Fan();								//温度控制风扇
//				Load1_Ctrl_Relay();							//负载1控制
//				Load2_Ctrl_Relay();							//负载2控制
//			}			
//		}
		if(1==g_configRead.b_debug_work)
		{
			if(g_configRead.collect_frq>999)
			{
				wdt();		get_all_device_info();		wdt();
			}
		}
		else
		{
			Get_Mppt_Data();			 wdt();  		//com2 	9600  rs485					数据接收 mppt数据读取		
		}
		if(Data_Tran())					   				//数据处理过程并形成结果   	g_Result  
		{
			//BackMotor_Ctrl();							//后备电机控制
			//Temp_Ctrl_Fan();							//温度控制风扇
			Load1_Ctrl_Relay();				wdt();			//负载1控制
			Load2_Ctrl_Relay();				wdt();			//负载2控制
			Load3_Ctrl_Relay();				wdt();			//负载3控制
			Load4_Ctrl_Relay();				wdt();			//负载4控制
		}			
		
		#if 0
		if(Master_sta && m_gprsinfo.g_bGPRSConnected && 1==g_configRead.b_gprs_work)		//4g模式  4g处于连接状态
		{			
			Com2Com(RS485,   COM_MASTER);				wdt();		//com1通讯处理
			Com2Com(RS485_2, COM_MASTER);				wdt();		//com1通讯处理
			if(1==g_configRead.b_wifi_work)
			{
				Com2Com(WIFI_BLE,COM_MASTER);			wdt();		//com1通讯处理
			}
			Com2Com(RS485_4, COM_MASTER);				wdt();		//com1通讯处理
			Com2Com(METER,   COM_MASTER);				wdt();		//com1通讯处理			
		}
		else
		{			
			#if 0
			Com2Com(RS485_2,COM_MASTER);			wdt();		//com1通讯处理
			if(1==g_configRead.b_wifi_work)
			{
					Com2Com(WIFI_BLE,COM_MASTER);			wdt();		//com1通讯处理
			}
			#else			
			//调试wifi模块，com2与wifi串口透传
			if(1==g_configRead.b_wifi_work)
			{
					Com2Com(RS485_2,WIFI_BLE);				wdt();		//com1通讯处理
					Com2Com(WIFI_BLE,RS485_2);				wdt();		//com1通讯处理
			}
			#endif
			
			Com2Com(RS485_4,COM_MASTER);			wdt();		//com1通讯处理
			Com2Com(METER,COM_MASTER);				wdt();		//com1通讯处理
			if(m_gprsinfo.g_bGPRSConnected && 1==g_configRead.b_gprs_work)							//gprs假设处于连接状态
			{
					Com2Com(G4,COM_MASTER);				wdt();			//com1通讯处理	
			}
		}
		#endif
		
		Clear_Uart_Buffer();							wdt();			//连续100ms没有收到任何数据则清空缓存,可能数据发送一直不成功 只有主口对接收的数据进行格式判断。
		Com1_Boot();											wdt();			//程序升级
		EtherNet_P();											wdt();						
			#if 0
				//读dip
				bit_status1 = (~(gpio_input_port_get(GPIOE)>>10))&0x3f;
				//bit_status2 = gpio_input_port_get(GPIOC);
				dip_sta  = ((bit_status1>>5)&0x1) | ((bit_status1>>3)&0x2) | ((bit_status1>>1)&0x4) | ((bit_status1<<5)&0x20) | ((bit_status1<<3)&0x10) | ((bit_status1<<1)&0x8);				
				//printf("\r\n 8bit dips = 0x%x %d %d %d %d %d %d %d %d \r\n",dip_sta,(dip_sta>>0)&0x1,(dip_sta>>1)&0x1,(dip_sta>>2)&0x1,(dip_sta)>>3&0x1,(dip_sta>>4)&0x1,(dip_sta>>5)&0x1,(dip_sta>>6)&0x1,(dip_sta>>7)&0x1);		
				//printf("\r\n 8bit dips = 0x%x %d \r\n",dip_status,(dip_status>>0)&0x1,(dip_status>>1)&0x1,(dip_status>>2)&0x1,(dip_status)>>3&0x1,(dip_status>>4)&0x1,(dip_status>>5)&0x1,(dip_status>>6)&0x1,(dip_status>>7)&0x1,);		
				
				if(dip_sta&0x1)
				{
						uart_can_test(RS485);					//跳线短路则进入透明传输模式  即将usb通道的数据和wifi&ble通道数据进行相互转换
				}
				else if(dip_sta&0x2)
				{
						uart_test(RS485_2);				//跳线短路则进入透明传输模式  即将usb通道的数据和wifi&ble通道数据进行相互转换
				}
				else if(dip_sta&0x4)
				{
						uart_test(WIFI_BLE);					//跳线短路则进入透明传输模式  即将usb通道的数据和wifi&ble通道数据进行相互转换
				}
				else if(dip_sta&0x8)
				{
						uart_test(RS485_4);						//跳线短路则进入透明传输模式  即将usb通道的数据和wifi&ble通道数据进行相互转换
				}
				else if(dip_sta&0x10)
				{
						uart_test(METER);						//跳线短路则进入透明传输模式  即将usb通道的数据和wifi&ble通道数据进行相互转换
				}
				else if(dip_sta&0x20)
				{
						uart_test(G4);						//跳线短路则进入透明传输模式  即将usb通道的数据和wifi&ble通道数据进行相互转换
				}
				else 
				{
					delay_1ms(2000);
					GetAtCmd();
					delay_1ms(2000);
					AllQueryCmdPrint();
					delay_1ms(2000);
					AllExecuteCmdPrint(1);
					AllExecuteCmdPrint(2);
					AllExecuteCmdPrint(3);					
					printf("\r\nDip is %d", dip_sta);				
				}			
				#endif				
							
				
				//FLASH_Data_Init();	//本项目中SPI FLash无用，暂时注释掉 与w5500公用一个spi2
				
				//======================================== CAN通讯 start===============================================
				
			  //g_transmit_message.tx_efid = DEV_CAN0_ID;	
				if(g_can_bus==1) 			
				{
					wdt();
					if(g_sysTick%5==0)
					{
						if(can_tick != g_sysTick)
						{				
							can_tick = g_sysTick;
							can_send(CAN0,DEV_CAN0_ID,(uint8_t*)"12345678",8);							
						}
					}
					communication_check();					
				}
				//======================================== CAN通讯 end===============================================	
				
				//======================================== usb通讯 start =============================================
				/* wait for standard USB enumeration is finished */
				else			//usb收发  cdc_acm_core.c
				{			
					if(USBD_CONFIGURED == usbd_cdc.cur_status) 
					{ 	
						wdt();						
						comm_process_usb();															//usb参数配置串口				清空补传数据									
						if (0U == cdc_acm_check_ready(&usbd_cdc)) 
						{
								cdc_acm_data_receive(&usbd_cdc);								
						}  
						else 
						{
								cdc_acm_data_send(&usbd_cdc);								//数据收发实现函数			
						}
					}
				}				
				//======================================== usb通讯 end=============================================	
				//vTaskDelay(5);   						/* 延时500个tick */	
		}    
		nvic_system_reset();
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    //usart_data_transmit(USART0, (uint8_t)ch);		
    //while(RESET == usart_flag_get(USART0, USART_FLAG_TBE)||RESET == usart_flag_get(USART0, USART_FLAG_TC));
		//Com_Send(USB,(unsigned char*)ch,1);		
    return ch; 
}

//启动代码启动的时候 0x800C000   0x34000    VECT_TAB_OFFSET  修改为  0xC000   CMSIS 中 system_gd32e50x.c
//自己启动的时候     0x8000000   0x40000    VECT_TAB_OFFSET  修改为  0x0


//增加freertos,设计原则，复杂功能，需要增加
//将wifi采用freertos
//4g采用freertos
//w5500采用freertos
//主线程主要实现di do ad 串口通讯，由于串口通讯实时性能比较高功能简单，无需重新定义线程。


//增加usb控制部分
//集成  1. 集成程序升级功能(上位机中)							
//集成  2. 系统控制器通讯协议(集成网络通讯协议)  	ok
//集成  3. 集成260程序，实现参数控制							ok

//集成  4. 集成lora参数设置功能(上位机中)					
//集成  5. 集成wifi+ble参数设置功能  usb可以透传  wifi+ble可以透传,方便wifi和ble开发和测试
//集成  6. 集成4g参数设置功能        usb可以透传  4g可以透传,方便4g开发和测试
//集成  7. usb可以控制所有协议,usb具备调试信息输出功能。调试printf有问题。

//esp32-s3  lcd  pclk max 40MHz  
//g_configRead.wifi_mode定义如下:
//0    表示不进行主口操作
//1    COM1   RS485
//2    COM2   RS485_2
//3    COM3   WIFI_BLE		独立逻辑 thread
//4    COM4   RS485_4
//5    COM5   METER
//6    COM6   G4					独立逻辑 thread
//7    USB    USB
//8    can    can总线
//9    rj45		ethernet   	独立逻辑 thread

//增加usb调试 wifi/ble 或4g功能。
//系统控制器上位机增加支持网络功能。将所有程序都合并到一个程序中。
//增加程序加密性。

//2022.12.27   g_configRead.save_frq改用为sht3x
// EtherNet_Tran2Com  806 将接收到的网络数据转发到
// Retransmission

// 行609      											增加rtc设置功能 usb  	ok		参考260   进行时间设置和读取
// 增加时间通过屏幕进行读写功能   												ok
// 增加显示部分																						ok
// 移植dw屏通讯部分																				ok
// 移植所有通讯读数据部分																	ok
// soc 从电池中读出来进行统计  暂时从第1路中读出来				ok
// dg运行时间，从整流器中读220V交流电压  直流电压和直流总电流就算  dg总功	ok


// Mppt   Data_Tran 中对.mask的判断，没有清空 准备处理； 统计发电量   mppt中读总充电电量
// 验证所有通讯读数据正确性   

// 负载用电时间，通过控制按钮来统计。
// dg mppt 负载 总功计算净功
// 统计最大功率和最小功率，每一路都统计  统计每一路的功  统计总功  总功/时间=平均功率

// 增加天数计算功能     						rtc_tm_count % 86400  得到  新的一天，对新的1天的能量进行计算		OK
// 增加运算部分
// 增加时间写存储器部分
// 准备测试
// 时间设置部分

// mppt能量统计需要验证
