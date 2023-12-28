//#include "prj.h"
//#include "iic.h"
//#include "CharL1fifodiv.h"
#include "string.h"
//#include "systick.h"
//#include "W5500.h"
#include "cfg_flash.h"
//#include "EEPROM.h"
//#include "wdg.h"

//t_SaveNetIPCfg gs_SaveNetIPCfg;

#include "gd32e50x_fmc.h"
#include "GlobalVar.h"
//#include "stm32f10x_conf.h"
//#include "hardware.h"


#define RJ45_START_ADDR  					0x8000000+0x2000*31				//gs_SaveNetIPCfg		 	RJ45网络参数   32*8
#define	WIFI_START_ADDR   				0x8000000+0x2000*28				//gs_SaveWifiCfg			wifi网络参数

#define ADS1247_START_ADDR 				0x8000000+0x2000*30				//g_Ads1247_Cali			ads1247校准参数
#define PARAM_START_ADDR  				0x8000000+0x2000*29				//g_configRead  			公共参数	


uint16_t 			value = 0;
uint32_t			value1 = 0,value2;				//*R1#
volatile fmc_state_enum FLASHStatus = FMC_READY;

#if 0
#define FLASH_START_ADDR  				0x8000000+2048*127
void WriteFlashOneWord(uint32_t WriteAddress,uint32_t WriteData)
{
	fmc_unlock();
	fmc_flag_clear(FMC_READY | FMC_PGERR | FMC_WPERR);
	FLASHStatus = fmc_page_erase(FLASH_START_ADDR);
	if(FLASHStatus == FMC_READY)
	{
		FLASHStatus = fmc_word_program(FLASH_START_ADDR + WriteAddress, WriteData); //flash.c 中API函数
	}
	fmc_lock();
}
#endif


void EnterCritical(void)
{
	//__disable_irq();  
}

void LeaveCritical(void)
{
	//__enable_irq();
}

void WriteFlash_CPU(unsigned int addr ,unsigned char *str,int len)
{
	fmc_state_enum FLASHStatus = FMC_READY;
	unsigned int	data;
	int cnt = (len+3)/4,i=0;
	EnterCritical();
	fmc_unlock();
	fmc_flag_clear(FMC_READY | FMC_PGERR | FMC_WPERR);
	FLASHStatus = fmc_page_erase(addr);
	fmc_lock();	
	if(FLASHStatus == FMC_READY)
	{
			fmc_unlock();		
			for(i=0;i<cnt;i++)
			{
				data = *((unsigned int*)(str+i*4));
				FLASHStatus = fmc_word_program(addr + i*4, data); 			//flash.c 中API函数
			}
			fmc_lock();
	}	
	LeaveCritical();
}

char ReadFlash_CPU(unsigned int addr,unsigned char *str,int len)
{
	unsigned int		*data = (unsigned int*)str;
	//t_SaveWifiCfg	
	int cnt = (len+3)/4,i=0;	
	for(i=0;i<cnt;i++)
	{
			data[i] = *(unsigned int*)(addr+i*4);			
	}	
	return 1;	
}


