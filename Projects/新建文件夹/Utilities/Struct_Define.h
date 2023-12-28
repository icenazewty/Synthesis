#include "gd32e50x.h"
#include <stdbool.h>

#define RSGPRS_REC_BUFF_SIZE			128				//com5 4g
#define RS2321_REC_BUFF_SIZE			1024			//usb
#define RS232_REC_BUFF_SIZE				4096
#define RS232_REC_CMD_SIZE				1023

#define D_USART_REC_BUFF_SIZE				1024
#define MAXRecvBuf 		1024    						//changed by wjj 30->128
#define	ATCMDCOUNT		128								//wifi&ble 命令个数

//#define	WIFI_BLE_COM	2								//wifi&ble com口 COM3   smtdog260 v3.62
//#define	USB_DBG_COM		0								//printf   com口 COM1  	smtdog260 v3.62

#define	AUTO_RS485_COM		0								//rs485    com口 COM1   	meter v1
#define	WIFI_BLE_COM			1								//wifi&ble com口 COM2   	meter v1
#define	USB_DBG_COM				2								//printf   com口 COM3   	meter v1
#define	METER_COM					3								//meter    com口 COM1   	meter v1



#if USB_DBG_COM == 0
	#define  DBGCOM	  USART0
#elif	USB_DBG_COM == 1
	#define  DBGCOM	  USART1
#elif	USB_DBG_COM == 2
	#define  DBGCOM	  USART2
#elif	USB_DBG_COM == 3
	#define  DBGCOM	  UART3
#elif	USB_DBG_COM == 4
	#define  DBGCOM	  UART4
#endif

typedef struct
{
	volatile unsigned char  RX_Buf[D_USART_REC_BUFF_SIZE];
	volatile unsigned short usRec_WR;
	volatile unsigned short usRec_RD;
	volatile unsigned int   usTick;
	volatile unsigned short usDelay_Rev;				//连续接收2个字节之间最大时间间隔 单位ms  4800bps  500字节/秒  1字节最晚为2ms
}struct_usart;

typedef struct
{	
	unsigned char 	ucMode;							//目前发送模式       0=表示空闲  1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45
	unsigned char 	ucFlag;							//是否处于等待状态	 0=空闲      1=表示已经送，等待返回。
	unsigned int    uiTick[3];					//本地发送  0 = Local   1=com1   2=rj45	 发送开始时刻 三种模式
	unsigned short 	usDelay_Rev;				//例如向某个口发送数据，要求规定时间内数据得到回应，如果没有回应，则放弃。
}struct_switch_com;

typedef struct{
	struct_usart 			Usart[9];		//6个串口  1个usb   增加2个串口
	//struct_usart 		Usart2;
	//struct_usart	 	Usart3;
	//struct_usart 		Uart4;
	//struct_usart 		Uart5;
	unsigned char 	RX_Analy_Buf[D_USART_REC_BUFF_SIZE];
	unsigned short 	usRX_Analy_Buf_Len;
	unsigned char 	TX_Buf[D_USART_REC_BUFF_SIZE];
	unsigned char		uctmpID;
	unsigned char  	ucCommand;
	unsigned short 	ucActiveRd;
	unsigned short 	ustmpInt;
	unsigned short 	usBufLen;
}struct_com;


#define D_DI_DO_CH_MAX_NUM 8
typedef struct{
	bool bInput_State[D_DI_DO_CH_MAX_NUM];
//	unsigned char ucInput_State[D_DI_DO_CH_MAX_NUM];
	unsigned char  ucJudge_Input_State[D_DI_DO_CH_MAX_NUM];
}DI_struct;



typedef struct  
{
	unsigned char ucStartFlag;
	unsigned char ucSocket0Mode;
	unsigned char ucSocket1Mode;
	unsigned char ucSocket2Mode;		
		
	//合计37字节  	strat		8+12+8+2+8=38
	
	unsigned char ucMAC[6];						//mac地址
	unsigned char ucDhcpMode[2];			//dhcp模式
	unsigned char ucSelfIP[4];				//静态ip
	unsigned char ucSubMASK[4];				//静态子网掩码
	unsigned char ucGateWay[4];				//静态网关
	
	
	unsigned char ucDestIP[4] ;				//tcp client  目标ip
	unsigned char ucDestPort[2];			//tcp client	目标端口
	unsigned char ucSourcePort[2];		//tcp client	本地端口   =0表示任意
	
	unsigned char ucMonitorPort[2];		//tcp server  监听端口
	
	unsigned char ucUdpDestIP[4] ;		//udp  target  ip
	unsigned char ucUdpDestPort[2];		//udp  target	port
	unsigned char ucUdpSourcePort[2];	//udp  source port(udp monitor port)
	//合计36字节  	end		6+12+8+2+8=38
		
	//以下为uart配置信息	
	unsigned char	g_ModbusID;				//位置顺序不能更换  连续三个
	unsigned char	chekbit[5];				//3
	int						baud[5];					//10	
	unsigned char ucEndFlag;
}t_SaveNetIPCfg ;

typedef struct  
{
	//以下为uart配置信息	
	unsigned char	g_ModbusID;				//位置顺序不能更换  连续三个
	unsigned char	chekbit[7];				//3
	int						baud[5];					//10	
}t_BaudCfg ;

typedef struct  
{	
	unsigned char ucCmdName[ATCMDCOUNT][20];		//at command name
	unsigned char ucTest[ATCMDCOUNT];						//support test commad 
	unsigned char ucQuery[ATCMDCOUNT];					//support query commad 
	unsigned char ucSet[ATCMDCOUNT];						//support set commad 
	unsigned char ucExecute[ATCMDCOUNT];				//support execute commad 	
	unsigned char ucActiveCount;								//实际命令个数
}t_WifiBleAtCmd;

typedef struct {
	float fBAT,fP12;
}struct_Voltage;

#define D_NUM_STRING_BYTES	16
typedef struct{
	unsigned char	ucLen;
	unsigned char ucStr[D_NUM_STRING_BYTES];
}num_string_struct;

typedef struct {
	num_string_struct COMPANY_TB;
	num_string_struct COMPANY_TB2;
	num_string_struct HW_TB;
	num_string_struct SW_TB;
	num_string_struct SERIAL_NO_TB;
	num_string_struct PRO_NAME_TB;
	unsigned char ucID;
}device_information_struct;

typedef struct{
	signed char scRTS;
	signed char scAmb;
}struct_Temperature;

//#define D_V_ADC_MAX_NUM 20
//#define D_I_ADC_MAX_NUM	100
typedef struct {
	unsigned short BAT;		//[D_V_ADC_MAX_NUM];
	unsigned short P12;		//[D_V_ADC_MAX_NUM];
	unsigned short Trts;	//[3];    
	unsigned short Tamb;	//[3];
//	unsigned short TempBuf[D_V_ADC_MAX_NUM];
}struct_ADC;

typedef struct {
	unsigned int uiRTC_Count;		//rtc ë¼Ɗýֵ£¬´Ӳ000-01-01 0:00:00¿ªʼˣư£¬ȧ2001Ī1Ԃ1ȕ0:00:00µĖµΪ£º366*24*3600=31622400
}Tm_struct;


typedef struct {
	unsigned short usIWDG;
	unsigned short usMain_Program_Non_Normal_Run;
}CNT_struct;



typedef struct{
	bool bOutput_State[D_DI_DO_CH_MAX_NUM];
}DO_struct;

typedef struct {
	bool bMain_Program_Normal_Run;
	unsigned int uiComm_modify;
	bool bSec_INT;
	bool bCAN_RX;
}Flag_struct;

typedef struct {
	unsigned short Year;				//Ī·ݸ߲λ£¬Hex¸񊽣¬Àý2000Ī£¬Year=0x7D0
	unsigned char  Month;				//Ԃ·ݣ¬Hex¸񊽣¬Àý12Ԃ£¬Month=0x0C
	unsigned char  Day;					//̬£¬Hex¸񊽣¬Àý20ºţ¬Day=0x14
	unsigned char  Week;
	unsigned char	 Hour,Minute,Second;
}calendar_struct;


