//#include "stm32f10x.h" 
#include "GlobalVar.h"
#include "Define.h"

//u8     Overload_Restart_Check_Flag;  //过载重启检测标志位
//u16    DC_LVD_Disconnect_Vol;        //低压断开电压
//u16    DC_LVD_Warn_Vol;              //低压报警电压
//u16    DC_LVD_Restore_Vol;           //低压恢复电压

//u8     Comm_ON_OFF_Inverter;       //通讯开关逆变器命令字，=1：开；=2：关
//u8     Comm_ON_Inverter_Flag;      //通讯开逆变器状态标志
//u8     Comm_OFF_Inverter_Flag;     //通讯关逆变器状态标志

//u8     LED1_Mode;                  //led1命令字  =0:灭；=1:绿亮；=2:黄亮；=3:红亮；
//u8     LED2_Mode;                  //led2命令字  =0:灭；=1:绿亮；=2:绿闪；=3:黄闪；=4:红亮
//u8     LED2_Time;                    

//u8     Check_Times;                //温度传感器是否正常判断次数
//u8     Temp_Sensor_Check[3][5];    //温度传感器采集数据存储寄存器   这个地方有错误，应该将5改成6否则，会出现内存误操作。

//u8     Judge_LVD_HVD_Temp_AC_OFF;   //逆变器的断开原因表述寄存器 1:StartLVD_Disconnect低压断开，2:DC_Min_Disconnect低压断开；3:StartHVD_Disconnect高压断开；4:DC_Max_Disconnect高压断开；5：高温断开
//u8     flash_data[60];   

//u8     com_enable;                  //串口通信控制设备使能变量，1，获得使能；0，没有获得使能。
//u8     com_enable_count;            //串口通信控制使能位采样滤波器。
//u8     remote_switch;               //远程开关机触点。
//u8     remote_switch_count;         //远程开关机触点采样滤波器。
//u8     timingfunc_enable;              //定时启停机模式使能变量。
//u8     timingfunc_enable_count;        //定时启停机模式使能变量采样滤波。
//u8     savingfunc_enable;              //节电启停机模式使能变量。
//u8     savingfunc_enable_count;        //节电其停机模式使能变量采样滤波。
//u8     boat_switch_1;                //船型开关变量，1，船型开关1档有效；0，船型开关1档无效。  手动开机
//u8     boat_switch_2;                //船型开关变量，1，船型开关2档有效；0，船型开关2档无效。  远程开关机（包括通信开关机和远程触点开关机）
//u8     boat_switch_3;                //船型开关变量，1，船型开关M档有效；0，船型开关M档无效。  关机
//u8     boat_switch_count1;           //船型开关变量采样滤波器。
//u8     boat_switch_count2;           //船型开关变量采样滤波器。
//u8     boat_switch_count3;           //船型开关变量采样滤波器。
//u8     overload_detection;          //过流检测，用于过流重合闸过程中，重合闸。
//u8     standby_off;                 //省电待机模式下的关机命令
////开关机状态机函数需要锁存的是7个变量分别是：1，故障标志，2，船型开关状态（本地/关机/远程），3，远程通信控制使能标志（远程通信/远程开关），4，远程通信开关机指令，5，远程开关指令开关机指令，6，定时启停开关机指令，7，省电开关机指令。
////分别变量如下：1，device_fault;2,boatswitch_position;3,con_communication_enable; 4, on_off_communication;5 on_off_remoteswitch;6,on_off_timing;7,on_off_saving;
//u8     device_fault;                //故障标志
//u8     boatswitch_position;         //船型开关状态（本地/关机/远程）
//u8     con_communication_enable;    //远程通信控制使能标志（远程通信/远程开关）
//u8     on_off_communication;        //远程通信开关机指令。
//u8     on_off_remoteswitch;         //远程开关指令开关机指令。
//u8     on_off_timing;               //定时启停开关机指令。
//u8     on_off_saving;               //省电开关机指令。
//u8     ac_output_enable;            //由外部条件给定的开关机命令
//u8     LVD_OFF_Flag;                  //检查到直流电压低标志变量，0，没有检测到；1，检测到。
//u16    LVD_OFF_Flag_count;            //检查到直流电压低标志变量滤波信号
//u8     HVD_OFF_Flag;                  //检查到直流电压高标志变量，0，没有检测到；1，检测到。
//u16    HVD_OFF_Flag_count;            //检查到直流电压高标志变量滤波信号
//u8     TEMP_OFF_Flag;                 //检测到高温标志变量，0，没有检测到；1，检测到。
//u16    TEMP_OFF_Flag_count;           //检测到高温标志变量滤波信号
//u8     LVD_FUNC_ENABLE;               //检测低压关机硬件阀值使能信号，0，没有检测到；1，检测到。
//u8     LVD_FUNC_ENABLE_count;         //检测低压关机硬件阀值使能滤波信号。
//u8     LVD_FUNC_VALUE;                //检测低压关机硬件阀值信号，0，10.5v；1，11.5v
//u8     LVD_FUNC_VALUE_count;          //检测低压关机硬件阀值滤波信号。
//u8      on_off_and_savinglogic;
////u8      test_bit1;
////u8      test_bit2;
//u32    QITONG_MAX;                   //测试用。
//u32    QITONG_MAX1;                   //测试用。
//u32    QITONG_MAX2;                    //测试用。
//u32    QITONG_MAX3;                    //测试用。
//u8     QITONG_MAX3_COUNT;              //测试用。
//u32    QITONG_MAX4;                    //测试用。
//float  QITONG_VALUE;                   //测试用。
//float  QITONG_VALUE1;                   //测试用。
//float  QITONG_VALUE2;                   //测试用。
//float  QITONG_VALUE3;                   //测试用。
//float  QITONG_VALUE4;                   //测试用。
//u8     sine_P_transition_sampling;     //用于采样节拍器
//u8     sine_P_transition_regulation;   //用于电压调节的节拍器
//u32    ac_voltage_accu;                //交流电压采样累加器
//u32    ac_current_accu;                //交流电流采样累加器
//u8     ac_sampling_count;              //交流采样窗口计数器