/*******************************************************************************
* ������  : Load_Net_Parameters
* ����    : װ���������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ���ء����롢������ַ������IP��ַ���˿ںš�Ŀ��IP��ַ��Ŀ�Ķ˿ںš��˿ڹ���ģʽ
*******************************************************************************/
void Load_Net_Parameters(void)
{	
	gs_SaveNetIPCfg.ucStartFlag = '*';
	gs_SaveNetIPCfg.ucEndFlag = '#';
	
	gs_SaveNetIPCfg.ucMAC[0]=0x00;				//MAC��ַ
	gs_SaveNetIPCfg.ucMAC[1]=0x29;
	gs_SaveNetIPCfg.ucMAC[2]=0xab;
	gs_SaveNetIPCfg.ucMAC[3]=0x7c;
	gs_SaveNetIPCfg.ucMAC[4]=0x00;
	gs_SaveNetIPCfg.ucMAC[5]=0x0D;
	
	gs_SaveNetIPCfg.ucSelfIP[0]=192;			//������IP��ַ
	gs_SaveNetIPCfg.ucSelfIP[1]=168;
	gs_SaveNetIPCfg.ucSelfIP[2]=1;
	gs_SaveNetIPCfg.ucSelfIP[3]=188;
	
	gs_SaveNetIPCfg.ucGateWay[0] = 192;		//����
	gs_SaveNetIPCfg.ucGateWay[1] = 168;
	gs_SaveNetIPCfg.ucGateWay[2] = 1;
	gs_SaveNetIPCfg.ucGateWay[3] = 1;

	gs_SaveNetIPCfg.ucSubMASK[0]=255;			//��������
	gs_SaveNetIPCfg.ucSubMASK[1]=255;
	gs_SaveNetIPCfg.ucSubMASK[2]=255;
	gs_SaveNetIPCfg.ucSubMASK[3]=0;	

	gs_SaveNetIPCfg.ucDhcpMode[0]=0;
	gs_SaveNetIPCfg.ucDhcpMode[1]=0;
	
	gs_SaveNetIPCfg.ucDestPort[0] = 0x1B;					//0x1bc3=7107    tcp client server port
	gs_SaveNetIPCfg.ucDestPort[1] = 0xC3;		
	
	gs_SaveNetIPCfg.ucSourcePort[0] = 0x13;				//0x1389=5001		 tcp client client source port
	gs_SaveNetIPCfg.ucSourcePort[1] = 0x89;

	gs_SaveNetIPCfg.ucDestIP[0]=82;							//TCP client  server ip  82.156.190.184
	gs_SaveNetIPCfg.ucDestIP[1]=156;
	gs_SaveNetIPCfg.ucDestIP[2]=190;
	gs_SaveNetIPCfg.ucDestIP[3]=184;				
	
	gs_SaveNetIPCfg.ucUdpDestIP[0]=192;						//udp traget ip
	gs_SaveNetIPCfg.ucUdpDestIP[1]=168;
	gs_SaveNetIPCfg.ucUdpDestIP[2]=1;
	gs_SaveNetIPCfg.ucUdpDestIP[3]=114;				
	
	gs_SaveNetIPCfg.ucUdpDestPort[0] = 0x22;			//0x12b8=8887    udp target port
	gs_SaveNetIPCfg.ucUdpDestPort[1] = 0xB7;		
	
	gs_SaveNetIPCfg.ucUdpSourcePort[0] = 0x22;		//0x12b7=8888    udp source port(udp monitor port)
	gs_SaveNetIPCfg.ucUdpSourcePort[1] = 0xB8;
	
		
	gs_SaveNetIPCfg.ucMonitorPort[0] = 0x01;			//tcp server 0x01f6=502		
	gs_SaveNetIPCfg.ucMonitorPort[1] = 0xF6;
	
	gs_SaveNetIPCfg.ucSocket0Mode = TCP_CLIENT;		//socket0����ģʽ=TCP�ͻ���ģʽ
	gs_SaveNetIPCfg.ucSocket1Mode = TCP_SERVER;		//socket1����ģʽ=TCP�����ģʽ	
	gs_SaveNetIPCfg.ucSocket2Mode = UDP_MODE;
	
	gs_SaveNetIPCfg.baud[0] = 115200;				//通讯口android
	gs_SaveNetIPCfg.baud[1] = 9600;
	gs_SaveNetIPCfg.baud[2] = 115200;				//usb口
	gs_SaveNetIPCfg.baud[3] = 9600;
	gs_SaveNetIPCfg.baud[4] = 9600;
		
	gs_SaveNetIPCfg.chekbit[0] = 0;
	gs_SaveNetIPCfg.chekbit[1] = 0;
	gs_SaveNetIPCfg.chekbit[2] = 0;
	gs_SaveNetIPCfg.chekbit[3] = 0;
	gs_SaveNetIPCfg.chekbit[4] = 0;	
		
	
	gs_SaveNetIPCfg.g_ModbusID = 0x01;
	
}

