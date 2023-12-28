//#include "stm32f10x.h"
#include "define.h"
#include "Struct_Define.h"
#include "cdc_acm_core.h"


//extern	unsigned char g_Mppt_Cnt							;			//mppt个数
//extern	unsigned int  g_Modbus_Interval_Get   ;			//命令发送间隔间隔



extern 	float						g_LoadRelayOpenVoltage[4]     ; 			//负载继电器开路电压  	(第1路)	46	
extern	float						g_LoadRelayCloseVoltage[4]    ; 			//负载继电器闭合电压  	(第1路)	48	
extern  int							g_LoadDiscWaitingTime					;				//单位分钟

extern 	float 					g_LowBatteryVoltageStartGenerator ;		//
extern 	float 					g_HighBatteryVoltageStopGenerator ;		//
extern	float 					g_EnclosureFanTurnOnTemperature	;			//
extern	float 					g_EnclosureFanTurnOffTemperature ;		//
extern  float 					g_HighPVVoltageStopGenerator;   			//PV高压停止发电机

extern usb_dev usbd_cdc;
extern unsigned char 							wifi_reset_sta;			//wifi重启后的状态  0=表示重启开始  1=表示进入已开机但非透传模式    2=表示进入了透传模式   3=没有接收到任何数据超时
extern unsigned int								wifi_rev_cnt;				//wifi串口接收的数据总量
extern unsigned int								wifi_reset_tick;		//重启时刻

//extern unsigned char							ucDestIP[4];			//来自g_configRead中的目标ip和目标端口
//extern int  											ucDestPort;				//

extern unsigned int  							cpu_id[3];
extern unsigned char							COM_MASTER;				//将RS485  COM1作为主要转发口
extern unsigned char							g_can_bus;				//0=usb 1=can
extern unsigned int   						g_sysTick;				//单位s
extern unsigned int   						systickCount;			//单位ms

extern	unsigned char 						g_rxbuf_UDP[4][MAXRecvBuf];
extern	unsigned int 							g_wr_UDP[4],g_rd_UDP[4];
extern  unsigned int   						g_UDP_usTick[4];

extern	unsigned char 						g_rxbuf[MAXRecvBuf];
extern	unsigned int 							g_wr,g_rd;
extern	unsigned char 						g_rxbuf_tcp[MAXRecvBuf];
extern	unsigned int 							g_wr_tcp,g_rd_tcp;

extern unsigned char 							RSGPRS_buff[RSGPRS_REC_BUFF_SIZE];
extern unsigned int  							RSGPRS_rec_WR;									
extern unsigned int  							RSGPRS_rec_RD;
extern unsigned char							g_gprs_discon;				//gprs是否断开  =1表示通过检测字符串判断出gprs断开

extern GPRS_INFO									m_gprsinfo;
extern SIM_INFO										m_siminfo;

extern ConfigInfo 								g_configRead;	

extern	t_SaveWifiCfg  						gs_SaveWifiCfg;
//usb 缓冲
extern unsigned char 							RS2321_buff[RS2321_REC_BUFF_SIZE];
extern unsigned int  							RS2321_rec_WR;
extern unsigned int  							RS2321_rec_RD;
extern unsigned int								usb_para_tick;
	
extern unsigned int 							g_usbFlag;
extern	unsigned char 						g_usbbuf[RS2321_REC_BUFF_SIZE];
extern	unsigned int 							g_usblen;

//extern unsigned char							g_gprs_work;				//gprs是否工作 1表示工作  0xff表示未配置，默认工作	   其他值表示不工作  不工作则设置成0
//extern unsigned char							g_wifi_work;				//wifi是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0 
extern unsigned char							g_rj45_work;					//rj45是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0

extern struct_switch_com					Sw_com[4];						//0=com2(0x31)   1=com10(0x39)    2=com4(0x33)   3=com11(0x3a)
extern struct_com									Com;
extern DI_struct									DI;

extern unsigned char 							boot_buf[128];				//boot
extern unsigned char							b_wr,b_rd;						//boot

