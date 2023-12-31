//#include "stm32f10x.h"
#include "define.h"
#include "Struct_Define.h"

////extern u16 Again_Start_Up_Delay; //待机和定时启动后延时一点时间后，等输出稳定后再去判断功率是否还是待机，如果不延时，启动不起来

//#define ROUND_TO_UINT16(x)   ((uint16_t)(x)+0.5)>(x)? ((uint16_t)(x)):((uint16_t)(x)+1) ;
//extern u16 DC_LVD_Disconnect_Vol; //低压断开电压
//extern u16 DC_LVD_Warn_Vol;       //低压报警电压
//extern u16 DC_LVD_Restore_Vol;    //低压恢复电压

//extern u8  Overload_Restart_Check_Flag;

//extern u8  Comm_ON_OFF_Inverter; //通讯开关逆变器，=1：开；=2：关
//extern u8  Comm_ON_Inverter_Flag;//通讯开逆变器标志
//extern u8  Comm_OFF_Inverter_Flag;//通讯关逆变器标志

//extern u8  LED1_Mode; //=0:灭；=1:绿亮；=2:黄亮；=3:红亮；
//extern u8  LED2_Mode; //=0:灭；=1:绿亮；=2:绿闪；=3:黄闪；=4:红亮
//extern u8  LED2_Time;

//extern u8  Check_Times; //温度传感器是否正常判断次数
//extern u8 Temp_Sensor_Check[3][5];

//extern u8 Judge_LVD_HVD_Temp_AC_OFF;	     //1:低压断开，2：高压断开；3：高温断开
// 					   
//extern u8 flash_data[60];
//extern uc16 sine[180];
//extern uc16 sine60[200];

//extern uc16 temperature_mid_pos[];
//extern uc16 temperature_mid_neg[];

//extern struct adjust adjust;
//extern struct status status;
//extern struct setting setting;
//extern struct sw SW_setup; 
////extern struct rtc_time systemtime;
//extern struct MODE_delay Set_run_time;
//extern struct TimeStruct Time;
//extern struct Card M1Card; 
//extern struct Machine MachineData;

//extern struct Machine_data  Machine_Setting_data;
//extern struct Time_Count Time_Count; 
//extern struct AC_Parameter AC_Parameter; 

//extern struct Record              Daily_Record,Record_Read;
//extern struct Record_Run_Status   Record_Running;
//extern struct Record_Index        Record_Daily_Index,Record_Running_Index;  //这个结构体长度4字节。

////extern Flag_struct  	Flag;
////extern Tm_struct     Tm;

////extern struct rtc_time systemtime;  //从时钟采集的日期数据。


///*正弦波相位计数器和正负半波标志器     sign=0正半波，sign=1负半波*/

//extern u8     com_enable;                   //串口通信控制设备使能变量，1，获得使能；0，没有获得使能。
//extern u8     com_enable_count;             //串口通信控制使能位采样滤波器。
//extern u8     boat_switch_1;                //船型开关变量，1，船型开关1档有效；0，船型开关1档无效。
//extern u8     boat_switch_2;                //船型开关变量，1，船型开关2档有效；0，船型开关2档无效。
//extern u8     boat_switch_3;                //船型开关变量，1，船型开关M档有效；0，船型开关M档无效。
//extern u8     boat_switch_count1;           //船型开关变量采样滤波器。
//extern u8     boat_switch_count2;           //船型开关变量采样滤波器。
//extern u8     boat_switch_count3;           //船型开关变量采样滤波器。
//extern u8     overload_detection;          //过流检测，用于过流重合闸过程中，重合闸。