void  WriteParametersNetDefault(void)
{
	  unsigned char i,j,temp[128],*p;	 
		int len = sizeof(gs_SaveNetIPCfg);
	
		gs_SaveNetIPCfg.ucSelfIP[0]=192;		//������IP��ַ
		gs_SaveNetIPCfg.ucSelfIP[1]=168;
		gs_SaveNetIPCfg.ucSelfIP[2]=1;
		gs_SaveNetIPCfg.ucSelfIP[3]=188;
	
		gs_SaveNetIPCfg.ucGateWay[0] = 192;		//����
		gs_SaveNetIPCfg.ucGateWay[1] = 168;
		gs_SaveNetIPCfg.ucGateWay[2] = 1;
		gs_SaveNetIPCfg.ucGateWay[3] = 1;
	
		gs_SaveNetIPCfg.ucSubMASK[0]=255;			//��������
		gs_SaveNetIPCfg.ucSubMASK[1]=255;
		gs_SaveNetIPCfg.ucSubMASK[2]=255;
		gs_SaveNetIPCfg.ucSubMASK[3]=0;	
	
	  gs_SaveNetIPCfg.ucMonitorPort[0] = 0x01;	//������SOCKET1�˿ں�502 
	  gs_SaveNetIPCfg.ucMonitorPort[1] = 0xF6;
	
		gs_SaveNetIPCfg.ucSocket0Mode = TCP_CLIENT;		//socket0¹¤׷ģʽ=TCP¿ͻ§¶˄£ʽ
		gs_SaveNetIPCfg.ucSocket1Mode = TCP_SERVER;	//socket1����ģʽ=TCP�����ģʽ	
		gs_SaveNetIPCfg.ucSocket2Mode = UDP_MODE;
		
		gs_SaveNetIPCfg.baud[0] = 115200;			//通讯口android
		gs_SaveNetIPCfg.baud[1] = 9600;
		gs_SaveNetIPCfg.baud[2] = 115200;				//usb口
		gs_SaveNetIPCfg.baud[3] = 9600;
		gs_SaveNetIPCfg.baud[4] = 9600;
		
		gs_SaveNetIPCfg.chekbit[0] = 0;
		gs_SaveNetIPCfg.chekbit[1] = 0;
		gs_SaveNetIPCfg.chekbit[2] = 0;
		gs_SaveNetIPCfg.chekbit[3] = 0;
		gs_SaveNetIPCfg.chekbit[4] = 0;
		
		
		gs_SaveNetIPCfg.g_ModbusID = 0x01;
		
	  p = &gs_SaveNetIPCfg.ucStartFlag;
		for(i=0;i<10;i++)
	  {
				
		
				WriteFlash_CPU(RJ45_START_ADDR,&gs_SaveNetIPCfg.ucStartFlag,len);
			
				if(ReadFlash_CPU(RJ45_START_ADDR,&temp[0],len))						//ɧڻּֽׁȷ, ҈ޏnۍn+0x800λ׃ˇرΪ״ëìɧڻˇղ׵ܘ1.
		
				{
					for(j=0;j<len;j++)
					{
						if(p[j]!=temp[j])
						{
							break;
						}
					}
					if(j==len)
					{
						break;
					}
				}				
	  }				
		if(i==10)
		{
				//дflash������
		}
}

void  WriteParametersToIICAll(void )
{	
	  unsigned char i,j,temp[256],*p;	 
		int len = sizeof(gs_SaveNetIPCfg);
	  p = &gs_SaveNetIPCfg.ucStartFlag;
		for(i=0;i<10;i++)
	  {
				WriteFlash_CPU(RJ45_START_ADDR,&gs_SaveNetIPCfg.ucStartFlag,len);				
				if(ReadFlash_CPU(RJ45_START_ADDR,&temp[0],len))
				
				{
					for(j=0;j<len;j++)
					{
						if(p[j]!=temp[j])
						{
							break;
						}
					}
					if(j==len)
					{
						break;
					}
				}				
	  }				
		if(i==10)
		{
				//дflash������
		}
}