typedef struct  
{
	unsigned char ucStartFlag;
	signed int AdC[8][3] ;				//代表8个通道,3个数据，分别为-6V    0V   6V	
	unsigned char ucEndFlag;
}t_AdCali ;


	typedef struct
	{
		//gprs部分
	//	int						iGprsDisConnCnt;						//gprs断开次数
		bool					g_bGPRSConnected;							//GPRS连接状态	
//		unsigned int	gprs_last_succ_time;				//gprs最后连接成功时间
		unsigned int	gprs_last_fail_time;					//gprs最后连接失败时间
		int		    		gprs_last_faile_reason;				//gprs最后连接失败的原因			Z
		int						gprs_lastsuc_faile;						//gprs最后连接成功到目前失败次数	Z
//		int						gprs_success;								//成功连接gprs的次数
		int						gprs_faile;										//连接gprs失败的次数
		//tcp部分
			
		//数据发送部分		
		unsigned int	dat_last_succ_time;						//最后发送成功时间
	//	unsigned int	dat_last_fail_time;					//最后发送失败时间	
//		int						dat_last_fail_reson;				//最后发送失败的原因
		//int						dat_lastsuc_faile;					//最后发送成功到目前失败次数
//		int						dat_success;								//发送成功次数
//		int						dat_faile;									//发送不成功次数
	}GPRS_INFO;

	
typedef struct CONFIG// 最多只能256字节  当前结构体占用244字节 。CPU内部flash最多2048字节
{
  unsigned char 		b_success;					//数据是否成功读取 '*'表示读取成功 
  char  						device_ID[6];				//设备ID 占3个字节24位    
	
	unsigned short 		save_frq;						//存储频率 单位s
	unsigned short 		collect_frq;				//采集频率 单位s		1=wifi udp和188通讯   2=rj45 upd和188通讯
  unsigned short 		send_frq;    				//主动发送频率

	unsigned char  		bCH[2];							//通道1是否工作
  
  short   int     	TMax[2];						//通道1温度上限 实际温度*10   
  short   int     	TMin[2];						//通道1温度下限
  short   int     	HMax[2];						//通道1湿度上限
  short   int    		HMin[2];						//通道1湿度下限
  short   int     	TXZ[2];							//温度修正
  short   int     	HXZ[2];							//湿度修正    
	short 	int 			VMin;       				//电压下限   -->  2014-08-14 目前修改为掉电是否（短信,电话）报警,0表示不报
  char  						alarmNum[3][13];   	//报警号码 //*+电话号码 *表示电话号码是否有效 最后结束符 0
  char  						remoteIP[15];      	//GPRS接收端IP 255.255.255.255
  unsigned char  		IPLen;             	//IP地址长度
  char  						remotePort[5];     	//接收端端口  
  unsigned char  		PortLen;           	//端口长度    
   char  						Danwei[8];					//公司编号
   char  						Depart[2];					//部门编号 [1]=0~255	TELT电话报警延时间隔;	[0]=0或[3,240]RECE,每间隔几分钟拨打一次报警电话,=0表示禁止。			
  unsigned char  		alarmDelyMinute;		//警报检测延时 单位Min 		最多255min即4个小时15min。
  unsigned char  		beep;        				//1表示开启beep,0表示关闭beep
  unsigned char  		wifi_mode;   				//wifi工作模式定义
 // 	0=wifi	不工作
 //  	1=wifi 	udp通讯 查询方式和mec188配合使用,可以进行设置
 //  	2=wifi 	采用tcp的client模式，和gprs模式传输格式完全一样 
 //  	3=wifi 	路由器模式，开机默认不工作，可以通过按键来设置是否工作。用于数据导出和配置信息等。
  unsigned char 		b_light_work;				//保证被4整除对齐。	Revese
  unsigned char 		b_success1;					//是否成功读取    '!'
	 char			NationCode[6];			//1-5个字节
	unsigned char 		NameLen;						//名称长度
	 char			sysName[20];				//最多10个汉字或10个字母或10个汉字和字母混合,并且必须为unicode码
	unsigned char			b_gprs_work;				//gprs是否工作 1表示工作  0xff表示未配置，默认工作	   其他值表示不工作  不工作则设置成0
	unsigned char			b_wifi_work;				//wifi是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0 
	unsigned char			b_rj45_work;				//rj45是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0
	//==============================以上信息148字节=============================================
	unsigned char			b_debug_work;					//debug 是否工作,=1表示工作，否则不工作。com1打印调试信息。gprs的。
	unsigned char			reset_time;						//gprs,在x小时中一条数据也没有收到，则进行系统重启。0=表示不重启，<4则=4。范围[0,240]																				
	unsigned char			AMPER_RTC_PC13;				//RTC输出调试				=1表示，调试输出，否则不输出。																				
	unsigned char			b_mco_PA8;						//MCO输出调试				暂时保留，不用。
																					//SYSTICK_Source;		//秒定时器从什么地方来。直接修改宏。
																					//SMTDOG_V33  设备版本，决定程序执行有所不同。			
  unsigned char			b_Sms_Test;						//是否支持发短信测试功能,即在固定时间点发测试短信，证明手机卡有费。并在此后每固定天数再发测试短信。固定日和时由宏来定义。
	unsigned char     b_Sms_FxiTime;				//每间隔多少天发送1条短信。
	unsigned char			Reserve;							//4对齐用。
	char 							alarmTel[3][14];			//增加3个报警电话号码: 可以是固定电话,合计12字节，如果不足则前面采用空格表示。保持后面位数有效。																					//报警号码 *+电话号码 *表示电话号码是否有效 最后结束符 0
 
	short   int     	TMax2;						//通道1温度上限 实际温度*10   
  short   int     	TMin2;						//通道1温度下限
  short   int     	HMax2;						//通道1湿度上限
  short   int    		HMin2;						//通道1湿度下限
} ConfigInfo;

typedef struct
{
		char					ip_address[16];						//tcp连接成功的ip
		int						port_number;							//tcp连接成功的port
//		int						gsm_reset_cnt;					//gsm模块重启次数
//		unsigned int	gsm_reset_time;					//模块最后重启时间，如果为0则表示系统开始工作的时间。
		int						gsm_reset_reson;					//gsm模块重启原因
		int						sms_success;							//成功发送短信条数
//		int						sms_faile;							//发送失败短信条数
//		int						sms_lastsuc_faile;			//最后成功到目前发送失败的次数;		Z
//		unsigned int	sms_last_faile_time;		//最后1次失败时间					Z
		int						sms_last_faile_reson;			//最后1次失败原因					Z
		
		bool					bComm;										//与模块是否可以通讯即CPU向mg301发命令是否有回应。 
		int						CommFailCnt;							//连续通讯失败的次数
		unsigned int	FailStartTime;						//失败开始时刻
		char					singal;										//信号强度
		bool					bExist;										//手机卡是否存在
		char					imsi[16];									//最多15字节IMSI			
		char					iccid[21];								//20字节iccid		
		char					service;									//服务商 0:未检测到运营商信息  1:联通  2:移动 （暂时只支持这两个）
		bool					bRegGSM;									//GSM网络注册成功否
		bool					bRegNet;									//GPRS网络注册是否成功		
		//以下与短信有关
		bool					bSmsPDU;									//PDU工作模式设置成功否
		char					strCSCA[24];							//短信中心电话号码
		char					phonenum[24];							//本机手机号码  暂时无用
}SIM_INFO;

typedef struct  
{
	unsigned char ucLocalPort[2];
	unsigned char ucRemotePort[2];	
	unsigned char ucProtocol;	
	unsigned char ucCSMode;	
	unsigned char ucConnState;			//连接状态，tcp client
	unsigned char Reserved;
	unsigned char ucDestIP[4];		
}t_TcpIPCfg ;

