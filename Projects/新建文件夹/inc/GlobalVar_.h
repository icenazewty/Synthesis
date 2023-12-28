//#include "stm32f10x.h"
#include "define.h"
#include "Struct_Define.h"

////extern u16 Again_Start_Up_Delay; //´ı»úºÍ¶¨Ê±Æô¶¯ºóÑÓÊ±Ò»µãÊ±¼äºó£¬µÈÊä³öÎÈ¶¨ºóÔÙÈ¥ÅĞ¶Ï¹¦ÂÊÊÇ·ñ»¹ÊÇ´ı»ú£¬Èç¹û²»ÑÓÊ±£¬Æô¶¯²»ÆğÀ´

//#define ROUND_TO_UINT16(x)   ((uint16_t)(x)+0.5)>(x)? ((uint16_t)(x)):((uint16_t)(x)+1) ;
//extern u16 DC_LVD_Disconnect_Vol; //µÍÑ¹¶Ï¿ªµçÑ¹
//extern u16 DC_LVD_Warn_Vol;       //µÍÑ¹±¨¾¯µçÑ¹
//extern u16 DC_LVD_Restore_Vol;    //µÍÑ¹»Ö¸´µçÑ¹

//extern u8  Overload_Restart_Check_Flag;

//extern u8  Comm_ON_OFF_Inverter; //Í¨Ñ¶¿ª¹ØÄæ±äÆ÷£¬=1£º¿ª£»=2£º¹Ø
//extern u8  Comm_ON_Inverter_Flag;//Í¨Ñ¶¿ªÄæ±äÆ÷±êÖ¾
//extern u8  Comm_OFF_Inverter_Flag;//Í¨Ñ¶¹ØÄæ±äÆ÷±êÖ¾

//extern u8  LED1_Mode; //=0:Ãğ£»=1:ÂÌÁÁ£»=2:»ÆÁÁ£»=3:ºìÁÁ£»
//extern u8  LED2_Mode; //=0:Ãğ£»=1:ÂÌÁÁ£»=2:ÂÌÉÁ£»=3:»ÆÉÁ£»=4:ºìÁÁ
//extern u8  LED2_Time;

//extern u8  Check_Times; //ÎÂ¶È´«¸ĞÆ÷ÊÇ·ñÕı³£ÅĞ¶Ï´ÎÊı
//extern u8 Temp_Sensor_Check[3][5];

//extern u8 Judge_LVD_HVD_Temp_AC_OFF;	     //1:µÍÑ¹¶Ï¿ª£¬2£º¸ßÑ¹¶Ï¿ª£»3£º¸ßÎÂ¶Ï¿ª
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
//extern struct Record_Index        Record_Daily_Index,Record_Running_Index;  //Õâ¸ö½á¹¹Ìå³¤¶È4×Ö½Ú¡£

////extern Flag_struct  	Flag;
////extern Tm_struct     Tm;

////extern struct rtc_time systemtime;  //´ÓÊ±ÖÓ²É¼¯µÄÈÕÆÚÊı¾İ¡£


///*ÕıÏÒ²¨ÏàÎ»¼ÆÊıÆ÷ºÍÕı¸º°ë²¨±êÖ¾Æ÷     sign=0Õı°ë²¨£¬sign=1¸º°ë²¨*/

