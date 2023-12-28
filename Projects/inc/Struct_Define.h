#include "gd32e50x.h"
#include <stdbool.h>

#define D_USART_REC_BUFF_SIZE				4096
#define MAXRecvBuf 		512    					//changed by wjj 30->128
#define	ATCMDCOUNT		128								//wifi&ble å‘½ä»¤ä¸ªæ•°

//#define	WIFI_BLE_COM	2								//wifi&ble comå£ COM3   smtdog260 v3.62
//#define	USB_DBG_COM		0								//printf   comå£ COM1  	smtdog260 v3.62

#define	AUTO_RS485_COM		0								//rs485    comå£ COM1   	meter v1
#define	WIFI_BLE_COM			1								//wifi&ble comå£ COM2   	meter v1
#define	USB_DBG_COM				2								//printf   comå£ COM3   	meter v1
#define	METER_COM					3								//meter    comå£ COM1   	meter v1

#define USB_CAN						0			//1=can  0=usb

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

typedef struct{
	volatile unsigned char  RX_Buf[D_USART_REC_BUFF_SIZE];
	volatile unsigned short usRec_WR;
	volatile unsigned short usRec_RD;
	volatile unsigned int   usTick;
	volatile unsigned short usDelay_Rev;		//è¿ç»­æ¥æ”¶2ä¸ªå­—èŠ‚ä¹‹é—´æœ€å¤§æ—¶é—´é—´éš” å•ä½ms  4800bps  500å­—èŠ‚/ç§’  1å­—èŠ‚æœ€æ™šä¸º2ms
	
}struct_usart;

typedef struct{
	struct_usart 			Usart[6];
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

typedef struct {
	float fBAT,fP12;
}struct_Voltage;

typedef struct{
	signed char scRTS;
	signed char scAmb;
}struct_Temperature;

#define D_V_ADC_MAX_NUM 20
#define D_I_ADC_MAX_NUM	100
typedef struct {
	unsigned short BAT[D_V_ADC_MAX_NUM];
	unsigned short P12[D_V_ADC_MAX_NUM];
	unsigned short Trts[3];    
	unsigned short Tamb[3];
//	unsigned short TempBuf[D_V_ADC_MAX_NUM];
}struct_ADC;

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

typedef struct {
	unsigned int uiRTC_Count;//rtc Ãë¼ÆÊıÖµ£¬´Ó2000-01-01 0:00:00¿ªÊ¼ËãÆğ£¬Èç2001Äê1ÔÂ1ÈÕ0:00:00µÄÖµÎª£º366*24*3600=31622400
}Tm_struct;


typedef struct {
	unsigned short usIWDG;
	unsigned short usMain_Program_Non_Normal_Run;
}CNT_struct;

#define D_DI_DO_CH_MAX_NUM 8
typedef struct{
	bool bInput_State[D_DI_DO_CH_MAX_NUM];
//	unsigned char ucInput_State[D_DI_DO_CH_MAX_NUM];
	unsigned char  ucJudge_Input_State[D_DI_DO_CH_MAX_NUM];
}DI_struct;

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
	unsigned short Year;				//Äê·İ¸ß2Î»£¬Hex¸ñÊ½£¬Àı2000Äê£¬Year=0x7D0
	unsigned char  Month;				//ÔÂ·İ£¬Hex¸ñÊ½£¬Àı12ÔÂ£¬Month=0x0C
	unsigned char  Day;					//Ìì£¬Hex¸ñÊ½£¬Àı20ºÅ£¬Day=0x14
	unsigned char  Week;
	unsigned char	 Hour,Minute,Second;
}calendar_struct;