//extern u8     com_enable;                  //串口通信控制设备使能变量，1，获得使能；0，没有获得使能。
//extern u8     com_enable_count;            //串口通信控制使能位采样滤波器。
//extern u8     remote_switch;               //远程开关机触点。
//extern u8     remote_switch_count;         //远程开关机触点采样滤波器。
//extern u8     boat_switch_1;                //船型开关变量，1，船型开关1档有效；0，船型开关1档无效。  手动开机
//extern u8     boat_switch_2;                //船型开关变量，1，船型开关2档有效；0，船型开关2档无效。  远程开关机（包括通信开关机和远程触点开关机）
//extern u8     boat_switch_3;                //船型开关变量，1，船型开关M档有效；0，船型开关M档无效。  关机
//extern u8     boat_switch_count1;           //船型开关变量采样滤波器。
//extern u8     boat_switch_count2;           //船型开关变量采样滤波器。
//extern u8     boat_switch_count3;           //船型开关变量采样滤波器。
//extern u8     overload_detection;          //过流检测，用于过流重合闸过程中，重合闸。
//extern u8     standby_off;                 //省电待机模式下的关机命令
////开关机状态机函数需要锁存的是7个变量分别是：1，故障标志，2，船型开关状态（本地/关机/远程），3，远程通信控制使能标志（远程通信/远程开关），4，远程通信开关机指令，5，远程开关指令开关机指令，6，定时启停开关机指令，7，省电开关机指令。
////分别变量如下：1，device_fault;2,boatswitch_position;3,con_communication_enable; 4, on_off_communication;5 on_off_remoteswitch;6,on_off_timing;7,on_off_saving;
//extern u8     device_fault;                //故障标志
//extern u8     boatswitch_position;         //船型开关状态（本地/关机/远程）
//extern u8     con_communication_enable;    //远程通信控制使能标志（远程通信/远程开关）
//extern u8     on_off_communication;        //远程通信开关机指令。
//extern u8     on_off_remoteswitch;         //远程开关指令开关机指令。
//extern u8     on_off_timing;               //定时启停开关机指令。
//extern u8     on_off_saving;               //省电开关机指令。
//extern u8     ac_output_enable;            //由外部条件给定的开关机命令
//extern u8     LVD_OFF_Flag;                  //检查到直流电压低标志变量，0，没有检测到；1，检测到。
//extern u16    LVD_OFF_Flag_count;            //检查到直流电压低标志变量滤波信号
//extern u8     HVD_OFF_Flag;                  //检查到直流电压高标志变量，0，没有检测到；1，检测到。
//extern u16    HVD_OFF_Flag_count;            //检查到直流电压高标志变量滤波信号
//extern u8     TEMP_OFF_Flag;                 //检测到高温标志变量，0，没有检测到；1，检测到。
//extern u16    TEMP_OFF_Flag_count;           //检测到高温标志变量滤波信号


//extern u8     timingfunc_enable;              //定时启停机模式使能变量。
//extern u8     timingfunc_enable_count;        //定时启停机模式使能变量采样滤波。
//extern u8     savingfunc_enable;              //节电启停机模式使能变量。
//extern u8     savingfunc_enable_count;        //节电其停机模式使能变量采样滤波。
//extern u8     LVD_FUNC_ENABLE;
//extern u8     LVD_FUNC_ENABLE_count;
//extern u8     LVD_FUNC_VALUE;                //检测低压关机硬件阀值信号，0，10.5v；1，11.5v
//extern u8     LVD_FUNC_VALUE_count;          //检测低压关机硬件阀值滤波信号。
//extern u8     on_off_and_savinglogic;
//extern u32    QITONG_MAX;
//extern u32    QITONG_MAX1;
//extern u32    QITONG_MAX2;                    //测试用。
//extern u32    QITONG_MAX3;                    //测试用。
//extern u8     QITONG_MAX3_COUNT;              //测试用。
//extern u32    QITONG_MAX4;                    //测试用。
//extern float  QITONG_VALUE;                   //测试用。
//extern float  QITONG_VALUE1;                   //测试用。
//extern float  QITONG_VALUE2;                   //测试用。
//extern float  QITONG_VALUE3;                   //测试用。
//extern float  QITONG_VALUE4;                   //测试用。
////extern u8      test_bit2;
//extern u8     sine_P_transition_sampling;     //用于采样节拍器
//extern u8     sine_P_transition_regulation;   //用于电压调节的节拍器

////直流电压滤波数组，DC_VOL_FILTER[20]
//extern u16   DC_VOL_FILTER[60];
//extern u8     DC_VOL_FILTER_count; 
//extern u32   DC_VOL_FILTER_sum;

////直流电压前馈滤波数组，DC_VOL_FFB_FILTER[20]
//extern u16   DC_VOL_FFB_FILTER[300];
//extern u16    DC_VOL_FFB_FILTER_count; 
//extern u16   DC_VOL_FFB_FILTER_samplecount; 
//extern u32   DC_VOL_FFB_FILTER_sum;
//extern u16   DC_VOL_FFB_VOL;
//extern float   DC_VOL_FFB_CONTROLVALUE;

