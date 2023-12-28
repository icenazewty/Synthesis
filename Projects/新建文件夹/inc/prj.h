#define D_NORMALREADAD 1
#define D_INFO  "\nSTM32F103RC;TCP_Service;TCP_Client;Ch340;IIC 24C32/4K;KEY1/Input;Version 1.1;DATE="##__DATE__##";TIME="##__TIME__##";"
	
#define		 REALY_ALARM						0				//将rs485的方向控制脚作为声光报警控制，当使用pt100和485的时候，必须设为0
#define    CO2										0       //将pt100定义为CO2
#define 	 FIX_RJ45								0				//固定为rj45模式
#define    EEPROM		0											//定义参数存储是否为eeprom
#define		 RJ45_TCPSERVER_S1			0				//rj45 tcp server是否开启	
#define		 SMS_TEST_DAY						5				//每个月几号发测试短信
#define		 SMS_TEST_HOUR					8				//几点发测试短信 	

//#define	WIFI_START_ADDR   				0x8000000+2048*126				//gs_SaveWifiCfg			wifi网络参数


#define			RS232_MODULE_COM2			0						//wifi的com2第一个串口是否为串口模式
#define			MODULE_4G  			1									//是否4g版本。=1表示4g版本 	ec20

#define SMTDOG_V34					1									//在3.3的基础上修改如下: wdt的非门修改为异或门，电路。 lcd增加reset线boot1(pb2)默认下拉。
#define	SMTDOG_V33					1									//3.3版本报警按键盘为高有效=1表示3.3  =0表示3.1及更低版本
#define	DISABLE_SWD_DEBUG		1									//3.3版本  在调试的时候必须=0表示swd功能开启,在发行的时候必须=1表示swd功能关闭。
#define	DGG									1									//ble超时调试,=1表示不表示   =0表示ble进入调试状态, ble工作模式用红灯表示  ble已连接用beep响表示。  =0则报警灯,beep不能控制
#define TIME_DBG						0									//时间调试,当时间<2002年则beep响
#define	SYSTICK_RTC					0									//g_sysTick变量从rtc中断中取出。时间可以统一，如果不从中取出，则上传时间间隔可能秒上有变化。1ms定时器从什么地方来。=1表示32.768KHz。
//#define AMPER_RTC_PC13		1									//RTC时钟/64 输出。
#define MCO_PA8							0									//系统时钟 输出。HSE输出。=0表示LCD功能  =1表示MCO输出。
#define GPRS_DEBUG					1									//gprs工作过程打印到com1口
#define GPRS_TCP_DEBUG			1									//gprs工作过程打印到com1口
#define	PROTOCOL 						1									//是否增加应答协议  即在发送(合计78字节)33字节位置将','-->'X'(X=A+可变数字)，返回则可以判断 13位置为' '或发送中的X。
#define 										G3_DUST	1					//将土壤温湿修改为粉尘。传感器类型=5 并且第2通道
#define 										RT	1							//将土壤温湿修改为噪声。传感器类型=5 并且第1通道
#define 			BUFFER_SIZE_GPRS	80						//实际78字节	80+12=92
#define 			BUFFER_SIZE_GSM 	256
#define 			bool		unsigned char
#define       UPDATANUM						12
#define 			GPRS_TIMEOUT				300				//5min 如果gprs数据5分钟还没有发送出去，则直接进入补传队列中		240->300->200
#if 0
typedef struct  
{
	unsigned char ucStartFlag;
	unsigned char ucSelfIP[4] ;
	unsigned char ucSubMASK[4];
	unsigned char ucGateWay[4];
	unsigned char ucMAC[6];	
	unsigned char ucDestIP[4] ;	
	unsigned char ucDestPort[2];
	unsigned char ucSourcePort[2];
	unsigned char ucMonitorPort[2];
	unsigned char ucSocket0Mode;
	unsigned char ucSocket1Mode;	
	unsigned char ucEndFlag;
}t_SaveNetIPCfg ;
#endif