struct adjust
{
	int16_t  DC_Current_Offset;
	int16_t  AC_Current_Offset;
	int16_t  AC_Voltage_Offset;
	uint16_t DC_Voltage_Coefficient; //µ÷ÕûÏµÊı
	uint16_t DC_Current_Coefficient;
	uint16_t AC_Voltage_Coefficient;
	uint16_t AC_Current_Coefficient;
	uint16_t AC_Current_xishu;
  //uint16_t AC_Current_begin;
	uint16_t DC_Current_xishu;
  //uint16_t DC_Current_begin;
	uint16_t DC_Voltage_xishu;
  //uint16_t DC_Voltage_begin;
	uint16_t AC_Voltage_xishu;   
  uint16_t AC_Voltage_high1;
  uint16_t AC_Voltage_high2;
  uint16_t AC_Voltage_high3;
  uint16_t AC_Voltage_low1;
  uint16_t AC_Voltage_low2;
  uint16_t AC_Voltage_low3;
  uint16_t Vol_init_ADJ1;
  uint16_t Vol_init_ADJ2;
  uint16_t Vol_init_ADJ3;
	unsigned char  Date[6];                        //¸ÄĞ´ÈÕÆÚ 	
	unsigned char  CRC_Cal[2];
  //uint16_t AC_Voltage_begin;  
  //uint8_t  Write_Flag;	
  //uint8_t  Write_time[3]; //¸ÄĞ´Ê±¼ä
  //uint8_t  Run_Flag;
  //uint8_t  data[3];
  //uint16_t data16[3];
};

struct status
{	
	uint32_t bal;   
	uint16_t DC_VOL;    
	//uint16_t DC_CUR; 
	uint16_t DC_CUR; 
	uint16_t AC_VOL;	
	uint16_t AC_CUR;	
	int16_t  tempture[2];              //ÎÂ¶ÈÏÔÊ¾Öµ
	uint8_t  Flag_Main_Program_Normal_Run;
	uint32_t Main_Program_Non_Normal_Run_Count;
	uint8_t   Power_ON_First;
	uint8_t   AC_SW_Previous;
	uint8_t  count;
	uint8_t  Error_Record_No;
	uint8_t  Boat_SWitch;
	uint8_t  LVD_HVD_TEMP_OFF_Flag;	   //µÍ¡¢¸ßÑ¹¶Ï¿ª±êÖ¾
	uint32_t LVD_HVD_TEMP_OFF_Time;    //µÍ¡¢¸ßÑ¹¶Ï¿ªÊ±¼ä
	uint8_t  LVD_HVD_TEMP_Shutdown;
	uint8_t  Heatsink_A_TEMP_State;    //É¢ÈÈÆ÷AÎÂ¶È×´Ì¬=0£ºÕı³££»=1£ºÎÂ¶È¸ß£»=2ÎÂ¶È´«¸ĞÆ÷¹ÊÕÏ
	uint8_t  Heatsink_B_TEMP_State;    //É¢ÈÈÆ÷BÎÂ¶È×´Ì¬=0£ºÕı³££»=1£ºÎÂ¶È¸ß£»=2ÎÂ¶È´«¸ĞÆ÷¹ÊÕÏ
	uint8_t  Ambient_TEMP_State;       //»·¾³ÎÂ¶È×´Ì¬   =0£ºÕı³££»=1£ºÎÂ¶È¸ß£»=2ÎÂ¶È´«¸ĞÆ÷¹ÊÕÏ
	uint8_t	 dip_switch;               //²¦Âë¿ª¹Ø
	uint16_t Load_State;              //¸ºÔØ×´Ì¬
//bit0: 	ÊÖ¶¯¿ª»ú
//bit1: 	ÊÖ¶¯¹Ø»ú
//bit2: 	Ô¶³Ì¿ª¹Ø¿ª»ú
//bit3: 	Ô¶³Ì¿ª¹Ø¹Ø»ú
//bit4: 	¹ÊÕÏ¹Ø»ú
//bit5: 	¶¨Ê±×Ô¶¯¿ª»ú
//bit6: 	¶¨Ê±×Ô¶¯¹Ø»ú
//bit7: 	µÍ¹¦ºÄ¹Ø»ú
//bit8: 	µÍ¹¦ºÄ¿ª»ú 
//bit9: 	¸ºÔØ¹ıÔØ¹ÊÕÏ
//bit10:	¸ºÔØ¶ÌÂ·¹ÊÕÏ
//bit11:	HVD¸ßÑ¹¹ÊÕÏ
//bit12:	LVDµÍÑ¹¹ÊÕÏ
//bit13:	¸ºÔØ¸ßÎÂ¹ÊÕÏ
//bit14:	Í¨Ñ¶ÃüÁî¹Ø±Õ	
//bit15   Í¨Ñ¶ÃüÁî¿ª»ú
	//ÉÏÃæÊÇ×îĞÂµÄ×´Ì¬±äÁ¿µãÎ»¶¨Òå¡£
	
