//#include "stm32f10x.h" 
#include "GlobalVar.h"
#include "Define.h"
#include "gd32e503v_eval.h"

//unsigned char g_Mppt_Cnt							= 2;							//mppt个数
//unsigned int  g_Modbus_Interval_Get   = 5000;						//命令发送间隔间隔
float		g_NonEssentialLoadRelayOpenVoltage  = 46.0;	 			//备用负载继电器开路电压  (第1路) 46
float		g_NonEssentialLoadRelayCloseVoltage = 48.0;   		//备用负载继电器闭合电压  (第1路) 48
float		g_EssentialLoadRelayOpenVoltage     = 46.0; 			//主负载继电器开路电压    (第1路)		46	
float		g_EssentialLoadRelayCloseVoltage    = 48.0; 			//主负载继电器闭合电压    (第1路)		48	

float		g_NonEssentialLoadRelayOpenVoltage_Sec  = 46.0 ;	//备用负载继电器开路电压  (第2路) 46
float		g_NonEssentialLoadRelayCloseVoltage_Sec = 48.0;   //备用负载继电器闭合电压  (第2路) 48
float		g_EssentialLoadRelayOpenVoltage_Sec     = 46.0; 	//主负载继电器开路电压  	(第2路)	46	
float		g_EssentialLoadRelayCloseVoltage_Sec    = 48.0; 	//主负载继电器闭合电压  	(第2路)	48


float 	g_LowBatteryVoltageStartGenerator   = 46.0;				//
float 	g_HighBatteryVoltageStopGenerator   = 58.0;				//
float 	g_EnclosureFanTurnOnTemperature    	= 40.0;				//40.0;
float 	g_EnclosureFanTurnOffTemperature   	= 35.0; 			//35.0;
float 	g_HighPVVoltageStopGenerator     		= 100.0;   		//PV高压停止发电机

usb_dev usbd_cdc;

unsigned int			wifi_rev_cnt = 0;				//wifi串口接收的数据总量
unsigned char 		wifi_reset_sta = 0;			//wifi重启后的状态  0=表示重启开始  1=表示进入已开机但非透传模式    2=表示进入了透传模式   3=没有接收到任何数据超时
unsigned int			wifi_reset_tick = 0;		//重启时刻
//unsigned char		ucDestIP[4];						//来自g_configRead中的目标ip和目标端口
//int  						ucDestPort;							//

unsigned int  	cpu_id[3];
unsigned char		g_can_bus = 0;							//0=can 1=usb
unsigned char		COM_MASTER=RS485;						//将RS485  COM1作为主要转发口

unsigned int   	g_sysTick=0;
unsigned int   	systickCount=0;

unsigned char 	g_rxbuf_UDP[4][MAXRecvBuf];
unsigned int 		g_wr_UDP[4],g_rd_UDP[4];
unsigned int   	g_UDP_usTick[4];
	
unsigned char 	g_rxbuf[MAXRecvBuf];
unsigned int 		g_wr=0,g_rd=0;
unsigned char 	g_rxbuf_tcp[MAXRecvBuf];
unsigned int 		g_wr_tcp=0,g_rd_tcp=0;
unsigned char 	com5_sended=1;
unsigned char		com4_sended=1;
unsigned char		com2_sended=1;
unsigned char		com1_sended=1;


t_SaveWifiCfg  	gs_SaveWifiCfg;
unsigned char 	RS2321_buff[RS2321_REC_BUFF_SIZE] = {0};
unsigned int  	RS2321_rec_WR=0;
unsigned int  	RS2321_rec_RD=0;
unsigned int		usb_para_tick = 0;

unsigned int 		g_usbFlag=0;
unsigned char 	g_usbbuf[RS2321_REC_BUFF_SIZE] = {0};
unsigned int 		g_usblen=0;


unsigned char boot_buf[128];				//boot
unsigned char	b_wr=0,b_rd=0;				//boot

unsigned char 				RSGPRS_buff[RSGPRS_REC_BUFF_SIZE] = {0};
unsigned int  				RSGPRS_rec_WR=0;									
unsigned int  				RSGPRS_rec_RD=0;
unsigned char					g_gprs_discon = 0 ;			//gprs是否断开  =1表示通过检测字符串判断出gprs断开
GPRS_INFO							m_gprsinfo;
SIM_INFO							m_siminfo;
ConfigInfo 						g_configRead;	

//unsigned char					g_gprs_work=1;				//gprs是否工作 1表示工作  0xff表示未配置，默认工作	   其他值表示不工作  不工作则设置成0
//unsigned char					g_wifi_work=1;				//wifi是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0 
unsigned char					g_rj45_work=1;				//rj45是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0

unsigned char	Do_Sta[8]={0};					//do状态
unsigned char Sw_Sta[8]={0};					//拨码盘状态
unsigned char Android_Sta=0;					//Android电源状态
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

short	int						Adc_Data[6]={0};				//ad采集后转换后的数据
int									Ads1247_Value[8]={0};		//ads1247 8个通道的ad数据 
unsigned char 			Irq_Ads1247_Ready=0;		//ads1247转换完成中断
float								Ads_1247_Fvalue[8]={0};						//float 电压数据 v


t_AdCali							g_Ads1247_Cali;
t_SaveNetIPCfg 				gs_SaveNetIPCfg;
t_WifiBleAtCmd				gs_WifiBleAtCmd;	

struct_switch_com			Sw_com[4];						//0=com2(0x31)   1=com10(0x39)    2=com4(0x33)   3=com11(0x3a)
struct_com						Com;
struct_Voltage				VOLT;
struct_Temperature		Temp;
struct_ADC            ADC_Value;
Tm_struct							Tm;
CNT_struct            CNT;
DI_struct							DI;
DO_struct             DO;
Flag_struct           Flag;
calendar_struct       Calendar;


device_information_struct DevInfo={
	//             1         2  
	//   012345678901234567890123456789
	{6, "Owrely",},//COMPANY_TB
	{6, "Owrely",},//COMPANY_TB2
	{12,"Master-MCU_V1.0",},//HW_TB
	{12,"Master-MCU_V1.0",},//SW_TB
	{8, "14000000",},//SERIAL_NO_TB
	{7, "Master-MCU",},//PRO_NAME_TB
	1,//ID
};

struct_Mppt						g_Mttp[16];
struct_Result					g_Result;	
struct_Result2				g_Result2;
PVT6_ParaInfo					g_PVTPara;
//----------------------------------------------------
struct_Rectifier    			g_Rectifiter;
struct_PYLON_Battery			g_PYLON_BAT;
struct_get_air_info				g_Air;
struct_inverter     			g_Inverter;
struct_fan            		g_Fan;
struct_diesel_generator   g_Diesel_Generator;
unsigned char 						g_Result_Status;
unsigned char 						ucPYLON_BAT_SETS=2;		//PYLON电池组数
unsigned char   					g_modbus_id[4];			//