typedef struct  
{
	unsigned char ucStartFlag;
	unsigned char ucDhcp;
	unsigned char ucWorkMode;
	unsigned char ucEncryptMode;	
	unsigned char ucSelfIP[4];
	unsigned char ucSubMASK[4];
	unsigned char ucGateWay[4];	
	unsigned char ucMAC[6];	
	unsigned char ucDNS[4];							//增加dns
	unsigned char ucWifiState;					//wifi连接状态(2022-06-21增加)
	unsigned char ucWifiSingal;					//wifi信号强度(2022-06-21增加) 上位机需要*-1
	unsigned char reserved[2];					//
	t_TcpIPCfg		TdInfo[3];						//合计2个通道,其中一个为备用的。
	unsigned char ucSSID_PWD[37];				//ssid+pwd,合计35个字节。加两个\0
	unsigned char ucEndFlag;
}t_SaveWifiCfg;

typedef struct
{
	 unsigned char   Mask;			//公共部分是否已发送,并成功收到数据 bit0表示 v_pu  i_pu   ;  bit1=表示 sn等
	 unsigned char 	 ComPort;		//连接MPPT的串口编号			增加部分
	 unsigned char   modbus_id;	//modbus_id								增加部分
	
	 unsigned char   SN[8];			//Serial Number
	 unsigned char   Model;			//1=60A  0=45A
	
	 unsigned short  HardVer;		//硬件版本
	 unsigned short  SoftVer;		//软件版本
	
	 unsigned short  V_PU_Hi;		//电压整数部分
	 unsigned short  V_PU_Lo;		//电压小数部分
	 
	 unsigned short  I_PU_Hi;		//电流整数部分
	 unsigned short  I_PU_Lo;		//电流小数部分
	
	 unsigned short  Reg[27];		//reg原始数据
	signed short  	 Reg2[19];			//reg原始数据E000开始
	
	
  unsigned short   Fault_all_2C;     //Controller faults bitfield
  unsigned short   dip_all_30;      //DIP switch positions bitfield
  unsigned short   led_state_31;   //State of LED indications
  float					   vb_ref_33;
  unsigned short   kwhc_r_38;       //kWhr charge resetable
  unsigned short  kwhc_t_39;       //kWhr charge total  
	
	unsigned short  vb_ref;       //Target regulation voltage
  unsigned long   Mppt_HM;
  unsigned long   Mppt_alarm;
  float  					Mppt_Ahc_r;
  float  					Mppt_Ahc_t;

    
  unsigned short  Ahc_daily;        //Total Ah charge daily n*0.1
  unsigned short  whc_daily;        //Total Wh charge daily
  unsigned short  flags_daily;      //Daily flags bitfield
  float  					Pout_max_daily;   //Max. Power Out, daily
  unsigned short  alarm_daily_HI;
	unsigned short  alarm_daily_LO;
  unsigned short  time_ab_daily;    //cumulative time in absorption, daily
  unsigned short  time_eq_daily;    //cumulative time in equalize, daily
  unsigned short  Vtime_fl_daily;   //cumulative time in float,daily
  
	short   T_rts;          //温度 RTS temperature(0x80 = disconnect) 此处必须使用16位数据，否则会溢出。
	short   T_hs;           //Heatsink temperature
}struct_Mppt;

typedef struct
{
	 unsigned char	load1_ctrl;						//负载1控制    ka1  do0  do1
	 unsigned char	load2_ctrl;						//负载2控制    ka2  do2  do3
	 unsigned char  fan_ctrl;							//风扇控制		 ka3  do4
	 unsigned char  reserve_motor_ctrl;		//后备电机控制 ka4  do5
	
	 float		batt_v;											//电池电压
	 float		lem4_batt;									//电池电流in4	-->in6 
	 float    batt_p;											//电池功率
	
	 float    lem1_load1;									//负载1电流in2 
	 float    lem2_load2;									//负载2电流in1 
	 float    load_i;											//负载电流
	 float    load_p;											//负载功率	
		
	 float		pv_v;												//光伏电压	
	 float		pv_p;												//光伏功率
	
	 float		ir;													//太阳能光照强度 in0	
	 float		temp[2];										//2个温度,箱子内部温度和外部环境温度     in5(内部温度)X     in7(环境温度)OK
	 float		mppt_temp;									//mppt温度
		
	 unsigned char  batt_status;					//电池状态 电池空气开关是否都合闸 	 DI0    led亮为正常
	 unsigned char  load_status;					//负载状态 负载1 2空气开关是否都合闸 DI1		led亮为正常	 
	 unsigned char  doormagnetic_status;	//门磁状态 													 DI2		led亮为正常	 
	 unsigned char  battlose_status;			//电池防盗状态  										 DI3		led亮为正常
	 unsigned char  pvlose_status;				//pv防盗状态          							 DI4		led亮为正常	 	 
	 unsigned char  load34_status;				//负载状态 负载3 4空气开关是否都合闸 DI5		led亮为正常	 
	 //-----------------------------------------------以下为增加部分2023.7.17---------------------------------------
	 unsigned char	load3_ctrl;						//负载3控制    ka1  do4  do5
	 unsigned char	load4_ctrl;						//负载4控制    ka2  do6  do7
	 unsigned char  smog_alarm;						//烟雾报警 	 	 DI6       led亮为报警
	 unsigned char  waterout_alarm;				//水浸报警     DI7       led亮为报警
	 unsigned char  resever[2];						//4字节对齐
			
	 float    			lem_load3;						//负载3电流in3
	 float    			lem_load4;						//负载4电流in4
	 signed   short int  main_carbin_T;		//温度
	 unsigned short int  main_carbin_H;		//湿度
	 signed   short int  second_carbin_T;	//温度
	 unsigned short int  second_carbin_H;	//湿度
   float					pv_i;									//光伏电流
	 float					mppt_i;								//mppt总电流
	 float					mppt_p;								//mppt总功率
	 //++++++++++++++++2023.9.13增加+++++++++++++++++++++++++++++++++++++++++++++
	 float fVoltage_AC;												//逆变器交流电压               0x000A
	 float fCurrent_AC;												//逆变器交流电流               0x000B
	 
	 float fV_RC_Distribution_Output;											//dc输出电压  整流器
	 float fI_RC_Distribution_Moudle_Output_Total;				//所有模块输出总电流  整流器
	 
	signed long slGenerator_L1L2L3_watts;			//发电机总功率
	float fGenerator_L1L2L3_current;					//发电机总电流
	
	float faverage_module_voltage;						//电池平均电压				V		average module voltage  		.3f 	2Byte
	float ftotal_current;											//电池总电流					A		total current   						.2f		2Byte
	unsigned short 	 ucaverage_SOC;						//电池平均SOC					%		average SOC 								0			1Byte
	
	unsigned short			b_gprs_work;			//gprs是否工作 1表示工作  0xff表示未配置，默认工作	   其他值表示不工作  不工作则设置成0
	unsigned short			b_wifi_work;			//wifi是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0 
	unsigned short			b_rj45_work;			//rj45是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0
	unsigned short 			b_TH_work;				//增加部分  bit 0控制  第1路温湿度    bit1控制第2路温湿度        bit4控制a/c  bit5控制fan
	unsigned short  		alarmDelyMinute;	//警报检测延时 单位Min 		最多255min即4个小时15min。  mppt 个数
	unsigned short 			device_ID;				//设备ID 占3个字节24位   最大 65535
	 
   //增加部分mppt状态 信息，是否需要报警信息
	 //mqtt    snmp    dhcp      ntp       freertos     gd32e503     bootloader      esp32-c3        usb       24bit-adc采集   
	 //剩余    dw屏    llcc68    低功耗    
}struct_Result;

