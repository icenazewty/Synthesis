//#include "stm32f10x.h"
#include "define.h"
#include "Struct_Define.h"

////extern u16 Again_Start_Up_Delay; //�����Ͷ�ʱ��������ʱһ��ʱ��󣬵�����ȶ�����ȥ�жϹ����Ƿ��Ǵ������������ʱ������������

//#define ROUND_TO_UINT16(x)   ((uint16_t)(x)+0.5)>(x)? ((uint16_t)(x)):((uint16_t)(x)+1) ;
//extern u16 DC_LVD_Disconnect_Vol; //��ѹ�Ͽ���ѹ
//extern u16 DC_LVD_Warn_Vol;       //��ѹ������ѹ
//extern u16 DC_LVD_Restore_Vol;    //��ѹ�ָ���ѹ

//extern u8  Overload_Restart_Check_Flag;

//extern u8  Comm_ON_OFF_Inverter; //ͨѶ�����������=1������=2����
//extern u8  Comm_ON_Inverter_Flag;//ͨѶ���������־
//extern u8  Comm_OFF_Inverter_Flag;//ͨѶ���������־

//extern u8  LED1_Mode; //=0:��=1:������=2:������=3:������
//extern u8  LED2_Mode; //=0:��=1:������=2:������=3:������=4:����
//extern u8  LED2_Time;

//extern u8  Check_Times; //�¶ȴ������Ƿ������жϴ���
//extern u8 Temp_Sensor_Check[3][5];

//extern u8 Judge_LVD_HVD_Temp_AC_OFF;	     //1:��ѹ�Ͽ���2����ѹ�Ͽ���3�����¶Ͽ�
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
//extern struct Record_Index        Record_Daily_Index,Record_Running_Index;  //����ṹ�峤��4�ֽڡ�

////extern Flag_struct  	Flag;
////extern Tm_struct     Tm;

////extern struct rtc_time systemtime;  //��ʱ�Ӳɼ����������ݡ�


///*���Ҳ���λ�������������벨��־��     sign=0���벨��sign=1���벨*/

//extern u8     com_enable;                   //����ͨ�ſ����豸ʹ�ܱ�����1�����ʹ�ܣ�0��û�л��ʹ�ܡ�
//extern u8     com_enable_count;             //����ͨ�ſ���ʹ��λ�����˲�����
//extern u8     boat_switch_1;                //���Ϳ��ر�����1�����Ϳ���1����Ч��0�����Ϳ���1����Ч��
//extern u8     boat_switch_2;                //���Ϳ��ر�����1�����Ϳ���2����Ч��0�����Ϳ���2����Ч��
//extern u8     boat_switch_3;                //���Ϳ��ر�����1�����Ϳ���M����Ч��0�����Ϳ���M����Ч��
//extern u8     boat_switch_count1;           //���Ϳ��ر��������˲�����
//extern u8     boat_switch_count2;           //���Ϳ��ر��������˲�����
//extern u8     boat_switch_count3;           //���Ϳ��ر��������˲�����
//extern u8     overload_detection;          //������⣬���ڹ����غ�բ�����У��غ�բ��


//extern u8     com_enable;                  //����ͨ�ſ����豸ʹ�ܱ�����1�����ʹ�ܣ�0��û�л��ʹ�ܡ�
//extern u8     com_enable_count;            //����ͨ�ſ���ʹ��λ�����˲�����
//extern u8     remote_switch;               //Զ�̿��ػ����㡣
//extern u8     remote_switch_count;         //Զ�̿��ػ���������˲�����
//extern u8     boat_switch_1;                //���Ϳ��ر�����1�����Ϳ���1����Ч��0�����Ϳ���1����Ч��  �ֶ�����
//extern u8     boat_switch_2;                //���Ϳ��ر�����1�����Ϳ���2����Ч��0�����Ϳ���2����Ч��  Զ�̿��ػ�������ͨ�ſ��ػ���Զ�̴��㿪�ػ���
//extern u8     boat_switch_3;                //���Ϳ��ر�����1�����Ϳ���M����Ч��0�����Ϳ���M����Ч��  �ػ�
//extern u8     boat_switch_count1;           //���Ϳ��ر��������˲�����
//extern u8     boat_switch_count2;           //���Ϳ��ر��������˲�����
//extern u8     boat_switch_count3;           //���Ϳ��ر��������˲�����
//extern u8     overload_detection;          //������⣬���ڹ����غ�բ�����У��غ�բ��
//extern u8     standby_off;                 //ʡ�����ģʽ�µĹػ�����
////���ػ�״̬��������Ҫ�������7�������ֱ��ǣ�1�����ϱ�־��2�����Ϳ���״̬������/�ػ�/Զ�̣���3��Զ��ͨ�ſ���ʹ�ܱ�־��Զ��ͨ��/Զ�̿��أ���4��Զ��ͨ�ſ��ػ�ָ�5��Զ�̿���ָ��ػ�ָ�6����ʱ��ͣ���ػ�ָ�7��ʡ�翪�ػ�ָ�
////�ֱ�������£�1��device_fault;2,boatswitch_position;3,con_communication_enable; 4, on_off_communication;5 on_off_remoteswitch;6,on_off_timing;7,on_off_saving;
//extern u8     device_fault;                //���ϱ�־
//extern u8     boatswitch_position;         //���Ϳ���״̬������/�ػ�/Զ�̣�
//extern u8     con_communication_enable;    //Զ��ͨ�ſ���ʹ�ܱ�־��Զ��ͨ��/Զ�̿��أ�
//extern u8     on_off_communication;        //Զ��ͨ�ſ��ػ�ָ�
//extern u8     on_off_remoteswitch;         //Զ�̿���ָ��ػ�ָ�
//extern u8     on_off_timing;               //��ʱ��ͣ���ػ�ָ�
//extern u8     on_off_saving;               //ʡ�翪�ػ�ָ�
//extern u8     ac_output_enable;            //���ⲿ���������Ŀ��ػ�����
//extern u8     LVD_OFF_Flag;                  //��鵽ֱ����ѹ�ͱ�־������0��û�м�⵽��1����⵽��
//extern u16    LVD_OFF_Flag_count;            //��鵽ֱ����ѹ�ͱ�־�����˲��ź�
//extern u8     HVD_OFF_Flag;                  //��鵽ֱ����ѹ�߱�־������0��û�м�⵽��1����⵽��
//extern u16    HVD_OFF_Flag_count;            //��鵽ֱ����ѹ�߱�־�����˲��ź�
//extern u8     TEMP_OFF_Flag;                 //��⵽���±�־������0��û�м�⵽��1����⵽��
//extern u16    TEMP_OFF_Flag_count;           //��⵽���±�־�����˲��ź�


//extern u8     timingfunc_enable;              //��ʱ��ͣ��ģʽʹ�ܱ�����
//extern u8     timingfunc_enable_count;        //��ʱ��ͣ��ģʽʹ�ܱ��������˲���
//extern u8     savingfunc_enable;              //�ڵ���ͣ��ģʽʹ�ܱ�����
//extern u8     savingfunc_enable_count;        //�ڵ���ͣ��ģʽʹ�ܱ��������˲���
//extern u8     LVD_FUNC_ENABLE;
//extern u8     LVD_FUNC_ENABLE_count;
//extern u8     LVD_FUNC_VALUE;                //����ѹ�ػ�Ӳ����ֵ�źţ�0��10.5v��1��11.5v
//extern u8     LVD_FUNC_VALUE_count;          //����ѹ�ػ�Ӳ����ֵ�˲��źš�
//extern u8     on_off_and_savinglogic;
//extern u32    QITONG_MAX;
//extern u32    QITONG_MAX1;
//extern u32    QITONG_MAX2;                    //�����á�
//extern u32    QITONG_MAX3;                    //�����á�
//extern u8     QITONG_MAX3_COUNT;              //�����á�
//extern u32    QITONG_MAX4;                    //�����á�
//extern float  QITONG_VALUE;                   //�����á�
//extern float  QITONG_VALUE1;                   //�����á�
//extern float  QITONG_VALUE2;                   //�����á�
//extern float  QITONG_VALUE3;                   //�����á�
//extern float  QITONG_VALUE4;                   //�����á�
////extern u8      test_bit2;
//extern u8     sine_P_transition_sampling;     //���ڲ���������
//extern u8     sine_P_transition_regulation;   //���ڵ�ѹ���ڵĽ�����

////ֱ����ѹ�˲����飬DC_VOL_FILTER[20]
//extern u16   DC_VOL_FILTER[60];
//extern u8     DC_VOL_FILTER_count; 
//extern u32   DC_VOL_FILTER_sum;

