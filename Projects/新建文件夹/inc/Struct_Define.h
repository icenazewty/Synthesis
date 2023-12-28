#include "gd32e50x.h"
#include <stdbool.h>

#define D_USART_REC_BUFF_SIZE				4096
#define MAXRecvBuf 		512    					//changed by wjj 30->128
#define	ATCMDCOUNT		128								//wifi&ble 命令个数

//#define	WIFI_BLE_COM	2								//wifi&ble com口 COM3   smtdog260 v3.62
//#define	USB_DBG_COM		0								//printf   com口 COM1  	smtdog260 v3.62

#define	AUTO_RS485_COM		0								//rs485    com口 COM1   	meter v1
#define	WIFI_BLE_COM			1								//wifi&ble com口 COM2   	meter v1
#define	USB_DBG_COM				2								//printf   com口 COM3   	meter v1
#define	METER_COM					3								//meter    com口 COM1   	meter v1

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
	volatile unsigned short usDelay_Rev;		//连续接收2个字节之间最大时间间隔 单位ms  4800bps  500字节/秒  1字节最晚为2ms
	
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
	unsigned int uiRTC_Count;//rtc �����ֵ����2000-01-01 0:00:00��ʼ������2001��1��1��0:00:00��ֵΪ��366*24*3600=31622400
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
	unsigned short Year;				//��ݸ�2λ��Hex��ʽ����2000�꣬Year=0x7D0
	unsigned char  Month;				//�·ݣ�Hex��ʽ����12�£�Month=0x0C
	unsigned char  Day;					//�죬Hex��ʽ����20�ţ�Day=0x14
	unsigned char  Week;
	unsigned char	 Hour,Minute,Second;
}calendar_struct;


struct adjust
{
	int16_t  DC_Current_Offset;
	int16_t  AC_Current_Offset;
	int16_t  AC_Voltage_Offset;
	uint16_t DC_Voltage_Coefficient; //����ϵ��
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
	unsigned char  Date[6];                        //��д���� 	
	unsigned char  CRC_Cal[2];
  //uint16_t AC_Voltage_begin;  
  //uint8_t  Write_Flag;	
  //uint8_t  Write_time[3]; //��дʱ��
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
	int16_t  tempture[2];              //�¶���ʾֵ
	uint8_t  Flag_Main_Program_Normal_Run;
	uint32_t Main_Program_Non_Normal_Run_Count;
	uint8_t   Power_ON_First;
	uint8_t   AC_SW_Previous;
	uint8_t  count;
	uint8_t  Error_Record_No;
	uint8_t  Boat_SWitch;
	uint8_t  LVD_HVD_TEMP_OFF_Flag;	   //�͡���ѹ�Ͽ���־
	uint32_t LVD_HVD_TEMP_OFF_Time;    //�͡���ѹ�Ͽ�ʱ��
	uint8_t  LVD_HVD_TEMP_Shutdown;
	uint8_t  Heatsink_A_TEMP_State;    //ɢ����A�¶�״̬=0��������=1���¶ȸߣ�=2�¶ȴ���������
	uint8_t  Heatsink_B_TEMP_State;    //ɢ����B�¶�״̬=0��������=1���¶ȸߣ�=2�¶ȴ���������
	uint8_t  Ambient_TEMP_State;       //�����¶�״̬   =0��������=1���¶ȸߣ�=2�¶ȴ���������
	uint8_t	 dip_switch;               //���뿪��
	uint16_t Load_State;              //����״̬
//bit0: 	�ֶ�����
//bit1: 	�ֶ��ػ�
//bit2: 	Զ�̿��ؿ���
//bit3: 	Զ�̿��عػ�
//bit4: 	���Ϲػ�
//bit5: 	��ʱ�Զ�����
//bit6: 	��ʱ�Զ��ػ�
//bit7: 	�͹��Ĺػ�
//bit8: 	�͹��Ŀ��� 
//bit9: 	���ع��ع���
//bit10:	���ض�·����
//bit11:	HVD��ѹ����
//bit12:	LVD��ѹ����
//bit13:	���ظ��¹���
//bit14:	ͨѶ����ر�	
//bit15   ͨѶ�����
	//���������µ�״̬������λ���塣
	