typedef struct
{
	 float 					ff_E_UIt_J_Sum[2][5];				//U*I *t=E(J)  能量  单位:焦耳   当前天总计用了多少能量	 当前天和上一天  总计负载功  load1 load2 load3 load4 总计负载功	  
	 float 					f_E_MaxLoadP[2][5];					//最大负载功率  4个中统计  load1  load2 load3  load4  总计最大
	 float 					f_E_MinLoadP[2][5];					//最小负载功率	4个中统计  
	 float					f_E_AverLoadP[2][5];				//平均负载功率+总平均负载功率
	 
	 float					f_E_TotalDG_KWH[2];					//柴油发电机总功  通过整流器来计算 U*I
	 float					f_E_TotalMPPT_KWH[2];				//PV发电总功											
	 float					f_E_NetKWH[2];							//净总功
	 //41*2=82  + 10 = 92
	 unsigned short i_DGRunTime[2];							//DG使用时间  单位min
	 unsigned short i_MaxBatSoc[2];							//电池最大SOC
	 unsigned short i_MinBatSOC[2];							//电池最小SOC
	 unsigned short i_PowerAvailability[2];			//当天电源供电正常时长，单位min
	 unsigned short	i_PowerAvailabilityPer[2];	//电源可用性%
	 
   //增加部分mppt状态 信息，是否需要报警信息
	 //mqtt    snmp    dhcp      ntp       freertos     gd32e503     bootloader      esp32-c3        usb       24bit-adc采集   
	 //剩余    dw屏    llcc68    低功耗    
}struct_Result2;

// 统计最大功率和最小功率，每一路都统计  统计每一路的功  统计总功  总功/时间=平均功率
// soc 从电池中读出来进行统计
// dg运行时间，从整流器中读220V交流电压  直流电压和直流总电流就算  dg总功
// mppt中读总充电电量
// dg mppt 负载 总功计算净功
// 负载用电时间，通过控制按钮来统计。

typedef struct // 最多只能256字节  当前结构体占用244字节 。CPU内部flash最多2048字节
{  
  unsigned short 		device_ID;							//设备ID 占3个字节24位   最大 65535
  unsigned short 		collect_frq;						//采集频率 单位s		1=wifi udp和188通讯   2=rj45 upd和188通讯  rs485采集命令发送间隔  ms
  unsigned short 		send_frq;    						//主动发送频率   暂无  但保留   mqtt上传到服务器的时间间隔
	short   int     	TMax;										//通道1温度上限 实际温度*10   主负载继电器闭合高压
  short   int     	TMin;										//通道1温度下限			主负载继电器开路低压
  short   int     	HMax;										//通道1湿度上限			备负载继电器闭合高压
  short   int    		HMin;										//通道1湿度下限			备负载继电器开路低压
  short   int     	TXZ[2];									//温度修正	0=启动发电机电池电压偏低   1=外壳风扇打开高温
  short   int     	HXZ[2];									//湿度修正  0=停止发电机电池电压偏高   1=外壳风扇断开低温  
	short 	int 			VMin;       						//电压下限   -->  2014-08-14 目前修改为掉电是否（短信,电话）报警,0表示不报   本地UTC偏移（分钟）
  unsigned short 	int  		bCH[2];						//通道1是否工作    0=电池类型   1=开机负载连接使能
  unsigned short 	int  		alarmDelyMinute;	//警报检测延时 单位Min 		最多255min即4个小时15min。 mppt个数
  unsigned short 	int  		beep;        			//1表示开启beep,0表示关闭beep    开机beep是否响一声
	
  unsigned short 	int  		wifi_mode;   			//通讯方式  15
 // 	0=wifi	不工作
 //  	1=wifi 	udp通讯 查询方式和mec188配合使用,可以进行设置
 //  	2=wifi 	采用tcp的client模式，和gprs模式传输格式完全一样 
 //  	3=wifi 	路由器模式，开机默认不工作，可以通过按键来设置是否工作。用于数据导出和配置信息等。
  unsigned short 	int 		b_light_work;			//硬件工作模式 0=表示透传  1=表示自主工作模式
  unsigned short 	int			b_gprs_work;			//gprs是否工作 1表示工作   0xff表示未配置，默认工作	   其他值表示不工作  不工作则设置成0
	unsigned short 	int			b_wifi_work;			//wifi是否工作 1表示工作   0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0 
	unsigned short 	int			b_rj45_work;			//rj45是否工作 1表示工作   0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0
	unsigned short 	int			b_debug_work;			//debug 是否工作,=1表示工作，否则不工作。com1打印调试信息。gprs的。
	unsigned short 	int			reset_time;				//负载断开等待时间  min	
	unsigned short 	int			b_Sms_Test;				//停止发电机PV高压
	unsigned short 	int     b_Sms_FxiTime;		//电池容量	
  unsigned short 	int			remotePort;     	//接收端端口  
	unsigned char  					remoteIP[4];      //GPRS接收端IP 255.255.255.255    	
	short   int     				TMax2;						//通道1温度上限 实际温度*10   主负载继电器闭合高压
  short   int     				TMin2;						//通道1温度下限			主负载继电器开路低压
  short   int     				HMax2;						//通道1湿度上限			备负载继电器闭合高压
  short   int    					HMin2;						//通道1湿度下限			备负载继电器开路低压
	unsigned short int 			b_TH_work;				//增加部分
	unsigned char						sysctrl_time[6];	//系统控制器时间分别为 年 月 日 时分 秒  各1字节  10进制
} PVT6_ParaInfo;

//pci-e  lora  wifi   rj45   4g   增加
//先运行rtc



//-------------------- 整流器 -----------------------------------------------------------------------------------------------
typedef struct{
	unsigned char ucValue_Charge_Overcurrent;							//充电过流告警值,单位0.01C10
}struct_alarm_para;														//告警参数

typedef struct{
	unsigned char ucNum_Pack;															//电池组数；1:1组电池；2:2组电池；3:通讯方式
	unsigned short int usCapacity_Rated;												//电池额定容量；单位1Ah
	unsigned short int usRange_Current_Full;										//电池电流满量程；单位1A
}struct_bat_para;															//电池参数

typedef struct{
	unsigned char ucSwitch;																//温度补偿开关；0:关闭补偿1:浮充补偿2:均浮充补偿
}struct_temperature_compensation_para;				//温度补偿参数

typedef struct{
	unsigned char ucMode_BAT_LVDS;													//电池LVDS模式
}struct_lvds_para;														//LVDS 参数
	
typedef struct{
	unsigned char ucFunction_Switch;												//均充功能开关；0 关 闭 1 开 启
	unsigned char ucMode;																	//均充模式；0:手动1:自动
	unsigned char ucPerioid;																//均充周期；单 位 1 月
}struct_equalize_charge_para;									//均充参数

typedef struct{
	unsigned char ucValue_Constant_Voltage_Current_Limit;	//恒压限流值
}struct_charge_current_limit_para;						//充电限流参数

typedef struct
{
	//整流配电
	unsigned char    Mask;																												//多个读同步
	unsigned char 		ComPort;																										//连接整流器的串口编号
	float fV_RC_Distribution_Output;																	//dc输出电压
	float fI_RC_Distribution_Moudle_Output[16];												//RC:Rectification
	float fI_RC_Distribution_Moudle_Output_Total;											//所有模块输出总电流
	unsigned char ucNum_RC_Distribution_Monitor_Module;													//模块个数
	
	unsigned char ucState_RC_Distribution_System_Input_Switch[16][3];						//[0]:开机/关机；[1]:限流/不限流；[2]:浮充/均充/测试
	unsigned char ucAlarm_State_RC_Distribution_Moudle[16][4];										//[0]:模块告警；[1]:自定义1；[2]:自定义2；[3]:自定义3；	
	//交流配电
	float fV_AC_Distribution_A_Phase;																	//交流电压
	float fHz_AC_Distribution;																				//交流频率
	
	struct_alarm_para                     AC_Distribution_Get_Custom_PARA1_Alarm_PARA;									//交流配电获取自定义参数的报警参数
	struct_bat_para                       AC_Distribution_Get_Custom_PARA1_BAT_PARA;										//交流配电获取自定义参数的电池参数
	struct_temperature_compensation_para	AC_Distribution_Get_Custom_PARA1_TC_PARA;											//交流配电获取自定义参数的温度补偿参数，TC为temperature compensation的第一个字母
	struct_lvds_para                      AC_Distribution_Get_Custom_PARA1_LVDS_PARA;										//交流配电获取自定义参数的LVDS参数
	struct_equalize_charge_para           AC_Distribution_Get_Custom_PARA1_EQ_Charge_PARA;							//交流配电获取自定义参数的均充参数
	struct_charge_current_limit_para      AC_Distribution_Get_Custom_PARA1_Charge_Current_Limit_PARA;		//交流配电获取自定义参数的充电限流参数	
	
	//直流配电
	float fV_DC_Distribution_Output;																	//直流配电电压
	float fI_DC_Distribution_Total_Load;															//直流配电总负载电流
	float fPara_DC_Distribution_Get[7];																//[0]:直流V上限；[1]:直流V上限；[2]:电池高温告警值；[3]:环境高温告警值；[4]:高压关机值；[5]:均充电压；[6]:浮充电压
}struct_Rectifier;