//extern u8     com_enable;                   //´®¿ÚÍ¨ĞÅ¿ØÖÆÉè±¸Ê¹ÄÜ±äÁ¿£¬1£¬»ñµÃÊ¹ÄÜ£»0£¬Ã»ÓĞ»ñµÃÊ¹ÄÜ¡£
//extern u8     com_enable_count;             //´®¿ÚÍ¨ĞÅ¿ØÖÆÊ¹ÄÜÎ»²ÉÑùÂË²¨Æ÷¡£
//extern u8     boat_switch_1;                //´¬ĞÍ¿ª¹Ø±äÁ¿£¬1£¬´¬ĞÍ¿ª¹Ø1µµÓĞĞ§£»0£¬´¬ĞÍ¿ª¹Ø1µµÎŞĞ§¡£
//extern u8     boat_switch_2;                //´¬ĞÍ¿ª¹Ø±äÁ¿£¬1£¬´¬ĞÍ¿ª¹Ø2µµÓĞĞ§£»0£¬´¬ĞÍ¿ª¹Ø2µµÎŞĞ§¡£
//extern u8     boat_switch_3;                //´¬ĞÍ¿ª¹Ø±äÁ¿£¬1£¬´¬ĞÍ¿ª¹ØMµµÓĞĞ§£»0£¬´¬ĞÍ¿ª¹ØMµµÎŞĞ§¡£
//extern u8     boat_switch_count1;           //´¬ĞÍ¿ª¹Ø±äÁ¿²ÉÑùÂË²¨Æ÷¡£
//extern u8     boat_switch_count2;           //´¬ĞÍ¿ª¹Ø±äÁ¿²ÉÑùÂË²¨Æ÷¡£
//extern u8     boat_switch_count3;           //´¬ĞÍ¿ª¹Ø±äÁ¿²ÉÑùÂË²¨Æ÷¡£
//extern u8     overload_detection;          //¹ıÁ÷¼ì²â£¬ÓÃÓÚ¹ıÁ÷ÖØºÏÕ¢¹ı³ÌÖĞ£¬ÖØºÏÕ¢¡£


