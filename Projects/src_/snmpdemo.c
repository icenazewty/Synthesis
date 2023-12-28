#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "gd32e503v_eval.h"
#include "w5500.h"
#include "snmpLib.h"
#include "snmpDemo.h"
#include "GlobalVar.h"
#include "usb_para.h"

extern short	int						Adc_Data[6];				//ad采集后转换后的数据
time_t startTime;
dataEntryType snmpData[] =
{
	// System MIB  0x2b=+
	//SNMPDTYPE_OCTET_STRING  注意最长48字节，否则修改 MAX_STRING 
	// SysDescr Entry  		6121110
	{8, {0x2b, 6, 1, 2, 1, 1, 1, 0}, 
	SNMPDTYPE_OCTET_STRING, 30, {"WIZnet Embedded SNMP Agent"}, 
	NULL, NULL},

	// SysObjectID Entry 	6121120
	{8, {0x2b, 6, 1, 2, 1, 1, 2, 0}, 
	SNMPDTYPE_OBJ_ID, 8, {"\x2b\x06\x01\x02\x01\x01\x02\x00"},
	NULL, NULL},

	// SysUptime Entry
//	{8, {0x2b, 6, 1, 2, 1, 1, 3, 0}, 
//	SNMPDTYPE_TIME_TICKS, 0, {""},
//	currentUptime, NULL},

	// sysContact Entry			6121140
	{8, {0x2b, 6, 1, 2, 1, 1, 4, 0}, 		
	SNMPDTYPE_OCTET_STRING, 30, {"support@wiznet.co.kr"}, 
	NULL, NULL},

	// sysName Entry				6121150
	{8, {0x2b, 6, 1, 2, 1, 1, 5, 0}, 	
	SNMPDTYPE_OCTET_STRING, 30, {"http://www.wiznet.co.kr"}, 
	NULL, NULL},

	// Location Entry				6121160
	{8, {0x2b, 6, 1, 2, 1, 1, 6, 0}, 
	SNMPDTYPE_OCTET_STRING, 30, {"4F Humax Village"},
	NULL, NULL},

	// SysServices					6121170
	{8, {0x2b, 6, 1, 2, 1, 1, 7, 0}, 
	SNMPDTYPE_INTEGER, 4, {""}, 
	NULL, NULL},

	// WIZnet LED    get的回调函数  类型string  两个参数void*,unchar *			6141010
	{8, {0x2b, 6, 1, 4, 1, 0, 0, 0}, 
	SNMPDTYPE_OCTET_STRING, 30, {""},
	getWIZnetLed, NULL},
	
	// WIZnet LED    get的回调函数  类型string  两个参数void*,unchar *			6141010
	{8, {0x2b, 6, 1, 4, 1, 0, 1, 0}, 
	SNMPDTYPE_OCTET_STRING, 30, {""},
	getRTS, NULL},
	
	// WIZnet LED    get的回调函数  类型string  两个参数void*,unchar *			6141010
	{8, {0x2b, 6, 1, 4, 1, 0, 2, 0}, 
	SNMPDTYPE_OCTET_STRING, 30, {""},
	getDeviceTempe, NULL},
	
	// WIZnet LED    get的回调函数  类型string  两个参数void*,unchar *			6141010
	{8, {0x2b, 6, 1, 4, 1, 0, 3, 0}, 
	SNMPDTYPE_OCTET_STRING, 36, {""},
	getInputStatus, NULL},
	
	{8, {0x2b, 6, 1, 4, 1, 0, 4, 0}, 
	SNMPDTYPE_OCTET_STRING, 36, {""},
	getOutputStatus, NULL},
	
	{8, {0x2b, 6, 1, 4, 1, 0, 5, 0}, 
	SNMPDTYPE_OCTET_STRING, 36, {""},
	getDipStatus, NULL},	

	{8, {0x2b, 6, 1, 4, 1, 0, 6, 0}, 
	SNMPDTYPE_OCTET_STRING, 32, {""},
	getAndroid_Ctrl, NULL},
		
	{8, {0x2b, 6, 1, 4, 1, 0, 7, 0}, 
	SNMPDTYPE_OCTET_STRING, 48, {""},
	getAd, NULL},	
	
	{8, {0x2b, 6, 1, 4, 1, 0, 8, 0}, 
	SNMPDTYPE_OCTET_STRING, 48, {""},
	getBat, NULL},	
	
	{8, {0x2b, 6, 1, 4, 1, 0, 9, 0}, 
	SNMPDTYPE_OCTET_STRING, 48, {""},
	get12V, NULL},	
		
	{8, {0x2b, 6, 1, 4, 1, 0, 10, 0}, 
	SNMPDTYPE_OCTET_STRING, 48, {""},
	getModbus_ID, NULL},	
	{8, {0x2b, 6, 1, 4, 1, 0, 11, 0}, 
	SNMPDTYPE_OCTET_STRING, MAX_STRING, {""},
	getRj45, NULL},	
	{8, {0x2b, 6, 1, 4, 1, 0, 12, 0}, 
	SNMPDTYPE_OCTET_STRING, MAX_STRING, {""},
	getComInfo, NULL},	
	
	{8, {0x2b, 6, 1, 4, 1, 0, 20, 0}, 			
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, setOutput_On},
	
	{8, {0x2b, 6, 1, 4, 1, 0, 21, 0}, 			
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, setOutput_Off},	
	
	{8, {0x2b, 6, 1, 4, 1, 0, 22, 0}, 			
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, setAndroid_Ctrl},	
		
	//set的回调函数，类型int				6141020		//0x2b是ASN.1中1.3的缩写,即 1*40+3=0x2b。整个oid实际为1.3.6.1.4.1.0.2.0
	{8, {0x2b, 6, 1, 4, 1, 0, 23, 0}, 			
	SNMPDTYPE_INTEGER, 4, {""},
	NULL, setWIZnetLed},
	
	{8, {0x2b, 6, 1, 4, 1, 0, 24, 0}, 			
	SNMPDTYPE_OCTET_STRING, 32, {""},
	setRj45_MAC,NULL},			
};