	//status.Load_State:¸ºÔØ×´Ì¬	
//bit0: 	±ê×¼¸ºÔØ¿ª
//bit1: 	±ê×¼¸ºÔØ¹Ø
//bit2: 	±ê×¼µÍ¹¦ºÄ´ı»ú
//bit3: 	±ê×¼µÍ¹¦ºÄ¸ºÔØÕı³£
//bit4: 	ÊÖ¶¯¹Ø
//bit5: 	¶¨Ê±×Ô¶¯¿ª
//bit6: 	¶¨Ê±×Ô¶¯¹Ø
//bit7: 	µÍ¹¦ºÄ¹Ø»ú
//bit8: 	µÍ¹¦ºÄ¿ª»ú 
//bit9: 	¸ºÔØ¹ıÔØ
//bit10:	¸ºÔØ¶ÌÂ·
//bit11:	HVD¸ßÑ¹¶Ï¿ª
//bit12:	LVDµÍÑ¹¶Ï¿ª
//bit13:	¸ºÔØ¸ßÎÂ¶Ï¿ª
//bit14:	Í¨Ñ¶ÃüÁî¹Ø±Õ	
//bit15   Í¨Ñ¶ÃüÁî¿ª»ú
	
	
	
	
	uint32_t  Comm_modify_flag;
	uint32_t  rtc_tm_count;

	uint32_t Overcurrent_Time;      //¹ıÁ÷Ê±¼ä
	uint8_t  Overcurrent_Flag;      //¹ıÁ÷±êÖ¾
	uint8_t  Overcurrent_SN;        //>600WÎª1£»>450WÎª2£»>300WÎª3
	uint8_t  Overcurrent_Shutdown;
	uint8_t  AC_Start;
	uint8_t  run_status;            //ÔËĞĞ±êÖ¾
	uint8_t  Test_Status;
	uint16_t Test_Count;
	uint8_t  key;
	uint16_t Error_Flag;
	uint16_t Alarm_Flag;      										
	uint8_t  LED_Flag;
	uint16_t LED_Count;
	uint8_t  Buzzer_Flag;
	uint16_t Buzzer_Count;
	uint8_t  CUR_T;
	uint32_t Cur_T_time;
	uint8_t  Cur_T_Flag;
	uint8_t  Fan_Flag;
	uint8_t  Fan_EN;
	//uint8_t  time[7];
	//uint8_t  error_no[10];
	//uint8_t  alarm_no[10];
	uint16_t error_time;
	uint16_t alarm_time;
	uint32_t  idel_time; 
	uint8_t  AC_Enable;
	uint8_t  IDEL_Flag;
	uint8_t  AC_Output_EN;
	uint16_t AC_Exceeding;    
	//uint16_t AC_VOL;
	uint16_t AC_Power;
	uint16_t DC_Power;   
	uint16_t ac_vol_a;
	uint16_t ac_cur_a;
	uint16_t dc_12V_a;
	uint16_t dc_5V_a;
	uint16_t dc_Cell_a;
	uint16_t dc_vol_a;
	uint16_t dc_cur_a;		
	uint16_t dc_DCU_a;
	uint16_t DCV_12V;	
	uint16_t DCV_5V;
	uint16_t DCV_Cell;	
	uint16_t DCV_DCU;
//	int8_t  tempture[3];   //ÎÂ¶ÈÏÔÊ¾Öµ
	uint16_t tempture_a[2]; //ÎÂ¶ÈADÖµ
	//uint32_t Run_time;
	//uint32_t record_address;      
};

struct sw
{
	uint8_t  Buzzer_set;
	uint8_t  Time_set;
	uint8_t  LVD_set;
	uint8_t  Power_off_set;
	uint16_t Power_standby;
	uint16_t LVD_Disconnect_Vol;
	uint16_t LVD_Warn_Vol;
	uint16_t LVD_Restore_Vol;
};