//-------------------- PYLON 电池 ----------------------------------------------------------------------------------------------------
#define D_BAT_SETS	2 //电池组数

typedef struct{
	float fT[16][5];													//温度：[0]:Temperature of BMS board；[1]:Avg. temperature of cell 1~5；[2]:Avg. temperature of cell 6~10；[3]:Avg. temperature of cell 11~15；[4]:Temperature of MOS 
	float fI[16];															//电流：正为充电、负为放电
	float fV_Module[16];											//模块电压
	//For US2000B/US2000B-Plus, user defined items = 2. And use remain capacity 1 and module total capacity
	//For US3000 or big capacity (＞ 65Ah), user defined items = 4, the value: remain capacity 1= FFFF, the
  //module total capacity = FFFF. And please use remain capacity 2, and module total capacity 2
//	float fRemain_Capacity_1[D_BAT_SETS];//剩余容量1
//	float fModule_Total_Capacity_1[D_BAT_SETS];//总容量1
	float fRemain_Capacity_2[16];							//剩余容量2
	float fModule_Total_Capacity_2[16];				//总容量2
}struct_get_Analog_Data;

typedef struct{
	float fSingle_Cell_High_Voltage_Limit;		//单芯过压上限
	float fSingle_Cell_Low_Voltage_Limit;			//单芯低压(alarm)
	float fSingle_Cell_Under_Voltage_Limit;		//单芯欠压(protect)
	float fCharge_High_Temperature_Limit;			//充电温度上限
	float fCharge_Low_Temperature_Limit;			//充电温度下限
	float fCharge_Current_Limit;							//充电电流限值
	float fModule_High_Voltage_Limit;					//模块过压上限
	float fModule_Low_Voltage_Limit;					//模块低压(alarm)
	float fModule_Under_Voltage_Limit;				//模块欠压(protect)
	float fDischarge_High_Temperature_Limit;	//放电温度上限
	float fDischarge_Low_Temperature_Limit; 	//放电温度下限
	float fDischarge_Current_Limit;						//放电电流限值	
}struct_get_Host_system_para;								//获取主机系统参数；注意：只获取主机保护参数

typedef struct{
	unsigned char ucBAT_Num_Same_Group;									//同组中电池数量
	unsigned char ucSN_Master_Moudle[18];								//主机的SN
}struct_get_system_basic_info;							//系统基本信息

typedef struct{
	float fCharge_Voltage_Upper_Limit;		//充电电压上限
	float fDischarge_Voltage_Lower_Limit;	//放电电压下限
	float fCharge_Current_Limit;					//最大充电电流
	float fDischarge_Current_Limit;				//最大放电电流
	unsigned char    ucCharge_Discharge_Status;			//充放电状态	
}struct_get_BAT_charge_discharge_MI;				//电池充放电管理信息,MI:management information

typedef struct
{	
	//------------6.系统参数 Holding Register 10  cmd=3--------------------------------
	signed short int Heater_starting_temperature;													//0x0016制热启动温度			/10.0
	signed short int Heater_stop_return_difference_temperature;						//0x0017制热停止回差值		/10.0
	signed short int inside_high_temperature_limit;												//0x001C高温告警温度值		/10.0
	signed short int inside_low_temperature_limit;													//0x001D低温告警温度值		/10.0
	unsigned short int High_DC_voltage_alarm_value;													//0x0026直流电压告警高限	/10.0
	unsigned short int Low_DC_voltage_alarm_value;														//0x0027直流电压告警低限	/10.0
	signed short int Compressor_starting_temperature;											//0x0028制冷启动温度			/10.0
	signed short int Compressor_stop_return_difference_temperature;				//0x0029制冷停止回差值		/10.0
	signed short int Heat_exchanger_starting_temperature;									//0x0038换热启动温度			/10.0
	signed short int Heat_exchanger_stop_return_difference_temperature;		//0x0039换热停止回差值		/10.0
	//-----------------5.传感器数据 Input Register 8	cmd=4-----------------------------
	unsigned short int Internal_fan_speed;								//0内风机转速
	unsigned short int External_fan_speed;								//2外风机转速
	signed short int	Return_air_temp;									//4回风温度				/10.0
	signed short int	Ambient_temp;											//5柜外温度				/10.0
	unsigned short int DC_power_voltage;									//6直流电源电压		/10.0					 
	unsigned short int Heater_current;										//A加热器电流			/100.0
	signed short int Condenser_temp;										//D冷凝盘管温度		/10.0
  signed short int Return_air_humidity;							//E回风湿度				/10.0
	//----------------4.告警数据 Discrete Input Register  15 cmd=2----------------------
	unsigned char Internal_fan_alarm;								//0x0000内风机告警
	unsigned char External_fan_alarm;								//0x0002外风机告警 	
	unsigned char Compressor_fault;									//0x0004压缩机故障
  unsigned char Heater_overload_alarm;							//0x0008加热器过流告警						
	unsigned char Heater_underload_alarm;						//0x0009加热器欠流告警						
	unsigned char Return_air_temp_sensor_fault;			//0x000A回风温度传感器故障				
	unsigned char Condenser_temp_sensor_faul;				//0x000B冷凝盘管温度传感器故障		
	unsigned char High_DC_voltage_alarm; 						//0x000C直流高电压告警
	unsigned char Low_DC_voltage_alarm; 							//0x000D直流低电压告警
	unsigned char Inside_high_temp_alarm;						//0x000E柜内高温告警
	unsigned char Inside_low_temp_alarm;							//0x000F柜内低温告警
	unsigned char Ambient_temp_sensor_fault;					//0x0015柜外温度探头告警
	unsigned char High_humidity;											//0x0018高湿告警
	unsigned char Humidity_sensor_fault;							//0x001A湿度传感器故障
	unsigned char Filter_alarm;											//0x0023过滤网堵塞告警
	//----------------3.状态数据 Coil Register  15  cmd=1---------------------------------
	unsigned char	Internal_fan_state;								//0x0000内风机运行状态
	unsigned char	External_fan_state;								//0x0002外风机运行状态
	unsigned char	Cooing_state;											//0x0004制冷运行状态
	unsigned char	Heating_state;										//0x0005制热运行状态
	unsigned char	Self_check_state;									//0x000B自检状态
	unsigned char	Machine_state;										//0x000C机组运行状态
	unsigned char	heat_exchanger_state;							//0x000E换热状态		
  unsigned char  Mask;
	unsigned char  modbus_id;												//modbus_id
	unsigned char  ComPort;													//com口
}struct_get_air_info;