	//status.Load_State:����״̬	
//bit0: 	��׼���ؿ�
//bit1: 	��׼���ع�
//bit2: 	��׼�͹��Ĵ���
//bit3: 	��׼�͹��ĸ�������
//bit4: 	�ֶ���
//bit5: 	��ʱ�Զ���
//bit6: 	��ʱ�Զ���
//bit7: 	�͹��Ĺػ�
//bit8: 	�͹��Ŀ��� 
//bit9: 	���ع���
//bit10:	���ض�·
//bit11:	HVD��ѹ�Ͽ�
//bit12:	LVD��ѹ�Ͽ�
//bit13:	���ظ��¶Ͽ�
//bit14:	ͨѶ����ر�	
//bit15   ͨѶ�����
	
	
	
	
	uint32_t  Comm_modify_flag;
	uint32_t  rtc_tm_count;

	uint32_t Overcurrent_Time;      //����ʱ��
	uint8_t  Overcurrent_Flag;      //������־
	uint8_t  Overcurrent_SN;        //>600WΪ1��>450WΪ2��>300WΪ3
	uint8_t  Overcurrent_Shutdown;
	uint8_t  AC_Start;
	uint8_t  run_status;            //���б�־
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
//	int8_t  tempture[3];   //�¶���ʾֵ
	uint16_t tempture_a[2]; //�¶�ADֵ
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
	uint16_t Rated_DC_voltage;  //��������ѹ	
	uint16_t Rated_Power;       //�����
	uint16_t DC_HVD_Disconnect; //��������ѹ�Ͽ�ֵ
	uint16_t DC_LVD_Disconnect; //��С�����ѹ�Ͽ�ֵ
	uint16_t DC_HVD_Warn;       //�����ѹ��������ֵ
	uint16_t DC_LVD_Warn;       //�����ѹ��������ֵ
	uint16_t Power_Alarm;       //������ʱ���
	uint16_t Power_Error;       //������ʶϿ�
	uint8_t  Power_DC_Min[4];   //4���Ͽ������趨ֵ 	
	uint16_t Alarm_Time;        //������ʱ�Ͽ�ʱ��
	uint16_t Error_Time;        //������ʱ�Ͽ�ʱ��
	uint16_t IDLE_Time;         //����״̬���ʱ����
	uint16_t AC_Rated_Voltage;  //AC��������ѹ 
	uint16_t DC_LVR;            //�͵�ѹ�ָ�
	uint16_t DC_HVR;            //�ߵ�ѹ�ָ�
	uint16_t Bal_Alarm;         //����ֵ           500-65000
	uint16_t Bal_OFF;           //���Ͽ�ֵ           100-2000
	uint16_t StartLVD_Disconnect;  //������ʼLVD��ѹ,С�����ֵ��ʱ2��Ͽ�
	uint16_t StartHVD_Disconnect;  //������ʼHVD��ѹ,�������ֵ��ʱ2��Ͽ�
	uint16_t LVD_Disconnect_Delay; //LVD�Ͽ���ʱʱ��,��λ��
	uint16_t HVD_Disconnect_Delay; //HVD�Ͽ���ʱʱ��,��λ��	
	uint32_t  Time_Start[4];
	uint32_t  Time_Stop[4];
	uint8_t  frequence;         //�Ƶ��	
	uint8_t  Temp_Alarm;        //�����¶�
	uint8_t  Temp_Error;        //�Ͽ��¶�
	uint8_t  Temp_Fan_Start;    //���������¶�
	uint8_t  Temp_Fan_Stop;     //����ֹͣ�¶�
//	uint16_t DC_Rated_Voltage;  //���DC�����ѹ
//	uint8_t  DC_BL;             //�ָ���ѹ��ֵ  
//	uint8_t  made_time[3];      //��������
//	uint8_t  use_time[3];       //Ͷ������	
//	uint16_t LVDWarn_Beep_Start;//LVD������ѹ����ʼ��
//	uint16_t LVDWarn_Beep_Stop; //LVD������λ��ֹͣ����ѹ
//	uint16_t HVDWarn_Beep_Start;//HVD������ѹ����ʼ��
//	uint16_t HVDWarn_Beep_Stop; //HVD������λ��ֹͣ����ѹ	
	char Blue_ID[20];
	char Machine_ID[16];
	unsigned char  Date[6];    //��дʱ��	
	unsigned char  CRC_Cal[2];
};