struct setting
{	
	uint16_t Rated_DC_voltage;  //¶î¶ÈÊäÈëµçÑ¹	
	uint16_t Rated_Power;       //¶î¶¨¹¦ÂÊ
	uint16_t DC_HVD_Disconnect; //×î´óÊäÈëµçÑ¹¶Ï¿ªÖµ
	uint16_t DC_LVD_Disconnect; //×îĞ¡ÊäÈëµçÑ¹¶Ï¿ªÖµ
	uint16_t DC_HVD_Warn;       //ÊäÈëµçÑ¹±¨¾¯ÉÏÏŞÖµ
	uint16_t DC_LVD_Warn;       //ÊäÈëµçÑ¹±¨¾¯ÏÂÏŞÖµ
	uint16_t Power_Alarm;       //Êä³ö¹¦ÂÊ±¨¾¯
	uint16_t Power_Error;       //Êä³ö¹¦ÂÊ¶Ï¿ª
	uint8_t  Power_DC_Min[4];   //4µµ¶Ï¿ª¹¦ÂÊÉè¶¨Öµ 	
	uint16_t Alarm_Time;        //±¨¾¯ÑÓÊ±¶Ï¿ªÊ±¼ä
	uint16_t Error_Time;        //´íÎóÑÓÊ±¶Ï¿ªÊ±¼ä
	uint16_t IDLE_Time;         //¿ÕÏĞ×´Ì¬¼à²âÊ±¼ä¼ä¸ô
	uint16_t AC_Rated_Voltage;  //AC¶î¶ÈÊä³öµçÑ¹ 
	uint16_t DC_LVR;            //µÍµçÑ¹»Ö¸´
	uint16_t DC_HVR;            //¸ßµçÑ¹»Ö¸´
	uint16_t Bal_Alarm;         //Óà¶î±¨¾¯Öµ           500-65000
	uint16_t Bal_OFF;           //Óà¶î¶Ï¿ªÖµ           100-2000
	uint16_t StartLVD_Disconnect;  //Á¢¼´¿ªÊ¼LVDµçÑ¹,Ğ¡ÓÚÕâ¸öÖµÑÓÊ±2Ãë¶Ï¿ª
	uint16_t StartHVD_Disconnect;  //Á¢¼´¿ªÊ¼HVDµçÑ¹,´óÓÚÕâ¸öÖµÑÓÊ±2Ãë¶Ï¿ª
	uint16_t LVD_Disconnect_Delay; //LVD¶Ï¿ªÑÓÊ±Ê±¼ä,µ¥Î»Ãë
	uint16_t HVD_Disconnect_Delay; //HVD¶Ï¿ªÑÓÊ±Ê±¼ä,µ¥Î»Ãë	
	uint32_t  Time_Start[4];
	uint32_t  Time_Stop[4];
	uint8_t  frequence;         //¶î¶¨ÆµÂÊ	
	uint8_t  Temp_Alarm;        //±¨¾¯ÎÂ¶È
	uint8_t  Temp_Error;        //¶Ï¿ªÎÂ¶È
	uint8_t  Temp_Fan_Start;    //·çÉÈÆô¶¯ÎÂ¶È
	uint8_t  Temp_Fan_Stop;     //·çÉÈÍ£Ö¹ÎÂ¶È
//	uint16_t DC_Rated_Voltage;  //±ê³ÆDCÊäÈëµçÑ¹
//	uint8_t  DC_BL;             //»Ö¸´µçÑ¹²îÖµ  
//	uint8_t  made_time[3];      //³ö³§ÈÕÆÚ
//	uint8_t  use_time[3];       //Í¶ÓÃÈÕÆÚ	
//	uint16_t LVDWarn_Beep_Start;//LVD·äÃùµçÑ¹£¨¿ªÊ¼£©
//	uint16_t LVDWarn_Beep_Stop; //LVD·äÃù¸´Î»£¨Í£Ö¹£©µçÑ¹
//	uint16_t HVDWarn_Beep_Start;//HVD·äÃùµçÑ¹£¨¿ªÊ¼£©
//	uint16_t HVDWarn_Beep_Stop; //HVD·äÃù¸´Î»£¨Í£Ö¹£©µçÑ¹	
	char Blue_ID[20];
	char Machine_ID[16];
	unsigned char  Date[6];    //¸ÄĞ´Ê±¼ä	
	unsigned char  CRC_Cal[2];
};