////直流电压滤波数组，DC_VOL_FILTER[20]
//u16   DC_VOL_FILTER[60];
//u8    DC_VOL_FILTER_count; 
//u32   DC_VOL_FILTER_sum;

////直流电压前馈滤波数组，DC_VOL_FFB_FILTER[20]
//u16   DC_VOL_FFB_FILTER[300];
//u16   DC_VOL_FFB_FILTER_count; 
//u16   DC_VOL_FFB_FILTER_samplecount; 
//u32   DC_VOL_FFB_FILTER_sum;
//u16   DC_VOL_FFB_VOL;
//float   DC_VOL_FFB_CONTROLVALUE;

//uc16   sine[180]=    //50赫兹输出频率对应得正弦波数表,50HZ时，峰值为4000.
//{
//  0,70,140,209,279,349,418,487,557,626,              
//	695,763,832,900,968,1035,1103,1169,1236,1302,      
//	1368,1433,1498,1563,1627,1690,1753,1816,1878,1939, 
//	2000,2060,2120,2179,2237,2294,2351,2407,2463,2517,
//	2571,2624,2677,2728,2779,2828,2877,2925,2973,3019,
//	3064,3109,3152,3195,3236,3277,3316,3355,3392,3429,
//	3464,3498,3532,3564,3595,3625,3654,3682,3709,3734,
//	3759,3782,3804,3825,3845,3864,3881,3897,3913,3927,
//	3939,3951,3961,3970,3978,3985,3990,3995,3998,3999,
//	4000,3999,3998,3995,3990,3985,3978,3970,3961,3951,
//	3939,3927,3913,3897,3881,3864,3845,3825,3804,3782,
//	3759,3734,3709,3682,3654,3625,3595,3564,3532,3498,
//	3464,3429,3392,3355,3316,3277,3236,3195,3152,3109,
//  3064,3019,2973,2925,2877,2828,2779,2728,2677,2624,
//	2571,2517,2463,2407,2351,2294,2237,2179,2120,2060,
//	2000,1939,1878,1816,1753,1690,1627,1563,1498,1433,
//	1368,1302,1236,1169,1103,1035,968,900,832,763,
//	695,626,557,487,418,349,279,209,140,70
//};