////ֱ����ѹǰ���˲����飬DC_VOL_FFB_FILTER[20]
//extern u16   DC_VOL_FFB_FILTER[300];
//extern u16    DC_VOL_FFB_FILTER_count; 
//extern u16   DC_VOL_FFB_FILTER_samplecount; 
//extern u32   DC_VOL_FFB_FILTER_sum;
//extern u16   DC_VOL_FFB_VOL;
//extern float   DC_VOL_FFB_CONTROLVALUE;

//	typedef enum                                //�¼�ϵͳ����¼�����ö��
//{
//	   Startup=0x01,     //�ϵ�����
//	     time_align,     //ʱ��У׼
//	        Turn_on,     //��������
//	       Turn_off,     //�ػ�����	
//	    Error_occur,     //���Ϸ���
//	    Error_reset,     //���ϸ�λ
//	    Alarm_occur,     //��������
//	    Alarm_reset,     //������λ
//  
//}event_flag;


//extern float u_v;	  //�������������ֵ
//extern float u1_v;	//�ϴ�PI�����ֵ
//extern float u2_v;	//����PI�����ֵ
//#define  Kp_v  0.001;	//����ϵ��
//#define  Ki_v  0.0001;	//����ϵ��
//extern float Kpv;         // 0.001;
//extern float Kiv;      // 0.0001;
//extern float ek_v;	//�������
//extern float ek1_v;	//��һ�����

//extern float u_i;	//��������ֵ
//extern float u1_i;	//�ϴ������ֵ
//extern float Kp_i;	//����ϵ��
//extern float Ki_i;	//����ϵ��
////#define  Kp_i  0.01;	//����ϵ��
////#define  Ki_i  0.001;	//����ϵ��
////extern  float 	 Kp_i; // 0.01;
////extern  float 	 Ki_i; // 0.001;
//extern float ek_i;	//�������
//extern float ek1_i;	//��һ�����
//extern int  VALUE_CONTROL;

//extern u8 Time_preset[6];

//extern u32    ac_voltage_accu;                //������ѹ�����ۼ���
//extern u32    ac_current_accu;                //�������������ۼ���
//extern u8     ac_sampling_count;              //�����������ڼ�����

////�ٶϱ���ȫ�ֱ���  ʱ�䳣��10ms
////extern const float time_factor = 0.001;  //�������1ms
////extern const float tc_oc_first = 0.01;   //��������10ms
//extern float i2T_VALUE[10];	  //I2T ������
//extern u8    i2T_VALUE_index;	//I2T ������ָ��
//extern float i2T_SUM;          // �ٶϱ��� I2T�ۼӺ�
//extern float i2T_SUM_ref;      //�����ٶϱ�������ֵ
//extern u8    flag_tc_oc_first; //�ٶϱ�����־λ 0�����ٶϱ�����1�����ٶϱ���

////i2T ����ȫ�ֱ���  ʱ�䳣��5s
////const float time_factor = 0.001;  //�������1ms
////extern const float  tc_oc_second = 5;      //��������5s
//extern float        e_i2t;                 //i2t���
//extern float        I2T_OC_second;         //i2tֵ
//extern float        I2T_OC_second_ref;     //����I2T������ֵ
//extern u8           flag_I2T_OC_second;    //I2T������־λ 0����I2T������1����I2T����

////�������ñ���
//extern u8           hiccup_oc;             //���ö�����־λ
//extern u8           hiccup_oc1;            //���ö�����־λ		
//extern u32          hiccup_oc_count;       //���ö�����ʱ��
//extern u8           hiccup_tick;           //���ý�����
//extern u8           hiccup_type;           //���������־λ

extern unsigned int   						g_sysTick;				//单位s
extern unsigned int   						systickCount;			//单位ms

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
extern	unsigned char							Do_Sta[8];					//do状态
extern	unsigned char 						Sw_Sta[8];					//拨码盘状态
extern	unsigned char 						Android_Sta;				//Android电源状态
extern 	int												Ads1247_Value[8];		//ads1247 8个通道的ad数据 
extern  float											Ads_1247_Fvalue[8];	//float 电压数据 v
extern  unsigned char 						Irq_Ads1247_Ready;	//ads1247转换完成中断
extern  short	int									Adc_Data[4];				//ad采集后转换后的数据
extern 	t_SaveNetIPCfg 						gs_SaveNetIPCfg;		

extern 	unsigned char com5_sended;
extern  t_WifiBleAtCmd						gs_WifiBleAtCmd;	