/*struct serial
{
	uint8_t GetData[USART1_GetDataNum];              //¶¨Òå½ÓÊÕÊı×é  
    uint8_t GetDataCopy[USART1_GetDataNum];          //¶¨Òå½ÓÊÕÊı×éÓĞĞ§Êı¾İÔİ´æÇø 
    uint8_t OutData[USART1_OutDataNum];              //¶¨Òå·¢ËÍÊı×é  
    uint16_t MobusWord[USART1_MobusWordNum];         //MOBUS RTU¼Ä´æÆ÷16bit×Ö½ÚÊı×é 
    uint8_t GetDataCount;                            //´®¿Ú½ÓÊÕÊı¾İ¸öÊı  
    uint8_t GetDataCountCopy;                        //´®¿Ú½ÓÊÕÊı¾İ¸öÊı×ª´æÇø 
    uint8_t OutDataCount;                            //´®¿Ú·¢ËÍÊı¾İ¸öÊı  
    uint8_t GetDataFlag;                             //´®¿Ú½ÓÊÕÊı¾İ¼ÌĞøÓĞĞ§±êÊ¶·û,0ÎªÎŞĞ§£¬´óÓÚ0ÎªÓĞĞ§ 
    uint16_t GetMobusAddr;                           //´®¿Ú½ÓÊÕµ½Ö¸Áî²Ù×÷µØÖ·  
    uint16_t GetMobusAddrCopy;                       //´®¿Ú½ÓÊÕµ½Ö¸Áî²Ù×÷µØÖ·  
    uint8_t GetMobusOrder;                           //´®¿Ú½ÓÊÕµ½Ö¸Áî²Ù×÷ÃüÁî  
    uint8_t GetMobusOrderCopy;                       //´®¿Ú½ÓÊÕµ½Ö¸Áî²Ù×÷ÃüÁî 
    uint8_t GetMobusLen;                             //´®¿Ú½ÓÊÕµ½Ö¸Áî²Ù×÷Êı¾İ³¤¶È  
    uint8_t GetMobusOrderFlag;                       //´®¿Ú½ÓÊÕµ½Ö¸Áî²Ù×÷Êı¾İÊÇ·ñÓĞĞ§±ê¼Ç·û     
};*/

struct MODE_delay
{
    uint8_t Timer_Flag;
    uint8_t start_time[4][3];
    uint8_t stop_time[4][3]; 
};

struct Time_Count
{
    uint8_t  Time_Flag;
    uint16_t Invert_ON;
    uint16_t DC_Low_OFF_AC;
    uint16_t AC_Low;
    uint16_t Standby_Check;
    uint8_t  StartLVD_Disconnect;
    uint8_t DC_LVD_Disconnect;
    uint8_t DC_LVD_Warn;
    uint8_t  Temp_Error;
    uint16_t  Temp_Alarm;
    uint8_t  StartHVD_Disconnect;
    uint8_t	DC_HVD_Disconnect;
    uint8_t  DC_HVD_Warn;
    uint8_t  DC_LVD_Restore;
    uint8_t  DC_HVD_Restore;
    uint8_t  Standby_Flag;
    uint16_t Standby_Again_Check;
    uint16_t SysTick_count;
    uint8_t Overload_AgainCheck;
    uint16_t Overload_AgainCheck_times;
    uint8_t Blue;
    uint16_t Runtime_today;               //·ÖÖÓ¼ÆÊ±£¬Í¨µç2Ğ¡Ê±Éú²úÒ»ÌõÔËĞĞ¼ÇÂ¼
//    uint8_t AC_Undervoltage;
//    uint8_t AC_Overvoltage;
};

struct AC_Parameter
{
	uint32_t sin_table[200];
    uint16_t Sin_positive; 
    uint16_t sine_add;
    uint16_t Qidong;
    uint16_t Vol_init;
    uint8_t frequnce_select;
    float fCal_ws_count;
    float fValue;
};

struct TimeStruct 
{
    uint16_t Starting_up_Open_Inverter_Delay; //¿ª»ú´ò¿ªÄæ±äÆ÷Êä³öÑÓÊ±Ê±¼ä
};