//-------------------- 逆变器 ----------------------------------------------------------------------------------------------------
typedef struct
{
	unsigned short int usPower_Rated;                       	//额定功率               0x0003
	unsigned short int usVoltage_Nominal_DC_Input;					 	//标称直流输入电压       0x0004
	unsigned short int usVoltage_Nominal_AC_Output;					//标称交流输出电压       0x0005
	unsigned short int usFrequency_Nominal_AC_Output;				//标称交流输出频率       0x0006
	float fVoltage_BAT;												//蓄电池电压             0x0008
	float fCurrent_DC;												//直流电流               0x0009
	float fVoltage_AC;												//交流电压               0x000A
	float fCurrent_AC;												//交流电流               0x000B
	unsigned short int usFault;															//故障                   0x0015
	unsigned short int usAlarm;															//报警                   0x0016
	unsigned int ulTm_Accumulated_Work;								//累计工作时间，单位:分钟0x0022-0x0023
	unsigned int ulTotal_Load_Electricity_Consumption;	//负载总用电，单位:KWh0  x0051-0x0052
	unsigned char  usStatus_Run;                         //运行状态               0x005C	
	unsigned char  Mask;
	unsigned char  modbus_id;														//modbus_id
	unsigned char  ComPort;															//com口
//	unsigned char  ucSN[8];
}struct_inverter;
//-------------------- 风扇 ----------------------------------------------------------------------------------------------------
typedef struct
{
	unsigned short int usSpeed[8];            //风机转速,单位：R.P.M                          0x0008-0x000F
	unsigned char  ucState_Run[8];        //风机运行状态,0：停止；1：运行                 0x0010-0x0017
	signed char  ssT_RTS[4];            //4个RTS温度                                    0x0018-0x001B
	unsigned char  ucState_RTS_Connect[4];//4个RTS连接状态，0:正常；1:短路；2:开路        0x001C-0x001F
	unsigned char  ucMode_Drive;          //风机控制主、被动模式，0：主动模式；1=被动模式 0x0020
	unsigned char  ucAlarm[8];            //8路风机报警,0:正常；1:转速和PWM调速不一致     0x0022-0x0029
	unsigned char  ucFault[8];            //8路风机故障,0:正常；1:不转；2:转动不能停止    0x002A-0x0031
	unsigned char  Mask;										
	unsigned char  modbus_id;							//modbus_id
	unsigned char  ComPort;								//com口
}struct_fan;
//-------------------- 柴油发电机----------------------------------------------------------------------------------------------------
//数值显示
typedef struct
{																																																//读/写	范围L	  测量范围H	 比例因子	单位	位/符号
	unsigned int usOil_pressure;																			//发动机油压,					1024				R	0	       		10000	    	1	    Kpa	  16
	unsigned int usInternal_flexible_sender_A_analogue_input_reading;	//           					40873
	float fCoolant_temperature;																				//冷却液温度,					1025				R -50	     		200	      	1	    ℃	  16 S
	unsigned int usInternal_flexible_sender_B_analogue_input_reading;	//           					43925
	unsigned int usFule_level;                                        //           					1027				R 0	       		130	      	1	    %	    16
	unsigned int usInternal_flexible_sender_C_analogue_input_reading;	//           					43931
	unsigned int usInternal_flexible_sender_D_analogue_input_reading;	//           					43937
	unsigned int usInternal_flexible_sender_E_analogue_input_reading;	//           					43943
	unsigned int usInternal_flexible_sender_F_analogue_input_reading;	//          		 			43949
	float fCharge_alternator_voltage;																	//充电发动机电压,			1028				R	0	       		40	        0.1	  V			16
	float fEngine_battery_voltage;																		//发动机电池电压,			1029				R	0	       		40	        0.1		V			16
	unsigned int 	usEngine_speed;																			//发动机转速,					1030				R	0	       		6000	      1			RPM		16
	unsigned long ulEngine_run_time;																	//发动机运行时间,			1798和1799	R	0	       		4.29 x109		1			秒		32
	unsigned long ulNumber_of_starts;																	//启动次数,						1808和1809	R	0	       		99999										32
}struct_Engine_parameters;//发动机参数

typedef struct
{																															          //读/写	范围L	  测量范围H	 比例因子	单位	位/符号
	float fGenerator_frequency;								//发电机频率					1031				R	0						70					0.1		Hz			16
	float fGenerator_L1_N_voltage;						//L1-N相电压					1032和1033	R	0						18,000			0.1		V				32
	float fGenerator_L2_N_voltage;						//L2-N相电压					1034和1035	R	0						18,000			0.1		V				32
	float fGenerator_L3_N_voltage;						//L3-N相电压					1036和1037	R	0						18,000			0.1		V				32
	float fGenerator_L1_L2_voltage;						//L1-L2线电压					1038和1039	R	0						30,000			0.1		V				32
	float fGenerator_L2_L3_voltage;						//L2-L3线电压					1040和1041	R	0						30,000			0.1		V				32
	float fGenerator_L3_L1_voltage;						//L3-L1线电压					1042和1043	R	0						30,000			0.1		V				32
	float fGenerator_L1_current;							//L1相电流						1044和1045	R	0						99,999.90		0.1		A				32
	float fGenerator_L2_current;							//L2相电流						1046和1047	R	0						99,999.90		0.1		A				32
	float fGenerator_L3_current;							//L3相电流						1048和1049	R	0						99,999.90		0.1		A				32
	float fGenerator_earth_current;						//接地电流						1050和1051	R	0						99,999.90		0.1		A				32
	signed long 	slGenerator_L1_watts;				//L1相有功功率				1052和1053	R	-99,999,999	99,999,999	1			W				32 S
	signed long 	slGenerator_L2_watts;				//L2相有功功率				1054和1055	R	-99,999,999	99,999,999	1			W				32 S
	signed long 	slGenerator_L3_watts;				//L3相有功功率				1056和1057	R	-99,999,999	99,999,999	1			W				32 S
	signed int   ssGenerator_current_lag_lead;//电流超前/滞后				1058				R	-180				180					1			度			16 S
	signed   long slGenerator_total_watts;		//总的有功功率				1536和1537	R	-99,999,999	99,999,999	1			W				32S
	unsigned long ulGenerator_L1_VA;					//L1相视在功率				1538和1539	R	0						99,999,999	1			VA			32
	unsigned long ulGenerator_L2_VA;					//L2相视在功率				1540和1541	R	0						99,999,999	1			VA			32
	unsigned long ulGenerator_L3_VA;					//L3相视在功率				1542和1543	R	0						99,999,999	1			VA			32
	unsigned long ulGenerator_total_VA;				//总视在功率					1544和1545	R	0						99,999,999	1			VA			32S
	signed   long slGenerator_L1_Var;					//L1相无功功率				1546和1547	R	-99,999,999	99,999,999	1			Var			32S
	signed   long slGenerator_L2_Var;					//L2相无功功率				1548和1549	R	-99,999,999	99,999,999	1			Var			32S
	signed   long slGenerator_L3_Var;					//L3相无功功率				1550和1551	R	-99,999,999	99,999,999	1			Var			32S
	signed   long slGenerator_total_Var;			//总无功功率					1552和1553	R	-99,999,999	99,999,999	1	   	Var			32S
	float fGenerator_power_factor_L1;					//L1相功率因素				1554				R	-1	        1	          0.01					16S
	float fGenerator_power_factor_L2;					//L2相功率因素				1555				R	-1					1						0.01					16S
	float fGenerator_power_factor_L3;					//L3相功率因素				1556				R	-1					1						0.01					16S
	float fGenerator_average_power_factor;		//平均功率因素				1557				R	-1					1						0.01					16S
	float fGenerator_percentage_of_full_power;//总功率的百分比			1558				R	-999.9			999.9				0.1		%				16S
	float fGenerator_percentage_of_full_Var;	//总无功功率的百分比	1559				R	-999.9			999.9				0.1		%				16S
	float fGenerator_positive_KW_hours;				//正的千瓦时					1800和1801	R	0						4.29 x109		0.1		KW/h		32
	float fGenerator_negative_KW_hours;				//负的千瓦时					1802和1803	R	0						4.29 x109		0.1		KW/h		32
	float fGenerator_KVA_hours;								//正的KVA/h						1804和1805	R	0						4.29 x109		0.1		KVA/h		32
	float fGenerator_KVAr_hours;							//正的KVAr/h					1806和1807	R	0						4.29 x109		0.1		KVAr/h	32
	signed long slGenerator_L1L2L3_watts;			//总功率
	float fGenerator_L1L2L3_current;					//总电流
}struct_Generator_parameters;//发电机参数