//	typedef enum                                //事件系统里的事件类型枚举
//{
//	   Startup=0x01,     //上电启动
//	     time_align,     //时间校准
//	        Turn_on,     //开机操作
//	       Turn_off,     //关机操作	
//	    Error_occur,     //故障发生
//	    Error_reset,     //故障复位
//	    Alarm_occur,     //报警发生
//	    Alarm_reset,     //报警复位
//  
//}event_flag;


//extern float u_v;	  //控制器总输出的值
//extern float u1_v;	//上次PI输出的值
//extern float u2_v;	//本次PI输出的值
//#define  Kp_v  0.001;	//比例系数
//#define  Ki_v  0.0001;	//积分系数
//extern float Kpv;         // 0.001;
//extern float Kiv;      // 0.0001;
//extern float ek_v;	//当次误差
//extern float ek1_v;	//上一次误差

//extern float u_i;	//这次输出的值
//extern float u1_i;	//上次输出的值
//extern float Kp_i;	//比例系数
//extern float Ki_i;	//积分系数
////#define  Kp_i  0.01;	//比例系数
////#define  Ki_i  0.001;	//积分系数
////extern  float 	 Kp_i; // 0.01;
////extern  float 	 Ki_i; // 0.001;
//extern float ek_i;	//当次误差
//extern float ek1_i;	//上一次误差
//extern int  VALUE_CONTROL;

//extern u8 Time_preset[6];

//extern u32    ac_voltage_accu;                //交流电压采样累加器
//extern u32    ac_current_accu;                //交流电流采样累加器
//extern u8     ac_sampling_count;              //交流采样窗口计数器

////速断保护全局变量  时间常数10ms
////extern const float time_factor = 0.001;  //采样间隔1ms
////extern const float tc_oc_first = 0.01;   //采样周期10ms
//extern float i2T_VALUE[10];	  //I2T 变量表
//extern u8    i2T_VALUE_index;	//I2T 变量表指针
//extern float i2T_SUM;          // 速断保护 I2T累加和
//extern float i2T_SUM_ref;      //计算速断保护的阈值
//extern u8    flag_tc_oc_first; //速断保护标志位 0：无速断保护，1：有速断保护

////i2T 保护全局变量  时间常数5s
////const float time_factor = 0.001;  //采样间隔1ms
////extern const float  tc_oc_second = 5;      //采样周期5s
//extern float        e_i2t;                 //i2t误差
//extern float        I2T_OC_second;         //i2t值
//extern float        I2T_OC_second_ref;     //计算I2T保护阈值
//extern u8           flag_I2T_OC_second;    //I2T保护标志位 0：无I2T保护，1：有I2T保护

////过流打嗝变量
//extern u8           hiccup_oc;             //打嗝动作标志位
//extern u8           hiccup_oc1;            //打嗝动作标志位		
//extern u32          hiccup_oc_count;       //打嗝动作计时器
//extern u8           hiccup_tick;           //打嗝节拍数
//extern u8           hiccup_type;           //打嗝种类标志位

extern unsigned int   						g_sysTick;				//鍗曚綅s
extern unsigned int   						systickCount;			//鍗曚綅ms

extern	unsigned char 		g_rxbuf[MAXRecvBuf];
extern	unsigned int 			g_wr,g_rd;
extern	unsigned char 		g_rxbuf_tcp[MAXRecvBuf];
extern	unsigned int 			g_wr_tcp,g_rd_tcp;
extern  t_AdCali					g_Ads1247_Cali;							

extern unsigned char 							buf[256];
extern unsigned long 							gulSysClk1mS;

extern struct_com									Com;
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
extern	unsigned char							Do_Sta[8];					//do鐘舵��
extern	unsigned char 						Sw_Sta[8];					//鎷ㄧ爜鐩樼姸鎬�
extern	unsigned char 						Android_Sta;				//Android鐢垫簮鐘舵��
extern 	int												Ads1247_Value[8];		//ads1247 8涓�氶亾鐨刟d鏁版嵁 
extern  float											Ads_1247_Fvalue[8];	//float 鐢靛帇鏁版嵁 v
extern  unsigned char 						Irq_Ads1247_Ready;	//ads1247杞崲瀹屾垚涓柇
extern  short	int									Adc_Data[4];				//ad閲囬泦鍚庤浆鎹㈠悗鐨勬暟鎹�
extern 	t_SaveNetIPCfg 						gs_SaveNetIPCfg;		

extern 	unsigned char com5_sended;
extern  t_WifiBleAtCmd						gs_WifiBleAtCmd;	