struct Card
{	  
    uint8_t SN[8];
    uint8_t type;
    uint8_t Card_status;
    uint8_t issue_Date[5];
    uint8_t First_MY[32];
    uint32_t balance;
    uint8_t bal[4];	 
    uint8_t Charge_DATA[6];
    uint8_t Charge_ID[2];
    uint8_t Charge_value[4];
    uint8_t User_password[16];	
    char Machine_ID[12];
    uint8_t Md5[16];
    uint8_t Checkout_Times;
    uint8_t Error_Repeat_Times;
    uint8_t Shop_Mima[50][16];
    uint8_t Check_Mima_A[16][6];
    uint8_t Check_Mima_B[16][6];
    uint8_t temp[16];
    uint8_t temp6[6];
    uint8_t m_CN11[16];
    uint8_t m_CB20[16];
    uint8_t SelectedSnr[4];
    uint8_t Run_step;
    uint8_t Index;
};

struct Machine
{
	uint32_t balance;	
	uint8_t  Bal_index;
	uint8_t  bal[4];
	uint32_t Shoping_Total;
	uint32_t Shoping_Total_Nclear;
	uint32_t Shoping_Times;
	uint16_t Charge_Times;
	uint32_t Charge_Total;
	uint32_t Charge_Total_Nclear;
	char Machine_ID[6];	 
  uint8_t  Card_SN[8];	
};	
struct Record
{
	uint32_t NO;    
	uint32_t Hourmeter;
	uint16_t Logger_flags;
  uint16_t Alarm;
  uint16_t Fault;
  uint16_t Vb_max; 
	uint16_t Vb_min;
	int8_t  Tb_min;
	int8_t  Tb_max;
	uint32_t T_loadon;    //Time load on - daily ¸ºÔØ¿ªÆôÊ±¼ä-Ã¿Ìì	ÃëÎªµ¥Î»	
	uint16_t KWh_l;       //¸ºÔØ·ÅµçKWh	KWh	
	uint32_t Hourmeter_clr;
	uint32_t Hourmeter_total_noclr;	
	uint32_t Ekwh_load_clr;
	uint32_t Ekwh_load_total_noclr;
//	uint32_t Bal_Pre;            //Ç°ÈÕÓà¶î
//	uint32_t Bal_Now;            //µ±ÈÕÓà¶î
//	uint32_t Decrease_today;     //µ±ÈÕÏû·Ñ
	uint8_t Date[6];
	uint8_t CRC_Cal[2];
};

struct Machine_data
{
	unsigned char  COMPANY_TB[20];	     //Owrely   Éú²úÉÌÃû³Æ           ASCII  
	unsigned char  Machine_Model[20];	   //²úÆ·ĞÍºÅ£ºPL-450W-12          ASCII	
	unsigned char  Hardware_Version[20]; //Ó²¼ş°æ±¾ºÅ£ºPL-45012E-V1.12   ASCII
	unsigned char  Software_Version[20]; //Èí¼ş°æ±¾ºÅ£ºPL-12E-V1.00      ASCII  
	unsigned char  Machine_Serial_NO[20];//Éè±¸ĞòÁĞºÅ£ºBCD
	unsigned char  Costom_TB[20];        //Owrely   ¿Í»§Ãû³Æ             ASCII
	unsigned char  Date[6];              //ÆôÓÃÈÕÆÚ 	
	unsigned char  Modbus_ID;            //Modbus TriStar server ID, TriStar Modbus Í¨Ñ¶µØÖ·	" 	- 	1-247 
	unsigned char  MiMa[4];              //¸ÄĞ´ÃÜÂë  BCD	                                             
	unsigned char  CRC_Cal[2];
	unsigned char  length;
};