void setRj45_MAC(void *ptr,  unsigned char *len)
{
	//12:34:56:78:90:ab
	if(*len == 17)			//18-1
	{
		strtomac(gs_SaveNetIPCfg.ucMAC,(char*)ptr,*len);  
	}
}
//增加trap命令，打印调试信息

void setOutput_Off(int val)
{
	if(val > -1 && val <8)
	{
			DO_Output_Ctrl(val,D_OFF);
	}
}

void setOutput_On(int val)
{
	if(val > -1 && val <8)
	{
		DO_Output_Ctrl(val,D_ON);
	}
}

void setAndroid_Ctrl(int val)
{
	if(val==0)
	{
		Android_Ctrl(0);
	}
	else if(val==1)
	{
		Android_Ctrl(1);
	}		
}

void getAndroid_Ctrl(void *ptr,  unsigned char *len)
{
	if(Android_Sta)
	{
		*len = sprintf((char *)ptr, "10V On"); 
	}
	else
	{
		*len = sprintf((char *)ptr, "10V Off"); 
	}
}

void getAd(void *ptr,  unsigned char *len)
{		
	*len = sprintf((char *)ptr, "1:%.2f 2:%.2f 3:%.2f 4:%.2f 5:%.2f 6:%.2f 7:%.2f 8:%.2f(mV)",(Ads_1247_Fvalue[0]*1000),(Ads_1247_Fvalue[1]*1000),(Ads_1247_Fvalue[2]*1000),(Ads_1247_Fvalue[3]*1000),(Ads_1247_Fvalue[4]*1000),(Ads_1247_Fvalue[5]*1000),(Ads_1247_Fvalue[6]*1000),(Ads_1247_Fvalue[7]*1000)); 	
}
//1:-19000.00 2:-19000.00 3:-19000.00 4:-19000.00 5:-19000.00 6:-19000.00 7:-19000.00 8:-19000.00(mV)
//12*8=96 + 3 = 99字节 最长