extern	unsigned char							Do_Sta[8];					//do状态
extern	unsigned char 						Sw_Sta[8];					//拨码盘状态
extern	unsigned char 						Android_Sta;				//Android电源状态
extern 	t_SaveNetIPCfg 						gs_SaveNetIPCfg;		

extern	unsigned char 						com5_sended;
//extern	unsigned char 						com4_sended;
//extern	unsigned char 						com2_sended;
extern	unsigned char 						com1_sended;

extern  t_WifiBleAtCmd						gs_WifiBleAtCmd;	
extern 	t_AdCali									g_Ads1247_Cali;
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

extern struct_Voltage							VOLT;
extern struct_Temperature					Temp;
extern struct_ADC           			ADC_Value;
extern device_information_struct 	DevInfo;
extern Tm_struct									Tm;
extern CNT_struct            			CNT;
extern DI_struct									DI;
extern DO_struct             			DO;
extern Flag_struct           			Flag;
extern calendar_struct       			Calendar;

extern 	int												Ads1247_Value[8];		//ads1247 8个通道的ad数据 
extern  float											Ads_1247_Fvalue[8];	//float 电压数据 v
extern  unsigned char 						Irq_Ads1247_Ready;	//ads1247转换完成中断
extern  short	int									Adc_Data[6];				//ad采集后转换后的数据

extern  struct_Mppt								g_Mttp[16];
extern  struct_Result							g_Result;
extern  struct_Result2						g_Result2,g_Result3;
extern  PVT6_ParaInfo							g_PVTPara;
//-----------------------------------------------
extern	struct_Rectifier    			g_Rectifier;
extern  struct_PYLON_Battery			g_PYLON_BAT;
extern  struct_get_air_info				g_Air;
extern  struct_inverter     			g_Inverter;
extern  struct_fan            		g_Fan;
extern  struct_diesel_generator   g_Diesel_Generator;
extern  SLB48_PARA   							slb48[16];

extern  struct_mqtt								g_MQTT;
extern  unsigned char 						g_Result_Status;
extern  unsigned char 						ucPYLON_BAT_SETS;			//PYLON电池组数
extern  unsigned char   					g_modbus_id[4];					//

//u16  GS_CrcCheck(u8 *pData,u16 nLen);
//void  deal_rev_data(u8* Arr_rece,u16* Data_len,u8 Uart_num,u8 mode);
//void uart_get_data(u8 modbus_id,u16 addr, u16 num,u8 cmd,u8 com);
//void uart_set_data_modbus6(u16 addr , u16 dat);
//void uart_set_data_modbus16(u8 modbusid,u16 addr , u8 num, u16 *dat, u8 com);
//void uart_get_data_modbus1(u16 addr , u16 num);
//void uart_set_data_modbus5(u16 addr , u16 dat);






//u16 Length_Cal(u16 Para_usLen_INFO_ASCII_Data);
//unsigned char Hex_Ascii(unsigned char ucData_Hex);
//unsigned char Ascii_Hex(unsigned char ucData);
//unsigned char Ascii2_To_Hex1(unsigned char Para_ucAscii1,unsigned char Para_ucAscii2);
//u16 Ascii4_To_Hex2(u8 Para_ucAscii1,u8 Para_ucAscii2,u8 Para_ucAscii3,u8 Para_ucAscii4);
//void Hex1_To_Ascii2(unsigned char *pucBuffer,unsigned char ucD);
//u16 CheckSum_Cal(u8 *pData,u16 nLen);
//float Float_Cal(u8 Para_uc1,u8 Para_uc2,u8 Para_uc3,u8 Para_uc4);
//void DZ_Uart_Get_Data(u8 Para_ucVER,u8 Para_ucADR,u8 Para_ucCID1,u8 Para_ucCID2,u8 Para_ucCom);

//float Tristar_PU_Cal(unsigned int Para_usPU_HI,unsigned int Para_usPU_LO);
//float Tristar_Value_Cal(float Para_fValue_Original);