struct Record_Run_Status        //Éè±¸ÔËĞĞ×´Ì¬µÄÊı¾İ¸ñÊ½£¬´óĞ¡16¸ö×Ö½Ú¡£
{
    uint16_t NO;                    //±àºÅ
    // uint8_t Sector_on;           //¼ÇÂ¼ÔÚFLASHÖĞµÄÉÈÇøºÅ
    uint8_t Mode;                   //Éè±¸µÄÔËĞĞÄ£Ê½logging.
	/*Õâ¸öÃ¶¾Ù¶¨ÒåÔÚrecord_data_process.hÖĞ¡£Õâ¸ö±äÁ¿ÌåÏÖÉè±¸ÔËĞĞÄ£Ê½µÄ¸Ä±ä¡£
	BIT0  0:·Ç±¾µØ¿ª¹Ø¿ØÖÆ      1:±¾µØ¿ª¹Ø¿ØÖÆ
  BIT1  0:·ÇÔ¶³Ì¿ª¹Ø¿ØÖÆ      1£ºÔ¶³Ì¿ª¹Ø¿ØÖÆ
	BIT2  0:·ÇÔ¶³ÌÍ¨ĞÅ¿ØÖÆ      1£ºÔ¶³ÌÍ¨ĞÅ¿ØÖÆ
	BIT3  0:µÍ¹¦ºÄ×Ô¶¯¹Ø»ú¹¦ÄÜ¹Ø±Õ   1£ºµÍ¹¦ºÄ×Ô¶¯¹Ø»ú¹¦ÄÜ¿ªÆô
	BIT4  0:¶¨Ê±ÆôÍ£¹¦ÄÜ¹Ø±Õ    1£º¶¨Ê±ÆôÍ£¹¦ÄÜ¿ªÆô
	BIT5  0:Éè±¸ÎŞ±¨¾¯          1£ºÉè±¸ÓĞ±¨¾¯
	BIT6  0:Éè±¸ÎŞ¹ÊÕÏ          1£ºÉè±¸ÓĞ¹ÊÕÏ
	*/
	  uint8_t event;
			/*Õâ¸öÃ¶¾Ù¶¨ÒåÔÚrecord_data_process.hÖĞ¡£ Õâ¸ö±äÁ¿ÖĞ²»ÌåÏÖÉè±¸ÔËĞĞÄ£Ê½µÄ¸Ä±ä¡£
	//Õâ¸öeven¸üºÏÊÊ¡£
	typedef enum 
{
	   Startup=0x01,     //ÉÏµçÆô¶¯     //µ¥´ÎÊÂ¼ş
	     time_align,     //Ê±¼äĞ£×¼     //µ¥´ÎÊÂ¼ş
	        Turn_on,     //¿ª»ú²Ù×÷     //³ÖĞøÊÂ¼ş£¬µ¥´Î»¯
	       Turn_off,     //¹Ø»ú²Ù×÷	   //³ÖĞøÊÂ¼ş£¬µ¥´Î»¯
	    Error_occur,     //¹ÊÕÏ·¢Éú     //³ÖĞøÊÂ¼ş£¬µ¥´Î»¯
	    Error_reset,     //¹ÊÕÏ¸´Î»     //³ÖĞøÊÂ¼ş£¬µ¥´Î»¯
	    Alarm_occur,     //±¨¾¯·¢Éú     //³ÖĞøÊÂ¼ş£¬µ¥´Î»¯
	    Alarm_reset,     //±¨¾¯¸´Î»     //³ÖĞøÊÂ¼ş£¬µ¥´Î»¯
  
}event_flag;
	*/
	  
    uint8_t Date[6];                //ÈÕÆÚÊı¾İÖĞµÄÊ±·ÖÃë
    uint16_t Error_No;              //¹ÊÕÏ´úÂë
    uint16_t Alarm_No;              //±¨¾¯´úÂë
		unsigned char  CRC_Cal[2];
};

//ÊÂ¼ş¼ÇÂ¼µÄ´¥·¢»úÖÆ£º1£ºÔËĞĞ×´Ì¬×´Ì¬modeµÄÏà¹ØÁ¿·¢Éú±ä»¯¾Í¼ÇÂ¼Ò»´Î£¬2£¬ÔËĞĞÊÂ¼ş±äÁ¿EVENTµÄÏà¹ØÊÂ¼ş·¢ÉúÒ»´Î¾Í¼ÇÂ¼Ò»´Î¡£
//ÊÂ¼ş¼ÇÂ¼µÄ·¢Æğ³ÌĞò£¬ÔÚRTCÃëÖÓÖĞ¶ÏÖÓÖ´ĞĞ£¬ÊÂ¼ş¼ÇÂ¼³ÌĞòÃ¿ÃëÖÓµ÷ÓÃÒ»´Î¡£