typedef struct _Repair							// 最多只能256字节  当前结构体占用244字节
{
	int		active;											//芯片是否有效
	int		sn;													//芯片型号
  int 	pagesize;										//页大小256B
	int		sectorsize;									//扇区大小4096B
	int		blocksize;									//块大小32768B
	int	  pages;											//页数量
	int		sectors;										//扇区数量
	int		blocks;											//块数量
	
	int		RepairStartPos;							//补传数据开始位置
	int		RepairBlocks;								//补传数据占用的块数量
	int		RepairDatSize;							//补传数据大小
	int		RepairDatCnt;								//补传数据条数	
	int		RepairInfoStartPos;					//补传信息开始位置
	int		RepairInfoSectorsReserved;	//补传信息占用的扇区数量	
	
	int		SaveDataStartPos;							//存储数据开始位置
	int		SaveDataBlocks;								//存储数据占用的块数量
	int		SaveDatSize;									//存储数据大小
	int		SaveDatCnt;										//存储数据条数	
	int		SaveDataInfoStartPos;					//存储信息开始位置
	int		SaveDataInfoSectorsReserved;	//存储信息占用的扇区数量	
	
} RepairData;


typedef struct _Save			
{
	unsigned int		unixtime;					//unix 时间以2000年 1月1日 00:00:00作为0秒进行计算。
	int							data1;						//默认温度
  int							data2;						//默认湿度
	unsigned char   td_datafromat;		//通道及数据格式。第1字节高4位决定后面的数据格式(温度+湿度+报警状态);第1字节低4位表示通道号(最多16个通道)。
	unsigned char		alarm_satus;			//报警状态
	unsigned char   Reserved[2];			//保留2字节扩展用
} SaveData;													//合计16字节

#define MAXRecvBuf 1024    					//changed by wjj 30->128
#define MAXCFGBuf  500 							//512->500

//++++++++++++++++++++++++++++++gprs有关++++++++++++++++++++++++++++++++++++++++++++++++
// 短消息参数结构，编码/解码共用
// 其中，字符串以'\0'结尾
typedef struct
{
	char SCA[16];				// 短消息服务中心号码(SMSC地址)
	char TPA[16];				// 目标号码或回复号码(TP-DA或TP-RA)
	char TP_PID;				// 用户信息协议标识(TP-PID)
	char TP_DCS;				// 用户信息编码方式(TP-DCS)
	char TP_SCTS[16];		// 服务时间戳字符串(TP_SCTS), 接收时用到
	char TP_UD[161];		// 原始用户信息(编码前或解码后的TP-UD)
	char TP_UD_Len;			// tp_ud字符串长度
	char index;					// 短消息序号，在读取时用到
}SM_PARAM;

	
/*成员变量*/

typedef struct 
{
		char	gsm[BUFFER_SIZE_GSM];			//发送的短信数据
		char	telp[16];									//发送的目的手机号码
		int		len;											//发送的短信数据长度
		unsigned int	timeout_sec;			//超时时间	
		unsigned int	starttime;				//开始时间		
	  int						errcnt;						//失败次数
}gsm_data;

typedef struct 
{		
		char	telp[16];									//发送的目的手机号码	
		unsigned int	timeout_sec;			//超时时间	
		unsigned int	starttime;				//开始时间		
	  int						errcnt;						//失败次数
}tel_data;

typedef struct 
{
		unsigned int  alarmstarttime;		//报警开始时间
		int						alarmsmscnt;			//报警短信发送的条数,当为0则表示没有发送过报警短信，当>0则表示已报警过，当恢复正常则理解变为0。
		char					alarmstatus;			//报警状态，当=0则表示正常状态，当=1则表示处于报警状态		
}sms_data;

#define bit(x) (1<<(x))
//#define D_Packed  

struct t_RTC{
	unsigned short ushYear;
	unsigned char ucMonth;
	unsigned char ucDay;
	unsigned char ucHour;
	unsigned char ucMinite;
  unsigned char ucSecond;
};
struct t_Data{
	struct t_RTC s_RTC;
	//unsigned char ucA[100];
	unsigned long ucD;
	unsigned char ucXORCheck;
	unsigned long ulSumCheck;
};

int get_FMC_DataItemNumber_(void);
void getCheck(unsigned char *pucXORCheck,unsigned long *pulSumCheck,struct t_Data *ps_Data);
int FMC_Write_(struct t_Data *ps_Data);
int FMC_Read_(struct t_Data **ps_Data);

void	 RTC_Init(void);
extern volatile unsigned long gulLastError;