/*struct serial
{
	uint8_t GetData[USART1_GetDataNum];              //�����������  
    uint8_t GetDataCopy[USART1_GetDataNum];          //�������������Ч�����ݴ��� 
    uint8_t OutData[USART1_OutDataNum];              //���巢������  
    uint16_t MobusWord[USART1_MobusWordNum];         //MOBUS RTU�Ĵ���16bit�ֽ����� 
    uint8_t GetDataCount;                            //���ڽ������ݸ���  
    uint8_t GetDataCountCopy;                        //���ڽ������ݸ���ת���� 
    uint8_t OutDataCount;                            //���ڷ������ݸ���  
    uint8_t GetDataFlag;                             //���ڽ������ݼ�����Ч��ʶ��,0Ϊ��Ч������0Ϊ��Ч 
    uint16_t GetMobusAddr;                           //���ڽ��յ�ָ�������ַ  
    uint16_t GetMobusAddrCopy;                       //���ڽ��յ�ָ�������ַ  
    uint8_t GetMobusOrder;                           //���ڽ��յ�ָ���������  
    uint8_t GetMobusOrderCopy;                       //���ڽ��յ�ָ��������� 
    uint8_t GetMobusLen;                             //���ڽ��յ�ָ��������ݳ���  
    uint8_t GetMobusOrderFlag;                       //���ڽ��յ�ָ����������Ƿ���Ч��Ƿ�     
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
    uint16_t Runtime_today;               //���Ӽ�ʱ��ͨ��2Сʱ����һ�����м�¼
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
    uint16_t Starting_up_Open_Inverter_Delay; //����������������ʱʱ��
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
	uint32_t T_loadon;    //Time load on - daily ���ؿ���ʱ��-ÿ��	��Ϊ��λ	
	uint16_t KWh_l;       //���طŵ�KWh	KWh	
	uint32_t Hourmeter_clr;
	uint32_t Hourmeter_total_noclr;	
	uint32_t Ekwh_load_clr;
	uint32_t Ekwh_load_total_noclr;
//	uint32_t Bal_Pre;            //ǰ�����
//	uint32_t Bal_Now;            //�������
//	uint32_t Decrease_today;     //��������
	uint8_t Date[6];
	uint8_t CRC_Cal[2];
};

struct Machine_data
{
	unsigned char  COMPANY_TB[20];	     //Owrely   ����������           ASCII  
	unsigned char  Machine_Model[20];	   //��Ʒ�ͺţ�PL-450W-12          ASCII	
	unsigned char  Hardware_Version[20]; //Ӳ���汾�ţ�PL-45012E-V1.12   ASCII
	unsigned char  Software_Version[20]; //����汾�ţ�PL-12E-V1.00      ASCII  
	unsigned char  Machine_Serial_NO[20];//�豸���кţ�BCD
	unsigned char  Costom_TB[20];        //Owrely   �ͻ�����             ASCII
	unsigned char  Date[6];              //�������� 	
	unsigned char  Modbus_ID;            //Modbus TriStar server ID, TriStar Modbus ͨѶ��ַ	" 	- 	1-247 
	unsigned char  MiMa[4];              //��д����  BCD	                                             
	unsigned char  CRC_Cal[2];
	unsigned char  length;
};

struct Record_Run_Status        //�豸����״̬�����ݸ�ʽ����С16���ֽڡ�
{
    uint16_t NO;                    //���
    // uint8_t Sector_on;           //��¼��FLASH�е�������
    uint8_t Mode;                   //�豸������ģʽlogging.
	/*���ö�ٶ�����record_data_process.h�С�������������豸����ģʽ�ĸı䡣
	BIT0  0:�Ǳ��ؿ��ؿ���      1:���ؿ��ؿ���
  BIT1  0:��Զ�̿��ؿ���      1��Զ�̿��ؿ���
	BIT2  0:��Զ��ͨ�ſ���      1��Զ��ͨ�ſ���
	BIT3  0:�͹����Զ��ػ����ܹر�   1���͹����Զ��ػ����ܿ���
	BIT4  0:��ʱ��ͣ���ܹر�    1����ʱ��ͣ���ܿ���
	BIT5  0:�豸�ޱ���          1���豸�б���
	BIT6  0:�豸�޹���          1���豸�й���
	*/
	  uint8_t event;
			/*���ö�ٶ�����record_data_process.h�С� ��������в������豸����ģʽ�ĸı䡣
	//���even�����ʡ�
	typedef enum 
{
	   Startup=0x01,     //�ϵ�����     //�����¼�
	     time_align,     //ʱ��У׼     //�����¼�
	        Turn_on,     //��������     //�����¼������λ�
	       Turn_off,     //�ػ�����	   //�����¼������λ�
	    Error_occur,     //���Ϸ���     //�����¼������λ�
	    Error_reset,     //���ϸ�λ     //�����¼������λ�
	    Alarm_occur,     //��������     //�����¼������λ�
	    Alarm_reset,     //������λ     //�����¼������λ�
  
}event_flag;
	*/
	  
    uint8_t Date[6];                //���������е�ʱ����
    uint16_t Error_No;              //���ϴ���
    uint16_t Alarm_No;              //��������
		unsigned char  CRC_Cal[2];
};