struct Record_Index   //Õâ¸ö½á¹¹Ìå³¤¶È4×Ö½Ú¡£
{
    uint16_t NO; 
    uint16_t Record_length;
};

//typedef struct {
//	bool bBAT_Led_Flashing_Interval_20sec; //µç³ØµÆÉÁË¸±êÖ¾
//	unsigned char ucSave_Load_EKwh_Hour_meter_Data;
//}Flag_struct;

//typedef struct {
//	unsigned char  BAT_Led_Start_Flashing;    //µç³ØµÆÉÁË¸Ê±¼ä
//	unsigned char  BAT_Led_Interval_20sec_Flash; //µç³ØµÆÉÁË¸¼ä¸ôÊ±¼ä
//}Tm_struct;

typedef struct  
{
	unsigned char ucStartFlag;
	unsigned char ucSocket0Mode;
	unsigned char ucSocket1Mode;
	unsigned char ucSocket2Mode;		
		
	//åˆè®¡37å­—èŠ‚  	strat		8+12+8+2+8=38
	
	unsigned char ucMAC[6];						//macåœ°å€
	unsigned char ucDhcpMode[2];			//dhcpæ¨¡å¼
	unsigned char ucSelfIP[4];				//é™æ€ip
	unsigned char ucSubMASK[4];				//é™æ€å­ç½‘æ©ç 
	unsigned char ucGateWay[4];				//é™æ€ç½‘å…³
	
	
	unsigned char ucDestIP[4] ;				//tcp client  ç›®æ ‡ip
	unsigned char ucDestPort[2];			//tcp client	ç›®æ ‡ç«¯å£
	unsigned char ucSourcePort[2];		//tcp client	æœ¬åœ°ç«¯å£   =0è¡¨ç¤ºä»»æ„
	
	unsigned char ucMonitorPort[2];		//tcp server  ç›‘å¬ç«¯å£
	
	unsigned char ucUdpDestIP[4] ;		//udp  target  ip
	unsigned char ucUdpDestPort[2];		//udp  target	port
	unsigned char ucUdpSourcePort[2];	//udp  source port(udp monitor port)
	//åˆè®¡36å­—èŠ‚  	end		6+12+8+2+8=38
		
	//ä»¥ä¸‹ä¸ºuarté…ç½®ä¿¡æ¯	
	unsigned char	g_ModbusID;				//ä½ç½®é¡ºåºä¸èƒ½æ›´æ¢  è¿ç»­ä¸‰ä¸ª
	unsigned char	chekbit[5];				//3
	int						baud[5];					//10	
	unsigned char ucEndFlag;
}t_SaveNetIPCfg ;

typedef struct  
{
	//ä»¥ä¸‹ä¸ºuarté…ç½®ä¿¡æ¯	
	unsigned char	g_ModbusID;				//ä½ç½®é¡ºåºä¸èƒ½æ›´æ¢  è¿ç»­ä¸‰ä¸ª
	unsigned char	chekbit[7];				//3
	int						baud[5];					//10	
}t_BaudCfg ;


typedef struct  
{
	unsigned char ucStartFlag;
	signed int AdC[8][3] ;				//ä»£è¡¨8ä¸ªé€šé“,3ä¸ªæ•°æ®ï¼Œåˆ†åˆ«ä¸º-6V    0V   6V	
	unsigned char ucEndFlag;
}t_AdCali ;


typedef struct  
{	
	unsigned char ucCmdName[ATCMDCOUNT][20];		//at command name
	unsigned char ucTest[ATCMDCOUNT];						//support test commad 
	unsigned char ucQuery[ATCMDCOUNT];					//support query commad 
	unsigned char ucSet[ATCMDCOUNT];						//support set commad 
	unsigned char ucExecute[ATCMDCOUNT];				//support execute commad 	
	unsigned char ucActiveCount;								//å®é™…å‘½ä»¤ä¸ªæ•°
}t_WifiBleAtCmd;