//extern u8     com_enable;                  //´®¿ÚÍ¨ĞÅ¿ØÖÆÉè±¸Ê¹ÄÜ±äÁ¿£¬1£¬»ñµÃÊ¹ÄÜ£»0£¬Ã»ÓĞ»ñµÃÊ¹ÄÜ¡£
//extern u8     com_enable_count;            //´®¿ÚÍ¨ĞÅ¿ØÖÆÊ¹ÄÜÎ»²ÉÑùÂË²¨Æ÷¡£
//extern u8     remote_switch;               //Ô¶³Ì¿ª¹Ø»ú´¥µã¡£
//extern u8     remote_switch_count;         //Ô¶³Ì¿ª¹Ø»ú´¥µã²ÉÑùÂË²¨Æ÷¡£
//extern u8     boat_switch_1;                //´¬ĞÍ¿ª¹Ø±äÁ¿£¬1£¬´¬ĞÍ¿ª¹Ø1µµÓĞĞ§£»0£¬´¬ĞÍ¿ª¹Ø1µµÎŞĞ§¡£  ÊÖ¶¯¿ª»ú
//extern u8     boat_switch_2;                //´¬ĞÍ¿ª¹Ø±äÁ¿£¬1£¬´¬ĞÍ¿ª¹Ø2µµÓĞĞ§£»0£¬´¬ĞÍ¿ª¹Ø2µµÎŞĞ§¡£  Ô¶³Ì¿ª¹Ø»ú£¨°üÀ¨Í¨ĞÅ¿ª¹Ø»úºÍÔ¶³Ì´¥µã¿ª¹Ø»ú£©
//extern u8     boat_switch_3;                //´¬ĞÍ¿ª¹Ø±äÁ¿£¬1£¬´¬ĞÍ¿ª¹ØMµµÓĞĞ§£»0£¬´¬ĞÍ¿ª¹ØMµµÎŞĞ§¡£  ¹Ø»ú
//extern u8     boat_switch_count1;           //´¬ĞÍ¿ª¹Ø±äÁ¿²ÉÑùÂË²¨Æ÷¡£
//extern u8     boat_switch_count2;           //´¬ĞÍ¿ª¹Ø±äÁ¿²ÉÑùÂË²¨Æ÷¡£
//extern u8     boat_switch_count3;           //´¬ĞÍ¿ª¹Ø±äÁ¿²ÉÑùÂË²¨Æ÷¡£
//extern u8     overload_detection;          //¹ıÁ÷¼ì²â£¬ÓÃÓÚ¹ıÁ÷ÖØºÏÕ¢¹ı³ÌÖĞ£¬ÖØºÏÕ¢¡£
//extern u8     standby_off;                 //Ê¡µç´ı»úÄ£Ê½ÏÂµÄ¹Ø»úÃüÁî
////¿ª¹Ø»ú×´Ì¬»úº¯ÊıĞèÒªËø´æµÄÊÇ7¸ö±äÁ¿·Ö±ğÊÇ£º1£¬¹ÊÕÏ±êÖ¾£¬2£¬´¬ĞÍ¿ª¹Ø×´Ì¬£¨±¾µØ/¹Ø»ú/Ô¶³Ì£©£¬3£¬Ô¶³ÌÍ¨ĞÅ¿ØÖÆÊ¹ÄÜ±êÖ¾£¨Ô¶³ÌÍ¨ĞÅ/Ô¶³Ì¿ª¹Ø£©£¬4£¬Ô¶³ÌÍ¨ĞÅ¿ª¹Ø»úÖ¸Áî£¬5£¬Ô¶³Ì¿ª¹ØÖ¸Áî¿ª¹Ø»úÖ¸Áî£¬6£¬¶¨Ê±ÆôÍ£¿ª¹Ø»úÖ¸Áî£¬7£¬Ê¡µç¿ª¹Ø»úÖ¸Áî¡£
////·Ö±ğ±äÁ¿ÈçÏÂ£º1£¬device_fault;2,boatswitch_position;3,con_communication_enable; 4, on_off_communication;5 on_off_remoteswitch;6,on_off_timing;7,on_off_saving;
//extern u8     device_fault;                //¹ÊÕÏ±êÖ¾
//extern u8     boatswitch_position;         //´¬ĞÍ¿ª¹Ø×´Ì¬£¨±¾µØ/¹Ø»ú/Ô¶³Ì£©
//extern u8     con_communication_enable;    //Ô¶³ÌÍ¨ĞÅ¿ØÖÆÊ¹ÄÜ±êÖ¾£¨Ô¶³ÌÍ¨ĞÅ/Ô¶³Ì¿ª¹Ø£©
//extern u8     on_off_communication;        //Ô¶³ÌÍ¨ĞÅ¿ª¹Ø»úÖ¸Áî¡£
//extern u8     on_off_remoteswitch;         //Ô¶³Ì¿ª¹ØÖ¸Áî¿ª¹Ø»úÖ¸Áî¡£
//extern u8     on_off_timing;               //¶¨Ê±ÆôÍ£¿ª¹Ø»úÖ¸Áî¡£
//extern u8     on_off_saving;               //Ê¡µç¿ª¹Ø»úÖ¸Áî¡£
//extern u8     ac_output_enable;            //ÓÉÍâ²¿Ìõ¼ş¸ø¶¨µÄ¿ª¹Ø»úÃüÁî
//extern u8     LVD_OFF_Flag;                  //¼ì²éµ½Ö±Á÷µçÑ¹µÍ±êÖ¾±äÁ¿£¬0£¬Ã»ÓĞ¼ì²âµ½£»1£¬¼ì²âµ½¡£
//extern u16    LVD_OFF_Flag_count;            //¼ì²éµ½Ö±Á÷µçÑ¹µÍ±êÖ¾±äÁ¿ÂË²¨ĞÅºÅ
//extern u8     HVD_OFF_Flag;                  //¼ì²éµ½Ö±Á÷µçÑ¹¸ß±êÖ¾±äÁ¿£¬0£¬Ã»ÓĞ¼ì²âµ½£»1£¬¼ì²âµ½¡£
//extern u16    HVD_OFF_Flag_count;            //¼ì²éµ½Ö±Á÷µçÑ¹¸ß±êÖ¾±äÁ¿ÂË²¨ĞÅºÅ
//extern u8     TEMP_OFF_Flag;                 //¼ì²âµ½¸ßÎÂ±êÖ¾±äÁ¿£¬0£¬Ã»ÓĞ¼ì²âµ½£»1£¬¼ì²âµ½¡£
//extern u16    TEMP_OFF_Flag_count;           //¼ì²âµ½¸ßÎÂ±êÖ¾±äÁ¿ÂË²¨ĞÅºÅ