/******************************************
* 函数名:init_config
* 参数： 无
* 返回值： 无
*
* 说明：初始化默认配置信息并从449内部flash中读取配置信息
******************************************/
unsigned char  init_config_net(unsigned char flag)
{
   // unsigned char tmp; 
  //读取配置信息
	int len = sizeof(gs_SaveNetIPCfg);	

	ReadFlash_CPU(RJ45_START_ADDR,&gs_SaveNetIPCfg.ucStartFlag,len);
	

  if(gs_SaveNetIPCfg.ucStartFlag=='*' && gs_SaveNetIPCfg.ucEndFlag=='#' )//读取失败
	{
			return 1;
	}
	else
  {
//如果发现标志不对，则考虑延时并多次读(5次)数据，如果还不对，则查看系统电压，
//如果电压低于3.5V则系统直接使用默认参数，但不对Flash写操作。否则写默认值
//确认wifi启动时间，必须在读配置信息之后。避免wifi启动瞬间系统电压低。

    if(flag==0) //如果不对，不做任何处理，直接返回
    {
      return 0;
    }
    
    //如果flag不为0则表示要么默认值，要么向flash中写数据。
    memset(&gs_SaveNetIPCfg,0,sizeof(t_SaveNetIPCfg));
    //初始化默认配置信息，如果读取失败则使用默认配置 CONFIG 默认
    Load_Net_Parameters();
      
 //   Test_CaliInit();//程序测试用
   // memcpy(&g_configRead,&g_configDef,sizeof(ConfigInfo));//将默认配置信息作为当前读取的配置信息
    if(flag==2)         //写
    {		
		
				WriteFlash_CPU(RJ45_START_ADDR,&gs_SaveNetIPCfg.ucStartFlag,sizeof(t_SaveNetIPCfg));			

        return 1;
    }  
		return 0;		
  }  
}

void  WriteParametersADS1247(void )
{	
	  unsigned char i,j,temp[256],*p;	 
		int len = sizeof(g_Ads1247_Cali);
	  p = &g_Ads1247_Cali.ucStartFlag;
		for(i=0;i<10;i++)
	  {

		
				WriteFlash_CPU(ADS1247_START_ADDR,&g_Ads1247_Cali.ucStartFlag,len);
	
				if(ReadFlash_CPU(ADS1247_START_ADDR,&temp[0],len))
				
				{
					for(j=0;j<len;j++)
					{
						if(p[j]!=temp[j])
						{
							break;
						}
					}
					if(j==len)
					{
						break;
					}
				}				
	  }				
		if(i==10)
		{
				//дflashӐΊ̢
		}
}

void Load_ADS1247_Parameters()
{
	unsigned char i  = 0;
	g_Ads1247_Cali.ucStartFlag = '*';
	g_Ads1247_Cali.ucEndFlag = '#';
	for(i=0;i<8;i++)
	{
		g_Ads1247_Cali.AdC[i][0] = -1142600;			//-6v	ee9020
		g_Ads1247_Cali.AdC[i][1] = 100;						//0V
		g_Ads1247_Cali.AdC[i][2] = 1142800;				//6V
	}	
}
	