//typedef struct
//{
//	float fMains_frequency;										//市电频率						1059				R	0						70					0.1		Hz			16
//	float fMains_L1_N_voltage;								//L1-N相电压					1060和1061	R	0						18000				0.1		V				32
//	float fMains_L2_N_voltage;								//L2-N相电压					1062和1063	R	0						18000				0.1		V				32
//	float fMains_L3_N_voltage;								//L3-N相电压					1064和1065	R	0						18000				0.1		V				32
//	float fMains_L1_L2_voltage;								//L1-L2线电压					1066和1067	R	0						30000				0.1		V				32
//	float fMains_L2_L3_voltage;								//L2-L3线电压					1068和1069	R	0						30000				0.1		V				32
//	float fMains_L3_L1_voltage;								//L3-L1线电压					1070和1071	R	0						30000				0.1		V				32
//	signed int ssMains_voltage_phase_lag_lead;//市电相序滞后/超前		1072				R	-180				180					1			度			16 S
//	unsigned int 	usMains_phase_rotation;			//市电相序顺序				1074				R	0						3													16
//	signed int 	 	ssMains_current_lag_lead;		//电流滞后/超前				1075				R	-180				180					1			度			16 S
//	float fMains_L1_current;									//L1相电流						1076和1077	R	0						99999.9			0.1		A				32
//	float fMains_L2_current;									//L2相电流						1078和1079	R	0						99999.9			0.1		A				32
//	float fMains_L3_current;									//L3相电流						1080和1081	R	0						99999.9			0.1		A				32
//	float fMains_earth_current;								//接地电流						1082和1083	R	0						99999.9			0.1		A				32
//	signed long 	slMains_L1_watts;						//L1相有功功率				1084和1085	R	-99999999		99999999		1			W				32 S
//	signed long 	slMains_L2_watts;						//L2相有功功率				1086和1087	R	-99999999		99999999		1			W				32 S
//	signed long 	slMains_L3_watts;						//L3相有功功率				1088和1089	R	-99999999		99999999		1			W				32 S
//	signed long 	slMains_total_watts;				//市电总有功功率			1560和1561	R	-99999999		999999999		1			W				32S
//	unsigned long ulMains_L1_VA;							//L1相视在功率				1562和1563	R	0						99999999		1			VA			32
//	unsigned long ulMains_L2_VA;							//L2相视在功率				1564和1565	R	0						99999999		1			VA			32
//	unsigned long ulMains_L3_VA;							//L3相视在功率				1566和1567	R	0						99999999		1			VA			32
//	unsigned long ulMains_total_VA;						//总视在功率					1568和1569	R	0						999999999		1			VA			32
//	signed long 	slMains_L1_Var;							//L1相无功功率				1570和1571	R	-99999999		99999999		1			Var			32S
//	signed long 	slMains_L2_Var;							//L2相无功功率				1572和1573	R	-99999999		99999999		1			Var			32S
//	signed long 	slMains_L3_Var;							//L3相无功功率				1574和1575	R	-99999999		99999999		1			Var			32S
//	signed long 	slMains_total_Var;					//总无功功率					1576和1577	R	-999999999	999999999		1			Var			32S
//	float fMains_power_factor_L1;							//L1相功率因素				1578				R	-1					1						0.01					16S
//	float fMains_power_factor_L2;							//L2相功率因素				1579				R	-1					1						0.01					16S
//	float fMains_power_factor_L3;							//L3相功率因素				1580				R	-1					1						0.01					16S
//	float fMains_average_power_factor;				//平均功率因素				1581				R	-1					1						0.01					16S
//}struct_City_electricity_parameters;				//市电参数

typedef struct{	
	struct_Engine_parameters 						Engine_Para; 					//发动机参数
	struct_Generator_parameters					Generator_Para;				//发电机参数
	//struct_City_electricity_parameters  City_Electricity_Para;//市电参数	
}struct_value_display;//数值显示

////控制按钮:Write
//typedef struct
//{

//}struct_control_button;

//报警
typedef struct
{
	unsigned int  usReg_Addr_39425;																			 										//39425
//	急停	Emergency 		stop																	R	0	15	13/16-16/16
//	发动机低油压				Low oil pressure											R	0	15	9/16-12/16
//	发动机高水温				High coolant temperature							R	0	15	5/16-8/16
//	发机低水温					Low coolant temperature								R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39426;																													//39426
//	低速								Under speed														R	0	15	13/16-16/16
//	超速								Over speed														R	0	15	9/16-12/16
//	低频								Generator Under frequency							R	0	15	5/16-8/16
//	过频								Generator Over frequency							R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39427;																													//39427
//	低压								Generator low voltage									R	0	15	13/16-16/16
//	过压								Generator high voltage								R	0	15	9/16-12/16
//	蓄电池低电压				Battery low voltage										R	0	15	5/16-8/16
//	蓄电池高电压				Battery high voltage									R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39428;																													//39428
//	充电发电机失败	Charge alternator failure									R	0	15	13/16-16/16
//	启动失败						Fail to start													R	0	15	9/16-12/16
//	停机失败						Fail to stop													R	0	15	5/16-8/16
//	发电机合闸失败			Generator fail to close								R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39429;   																											//39429
//	市电合闸失败				Mains fail to close										R	0	15	13/16-16/16
//	油压传感器故障			Oil pressure sender fault							R	0	15	9/16-12/16
//	转速传感器信号丢失	Loss of magnetic pick up							R	0	15	5/16-8/16
//	转速传感器开路			Magnetic pick up open circuit					R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39430;																													//39430
//	过流								Generator high current								R	0	15	13/16-16/16
//																														R	0	15	9/16-12/16
//	低油位							Low fuel level												R	0	15	5/16-8/16
//	CAN ECU报警					CAN ECU Warning												R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39431;																													//39431
//	CAN ECU停机					CAN ECU Shutdown											R	0	15	13/16-16/16
//	CAN ECU数据通讯失败	CAN ECU Data fail											R	0	15	9/16-12/16
//	机油油位低开关量		Low oil level switch									R	0	15	5/16-8/16
//	高水温开关量				High temperature switch								R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39432;																													//39432
//	低燃油低开关量			Low fuel level switch									R	0	15	13/16-16/16
//	扩展模块看门狗报警	Expansion unit watchdog alarm					R	0	15	9/16-12/16
//	超载报警						kW overload alarm											R	0	15	5/16-8/16
//	三相电流不平衡报警	Negative phase sequence current alarm	R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39433;																													//39433
//	接地故障报警				Earth fault trip alarm								R	0	15	13/16-16/16
//	相序报警						Generator phase rotation alarm				R	0	15	9/16-12/16
//	自动电压检测失败		Auto Voltage Sense Fail								R	0	15	5/16-8/16
//	维护保养报警				Maintenance alarm											R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39434;                                 												//39434
//	带载频率报警				Loading frequency alarm								R	0	15	13/16-16/16
//	带载电压报警				Loading voltage alarm									R	0	15	9/16-12/16
//																														R	0	15	5/16-8/16
//																														R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39435;																													//39435
//																														R	0	15	13/16-16/16
//																														R	0	15	9/16-12/16
//	发电机短路					Generator Short Circuit								R	0	15	5/16-8/16
//	市电过流						Mains High Current										R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39436;																													//39436
//	市电接地故障				Mains Earth Fault											R	0	15	13/16-16/16
//	市电短路						Mains Short Circuit										R	0	15	9/16-12/16
//	ECU保护							ECU protect														R	0	15	5/16-8/16
//																														R	0	15	1/16-4/16
	unsigned int  usReg_Addr_39442;																													//39442
//	水温传感器开路报警	Coolant sensor open circuit						R	0	15	13/16-16/16
}struct_alarm;//报警