const int maxData = (sizeof(snmpData) / sizeof(dataEntryType));

int wiznetLedStatus = 0;

void getDipStatus(void *ptr, unsigned char *len)
{		
	*len = sprintf((char *)ptr, "DIP Input:(0bit)%d%d%d%d %d%d(5bit)",Sw_Sta[0],Sw_Sta[1],Sw_Sta[2],Sw_Sta[3],Sw_Sta[4],Sw_Sta[5]);
}

void getInputStatus(void *ptr, unsigned char *len)
{		
	*len = sprintf((char *)ptr, "Data Input:(0bit)%d%d%d%d %d%d%d%d(7bit)",DI.bInput_State[0],DI.bInput_State[1],DI.bInput_State[2],DI.bInput_State[3],DI.bInput_State[4],DI.bInput_State[5],DI.bInput_State[6],DI.bInput_State[7]);
}

void getOutputStatus(void *ptr, unsigned char *len)
{		
	*len = sprintf((char *)ptr, "Data Output:(0bit)%d%d%d%d %d%d%d%d(7bit)",Do_Sta[0],Do_Sta[1],Do_Sta[2],Do_Sta[3],Do_Sta[4],Do_Sta[5],Do_Sta[6],Do_Sta[7]);
}

void getWIZnetLed(void *ptr, unsigned char *len)
{
	if ( wiznetLedStatus==0 )	
      *len = sprintf((char *)ptr, "LED OFF");
	else	
      *len = sprintf((char *)ptr, "LED ON");	
}

void setWIZnetLed(int val)
{
	wiznetLedStatus = val;
	if ( wiznetLedStatus==0 )
	{		 
		Led_Ctrl(0,0);			
	}
	else
	{						
		Led_Ctrl(0,1);
	}
}

void getBat(void *ptr,  unsigned char *len)
{
	*len = sprintf((char *)ptr, "bat:%.2fV",(float)(Adc_Data[0]/100.0));	
}
void get12V(void *ptr,  unsigned char *len)
{
	*len = sprintf((char *)ptr, "12V:%.2fV",(float)(Adc_Data[1]/100.0));	
}
void getRTS(void *ptr,  unsigned char *len)
{
	*len = sprintf((char *)ptr, "Device T:%d",Adc_Data[3]);
}
void getDeviceTempe(void *ptr,  unsigned char *len)
{
	*len = sprintf((char *)ptr, "RTS T:%d",Adc_Data[2]);
}

void getModbus_ID(void *ptr,  unsigned char *len)
{
	*len = sprintf((char *)ptr, "Modbus ID:%d",gs_SaveNetIPCfg.g_ModbusID);
}

//获取socket 1的参数
void getSock1(void *ptr,  unsigned char *len)
{
	*len = sprintf((char *)ptr, "Modbus ID:%d",gs_SaveNetIPCfg.g_ModbusID);
}

//获取socket 2的参数
void getSock2(void *ptr,  unsigned char *len)
{
	*len = sprintf((char *)ptr, "Modbus ID:%d",gs_SaveNetIPCfg.g_ModbusID);
}

//获取socket 3的参数
void getSock3(void *ptr,  unsigned char *len)
{
	*len = sprintf((char *)ptr, "Modbus ID:%d",gs_SaveNetIPCfg.g_ModbusID);
}

//设置socket 1参数
//设置socket 2参数
//设置socket 3参数

//设置com口参数  分别控制1  2  3  4  5  6都可以控制
void setRj45_IP(void *ptr,  unsigned char *len)
{
	//12:34:56:78:90:ab
	if(*len == 17)			//18-1
	{
		strtomac(gs_SaveNetIPCfg.ucMAC,(char*)ptr,*len);  
	}
}

//格式   “1 9600”
void setCom_Info(void *ptr,  unsigned char *len)
{

}