//extern u8     timingfunc_enable;              //¶¨Ê±ÆôÍ£»úÄ£Ê½Ê¹ÄÜ±äÁ¿¡£
//extern u8     timingfunc_enable_count;        //¶¨Ê±ÆôÍ£»úÄ£Ê½Ê¹ÄÜ±äÁ¿²ÉÑùÂË²¨¡£
//extern u8     savingfunc_enable;              //½ÚµçÆôÍ£»úÄ£Ê½Ê¹ÄÜ±äÁ¿¡£
//extern u8     savingfunc_enable_count;        //½ÚµçÆäÍ£»úÄ£Ê½Ê¹ÄÜ±äÁ¿²ÉÑùÂË²¨¡£
//extern u8     LVD_FUNC_ENABLE;
//extern u8     LVD_FUNC_ENABLE_count;
//extern u8     LVD_FUNC_VALUE;                //¼ì²âµÍÑ¹¹Ø»úÓ²¼ş·§ÖµĞÅºÅ£¬0£¬10.5v£»1£¬11.5v
//extern u8     LVD_FUNC_VALUE_count;          //¼ì²âµÍÑ¹¹Ø»úÓ²¼ş·§ÖµÂË²¨ĞÅºÅ¡£
//extern u8     on_off_and_savinglogic;
//extern u32    QITONG_MAX;
//extern u32    QITONG_MAX1;
//extern u32    QITONG_MAX2;                    //²âÊÔÓÃ¡£
//extern u32    QITONG_MAX3;                    //²âÊÔÓÃ¡£
//extern u8     QITONG_MAX3_COUNT;              //²âÊÔÓÃ¡£
//extern u32    QITONG_MAX4;                    //²âÊÔÓÃ¡£
//extern float  QITONG_VALUE;                   //²âÊÔÓÃ¡£
//extern float  QITONG_VALUE1;                   //²âÊÔÓÃ¡£
//extern float  QITONG_VALUE2;                   //²âÊÔÓÃ¡£
//extern float  QITONG_VALUE3;                   //²âÊÔÓÃ¡£
//extern float  QITONG_VALUE4;                   //²âÊÔÓÃ¡£
////extern u8      test_bit2;
//extern u8     sine_P_transition_sampling;     //ÓÃÓÚ²ÉÑù½ÚÅÄÆ÷
//extern u8     sine_P_transition_regulation;   //ÓÃÓÚµçÑ¹µ÷½ÚµÄ½ÚÅÄÆ÷

////Ö±Á÷µçÑ¹ÂË²¨Êı×é£¬DC_VOL_FILTER[20]
//extern u16   DC_VOL_FILTER[60];
//extern u8     DC_VOL_FILTER_count; 
//extern u32   DC_VOL_FILTER_sum;

////Ö±Á÷µçÑ¹Ç°À¡ÂË²¨Êı×é£¬DC_VOL_FFB_FILTER[20]
//extern u16   DC_VOL_FFB_FILTER[300];
//extern u16    DC_VOL_FFB_FILTER_count; 
//extern u16   DC_VOL_FFB_FILTER_samplecount; 
//extern u32   DC_VOL_FFB_FILTER_sum;
//extern u16   DC_VOL_FFB_VOL;
//extern float   DC_VOL_FFB_CONTROLVALUE;

//	typedef enum                                //ÊÂ¼şÏµÍ³ÀïµÄÊÂ¼şÀàĞÍÃ¶¾Ù
//{
//	   Startup=0x01,     //ÉÏµçÆô¶¯
//	     time_align,     //Ê±¼äĞ£×¼
//	        Turn_on,     //¿ª»ú²Ù×÷
//	       Turn_off,     //¹Ø»ú²Ù×÷	
//	    Error_occur,     //¹ÊÕÏ·¢Éú
//	    Error_reset,     //¹ÊÕÏ¸´Î»
//	    Alarm_occur,     //±¨¾¯·¢Éú
//	    Alarm_reset,     //±¨¾¯¸´Î»
//  
//}event_flag;


//extern float u_v;	  //¿ØÖÆÆ÷×ÜÊä³öµÄÖµ
//extern float u1_v;	//ÉÏ´ÎPIÊä³öµÄÖµ
//extern float u2_v;	//±¾´ÎPIÊä³öµÄÖµ
//#define  Kp_v  0.001;	//±ÈÀıÏµÊı
//#define  Ki_v  0.0001;	//»ı·ÖÏµÊı
//extern float Kpv;         // 0.001;
//extern float Kiv;      // 0.0001;
//extern float ek_v;	//µ±´ÎÎó²î
//extern float ek1_v;	//ÉÏÒ»´ÎÎó²î