typedef struct
{
	unsigned char ucDigital_input_A_raw_status;										//数字输入A状态				R	0	1	16	43520
	unsigned char ucDigital_input_A_processed_status;							//数字输入A处理状态		R	0	1	16  43521
	unsigned char ucDigital_input_B_raw_status;										//数字输入B状态				R	0	1	16	43522
	unsigned char ucDigital_input_B_processed_status;							//数字输入B处理状态		R	0	1	16 	43523
	unsigned char ucDigital_input_C_raw_status;										//数字输入C状态				R	0	1	16	43524
	unsigned char ucDigital_input_C_processed_status;							//数字输入C处理状态		R	0	1	16	43525
	unsigned char ucDigital_input_D_raw_status;										//数字输入D状态				R	0	1	16	43526
	unsigned char ucDigital_input_D_processed_status;							//数字输入D处理状态		R	0	1	16	43527
	unsigned char ucDigital_input_E_raw_status;										//数字输入E态					R	0	1	16	43528
	unsigned char ucDigital_input_E_processed_status;							//数字输入E处理状态		R	0	1	16	43529
	unsigned char ucDigital_input_F_raw_status;										//数字输入F状态				R	0	1	16	43530
	unsigned char ucDigital_input_F_processed_status;							//数字输入F处理状态		R	0	1	16	43531
	unsigned char ucDigital_input_G_raw_status;										//数字输入G状态				R	0	1	16	43532
	unsigned char ucDigital_input_G_processed_status;							//数字输入G处理状态		R	0	1	16 	43533
	unsigned char ucDigital_input_H_raw_status;										//数字输入H状态				R	0	1	16	43534
	unsigned char ucDigital_input_H_processed_status;							//数字输入H处理状态		R	0	1	16	43535
}struct_digital_input;//自定义数字量输入

typedef struct
{
	unsigned char ucFuel_Relay;																		//燃油输出						R	0	1	16	48640
	unsigned char ucStart_Relay;																	//启动输出						R	0	1	16	48641
	unsigned char ucDigital_Output_C;															//自定义输出C（无源）	R	0	1	16	48647
	unsigned char ucDigital_Output_D;															//自定义输出D（无源）	R	0	1	16	48646
	unsigned char ucDigital_Output_E;															//自定义输出E					R	0	1	16	48642
	unsigned char ucDigital_Output_F;															//自定义输出F					R	0	1	16	48643
	unsigned char ucDigital_Output_G;															//自定义输出G					R	0	1	16	48644
	unsigned char ucDigital_Output_H;															//自定义输出H					R	0	1	16	48645
	unsigned char ucDigital_Output_I;															//自定义输出I					R	0	1	16	48660
	unsigned char ucDigital_Output_J;															//自定义输出J					R	0	1	16	48661
}struct_digital_output;//自定义数字量输出

typedef struct
{
	struct_digital_input 		Digital_Input;	//自定义数字量输入
	struct_digital_output 	Digital_Output;//自定义数字量输出
}struct_IO_Point;//IO点

typedef struct
{
	unsigned char ucSTOP_LED_status;												//停止状态							R	0	1	16	48648
	unsigned char ucMANUAL_LED_status;											//手动状态							R	0	1	16	48649
	unsigned char ucTEST_LED_status;												//测试状态							R	0	1	16	48650
	unsigned char ucAUTO_LED_status;												//自动状态							R	0	1	16	48651
	unsigned char ucGEN_LED_status;													//机组有效							R	0	1	16	48655
	unsigned char ucGEN_BREAKER_LED_status;									//机组断路器合闸				R	0	1	16	48654
	unsigned char ucMAINS_LED_status;												//市电有效 							R	0	1	16	48652
	unsigned char ucMAINS_BREAKER_LED_status;								//市电断路器合闸 				R	0	1	16	48653
	unsigned char ucUSER_LED_1_status;											//用户自定义LED1状态 		R	0	1	16	48656
	unsigned char ucUSER_LED_2_status;											//用户自定义LED2状态		R	0	1	16	48657
	unsigned char ucUSER_LED_3_status;											//用户自定义LED3状态		R	0	1	16	48658
	unsigned char ucUSER_LED_4_status;											//用户自定义LED4状态		R	0	1	16	48659
}struct_led_status;//LED 状态

typedef struct
{
	unsigned char Mask;
	unsigned char ComPort;
	unsigned char modbus_id;													//modbus_id
	struct_value_display 								Value_Display;//数值显示
	struct_alarm                        Alarm;        //报警
	struct_IO_Point											IO_Point;			//IO点
	struct_led_status										LED_Status;		//LED 状态
}struct_diesel_generator;
//--------------------------------------------------------------------------------------------------------------------------------

typedef struct
{
	float 							faverage_module_voltage;				//平均电压				V		average module voltage  		.3f 	2Byte
	float 							ftotal_current;									//总电流					A		total current   						.2f		2Byte
	unsigned char 			ucaverage_SOC;									//平均SOC					%		average SOC 								0			1Byte
	unsigned short int 	ucaverage_cycle_number;					//平均循环次数				average cycle number				0			2Byte
	unsigned char    		ucaverage_SOH;									//平均 SOH				%		average SOH 								0			1Byte
	float								faverage_cell_temperature; 			//单芯平均温度		K		average cell temperature   	.1f 	2Byte
	float								faverage_MOSFET_temperature;		//MOSFET 平均温度	K		average MOSFET temperature	.1f 	2Byte
	float								faverage_BMS_temperature;				//BMS 平均温度		K		average BMS temperature			.1 		2Byte
}struct_get_systemp_analog_info;						

typedef struct{
	unsigned char ucStatus[16][5];												//取文档中获取的报警状态中的status1-5
}struct_get_alarm_info;

typedef struct
{
		unsigned short int   	 													Mask;																				//多个读同步
		unsigned char 																	ComPort[D_BAT_SETS];												//电池连接的串口编号
//	float fT[D_BAT_SETS][16][5];//[0]:Temperature of BMS board；[1]:Avg. temperature of cell 1~5；[2]:Avg. temperature of cell 6~10；[3]:Avg. temperature of cell 11~15；[4]:Temperature of MOS 
//	float fI[D_BAT_SETS][16];//正为充电、负为放电
//	float fV_Module[D_BAT_SETS][16];
//	//For US2000B/US2000B-Plus, user defined items = 2. And use remain capacity 1 and module total capacity
//	//For US3000 or big capacity (＞ 65Ah), user defined items = 4, the value: remain capacity 1= FFFF, the
//  //module total capacity = FFFF. And please use remain capacity 2, and module total capacity 2
////	float fRemain_Capacity_1[D_BAT_SETS];//剩余容量1
////	float fModule_Total_Capacity_1[D_BAT_SETS];//总容量1
//	float fRemain_Capacity_2[D_BAT_SETS][16];//剩余容量2
//	float fModule_Total_Capacity_2[D_BAT_SETS][16];//总容量2
	unsigned char 																	ucSystem_Alarm[D_BAT_SETS][4];							//4.3 系统告警保护/alarm & protection info of system
	unsigned char    																ucSN[D_BAT_SETS][16];												//序列号
	
//	unsigned char    ucStatus_Alarm[D_BAT_SETS][16][5];//取文档中获取的报警状态中的status1-5
	
	struct_get_Analog_Data              Get_Analog_Data[D_BAT_SETS];
	struct_get_Host_system_para         Get_Host_System_PARA[D_BAT_SETS];
	struct_get_system_basic_info 				Get_System_Basic_INFO[D_BAT_SETS];
	struct_get_BAT_charge_discharge_MI  Get_BAT_Charge_Discharge_MI[D_BAT_SETS];
	struct_get_alarm_info               Get_Alarm_INFO[D_BAT_SETS];
	struct_get_systemp_analog_info			Get_System_Analog[D_BAT_SETS];
}struct_PYLON_Battery;
//--------------------------------------------------------------------------------------------------------------------------------