void getRj45(void *ptr,  unsigned char *len)
{	
	*len = sprintf((char *)ptr, "Rj45:%d %02X%02X%02X%02X%02X%02X %d.%d.%d.%d %d.%d.%d.%d %d.%d.%d.%d",gs_SaveNetIPCfg.ucDhcpMode[0],gs_SaveNetIPCfg.ucMAC[0],gs_SaveNetIPCfg.ucMAC[1],gs_SaveNetIPCfg.ucMAC[2],gs_SaveNetIPCfg.ucMAC[3],gs_SaveNetIPCfg.ucMAC[4],gs_SaveNetIPCfg.ucMAC[5],
				 gs_SaveNetIPCfg.ucSelfIP[0],gs_SaveNetIPCfg.ucSelfIP[1],gs_SaveNetIPCfg.ucSelfIP[2],gs_SaveNetIPCfg.ucSelfIP[3],
			   gs_SaveNetIPCfg.ucSubMASK[0],gs_SaveNetIPCfg.ucSubMASK[1],gs_SaveNetIPCfg.ucSubMASK[2],gs_SaveNetIPCfg.ucSubMASK[3],
				 gs_SaveNetIPCfg.ucGateWay[0],gs_SaveNetIPCfg.ucGateWay[1],gs_SaveNetIPCfg.ucGateWay[2],gs_SaveNetIPCfg.ucGateWay[3]);
}
//unsigned char ucDhcpMode[2];			//dhcp模式

void getComInfo(void *ptr,  unsigned char *len)
{
	*len = sprintf((char *)ptr, "COM:%d|%d|%d|%d|%d %d %d %d %d %d",gs_SaveNetIPCfg.baud[0],gs_SaveNetIPCfg.baud[1],gs_SaveNetIPCfg.baud[2],gs_SaveNetIPCfg.baud[3],gs_SaveNetIPCfg.baud[4],gs_SaveNetIPCfg.chekbit[0],gs_SaveNetIPCfg.chekbit[1],gs_SaveNetIPCfg.chekbit[2],gs_SaveNetIPCfg.chekbit[3],gs_SaveNetIPCfg.chekbit[4]);
}

void UserSnmpDemo(void)
{
	WDEBUG("\r\n\r\nStart UserSnmpDemo");
	SnmpXInit();
	{
		dataEntryType enterprise_oid = {8, {0x2b, 6, 1, 4, 1, 0, 0x10, 0}, SNMPDTYPE_OBJ_ID, 8, {"\x2b\x06\x01\x04\x01\x00\x10\x00"},	NULL, NULL};

		dataEntryType trap_oid1 = {8, {0x2b, 6, 1, 4, 1, 0, 11, 0}, SNMPDTYPE_OCTET_STRING, 30, {""}, NULL, NULL};
		dataEntryType trap_oid2 = {8, {0x2b, 6, 1, 4, 1, 0, 12, 0}, SNMPDTYPE_INTEGER, 4, {""}, NULL, NULL};

		strcpy((char*)trap_oid1.u.octetstring, "Alert!!!");
		trap_oid2.u.intval = 123456;
		
		//SnmpXTrapSend("222.98.173.250", "127.0.0.0", "public", enterprise_oid, 1, 0, 0);
		//SnmpXTrapSend("222.98.173.250", "127.0.0.0", "public", enterprise_oid, 6, 0, 2, &trap_oid1, &trap_oid2);
		SnmpXTrapSend("192.168.1.114", "192.168.1.111", "public", enterprise_oid, 1, 0, 0);			//目标端口  162  trap信息
		SnmpXTrapSend("192.168.1.111", "127.0.0.0", "public", enterprise_oid, 6, 0, 2, &trap_oid1, &trap_oid2);
	}
	SnmpXDaemon();
}

//snmpset -v 1 -c public 192.168.1.111 .1.3.6.1.4.1.0.2.0 i 1			led亮
//snmpset -v 1 -c public 192.168.1.111 .1.3.6.1.4.1.0.2.0 i 0			led灭

//snmpget -v 1 -c public 192.168.1.111 .1.3.6.1.4.1.0.1.0