//extern float u_i;	//Õâ´ÎÊä³öµÄÖµ
//extern float u1_i;	//ÉÏ´ÎÊä³öµÄÖµ
//extern float Kp_i;	//±ÈÀıÏµÊı
//extern float Ki_i;	//»ı·ÖÏµÊı
////#define  Kp_i  0.01;	//±ÈÀıÏµÊı
////#define  Ki_i  0.001;	//»ı·ÖÏµÊı
////extern  float 	 Kp_i; // 0.01;
////extern  float 	 Ki_i; // 0.001;
//extern float ek_i;	//µ±´ÎÎó²î
//extern float ek1_i;	//ÉÏÒ»´ÎÎó²î
//extern int  VALUE_CONTROL;

//extern u8 Time_preset[6];

//extern u32    ac_voltage_accu;                //½»Á÷µçÑ¹²ÉÑùÀÛ¼ÓÆ÷
//extern u32    ac_current_accu;                //½»Á÷µçÁ÷²ÉÑùÀÛ¼ÓÆ÷
//extern u8     ac_sampling_count;              //½»Á÷²ÉÑù´°¿Ú¼ÆÊıÆ÷

////ËÙ¶Ï±£»¤È«¾Ö±äÁ¿  Ê±¼ä³£Êı10ms
////extern const float time_factor = 0.001;  //²ÉÑù¼ä¸ô1ms
////extern const float tc_oc_first = 0.01;   //²ÉÑùÖÜÆÚ10ms
//extern float i2T_VALUE[10];	  //I2T ±äÁ¿±í
//extern u8    i2T_VALUE_index;	//I2T ±äÁ¿±íÖ¸Õë
//extern float i2T_SUM;          // ËÙ¶Ï±£»¤ I2TÀÛ¼ÓºÍ
//extern float i2T_SUM_ref;      //¼ÆËãËÙ¶Ï±£»¤µÄãĞÖµ
//extern u8    flag_tc_oc_first; //ËÙ¶Ï±£»¤±êÖ¾Î» 0£ºÎŞËÙ¶Ï±£»¤£¬1£ºÓĞËÙ¶Ï±£»¤

////i2T ±£»¤È«¾Ö±äÁ¿  Ê±¼ä³£Êı5s
////const float time_factor = 0.001;  //²ÉÑù¼ä¸ô1ms
////extern const float  tc_oc_second = 5;      //²ÉÑùÖÜÆÚ5s
//extern float        e_i2t;                 //i2tÎó²î
//extern float        I2T_OC_second;         //i2tÖµ
//extern float        I2T_OC_second_ref;     //¼ÆËãI2T±£»¤ãĞÖµ
//extern u8           flag_I2T_OC_second;    //I2T±£»¤±êÖ¾Î» 0£ºÎŞI2T±£»¤£¬1£ºÓĞI2T±£»¤

////¹ıÁ÷´òàÃ±äÁ¿
//extern u8           hiccup_oc;             //´òàÃ¶¯×÷±êÖ¾Î»
//extern u8           hiccup_oc1;            //´òàÃ¶¯×÷±êÖ¾Î»		
//extern u32          hiccup_oc_count;       //´òàÃ¶¯×÷¼ÆÊ±Æ÷
//extern u8           hiccup_tick;           //´òàÃ½ÚÅÄÊı
//extern u8           hiccup_type;           //´òàÃÖÖÀà±êÖ¾Î»

extern unsigned int   						g_sysTick;				//å•ä½s
extern unsigned int   						systickCount;			//å•ä½ms

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
extern	unsigned char							Do_Sta[8];					//doçŠ¶æ€
extern	unsigned char 						Sw_Sta[8];					//æ‹¨ç ç›˜çŠ¶æ€
extern	unsigned char 						Android_Sta;				//Androidç”µæºçŠ¶æ€
extern 	int												Ads1247_Value[8];		//ads1247 8ä¸ªé€šé“çš„adæ•°æ® 
extern  float											Ads_1247_Fvalue[8];	//float ç”µå‹æ•°æ® v
extern  unsigned char 						Irq_Ads1247_Ready;	//ads1247è½¬æ¢å®Œæˆä¸­æ–­
extern  short	int									Adc_Data[4];				//adé‡‡é›†åè½¬æ¢åçš„æ•°æ®
extern 	t_SaveNetIPCfg 						gs_SaveNetIPCfg;		

extern 	unsigned char com5_sended;
extern  t_WifiBleAtCmd						gs_WifiBleAtCmd;	