//uc16   sine60[200]=  
////六十赫兹输出频率对应的正弦波数表，60hz时最大峰值为4096，这个是不对的，因为60赫兹的pwm定时器最大值为3000.这样的数据进入比较器会导致出现波形平顶化加重三次谐波。
//{
////	0,64,129,193,257,321,385,449,513,577,             	 
////	640,704,767,830,893,956,1018,1080,1142,1204,      
////	1265,1326,1387,1447,1507,1567,1626,1685,1743,1801, 
////	1859,1916,1972,2028,2084,2139,2194,2248,2301,2354,
////	2405,2458,2509,2560,2610,2659,2708,2756,2803,2849,
////	2895,2940,2985,3028,3071,3113,3155,3195,3235,3274,
////	3313,3350,3387,3422,3457,3491,3525,3557,3588,3619,
////	3649,3677,3705,3732,3757,3783,3807,3831,3853,3874,
////	3896,3914,3933,3950,3967,3982,3997,4010,4023,4035,
////	4045,4055,4063,4071,4073,4083,4088,4091,4094,4095,
////	4096,4096,4094,4092,4088,4084,4078,4072,4064,4056,
////	4046,4036,4024,4012,3998,3984,3968,3952,3934,3916,
////	3897,3876,3855,3833,3810,3786,3761,3735,3708,3680,
////  3651,3622,3591,3560,3528,3495,3461,3426,3390,3354,
////	3316,3278,3239,3200,3159,3118,3076,3033,2989,2945,
////	2900,2854,2808,2760,2712,2664,2615,2565,2515,2463,
////	2412,2360,2307,2253,2199,2145,2090,2034,1978,1922,
////	1864,1807,1749,1691,1632,1573,1513,1453,1393,1332,
////	1271,1210,1148,1087,1024,962,899,837,774,710,
////	647,583,520,456,392,328,264,199,135,71
//	
//	0	,     47	,  94	,  141	,188	,235	,282	,329	,376	,423	,
//	469	,	  516	,  562	,608	,654	,700	,746	,792	,837	,882	,
//	927	,	  972	,  1016	,1060	,1104	,1148	,1191	,1235	,1277	,1320	,
//	1362	,	1404	,1445	,1486	,1527	,1567	,1607	,1647	,1686	,1725	,
//	1763	,	1801	,1839	,1876	,1912	,1948	,1984	,2019	,2054	,2088	,
//	2121	,	2154	,2187	,2219	,2250	,2281	,2312	,2341	,2370	,2399	,
//	2427	,	2454	,2481	,2507	,2533	,2558	,2582	,2606	,2629	,2651	,
//	2673	,	2694	,2714	,2734	,2753	,2772	,2789	,2806	,2823	,2838	,
//	2853	,	2867	,2881	,2894	,2906	,2917	,2928	,2938	,2947	,2955	,
//	2963	,	2970	,2976	,2982	,2987	,2991	,2994	,2997	,2999	,3000	,
//	3000	,	3000	,2999	,2997	,2994	,2991	,2987	,2982	,2976	,2970	,
//	2963	,	2955	,2947	,2938	,2928	,2917	,2906	,2894	,2881	,2867	,
//	2853	,	2838	,2823	,2806	,2789	,2772	,2753	,2734	,2714	,2694	,
//	2673	,	2651	,2629	,2606	,2582	,2558	,2533	,2507	,2481	,2454	,
//	2427	,	2399	,2370	,2341	,2312	,2281	,2250	,2219	,2187	,2154	,
//	2121	,	2088	,2054	,2019	,1984	,1948	,1912	,1876	,1839	,1801	,
//	1763	,	1725	,1686	,1647	,1607	,1567	,1527	,1486	,1445	,1404	,
//	1362	,	1320	,1277	,1235	,1191	,1148	,1104	,1060	,1016	,972	,
//	927	,	  882	,  837	,792	,746	,700	,654	,608	,562	,516	,
//	469	,   423	, 	376	,329	,282	,235	,188	,141	,94	,  47
//};

//              
//uc16   temperature_mid_pos[]=   //正温度区间温度表
//{//(temp<=temperature_mid_pos[i])&(temp>=temperature_mid_pos[i+1]) 
//	3138,3100,3061,3022,2981,2941,2899,2857,2814,2771,					  // 0 - 9
//	2728,2684,2639,2594,2549,2504,2458,2413,2367,2321,					  // 10~19
//	2275,2230,2184,2138,2093,2048,2003,1959,1914,1871,					  // 20-29
//	1827,1784,1742,1700,1659,1618,1578,1538,1499,1461,					  // 30-39
//	1423,1386,1350,1314,1279,1245,1211,1179,1147,1115,					  // 40-49
//	1085,1055,1025,997,969,942,915,889,864,840,					          // 50-59
//	816,793,770,748,727,706,686,666,647,629,					          // 60-69
//	611,593,576,560,544,528,513,498,484,470,					          // 70-79
//	457,444,431,419,407,396,385,374,364,354,						      // 80-89
//	344,334,325,316,307,299,291,283,275,268,							  // 10-99
//	261,254,247,240,234,228,222,216,210,205,							  // 100-109 		  
//	199,194,189,184,180,175,171,166,162,158,							  // 110-119
//	154,150,147,143,139,136												  // 120-125
//}; 

//uc16   temperature_mid_neg[]=  //负温度区间温度表
//{
//	
//	3978,3970,3962,3953,3944,3934,3924,3913,3902,3890 ,					  //-40~-31
//	3878,3864,3851,3836,3821,3805,3789,3771,3753,3734,			          //-30~-21
//	3715,3694,3673,3651,3628,3604,3579,3553,3527,3499,					  //-20~-11
//	3471,3442,3411,3380,3348,3315,3282,3247,3211,3175  					  //-10~-1
//};