//ADS1247_START_ADDR
//g_Ads1247_Cali
unsigned char  init_config_ads1247(unsigned char flag)
{	 
  //读取配置信息
	int len = sizeof(g_Ads1247_Cali);	
	ReadFlash_CPU(ADS1247_START_ADDR,&g_Ads1247_Cali.ucStartFlag,len);	

  if(g_Ads1247_Cali.ucStartFlag=='*' && g_Ads1247_Cali.ucEndFlag=='#' )			//读取失败
	{
			return 1;
	}
	else
  {
//如果发现标志不对，则考虑延时并多次读(5次)数据，如果还不对，则查看系统电压，
//如果电压低于3.5V则系统直接使用默认参数，但不对Flash写操作。否则写默认值
//确认wifi启动时间，必须在读配置信息之后。避免wifi启动瞬间系统电压低。

    if(flag==0) //如果不对，不做任何处理，直接返回
    {
      return 0;
    }
    
    //如果flag不为0则表示要么默认值，要么向flash中写数据。
    memset(&g_Ads1247_Cali,0,sizeof(t_AdCali));
    //初始化默认配置信息，如果读取失败则使用默认配置 CONFIG 默认
    Load_ADS1247_Parameters();
      
 //   Test_CaliInit();//程序测试用
   // memcpy(&g_configRead,&g_configDef,sizeof(ConfigInfo));//将默认配置信息作为当前读取的配置信息
    if(flag==2)         //写
    {		
		
				WriteFlash_CPU(ADS1247_START_ADDR,&g_Ads1247_Cali.ucStartFlag,sizeof(t_AdCali));			

        return 1;
    }  
		return 0;		
  } 
}