//�¼���¼�Ĵ������ƣ�1������״̬״̬mode������������仯�ͼ�¼һ�Σ�2�������¼�����EVENT������¼�����һ�ξͼ�¼һ�Ρ�
//�¼���¼�ķ��������RTC�����ж���ִ�У��¼���¼����ÿ���ӵ���һ�Ρ�

struct Record_Index   //����ṹ�峤��4�ֽڡ�
{
    uint16_t NO; 
    uint16_t Record_length;
};

//typedef struct {
//	bool bBAT_Led_Flashing_Interval_20sec; //��ص���˸��־
//	unsigned char ucSave_Load_EKwh_Hour_meter_Data;
//}Flag_struct;

//typedef struct {
//	unsigned char  BAT_Led_Start_Flashing;    //��ص���˸ʱ��
//	unsigned char  BAT_Led_Interval_20sec_Flash; //��ص���˸���ʱ��
//}Tm_struct;

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
	unsigned char ucStartFlag;
	signed int AdC[8][3] ;				//代表8个通道,3个数据，分别为-6V    0V   6V	
	unsigned char ucEndFlag;
}t_AdCali ;


typedef struct  
{	
	unsigned char ucCmdName[ATCMDCOUNT][20];		//at command name
	unsigned char ucTest[ATCMDCOUNT];						//support test commad 
	unsigned char ucQuery[ATCMDCOUNT];					//support query commad 
	unsigned char ucSet[ATCMDCOUNT];						//support set commad 
	unsigned char ucExecute[ATCMDCOUNT];				//support execute commad 	
	unsigned char ucActiveCount;								//实际命令个数
}t_WifiBleAtCmd;