//struct Machine       MachineData;                   //设备充值信息，大小30个字节
//struct Machine_data  Machine_Setting_data;          //设备出厂信息，大小134字节。

//struct adjust        adjust;    //四十个字节的一个数组
//struct status        status;    //
//struct setting       setting;   //117个字节的一个数组
//struct sw            SW_setup;  
//struct AC_Parameter  AC_Parameter; 


//struct Record              Daily_Record,Record_Read;      //日报数据数变量，大小50个字节。daily_Record内存中的日报数据变量。
//struct Record_Run_Status   Record_Running;                //运行记录数据变量，大小16个字节。
//struct Record_Index        Record_Daily_Index,Record_Running_Index;  //这个结构体长度4字节。


//struct Card          M1Card;                        //设备充值卡信息，大小28个字节


//struct Time_Count    Time_Count;            //各种软件定时器。
//struct TimeStruct    Time;                  //一个只有一个成员的结构体类型定义。包含一个U16的变量，这个变量的名称是Starting_up_Open_Inverter_Delay，启动逆变器延时计时变量。
//struct MODE_delay    Set_run_time;          //定时设备启停运行模式时间节点数组，大小25个字节。

//Flag_struct  	Flag;
//Tm_struct     Tm;

////struct rtc_time systemtime;



//float u_v;	//这次输出的值
//float u1_v;	//上次输出的值
//float u2_v;	//上次输出的值
//float Kpv;    // 0.001;
//float Kiv;    // 0.0001;
////float Kp_v;	//比例系数
////float Ki_v;	//积分系数
//float ek_v;	  //当次误差
//float ek1_v;	//上一次误差

//float u_i;	//这次输出的值
//float u1_i;	//上次输出的值
//float Kp_i;	//比例系数
//float Ki_i;	//积分系数
//float ek_i;	//当次误差
//float ek1_i;	//上一次误差
////  float 	 Kp_i;  //0.01;
////  float 	 Ki_i; // 0.001;
////  float 	 Kp_v;  //0.001;
////  float 	 Ki_v; // 0.0001;

//int  VALUE_CONTROL;
//u8 Time_preset[6]={19,10,12,10,18,50};

////速断保护全局变量  时间常数10ms
////const float time_factor = 0.001;  //采样间隔1ms
////const float tc_oc_first = 0.01;   //采样周期10ms
//float i2T_VALUE[10];	  //I2T 变量表
//u8    i2T_VALUE_index;	//I2T 变量表指针
//float i2T_SUM;          // 速断保护 I2T累加和
//float i2T_SUM_ref;      //计算速断保护的阈值
//u8    flag_tc_oc_first; //速断保护标志位 0：无速断保护，1：有速断保护

////i2T 保护全局变量  时间常数5s
////const float time_factor = 0.001;  //采样间隔1ms
////const float  tc_oc_second = 5;      //采样周期5s

//float        e_i2t;                 //i2t误差
//float        I2T_OC_second;         //i2t值
//float        I2T_OC_second_ref;     //计算I2T保护阈值
//u8           flag_I2T_OC_second;    //I2T保护标志位 0：无I2T保护，1：有I2T保护

////过流打嗝变量
//u8           hiccup_oc;             //打嗝动作标志位
//u8           hiccup_oc1;            //打嗝动作标志位		
//u32          hiccup_oc_count;       //打嗝动作计时器
//u8           hiccup_tick;           //打嗝节拍数
//u8           hiccup_type;           //打嗝种类标志位

unsigned char buf[256];
unsigned long gulSysClk1mS;

unsigned int   g_sysTick=0;
unsigned int   systickCount=0;

unsigned char g_rxbuf[MAXRecvBuf];
unsigned int 	g_wr=0,g_rd=0;
unsigned char g_rxbuf_tcp[MAXRecvBuf];
unsigned int 	g_wr_tcp=0,g_rd_tcp=0;
unsigned char com5_sended=0;

t_AdCali							g_Ads1247_Cali;
t_SaveNetIPCfg 				gs_SaveNetIPCfg;
t_WifiBleAtCmd				gs_WifiBleAtCmd;	

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

unsigned char	Do_Sta[8]={0};					//do鐘舵��
unsigned char Sw_Sta[8]={0};					//鎷ㄧ爜鐩樼姸鎬�
unsigned char Android_Sta=0;					//Android鐢垫簮鐘舵��
short	int			Adc_Data[4]={0};				//ad閲囬泦鍚庤浆鎹㈠悗鐨勬暟鎹�
int						Ads1247_Value[8]={0};		//ads1247 8涓�氶亾鐨刟d鏁版嵁 
unsigned char Irq_Ads1247_Ready=0;		//ads1247杞崲瀹屾垚涓柇
float	Ads_1247_Fvalue[8]={0};						//float 鐢靛帇鏁版嵁 v

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