/******************************************
* 函数名:init_config
* 参数： 无
* 返回值： 无
*
* 说明：初始化默认配置信息并从449内部flash中读取配置信息
******************************************/
unsigned char  init_config(unsigned char flag)
{
   // unsigned char tmp; 
  //读取配置信息
	int len = sizeof(g_configRead),i=0;	
	ReadFlash_CPU(PARAM_START_ADDR,&g_configRead.b_success,len);
			
	
  if(g_configRead.b_success=='*' && g_configRead.b_success1=='#' )	//读取成功
	{
		if(g_configRead.NationCode[0]<0x30||g_configRead.NationCode[0]>0x39)	//如果不是数字则默认值
		{
			memset(g_configRead.NationCode,0,sizeof(g_configRead.NationCode));
			g_configRead.NationCode[0] = '8';
			g_configRead.NationCode[1] = '6';
			g_configRead.NationCode[2] = '\0';
		}
		if(g_configRead.sysName[0]==0xff&&g_configRead.sysName[1]==0xff)
		{
			memset(g_configRead.sysName,0,sizeof(g_configRead.sysName));
			g_configRead.sysName[0] = 0x8B;		//设备名  8BBE5907540D
			g_configRead.sysName[1] = 0xBE;		
			g_configRead.sysName[2] = 0x59;		
			g_configRead.sysName[3] = 0x07;		
			g_configRead.sysName[4] = 0x54;		
			g_configRead.sysName[5] = 0x0D;		
			g_configRead.NameLen = 6;			
		}	
		
		if(g_configRead.b_gprs_work!=0)
		{
				g_configRead.b_gprs_work = 1;		
		}
		
		if(g_configRead.b_wifi_work!=1)
		{
				g_configRead.b_wifi_work = 0;		
		}
		
		if(g_configRead.b_rj45_work!=1)
		{
				g_configRead.b_rj45_work = 0;		
		}	
		
		if(g_configRead.b_debug_work!=1)
		{
				g_configRead.b_debug_work = 0;
		}
		
		if(g_configRead.AMPER_RTC_PC13!=1)
		{
				g_configRead.AMPER_RTC_PC13 = 0;
		}
		
		if(g_configRead.Depart[0]==0xff)
		{
				g_configRead.Depart[0] = 0x00;
		}
		
		if(g_configRead.Depart[1]==0xff)
		{
				g_configRead.Depart[1] = 0x00;
		}		
				
		g_EssentialLoadRelayCloseVoltage 		= g_configRead.TMax[1]/10.0 ;
		g_EssentialLoadRelayOpenVoltage 		= g_configRead.TMin[1]/10.0;
		g_NonEssentialLoadRelayCloseVoltage = g_configRead.HMax[1]/10.0;
		g_NonEssentialLoadRelayOpenVoltage 	= g_configRead.HMin[1]/10.0;
		
		g_EssentialLoadRelayCloseVoltage_Sec 		= g_configRead.TMax2/10.0 ;
		g_EssentialLoadRelayOpenVoltage_Sec 		= g_configRead.TMin2/10.0;
		g_NonEssentialLoadRelayCloseVoltage_Sec = g_configRead.HMax2/10.0;
		g_NonEssentialLoadRelayOpenVoltage_Sec 	= g_configRead.HMin2/10.0;
		
		g_LowBatteryVoltageStartGenerator 	= g_configRead.TXZ[0]/10.0;
		g_HighBatteryVoltageStopGenerator 	= g_configRead.HXZ[0]/10.0;
		g_EnclosureFanTurnOnTemperature 		= g_configRead.HXZ[1]/10.0;
		g_EnclosureFanTurnOffTemperature 		= g_configRead.TXZ[1]/10.0;
		
		g_HighPVVoltageStopGenerator 				= g_configRead.b_Sms_Test;		
		return 1;
	}
	else
  {
//如果发现标志不对，则考虑延时并多次读(5次)数据，如果还不对，则查看系统电压，
//如果电压低于3.5V则系统直接使用默认参数，但不对Flash写操作。否则写默认值
//确认wifi启动时间，必须在读配置信息之后。避免wifi启动瞬间系统电压低。

    if(flag==0) //如果不对，不做任何处理，直接返回
    {
      return 0;
    }
    
    //如果flag不为0则表示要么默认值，要么向flash中写数据。
    memset(&g_configRead,0,sizeof(ConfigInfo));
    //初始化默认配置信息，如果读取失败则使用默认配置 CONFIG 默认
    g_configRead.b_success='*';
    g_configRead.b_success1='#';
    
    memcpy(g_configRead.device_ID,"706110",6);
    g_configRead.save_frq=1;						//30Min  是否开启温湿度传感器
    g_configRead.collect_frq=1000;			//ms
    g_configRead.send_frq=300;					//5min
			
    g_configRead.bCH[0]=2;  //通道1默认工作   
    g_configRead.bCH[1]=0; 	//通道2默认不工作   
    
    g_configRead.TMax[0]=0;//-88表示没有上下限制  温度*10
    g_configRead.TMin[0]=0;//
    g_configRead.HMax[0]=0;//
    g_configRead.HMin[0]=0;//
  
    g_configRead.TMax[1]=480.0;		//-88表示没有上下限制  温度*10
    g_configRead.TMin[1]=460.0;		//
    g_configRead.HMax[1]=480.0;		//
    g_configRead.HMin[1]=460.0;		//

		g_configRead.TMax2=480.0;		//-88表示没有上下限制  温度*10
    g_configRead.TMin2=460.0;		//
    g_configRead.HMax2=480.0;		//
    g_configRead.HMin2=460.0;		//
    
    g_configRead.TXZ[0]=460.0;						//温度修正   实际大小*100
    g_configRead.HXZ[0]=580.0;						//湿度修正   实际大小*100
    g_configRead.TXZ[1]=350.0;						//温度修正   实际大小*100
    g_configRead.HXZ[1]=400.0;						//湿度修正   实际大小*100

		g_configRead.VMin=0;									//默认掉电报警
		g_configRead.alarmDelyMinute=1;				//警报延时 单位分钟
		g_configRead.beep=1;
		#if FIX_RJ45
    g_configRead.wifi_mode=4;   										//默认为udp通讯格式
		#else
		g_configRead.wifi_mode=1;   										//默认为udp通讯格式
		#endif
		
    memcpy(g_configRead.Danwei ,"00000001",8);			//?ノ槐嗪?
		g_configRead.Depart[0] = 0;					//RECE  0或[3,240], 每间隔几分钟拨打一次报警电话。
		g_configRead.Depart[1] = 0;					//TELT  0~255	      第一次电话报警延时间隔;	
 //   memcpy(g_configRead.Depart,"01",2);						//部门编号
	//	g_configRead.Depart[0]='0';				//默认不打电话，当Depart[0]='1'则表示打电话，当所有警报都回复正常的时候。											
//		g_configRead.Depart[1]=10;				//报警电话，在发现报警的时候，需要延时多长时间后进行拨打电话。		默认10分钟。最大0~255					
		
  //  memcpy(g_configRead.alarmNum[0],"*18600653082",12);
  //  memcpy(g_configRead.alarmNum[1],"*18600653082",12);  

    memset(g_configRead.remoteIP,0,sizeof(g_configRead.remoteIP));
    memcpy(g_configRead.remoteIP,"82.156.190.184",14);		//远程IP
    g_configRead.IPLen=14;    
    memset(g_configRead.remotePort,0,sizeof(g_configRead.remotePort));
    memcpy(g_configRead.remotePort,"7106",4);
    g_configRead.PortLen=4;  													

		//增加2017-04-19
		g_configRead.NationCode[0] = '8';
		g_configRead.NationCode[1] = '6';
		g_configRead.NationCode[2] = '\0';
		g_configRead.sysName[0] = 0x8B;		//设备名  8BBE5907540D
		g_configRead.sysName[1] = 0xBE;		
		g_configRead.sysName[2] = 0x59;		
		g_configRead.sysName[3] = 0x07;		
		g_configRead.sysName[4] = 0x54;		
		g_configRead.sysName[5] = 0x0D;		
		g_configRead.sysName[6] = 0x00;		
		g_configRead.sysName[7] = 0x00;				
		g_configRead.NameLen = 6;			
		
		g_configRead.b_gprs_work = 1;			//默认工作模式
		g_configRead.b_wifi_work = 0;			//默认不工作
		g_configRead.b_rj45_work = 0;			//默认不工作
		
		g_configRead.reset_time = 30;			//默认48小时重启1次。	
		g_configRead.b_debug_work = 1;		
		g_configRead.AMPER_RTC_PC13 = 0;		
		g_configRead.b_Sms_Test = 30;
		g_configRead.b_Sms_FxiTime = 100;		
 //   Test_CaliInit();//程序测试用
   // memcpy(&g_configRead,&g_configDef,sizeof(ConfigInfo));//将默认配置信息作为当前读取的配置信息
    if(flag==2)         //写
    {		
				WriteFlash_CPU(PARAM_START_ADDR,&g_configRead.b_success,sizeof(ConfigInfo));							
        return 1;
    }  
		return 0;		
  }  
}

int g_tj = 0;
void  WriteConfigParaFromIICAll()
{	
	  unsigned char i,j,temp[256],*p;	 
		int len = sizeof(ConfigInfo);
		p = &g_configRead.b_success;
		//for(i=0;i<10;i++)
	  {				
			
				WriteFlash_CPU(PARAM_START_ADDR,&g_configRead.b_success,len);	
				if(ReadFlash_CPU(PARAM_START_ADDR,&temp[0],len))			
				{
					for(j=0;j<len;j++)
					{
						if(p[j]!=temp[j])
						{
							g_tj++;
							break;
						}
					}
					//if(j==len) 
					//{
					//	break;
					//}
				}				
	  }				
	//	if(i==10)
		{
				//дflashӐΊ̢
		}
}

void WriteFlashWifi(void)
{
	WriteFlash_CPU(WIFI_START_ADDR,&gs_SaveWifiCfg.ucStartFlag,sizeof(gs_SaveWifiCfg));			
}

void ReadFlashWifi(void)
{
	int len = sizeof(gs_SaveWifiCfg),i=0;	
	ReadFlash_CPU(WIFI_START_ADDR,&gs_SaveWifiCfg.ucStartFlag,len);
}
