/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : spi_flash.c
* Author             : MCD Application Team
* Date First Issued  : 02/05/2007
* Description        : This file provides a set of functions needed to manage the
*                      communication between SPI peripheral and SPI M25P64 FLASH.
********************************************************************************
* History:
* 05/21/2007: V0.3
* 04/02/2007: V0.2
* 02/05/2007: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_para.h"
#include "prj.h"
#include "string.h"
#include "systick.h"
#include "GlobalVar.h"
#include "gd32e503v_eval.h"
#include "cfg_flash.h"
#include "wifi_ble.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
//#include "gd32e50x_rcu.h"
#include "gd32e50x_rtc.h"
//#include "time.h"

extern int			 	  m_gsm_wr,m_gsm_rd;	
extern int					m_tel_wr,m_tel_rd;

extern TaskHandle_t 	EtherNet_Task_Handle;			/* w5500任务句柄 */

int  Year,Month,Day,Hour,Minute,Second;
int  SECOND_MONTH_TB[]={0x28DE80,0x24EA00,0x28DE80,0x278D00,0x28DE80,0x278D00,0x28DE80,0x28DE80,0x278D00,0x28DE80,0x278D00,0x28DE80};

void   Wifi_Rj45_Param( char * wifi_rxDat);
extern unsigned char 	S2_Port[2];					//׋�?ք׋ࠚ�?8888) 
extern unsigned char 	UDP_DIPR[4];				//UDP(ڣҥ)ģʽ,Ŀք׷ܺIPַ֘
extern unsigned char 	UDP_DPORT[2];				//UDP(ڣҥ)ģʽ,Ŀք׷ܺ׋ࠚ�?

extern unsigned char 	UDP_DIPR_A[4][4];					//UDP(ڣҥ)ģʽ,Ŀք׷ܺIPַ֘
extern unsigned char 	UDP_DPORT_A[4][2];				//UDP(ڣҥ)ģʽ,Ŀք׷ܺ׋ࠚۿ


void send_data(void);

unsigned char 	g_cSendMode = 0;							//参数配置，参数读取，读温度数据通讯方式�?=usb,2=upd,3=wifi
unsigned char   g_cSendMode_tcp = 0;					//是否tcp模式传输�?1表示tcp模式

unsigned char  	g_wifi_send_buf[1024];		
int						 	g_wifi_send_cnt = 0;
unsigned char  	g_ipport[6];									//wifi udp模式下记录目标ip地址�?
unsigned int  	g_udp_rec_time = 0;							//udp接收数据,�?0min中收不到任何数据则重启wifi

unsigned int  g_tcp_rec_time = 0;							//tcp接收数据,�?0min中收不到任何数据则重启wifi
unsigned int  g_tcp_err_cnt = 0;							//不成功的tcp数据条数

void 	GetTimeFromServer(unsigned char *strtime);
int 	g_vol;														//电压
char 	g_sysTime[14];				

unsigned int rtc_tm_count;					
unsigned int g_Send = 0;						
char         g_chSoftware_ver[]={"260_171223_V1.2.6"};		//15个字节电话报�?

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

void print_wr_rd(void);
void print_time(void);


unsigned char g_cQkValue[2];
unsigned char g_bQKFlag=0;

unsigned char  g_bCharge;
int 						g_wendu[2],g_shidu[2];			//通道1温度 湿度

RepairData	g_confRepair;
int  g_SaveDataInfo_Pos;
int  g_RepairInfo_Pos ;			//-1~447位置信息		当前�?-1表示从来没有写过。即需要格式化; �?1则表示位置信息所在的位置[0,447]�?
int  g_Repair_Wr ;
int	g_Repair_Rd ;
int	g_datacnt ;							//历史数据保存的条�?
int  g_SaveData_Loop;
int  g_Repair_Loop;


unsigned char g_TxBufTemp[79];
unsigned char g_TxBuf[UPDATANUM][78];
char					g_TxBuf_Status[UPDATANUM];		//由于tcp发送数据需要异步等待发送成功，并且待发送的数据事先进入队列。在清空成功的时候必须对队列中数据进行标志。只发送但不写读指针�?正常�?表示清空过数据�?

unsigned int 	g_TxStarttime[UPDATANUM];			//数据进入队列开始时�?

int  g_TxBufRd=0;
int  g_TxBufWr=0;
void PCF_getsystime(void);

void Gprs_Tcp_Log(char *str)
{				
	#if  GPRS_TCP_DEBUG
	int len = 0;
	if(1==g_configRead.b_debug_work)		//if(Sw_Sta[2]&&g_configRead.b_debug_work)
	{
		len = strlen(str);
		Com_Send(RS485,(unsigned char*)str,len);
	}
	#endif
}

char numtohex(int src)
{
	if(src>9)
	{
			return (src-10+'A');
	}
	else
	{
			return (src+0x30);
	}
}

unsigned char CRC82(unsigned char *ptr,unsigned char len)
{
    unsigned char i;
    unsigned char crc=0;
    while(len--!=0)
    {
        for(i=1; i!=0; i*=2)
        {
            if((crc&1)!=0)
            {
                crc/=2;
                crc^=0x8C;
            }
            else
                crc/=2;
            if((*ptr&i)!=0)
                crc^=0x8C;
        }
        ptr++;
    }
    return(crc);
}

int charToHexStr(char* dest,unsigned char src)
{
	int abc;
	abc=(src>>4)&0xf;
	dest[0] = numtohex(abc);	
	abc=src&0xf;
	dest[1] = numtohex(abc);	
	return 2;  
}

//0xffffffff=4294967295
int UIntToStr(char* dest, unsigned int src)
{
		int i=0,j=0,k=0;
		int src1=src;
  
    dest[0]=src1/1000000000%10+'0';
    dest[1]=src1/100000000%10+'0';
    dest[2]=src1/10000000%10+'0';
    dest[3]=src1/1000000%10+'0';
    dest[4]=src1/100000%10+'0';
    dest[5]=src1/10000%10+'0';	
		dest[6]=src1/1000%10+'0';
    dest[7]=src1/100%10+'0';
    dest[8]=src1/10%10+'0';
    dest[9]=src1%10+'0';
	
		
		for(i=0;i<10;i++)
		{
			if(dest[i]!='0')
			{
					break;
			}
		}
		
		if(i==10)
		{
			return 1;
		}
		else
		{
			//ȥ��ǰ���i��0
			j = 10-i;
			for(j=i;j<10;j++)
			{
					dest[k++]=dest[j];
			}
			return 10-i;
		}     
}


int IntToStr(char* dest, int src)
{
  int i=0,j=0,k=0,m=0;
  int src1=src;
  if(src1<0)
  {
    dest[0]='-';
    src1=src1*-1;
    dest[1]=src1/100000%10+'0';
    dest[2]=src1/10000%10+'0';
    dest[3]=src1/1000%10+'0';
    dest[4]=src1/100%10+'0';
    dest[5]=src1/10%10+'0';
    dest[6]=src1%10+'0';
		
		for(i=1;i<7;i++)
		{
			if(dest[i]!='0')
			{
					break;
			}
		}
		//dest[i] 位置为非0�?
		if(i==7)
		{
			dest[0]='0';
			return 1;
		}
		else if(i==1)
		{
				return 7;
		}
		else
		{
			//i的范围为2~6
			m = i - 1;		//合计去掉m个高�?
			j = 7 - i ;  	//i-1�?  即合计需要去掉m�?  剩余数据个数�?6-m-->6-i+1=7-i  不包�?-'			
			for(k=0;k<j;k++)
			{
					dest[k+1]=dest[k+1+m];
			}
			return 8-i;
		}		
    //return 7;
  }
  else
  {
    dest[0]=src1/100000%10+'0';
    dest[1]=src1/10000%10+'0';
    dest[2]=src1/1000%10+'0';
    dest[3]=src1/100%10+'0';
    dest[4]=src1/10%10+'0';
    dest[5]=src1%10+'0';
		
		for(i=0;i<6;i++)
		{
			if(dest[i]!='0')
			{
					break;
			}
		}
		
		if(i==6)
		{
			return 1;
		}
		else
		{
			//ȥ��ǰ���i��0
			j = 6-i;
			for(j=i;j<6;j++)
			{
					dest[k++]=dest[j];
			}
			return 6-i;
		}    
  }
}

unsigned long StrToInt(char* dat,int cnt)
{
  unsigned long res=0;
  if(cnt==5)
    res=(dat[0]-'0')*10000+(dat[1]-'0')*1000+(dat[2]-'0')*100+(dat[3]-'0')*10+(dat[4]-'0')*1;
  else if(cnt==4)
    res=(dat[0]-'0')*1000+(dat[1]-'0')*100+(dat[2]-'0')*10+(dat[3]-'0')*1;
  else if(cnt==3)
    res=(dat[0]-'0')*100+(dat[1]-'0')*10+(dat[2]-'0')*1;
  else if(cnt==2)
    res=(dat[0]-'0')*10+(dat[1]-'0')*1;
  else if(cnt==1)
    res=(dat[0]-'0')*1;
  return res;
}

int IntToHexStr(char* dest,unsigned int *src,int cnt)
{
	int abc,i,j;
	for( i = 0 ; i < cnt ;i++)
	{
			for( j = 7 ; j > -1 ;j--)
			{
					abc=(src[i]>>(j*4))&0xf;
					dest[i*8+7-j] = numtohex(abc);
			}		 
	}
	return (cnt*8);  
}

int chartoip(char*dest,unsigned char *src)
{
		int i,k=0;
	  unsigned char a,b,c;
	  for(i=0;i<4;i++)
		{
				a = src[i]/100+0x30;		
				b = (src[i]/10)%10+0x30;
			  c = src[i]%10+0x30;
			  if(a=='0'&&b=='0')
				{
						dest[k++] = c;
				}
				else if(a=='0')
				{
						dest[k++] = b;
						dest[k++] = c;
				}
				else
				{
						dest[k++] = a;
						dest[k++] = b;
						dest[k++] = c;
				}		
				dest[k++] = '.';				
		}
		return (k-1);
}


void configinfo_package2(char* cmd,int len)
{ 
  char ch[64];
  char chT[10];
  int  intTmp=0;
  int banary=0;
  int i=0;
  unsigned long int errorNum=0,Rd_error=0,Wr_error=0;  
  unsigned long int datCnt=0;
  if('1'==cmd[11])
  {
			putStr("\n",1);
//    putStr("SN=",3);			
//		putStr(g_configRead.device_ID,6);
		
		putStr(",CPUID=",7);			
		intTmp = IntToHexStr(ch,cpu_id,3);			
		putStr(ch,intTmp);
   
		
		putStr(",SFVER=",7)	;putStr(g_chSoftware_ver,strlen(g_chSoftware_ver));
			
				
		putStr(",MAC=",5);	
		for(i=0;i<6;i++)
		{
			charToHexStr(ch,gs_SaveNetIPCfg.ucMAC[i]);
			ch[2] = ':';
			if(i==5)
			{
				putStr(ch,2);
			}
			else
			{
				putStr(ch,3);
			}			
		}		
		wdt();
		putStr(",SELFIP=",8);	
		intTmp = chartoip(ch,gs_SaveNetIPCfg.ucSelfIP);			
		putStr(ch,intTmp);
		
		putStr(",SUBMASK=",9);
		intTmp = chartoip(ch,gs_SaveNetIPCfg.ucSubMASK);			
		putStr(ch,intTmp);
		
		putStr(",GATEWAY=",9);
		intTmp = chartoip(ch,gs_SaveNetIPCfg.ucGateWay);			
		putStr(ch,intTmp);       	   
		//send_data();
		putStr(",REMOTIP=",9);   
    intTmp = chartoip(ch,gs_SaveNetIPCfg.ucDestIP);			
		putStr(ch,intTmp);   
		
    putStr(",REMOTPORT=",11);  
    intTmp = (gs_SaveNetIPCfg.ucDestPort[0]<<8)+gs_SaveNetIPCfg.ucDestPort[1];	
		
    intTmp = IntToStr(ch,intTmp);			
		putStr(ch,intTmp);   
		
	  putStr(",LOCALPORT=",11);  
		intTmp = (gs_SaveNetIPCfg.ucSourcePort[0]<<8)+gs_SaveNetIPCfg.ucSourcePort[1];	
    intTmp = IntToStr(ch,intTmp);			
		putStr(ch,intTmp);   
				
	  putStr(",MONITOR=",9);  
    intTmp = (gs_SaveNetIPCfg.ucMonitorPort[0]<<8)+gs_SaveNetIPCfg.ucMonitorPort[1];			
		intTmp = IntToStr(ch,intTmp);			
		putStr(ch,intTmp);   	
		
		PCF_getsystime();												//获取时间�?    
		putStr(",RTIME=",7);	 		
		intTmp = UIntToStr(ch,rtc_tm_count);			
		putStr(ch,intTmp);		
    putStr(",\r",2);    
		send_data();
  }  
}

void unixtime2time(unsigned int time)
{
	int ln1=0,ln2=0;
	int i=0,a1=0,ii;
	
	i=time/0x7861F80;		//126230400		126230400/(3600*24)	= 1461 = 366+365+365+365=1461
	i*=4;
	ln1=time%0x7861F80;		//12350910

	if(ln1>=0x1E28500)				//0x1E28500  31622400/(3600*24) = 366
	{
		i+=1;						//�?1
		a1=1;						//非闰�?				
		ln1-=0x1E28500;				//0x1E28500  31622400/(3600*24) = 366
		if(ln1>=0x1E13380)			//365�?
		{
			i+=1;
			ln1-=0x1E13380;			//365�?
			if(ln1>=0x1E13380)
			{
				i+=1;
				ln1-=0x1E13380;
			}
		}
	}

	Year=2000;
	Year+=i;
	Month=1;	
	for(i=0;i<12;i++)
	{
		if((i==1)&&(a1==0))			//a1=0表示当年为润�?
		{
			ln2=0x263B80;			//2505600  =29�?
		}
		else
		{
			ln2=SECOND_MONTH_TB[i];
		}
		if(ln1>=ln2)	
		{
			Month+=1;
			ln1-=ln2;
		}
		else
		{
			i=12;
		}
	}
	Day=1;
	ln1/=0x00015180;		//ln1剩余的秒�?day 的计�?
	Day+=ln1;

	ln1 = time%0x00015180;
  Hour=ln1/3600;
  ii=ln1%3600;
  Minute=ii/60;
  Second=ii%60;
	
	//int  Year,Month,Day,Hour,Minute,Second;
		g_sysTime[0] = (Year/1000)+'0';
	  g_sysTime[1] = ((Year/100)%10)+'0';
	  g_sysTime[2] = ((Year/10)%10)+'0';
	  g_sysTime[3] = (Year%10)+'0';
        
		g_sysTime[4] = ((Month/10)%10)+'0';
	  g_sysTime[5] = (Month%10)+'0';
	
		g_sysTime[6] = ((Day/10)%10)+'0';
	  g_sysTime[7] = (Day%10)+'0';
	
		g_sysTime[8] = ((Hour/10)%10)+'0';
	  g_sysTime[9] = (Hour%10)+'0';
	
		g_sysTime[10] = ((Minute/10)%10)+'0';
	  g_sysTime[11] = (Minute%10)+'0';
	
		g_sysTime[12] = ((Second/10)%10)+'0';
	  g_sysTime[13] = (Second%10)+'0';	  
	}


							
int gettellen(char * p)
{
	int i = 0;
	for(i=0;i<11;i++)
	{
		if(p[i]==0)
		{
			return  i;
		}
	}
	return i;	
}

int gettellen2(char * p)
{
	int i = 0;
	for(i=0;i<12;i++)
	{
		if(p[i]==0)
		{
			return  i;
		}
	}
	return i;	
}

/***************************************
* 配置信息发送，主机通过指令获取配置信息
* 参数：cmd指令�?获取一般参数包括采集间隔，保存间隔，温湿度上下限。。。。。�?
                 2获取历史数据
                 3获取温度校准曲线的参�?
                 4读取SHT21以及DS18B20温湿度以及DS18B20的序列号
***************************************/
void configinfo_package(char* cmd,int len)
{ 
	int  data=0;
  int  cnt = 0, i = 0;
  char ch[16];
  char chT[10];
  int  intTmp=0;
  int banary=0;
  int bxz=0;
  unsigned  long int errorNum=0,Rd_error=0,Wr_error=0;  
  unsigned long int datCnt=0;
  if('1'==cmd[11])
  {
		wdt();
		if(g_cSendMode_tcp==1)		//tcp模式读参�?
		{
				putStr("*PA,",4);   
		}
		else
		{
				putStr("\n,",2);
		}
    
    //设备�?
		putStr("SFVER=",6)	;putStr(g_chSoftware_ver,strlen(g_chSoftware_ver));			//增加软件版本号查�?
		
    putStr(",SN=",4);		putStr(g_configRead.device_ID,6);
		
		putStr(",CPUID=",7);		
		intTmp = IntToHexStr(ch,cpu_id,3);			
		putStr(ch,intTmp);
		
#if SMTDOG_V34
			putStr(",HWVER=SMTDOG260 V3.4",21);		
#else		
		#if SMTDOG_V33
			putStr(",HWVER=SMTDOG260 V3.3",21);	
		#else 
			putStr(",HWVER=SMTDOG260 V3.1",21);		
			//putStr(",HWVER=SMTDOG270 V3.1",21);		
		#endif		
#endif		
		
		
    //采集间隔
    intTmp=IntToStr(ch,g_configRead.collect_frq);
    putStr(",CTIME=",7);putStr(ch,intTmp);
    //保存间隔
    intTmp=IntToStr(ch,g_configRead.save_frq);
    putStr(",STIME=",7);putStr(ch,intTmp);
    //发送间�?
    intTmp=IntToStr(ch,g_configRead.send_frq);
    putStr(",GTIME=",7);putStr(ch,intTmp);
		
		intTmp=IntToStr(ch,g_configRead.Depart[1]);
    putStr(",TELT=",6);putStr(ch,intTmp);
		
    //通道1温度上限		
    intTmp=IntToStr(ch,g_configRead.TMax[0]);
		#if CO2
		if(g_configRead.bCH[0]==3)
		{
			data = (unsigned short int)g_configRead.TMax[0];
			intTmp=IntToStr(ch,data);
			putStr(",TMAX1=",7);putStr(ch,intTmp);
		}
		else
		{
			intTmp=strIntTostrFloat(chT,ch,intTmp);
			putStr(",TMAX1=",7);putStr(chT,intTmp);
		}
		#else
    intTmp=strIntTostrFloat(chT,ch,intTmp);
		putStr(",TMAX1=",7);putStr(chT,intTmp);
		#endif
    
   
    //通道1湿度上限
    intTmp=IntToStr(ch,g_configRead.HMax[0]);
    intTmp=strIntTostrFloat(chT,ch,intTmp);
    putStr(",HMAX1=",7);putStr(chT,intTmp);
    //通道1温度下限
    intTmp=IntToStr(ch,g_configRead.TMin[0]);
		#if CO2
		if(g_configRead.bCH[0]==3)
		{
			 data = (unsigned short int)g_configRead.TMin[0];
			 intTmp=IntToStr(ch,data);
			 putStr(",TMIN1=",7);putStr(ch,intTmp);
		}
		else
		{
			 intTmp=strIntTostrFloat(chT,ch,intTmp);
			 putStr(",TMIN1=",7);putStr(chT,intTmp);
		}
		#else
		intTmp=strIntTostrFloat(chT,ch,intTmp);
		putStr(",TMIN1=",7);putStr(chT,intTmp);
		#endif
   
    //通道1湿度下限
    intTmp=IntToStr(ch,g_configRead.HMin[0]);
    intTmp=strIntTostrFloat(chT,ch,intTmp);
    putStr(",HMIN1=",7);putStr(chT,intTmp);
    //通道2温度上限
    intTmp=IntToStr(ch,g_configRead.TMax[1]);
		#if CO2
		if(g_configRead.bCH[1]==3)
		{
			 data = (unsigned short int)g_configRead.TMax[1];
			 intTmp=IntToStr(ch,data);
			 putStr(",TMAX2=",7);putStr(ch,intTmp);
		}
		else
		{
			intTmp=strIntTostrFloat(chT,ch,intTmp);
			putStr(",TMAX2=",7);putStr(chT,intTmp);
		}
		#else
		intTmp=strIntTostrFloat(chT,ch,intTmp);
		putStr(",TMAX2=",7);putStr(chT,intTmp);
		#endif
		
    //通道2湿度上限
    intTmp=IntToStr(ch,g_configRead.HMax[1]);		
    intTmp=strIntTostrFloat(chT,ch,intTmp);	
    putStr(",HMAX2=",7);putStr(chT,intTmp);
    //通道2温度下限
    intTmp=IntToStr(ch,g_configRead.TMin[1]);
		#if CO2
		if(g_configRead.bCH[1]==3)
		{
			 data = (unsigned short int)g_configRead.TMin[1];
			 intTmp=IntToStr(ch,data);
			 putStr(",TMIN2=",7);putStr(ch,intTmp);
		}
		else
		{
			intTmp=strIntTostrFloat(chT,ch,intTmp);
			putStr(",TMIN2=",7);putStr(chT,intTmp);
		}
		#else
    intTmp=strIntTostrFloat(chT,ch,intTmp);
    putStr(",TMIN2=",7);putStr(chT,intTmp);
		#endif
    //通道2湿度下限
    intTmp=IntToStr(ch,g_configRead.HMin[1]);
    intTmp=strIntTostrFloat(chT,ch,intTmp);
    putStr(",HMIN2=",7);putStr(chT,intTmp);
		if(g_cSendMode_tcp==0)									//udp模式读参�?
		{
				//send_data();
		}
		
		putStr(",GPRSWORK=",10);    
    if(g_configRead.b_gprs_work==1)
		{
			putStr("1",1);
		}
    else 
		{
      putStr("0",1);    
		}
		
		putStr(",WIFIWORK=",10);    
    if(g_configRead.b_wifi_work==1)
		{
			putStr("1",1);
		}
    else 
		{
      putStr("0",1);   
		}
		
		putStr(",LIGHTWORK=",11);    
    if(g_configRead.b_light_work==0)
		{
			putStr("0",1);
		}
    else 
		{
      putStr("1",1);   
		}
		
		putStr(",RJ45WORK=",10);  
			ch[0] = g_configRead.b_rj45_work+0x30;
      putStr(ch,1);		
//    if(g_configRead.b_rj45_work==1)
//		{
//			putStr("1",1);
//		}
//    else 
//		{
//      putStr("0",1);   
//		}
		
    //通道1工作状�?
    putStr(",CH1WORK=",9);    
    if(g_configRead.bCH[0]<8)
		{
			ch[0] = g_configRead.bCH[0]+0x30;
      putStr(ch,1);
		}
    else 
		{
      putStr("9",1);    //表示错误  
		}
    wdt();   
    //通道2工作状�?
    putStr(",CH2WORK=",9);
		if(g_configRead.bCH[1]<8)
		{
		  ch[0] = g_configRead.bCH[1]+0x30;
      putStr(ch,1);		 
		}
    else 
		{
      putStr("9",1);    //表示错误  
		}
			wdt();  
    //电压下限
    intTmp=IntToStr(ch,g_configRead.VMin);		
  //  intTmp=strIntTostrFloat(chT,ch,intTmp);		
  //  putStr(",VMIN=",6);putStr(chT,intTmp);
		//if(intTmp>1)		//如果大于1位数�?
		//{
				putStr(",VMIN=",6);	putStr(ch,intTmp);
		//}
		//else if(g_configRead.VMin<10)
		//{
		//		putStr(",VMIN=",6);	putStr("0",1);
		//}	
    #if 0
		if(g_configRead.Depart[0]=='1')
		{
					putStr(",RECE=",6);	putStr("1",1);		//RECE
		}
		else
		{
					putStr(",RECE=",6);	putStr("0",1);		//RECE
		}
		#else
		intTmp=IntToStr(ch,g_configRead.Depart[0]);
    putStr(",RECE=",6);putStr(ch,intTmp);
		#endif
		//报警号码
    putStr(",M1=",4);
		
    if(g_configRead.alarmNum[0][0]=='*')
		{
			cnt = gettellen(g_configRead.alarmNum[0]+1);
			putStr(g_configRead.alarmNum[0]+1,cnt); 
		}
    else{putStr("0",1);};
    putStr(",M2=",4);
    if(g_configRead.alarmNum[1][0]=='*'){cnt = gettellen(g_configRead.alarmNum[0]+1); putStr(g_configRead.alarmNum[1]+1,cnt); }
    else{putStr("0",1);};
    putStr(",M3=",4);
    if(g_configRead.alarmNum[2][0]=='*'){cnt = gettellen(g_configRead.alarmNum[0]+1);putStr(g_configRead.alarmNum[2]+1,cnt); }
    else{putStr("0",1);};
		
		putStr(",MT1=",5);
		
    if(g_configRead.alarmTel[0][0]=='*')	{		cnt = gettellen2(g_configRead.alarmTel[0]+1);			putStr(g_configRead.alarmTel[0]+1,cnt); 
		}
    else{putStr("0",1);};
    putStr(",MT2=",5);
    if(g_configRead.alarmTel[1][0]=='*'){cnt = gettellen2(g_configRead.alarmTel[1]+1); putStr(g_configRead.alarmTel[1]+1,cnt); }
    else{putStr("0",1);};
    putStr(",MT3=",5);
    if(g_configRead.alarmTel[2][0]=='*'){cnt = gettellen2(g_configRead.alarmTel[2]+1);putStr(g_configRead.alarmTel[2]+1,cnt); }
    else{putStr("0",1);};
		
    //服务器信�?
    putStr(",REMOTEIP=",10);
    if(g_configRead.IPLen>0)
      putStr(g_configRead.remoteIP,g_configRead.IPLen);
    putStr(",REMOTEPORT=",12);
    if(g_configRead.PortLen>0)
      putStr(g_configRead.remotePort,g_configRead.PortLen);
    //温湿度修�?
    intTmp=IntToStr(ch,g_configRead.TXZ[0]);
    intTmp=strIntTostrFloat(chT,ch,intTmp);
    putStr(",TXZ=",5);putStr(chT,intTmp);
    intTmp=IntToStr(ch,g_configRead.HXZ[0]);
    intTmp=strIntTostrFloat(chT,ch,intTmp);
    putStr(",HXZ=",5);putStr(chT,intTmp);
		if(g_cSendMode_tcp==0)									//udp模式读参�?
		{
				//send_data();
		}
		
    //温湿度修�?ch2
    intTmp=IntToStr(ch,g_configRead.TXZ[1]);
    intTmp=strIntTostrFloat(chT,ch,intTmp);
    putStr(",T2Z=",5);putStr(chT,intTmp);
    intTmp=IntToStr(ch,g_configRead.HXZ[1]);
    intTmp=strIntTostrFloat(chT,ch,intTmp);
    putStr(",H2Z=",5);putStr(chT,intTmp);
    
		//m_siminfo.iccid
		if(m_siminfo.iccid[0]!='\0')
		{
				cnt  = 0;
				for(i=0;i<20;i++)
				{
						if(m_siminfo.iccid[i]==0)
						{
								break;
						}
						cnt++;
				}
				putStr(",ICCID=",7);
				putStr(m_siminfo.iccid,cnt);
		}		
    //单位编号
//    putStr(",DANWEI=",8);putStr(g_configRead.Danwei,8);
//	  putStr(",BUMEN=",7);putStr(g_configRead.Depart,2);
    //报警延时
    intTmp=IntToStr(ch,g_configRead.alarmDelyMinute);
    putStr(",ALARMT=",8);putStr(ch,intTmp);
    
    //BEEP状�?
    intTmp=IntToStr(ch,g_configRead.beep);
    putStr(",BEEP=",6);putStr(ch,intTmp);

		//是否debug
		intTmp=IntToStr(ch,g_configRead.b_debug_work);
    putStr(",DEBUG=",7);putStr(ch,intTmp);
		wdt();
		
		//reset time
		intTmp=IntToStr(ch,g_configRead.reset_time);
    putStr(",RESETTIME=",11);putStr(ch,intTmp);
		wdt();
		
		intTmp=IntToStr(ch,g_configRead.AMPER_RTC_PC13);
    putStr(",AMPERRTC=",10);putStr(ch,intTmp);
		wdt();	
		
		
    //wifi状�?
    intTmp=IntToStr(ch,g_configRead.wifi_mode);
    putStr(",WIFIMODE=",10);putStr(ch,intTmp);
		wdt();
 
    //历史数据的条�?
    //readheadhisinfo();
    //if(g_datacnt>MAXDATACNT)
    //{
    //  g_datacnt = MAXDATACNT;
    //}    
		if(g_SaveData_Loop)												//已经超过一轮�?
		{
				datCnt = g_datacnt%256;								//256个记录为1扇区  0=没有格式�?56个数�?1=表示1个数�?255=表示255个数据�?
				if(datCnt==0)													//0=没有格式�?56个数�?
				{
						datCnt = g_confRepair.SaveDatCnt;	//
				}
				else
				{
						datCnt = g_confRepair.SaveDatCnt - 256 + datCnt;
				}				
		}
		else
		{
				datCnt = g_datacnt;
		}
    intTmp=IntToStr(ch,datCnt);
    putStr(",HISDAT=",8);putStr(ch,intTmp);
		
    intTmp=IntToStr(ch,g_confRepair.SaveDatCnt);
    putStr(",HISDATMAX=",11);putStr(ch,intTmp);
		
    //补充数据信息，包括待补传数据的条数和读写指针位置
    datCnt = g_Repair_Loop*g_confRepair.RepairDatCnt+g_Repair_Wr;			//合计写入过的补传数据条数
    intTmp=IntToStr(ch,datCnt);
    putStr(",RPRDATCNT=",11);putStr(ch,intTmp);
		
    intTmp=IntToStr(ch,g_Repair_Rd);
    putStr(",RDEOR=",7);putStr(ch,intTmp);
		
    intTmp=IntToStr(ch,g_Repair_Wr);
    putStr(",WREOR=",7);putStr(ch,intTmp);
		
    intTmp=IntToStr(ch,g_confRepair.RepairDatCnt);
    putStr(",RPRDATMAX=",11);putStr(ch,intTmp);
		
    //时间
		PCF_getsystime();																			//获取时间�?
    putStr(",RTIME=",7);
		putStr(g_sysTime,14);
		//增加  g_configRead.sysName  g_configRead.NameLen   增加 g_configRead.NationCode
		putStr(",NATIONCODE=",12);
		putStr(g_configRead.NationCode,strlen(g_configRead.NationCode));
		//增加两个参数  2019-05-16		
		intTmp=IntToStr(ch,g_configRead.b_Sms_Test);
		putStr(",SMST=",6);    putStr(ch,intTmp);
    		
		intTmp=IntToStr(ch,g_configRead.b_Sms_FxiTime);
    putStr(",SMSTIME=",9);	putStr(ch,intTmp);
		
		putStr(",DEVICENAME=",12);
		putStr(g_configRead.sysName,g_configRead.NameLen);
		
		if(g_cSendMode_tcp==1)	//tcp模式读参�?
		{
				putStr(",#",2);   
		}
		else
		{
				putStr(",\r",2);
		}        
		send_data();
  }  
  else if('3'==cmd[11])
  {
		cmd[11]='1';
		wdt();
		configinfo_package2(cmd,len);
  }
  else if('4'==cmd[11])
  {
  }
}


int strIntTostrFloat(char* dest, char* src, int srcLen)
{
  int i;
  if(srcLen<=0)
  {
    dest[0]='0';
    dest[1]='.';
    dest[2]='0';
    return 0;
  }
  else if(srcLen==1)
  {
    dest[0]='0';
    dest[1]='.';
    dest[2]=src[0];
    return 3;
  } 
	else if(srcLen==2 && src[0]=='-')
  {
		dest[0] = src[0];
    dest[1]='0';
    dest[2]='.';
    dest[3]=src[1];
    return 4;
  } 
  else
  {
		for(i=0;i<srcLen-1;i++)
    {
      dest[i]=src[i];
    }
    dest[srcLen-1]='.';
    dest[srcLen]=src[srcLen-1];
    return srcLen+1;
  }
	return 0;
}

unsigned char strcmp_(char* dat, char* dat1,unsigned char len)
{
  unsigned char i;
  for(i=0;i<len;i++)
  {
    if(dat[i]!=dat1[i])
      return 1;
  }
  return 0;
}

/**********************************************
*说明：将浮点型的字符串转换为整形  只保留小数点�? �?结果：原始数�?10 
*参数�?dat:只想浮点型字符的指针  len:字符长度
*res:转换后的结果   
返回�? 转换失败  1 转换成功
**********************************************/
void  FloatstrToUint(char* str,int len,short int* res)
{
	unsigned char i,Dot=0,k=0;
	unsigned char ch1[5],ch2[5];
	unsigned char chLen1=0,chLen2=0;
	unsigned char fla=0;//开始是否有'-'
	short tmp=0;
	*res=0;
	if(len==0)        
	{
		*res=0;
		 return;
	}
	if(str[0]=='.')//开始不能是小数�?
  {
    *res=0;
		return;
  }
	for(i=1;i<len;i++)//在字符串之间不可以出�?-'
	{
		if(str[i]=='-')
		{ 
      *res=0;
			return;
		}
		else if(str[i]=='.')
		{
			k++;
			if(k>1)
                        {
			  *res=0;
                          return;//数据中间只能有一个小数点   多有1个数据无�?
                        }
		}
	}	
	if(str[0]=='-' || str[0]=='+')//?
	{
	//	*res |= 0x8000;//最高位�?
		fla=1;
	}
	else
	{
	//	*res &= 0x7FFF;//最高位�?
		fla=0;
	}
	for(i=fla;i<len;i++)
	{
		
		if(Dot!=1)
		{
			if(str[i]!='.')//未找�?.'
			{
				
				ch1[chLen1]=str[i];
				chLen1++;
			}
			else
				Dot=1;
		}
		else if(Dot==1)
		{
			ch2[chLen2]=str[i];
			chLen2++;
                        break;//?????????
		}
	}
//	printf("\rlen1:%d,len2:%d\r",chLen1,chLen2);
	//组合为整形小数点后一�?
	if(chLen1==1)
	{
		tmp+=(ch1[0]-'0')*10;
	}
	else if(chLen1==2)
	{
		tmp+=(ch1[0]-'0')*100+(ch1[1]-'0')*10;
	}
	else if(chLen1==3)
	{
		tmp+=(ch1[0]-'0')*1000+(ch1[1]-'0')*100+(ch1[2]-'0')*10;
	}
	else if(chLen1==4)
	{
		tmp+=(ch1[0]-'0')*10000+(ch1[1]-'0')*1000+(ch1[2]-'0')*100+(ch1[3]-'0')*10;
	}
	else if(chLen1==5)
	{
		tmp+=(ch1[0]-'0')*100000+(ch1[1]-'0')*10000+(ch1[2]-'0')*1000+(ch1[3]-'0')*100+(ch1[4]-'0')*10;
	}
	if(chLen2>0)
	{
		tmp+=(ch2[0]-'0')*1;
	}
//	printf("\r%d\r",*res);
	if(str[0]=='-')//??
		tmp=tmp*-1;
	*res=tmp;
	return;
}

void strtoip(unsigned char *ip, char*src,int len)
{
		int i = 0 , j = 0 , k = 0;
	  unsigned char temp[4][5];
	  unsigned char cnt[4];	
		for(i=0;i<len;i++)
	  {
					if(src[i]=='.')
					{
							cnt[j] = k;
							j++;
						  k=0;							
					}
					else
					{
							temp[j][k] = src[i];							
							k++;
					}
		}
		cnt[j] = k;
		j++;
		k=0;	
		
		for(i=0;i<j;i++)
		{
				if(cnt[i]==1)
				{
						ip[i] = temp[i][0]-0x30;
				}
				else if(cnt[i]==2)
				{
						ip[i] = (temp[i][0]-0x30)*10+(temp[i][1]-0x30);
				}
				else if(cnt[i]==3)
				{
						ip[i] = (temp[i][0]-0x30)*100+(temp[i][1]-0x30)*10+(temp[i][2]-0x30);
				}
				else
				{
						ip[i] = 0;
				}
		}
}

unsigned char GetHex(unsigned char high,unsigned char low)
{
		unsigned char tt  = 0 , ty = 0;
		if(high<='9')
		{
			tt = (high-0x30)*16;
		}
		else if(high>='a'&& high < 'g')
		{
			tt = (high-'a'+10)*16;
		}
		else if(high>='A'&& high < 'G')
		{
			tt = (high-'A'+10)*16;
		}
		
		if(low<='9')
		{
			ty = (low-0x30);
		}
		else if(low>='a'&& low < 'g')
		{
			ty = (low-'a'+10);
		}
		else if(low>='A'&& low < 'G')
		{
			ty = (low-'A'+10);
		}
		tt = tt+ty;
		return tt;
}

void strtomac(unsigned char *ip, char*src,int len)
{
		int i = 0 , j = 0 , k = 0;
	  unsigned char temp[7][5];
	  unsigned char cnt[7];	
		for(i=0;i<len;i++)
	  {
					if(src[i]==':')
					{
							cnt[j] = k;
							j++;
						  k=0;							
					}
					else
					{
							temp[j][k] = src[i];							
							k++;
					}
		}
		cnt[j] = k;
		j++;
		k=0;	
		for(i=0;i<j;i++)
		{
				if(cnt[i]==1)
				{
						//ip[i] = temp[i][0]-0x30;
				}
				else if(cnt[i]==2)
				{
						ip[i] = GetHex(temp[i][0],temp[i][1]);
				}
				else if(cnt[i]==3)
				{
						//ip[i] = (temp[i][0]-0x30)*100+(temp[i][1]-0x30)*10+(temp[i][2]-0x30);
				}
				else
				{
						ip[i] = 0;
				}
		}
}


//设置PCF_8563的时�?
//unsigned int g_bkp1=0;
void PCF_setsystime(char* time,int len,int utc_sec)
{        
	unsigned int unix_time = 0;	
	int  SECOND_MONTH_TB_RUN[]={0x28DE80,0x263B80,0x28DE80,0x278D00,0x28DE80,0x278D00,0x28DE80,0x28DE80,0x278D00,0x28DE80,0x278D00,0x28DE80};
  int year  = (time[0]-0x30)*1000+(time[1]-0x30)*100+(time[2]-0x30)*10+(time[3]-0x30);
	int month = (time[4]-0x30)*10+(time[5]-0x30);
	int day   = (time[6]-0x30)*10+(time[7]-0x30);

	int hour = (time[8]-0x30)*10+(time[9]-0x30);
	int min  = (time[10]-0x30)*10+(time[11]-0x30);
	int sec  = (time[12]-0x30)*10+(time[13]-0x30);
	int i = 0, j = 0 , k =0;
	
	 if(len<14)
     return;
	 
	//将年转化�?
	//能被4整除但不能被100整除,或者可以被400整除的叫闰年�?
	if(year<2000)
	{
		unix_time = 0;
	}
	else
	{
		year = year - 2000;					//2000-01-01 00:00:00
		if(year>-1 && year<136)			//0xffffffff 秒  大概  136.2年
		{
			i = year/4;		//i=0表示 
			j = year%4;
			unix_time = i * 0x7861F80;		//0x7861F80 合计365+365+365+366
			if(j==0)		//2000 2004 
			{
				for(k=0;k<(month-1);k++)	
				{
					unix_time += SECOND_MONTH_TB_RUN[k];
				}
			}
			else 	//2001 2005
			{
				if(j==1)
				{  
					unix_time = unix_time + 0x1E28500;  	
				}
				else if(j==2)
				{
					unix_time = unix_time + 0x3C3B880;  	
				}
				else if(j==3)
				{
					unix_time = unix_time + 0x5A4EC00;  	
				}				
				for(k=0;k<(month-1);k++)	
				{
					unix_time += SECOND_MONTH_TB[k];
				}		
			}	
			//day  7  12:54:26
			unix_time = unix_time + (day - 1) * 86400 + hour*3600 + min*60+sec;			
		}
		else
		{
			unix_time = 0;	
		}
	}  
		unix_time += utc_sec;
	  rtc_tm_count=unix_time;
		wdt();
		rtc_counter_set(rtc_tm_count);		//	RTC_Configuration(rtc_tm_count);			/* RTC Configuration */
		wdt();   
    	
 //   RTC_SetCounter(rtc_tm_count);
    //BKP_WriteBackupRegister(BKP_DR1, 0x5050);  
		//Write_BKP(1, 0x5050);  
	//	g_bkp1 = BKP_ReadBackupRegister(BKP_DR1);
  //  RTC_WaitForSynchro(); /* Wait for RTC registers synchronization */		
}

unsigned int time2unixtime( char* time,int len)
{
	unsigned int unix_time = 0;	
	int  SECOND_MONTH_TB_RUN[]={0x28DE80,0x263B80,0x28DE80,0x278D00,0x28DE80,0x278D00,0x28DE80,0x28DE80,0x278D00,0x28DE80,0x278D00,0x28DE80};
  int year  = (time[0]-0x30)*1000+(time[1]-0x30)*100+(time[2]-0x30)*10+(time[3]-0x30);
	int month = (time[4]-0x30)*10+(time[5]-0x30);
	int day   = (time[6]-0x30)*10+(time[7]-0x30);

	int hour = (time[8]-0x30)*10+(time[9]-0x30);
	int min  = (time[10]-0x30)*10+(time[11]-0x30);
	int sec  = (time[12]-0x30)*10+(time[13]-0x30);
	int i = 0, j = 0 , k =0;
	
	// if(len<14)
  //   return;
	 
	//将年转化�?
	//能被4整除但不能被100整除,或者可以被400整除的叫闰年�?
	if(year<2000)
	{
		unix_time = 0;
	}
	else
	{
		year = year - 2000;		//2000-01-01 00:00:00
		if(year>-1&&year<100)
		{
			i = year/4;		//i=0表示 
			j = year%4;
			unix_time = i * 0x7861F80;		//0x7861F80 合计365+365+365+366
			if(j==0)		//2000 2004 
			{
				for(k=0;k<(month-1);k++)	
				{
					unix_time += SECOND_MONTH_TB_RUN[k];
				}
			}
			else 	//2001 2005
			{
				if(j==1)
				{  
					unix_time = unix_time + 0x1E28500;  	
				}
				else if(j==2)
				{
					unix_time = unix_time + 0x3C3B880;  	
				}
				else if(j==3)
				{
					unix_time = unix_time + 0x5A4EC00;  	
				}				
				for(k=0;k<(month-1);k++)	
				{
					unix_time += SECOND_MONTH_TB[k];
				}		
			}	
			//day  7  12:54:26
			unix_time = unix_time + (day - 1) * 86400 + hour*3600 + min*60+sec;			
		}
		else
		{
			unix_time = 0;	
		}
	}
	return unix_time;
}


void rtc_second_cal_active_date(void)	
{
	int ln1=0,ln2=0;
	int i=0,a1=0,ii;
	rtc_tm_count = rtc_counter_get();
	i=rtc_tm_count/0x7861F80;		//126230400		126230400/(3600*24)	= 1461 = 366+365+365+365=1461
	i*=4;
	ln1=rtc_tm_count%0x7861F80;		//12350910

	if(ln1>=0x1E28500)				//0x1E28500  31622400/(3600*24) = 366
	{
		i+=1;						//�?1
		a1=1;						//非闰�?				
		ln1-=0x1E28500;				//0x1E28500  31622400/(3600*24) = 366
		if(ln1>=0x1E13380)			//365�?
		{
			i+=1;
			ln1-=0x1E13380;			//365�?
			if(ln1>=0x1E13380)
			{
				i+=1;
				ln1-=0x1E13380;
			}
		}
	}

	Year=2000;
	Year+=i;
	Month=1;	
	for(i=0;i<12;i++)
	{
		if((i==1)&&(a1==0))			//a1=0表示当年为润�?
		{
			ln2=0x263B80;			//2505600  =29�?
		}
		else
		{
			ln2=SECOND_MONTH_TB[i];
		}
		if(ln1>=ln2)	
		{
			Month+=1;
			ln1-=ln2;
		}
		else
		{
			i=12;
		}
	}
	Day=1;
	ln1/=0x00015180;		//ln1剩余的秒�?day 的计�?
	Day+=ln1;

	ln1=rtc_tm_count%0x00015180;
  Hour=ln1/3600;
  ii=ln1%3600;
  Minute=ii/60;
  Second=ii%60;
}

void PCF_getsystime(void)
{
	  rtc_second_cal_active_date();
	//int  Year,Month,Day,Hour,Minute,Second;
		g_sysTime[0] = (Year/1000)+'0';
	  g_sysTime[1] = ((Year/100)%10)+'0';
	  g_sysTime[2] = ((Year/10)%10)+'0';
	  g_sysTime[3] = (Year%10)+'0';
        
		g_sysTime[4] = ((Month/10)%10)+'0';
	  g_sysTime[5] = (Month%10)+'0';
	
		g_sysTime[6] = ((Day/10)%10)+'0';
	  g_sysTime[7] = (Day%10)+'0';
	
		g_sysTime[8] = ((Hour/10)%10)+'0';
	  g_sysTime[9] = (Hour%10)+'0';
	
		g_sysTime[10] = ((Minute/10)%10)+'0';
	  g_sysTime[11] = (Minute%10)+'0';
	
		g_sysTime[12] = ((Second/10)%10)+'0';
	  g_sysTime[13] = (Second%10)+'0';	  
}

//sttime 时间字符�? 15-03-08 23:33:45,
void GetTimeFromServer(unsigned char *strtime)
{
		//g_TxBufTemp[]
		char  	time[15];
		unsigned int 	servertime =  0 , tem = 0;
		unsigned int 	localtime = rtc_tm_count;		
		//if(strtime[2]=='-'&&strtime[5]=='-'&&strtime[8]==' '&&strtime[11]==':'&&strtime[14]==':')
		if(strtime[2]=='-'&&strtime[5]=='-'&&strtime[11]==':'&&strtime[14]==':')
		{
				time[0]='2';
				time[1]='0';
				time[2]=strtime[0];
				time[3]=strtime[1];
			
				time[4]=strtime[3];
				time[5]=strtime[4];
			
				time[6]=strtime[6];
				time[7]=strtime[7];
				
				time[8]=strtime[9];
				time[9]=strtime[10];
				
				time[10]=strtime[12];
				time[11]=strtime[13];
			
				time[12]=strtime[15];
				time[13]=strtime[16];
				time[14]='\0';
		
			servertime =  time2unixtime(time,14);
			if(servertime > localtime)
			{
				tem = servertime-localtime;
			}
			else
			{
				tem = localtime-servertime;
			}
			if(tem>119)				//当时间相�?min则进行时间校�?5*60=300�?
			{
			  Gprs_Tcp_Log("RTC Time  from server:");
				Gprs_Tcp_Log(time);
			  Gprs_Tcp_Log("\r\n");
				rtc_tm_count = servertime;
			//	RTC_Configuration(rtc_tm_count);	/* RTC Configuration */		
			}
		}
}

int g_iNet_flag = 0;
int g_iCfg_flag = 0;

void ConfigDataDeal2(char* dat,int len)
{
	unsigned long int head[2];
	int i = 0, cnt = 0, data=0;
	wdt();
	if(strcmp_(dat,"MAC",3)==0)
  {
			strtomac(gs_SaveNetIPCfg.ucMAC,dat+4,len-4);  
			g_iNet_flag	= 1;		
  }	
	else if(strcmp_(dat,"REMOTIP",7)==0)
  {
			strtoip(gs_SaveNetIPCfg.ucDestIP,dat+8,len-8);  
			g_iNet_flag	= 1;
  }
	else if(strcmp_(dat,"SELFIP",6)==0)
  {
		strtoip(gs_SaveNetIPCfg.ucSelfIP,dat+7,len-7);  
		g_iNet_flag	= 1;
  }
	else if(strcmp_(dat,"SUBMASK",7)==0)
  {
		strtoip(gs_SaveNetIPCfg.ucSubMASK,dat+8,len-8);  
		g_iNet_flag	= 1;
  }
	else if(strcmp_(dat,"GATEWAY",7)==0)
  {
		strtoip(gs_SaveNetIPCfg.ucGateWay,dat+8,len-8);  
		g_iNet_flag	= 1;
  }	
  else if(strcmp_(dat,"REMOTPORT",9)==0)
  {
		cnt = len-10;
		data = 0;
		for(i=0;i<cnt;i++)
		{
				data = data*10 + (dat[10+i]-0x30);
		}
		gs_SaveNetIPCfg.ucDestPort[0] = (data>>8)&0xff;
		gs_SaveNetIPCfg.ucDestPort[1] = data&0xff; 
		g_iNet_flag	= 1;		
  }  	
	else if(strcmp_(dat,"LOCALPORT",9)==0)
  {
		cnt = len-10;
		data = 0;
		for(i=0;i<cnt;i++)
		{
				data = data*10 + (dat[10+i]-0x30);
		}
		gs_SaveNetIPCfg.ucSourcePort[0] = (data>>8)&0xff;
		gs_SaveNetIPCfg.ucSourcePort[1] = data&0xff; 
		g_iNet_flag	= 1;		
  } 
	else if(strcmp_(dat,"MONITOR",7)==0)
  {
		cnt = len-8;
		data = 0;
		for(i=0;i<cnt;i++)
		{
				data = data*10 + (dat[8+i]-0x30);
		}
		gs_SaveNetIPCfg.ucMonitorPort[0] = (data>>8)&0xff;
		gs_SaveNetIPCfg.ucMonitorPort[1] = data&0xff; 
		g_iNet_flag	= 1;		
  }   
}

/**************************************
*说明：处理配置信息的数组 例如:TMAX=12.5 
**************************************/
void ConfigDataDeal(char* dat,int len)
{
  unsigned long int head[2];
	wdt();
  if(strcmp_(dat,"CTIME",5)==0)//采集频率
  {
    g_configRead.collect_frq=StrToInt(dat+6,len-6);
		g_iCfg_flag = 1;
  }	
  else if(strcmp_(dat,"BEEP",4)==0)//保存beep工作�?
  {
    g_configRead.beep=StrToInt(dat+5,len-5);
		g_iCfg_flag = 1;
  }
	else if(strcmp_(dat,"RECE=",5)==0)
	{
		#if 0
		 if(dat[5]=='1')
				g_configRead.Depart[0]='1';
    else 
				g_configRead.Depart[0]='0'; 
		#else
			g_configRead.Depart[0]=StrToInt(dat+5,len-5);
			if(g_configRead.Depart[0]>0 && g_configRead.Depart[0]<3)
			{
					g_configRead.Depart[0] = 3;
			}
			else if(g_configRead.Depart[0]>240)
			{
					g_configRead.Depart[0] = 240;
			}
		#endif
		g_iCfg_flag = 1;
	}
	else if(strcmp_(dat,"TELT=",5)==0)
	{
			g_configRead.Depart[1]=StrToInt(dat+5,len-5);
			g_iCfg_flag = 1;
	}
	else if(strcmp_(dat,"DEBUG",5)==0)			//保存DEBUG工作�?
  {
    g_configRead.b_debug_work=StrToInt(dat+6,len-6);
		g_iCfg_flag = 1;
  }
	else if(strcmp_(dat,"AMPERRTC",8)==0)	//保存DEBUG工作�?
  {
		#if 1
		g_configRead.AMPER_RTC_PC13=StrToInt(dat+9,len-9);
		g_iCfg_flag = 1;
		#else			//ds18b20 序列号测试程序，在上位机中点 “不输出时钟�? 则在计算机中读出序列号�?通道ds18b20 gnd vcc clk
		unsigned char  abc[2],i;
		unsigned char  ds18b20_id[8];   
		putStr("\r\n",2);
		ReadID(0,ds18b20_id,8);
		for(i=0;i<8;i++)
		{
			charToHexStr(abc,ds18b20_id[i]);
			putStr(abc,2);
		}
		putStr("\r\n",2);
		return;
		#endif
  }	
  else if(strcmp_(dat,"WIFIMODE",8)==0)							//保存beep工作�?
  {
		#if FIX_RJ45
		g_configRead.wifi_mode=4;
		#else    
		g_configRead.wifi_mode=StrToInt(dat+9,len-9);
		#endif
		g_iCfg_flag = 1;
  }
	else if(strcmp_(dat,"RESETTIME",9)==0)						//保存resettime
  {
    g_configRead.reset_time = StrToInt(dat+10,len-10);
		g_LoadDiscWaitingTime   = g_configRead.reset_time;
		g_iCfg_flag = 1;
  }	
  else if(strcmp_(dat,"STIME",5)==0)//保存频率
  {
    g_configRead.save_frq=StrToInt(dat+6,len-6);
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"GTIME",5)==0)//发送频�?
  {
    g_configRead.send_frq=StrToInt(dat+6,len-6);
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"TMAX1",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.TMax[0]);
		#if CO2
		if(g_configRead.bCH[0]==3)
		{
			g_configRead.TMax[0]=StrToInt(dat+6,len-6); 
		}
		#endif
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"TMIN1",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.TMin[0]);
		#if CO2
		if(g_configRead.bCH[0]==3)
		{
			g_configRead.TMin[0]=StrToInt(dat+6,len-6); 
		}
		#endif
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"HMAX1",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.HMax[0]);
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"HMIN1",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.HMin[0]);
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"TMAX2",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.TMax[1]);
		g_LoadRelayCloseVoltage[0] = g_configRead.TMax[1]/10.0 ;
		#if CO2
		if(g_configRead.bCH[1]==3)
		{
			g_configRead.TMax[1]=StrToInt(dat+6,len-6); 
		}
		#endif
		g_iCfg_flag = 1;
  }
	else if(strcmp_(dat,"TMAX3",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.TMax2);
		g_LoadRelayCloseVoltage[2] = g_configRead.TMax2/10.0 ;		
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"TMIN2",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.TMin[1]);
		g_LoadRelayOpenVoltage[0] = g_configRead.TMin[1]/10.0;
		#if CO2
		if(g_configRead.bCH[1]==3)
		{
			g_configRead.TMin[1]=StrToInt(dat+6,len-6); 
		}
		#endif
		g_iCfg_flag = 1;
  }
	else if(strcmp_(dat,"TMIN3",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.TMin2);
		g_LoadRelayOpenVoltage[2] = g_configRead.TMin2/10.0;
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"HMAX2",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.HMax[1]);
		g_LoadRelayCloseVoltage[1] = g_configRead.HMax[1]/10.0;
		g_iCfg_flag = 1;
  }
	else if(strcmp_(dat,"HMAX3",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.HMax2);
		g_LoadRelayCloseVoltage[3] = g_configRead.HMax2/10.0;
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"HMIN2",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.HMin[1]);
		g_LoadRelayOpenVoltage[1] = g_configRead.HMin[1]/10.0;
		g_iCfg_flag = 1;
  }
	else if(strcmp_(dat,"HMIN3",5)==0)
  {
    FloatstrToUint(dat+6,len-6,&g_configRead.HMin2);
		g_LoadRelayOpenVoltage[3] = g_configRead.HMin2/10.0;
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"VMIN=",5)==0)
  {
		g_configRead.VMin=StrToInt(dat+5,len-5);    
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"ALARMT",6)==0)//警报延时
  {
    g_configRead.alarmDelyMinute=StrToInt(dat+7,len-7);
		g_iCfg_flag = 1;
  }	
  else if(strcmp_(dat,"M1",2)==0)
  {
    if(len==4 && dat[3]=='0')
      g_configRead.alarmNum[0][0]='0';
    else if((len-3)<=11)
    {
      g_configRead.alarmNum[0][0]='*';
      memcpy(g_configRead.alarmNum[0]+1,dat+3,len-3);
      g_configRead.alarmNum[0][len-3+1]='\0';
    }
    g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"M2",2)==0)
  {
    if(len==4 && dat[3]=='0')
      g_configRead.alarmNum[1][0]='0';
    else if((len-3)<=11)
    {
      g_configRead.alarmNum[1][0]='*';
      memcpy(g_configRead.alarmNum[1]+1,dat+3,len-3);
      g_configRead.alarmNum[1][len-3+1]='\0';
    }
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"M3",2)==0)
  {
    if(len==4 && dat[3]=='0')
      g_configRead.alarmNum[2][0]='0';
    else if((len-3)<=11)
    {
      g_configRead.alarmNum[2][0]='*';
      memcpy(g_configRead.alarmNum[2]+1,dat+3,len-3);
      g_configRead.alarmNum[2][len-3+1]='\0';
    }
		g_iCfg_flag = 1;
  }
	else if(strcmp_(dat,"MT1=",4)==0)
  {
    if(len==5 && dat[4]=='0')
      g_configRead.alarmTel[0][0]='0';
    else if((len-4)<13)
    {
      g_configRead.alarmTel[0][0]='*';
      memcpy(g_configRead.alarmTel[0]+1,dat+4,len-4);
      g_configRead.alarmTel[0][len-4+1]='\0';
    }
    g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"MT2=",4)==0)
  {
    if(len==5 && dat[4]=='0')
      g_configRead.alarmTel[1][0]='0';
    else if((len-4)<13)
    {
      g_configRead.alarmTel[1][0]='*';
      memcpy(g_configRead.alarmTel[1]+1,dat+4,len-4);
      g_configRead.alarmTel[1][len-4+1]='\0';
    }
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"MT3=",4)==0)
  {
    if(len==5 && dat[4]=='0')
      g_configRead.alarmTel[2][0]='0';
    else if((len-4)<13)
    {
      g_configRead.alarmTel[2][0]='*';
      memcpy(g_configRead.alarmTel[2]+1,dat+4,len-4);
      g_configRead.alarmTel[2][len-4+1]='\0';
    }
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"SN",2)==0)
  {
    memcpy(g_configRead.device_ID,dat+3,len-3);
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"REMOTEIP",8)==0)//远程IP
  {
    memcpy(g_configRead.remoteIP,dat+9,len-9);//远程IP
    g_configRead.IPLen=len-9;
		g_iCfg_flag = 1; 		
  }
  else if(strcmp_(dat,"REMOTEPORT",10)==0)//远程端口
  {
    memcpy(g_configRead.remotePort,dat+11,len-11);
    g_configRead.PortLen=len-11;
		g_iCfg_flag = 1;		
  }
	else if(strcmp_(dat,"NATIONCODE",10)==0)		//国际编号，发短信�?
	{
		if((len-11)>0&&(len-11)<6)	//1~5字节
		{
			memset(g_configRead.NationCode,0,sizeof(g_configRead.NationCode));
			memcpy(g_configRead.NationCode,dat+11,len-11);    
			g_iCfg_flag = 1;		
		}
	}
	else if(strcmp_(dat,"DEVICENAME",10)==0)					//设备名称
	{
		if((len-11)>0&&(len-11)<21&&((len-11)%2==0))	//1~20字节 并且为偶�?
		{
			memset(g_configRead.sysName,0,sizeof(g_configRead.sysName));
			memcpy(g_configRead.sysName,dat+11,len-11);    
			g_configRead.NameLen = len-11;
			g_iCfg_flag = 1;		
		}
	}
  else if(strcmp_(dat,"TXZ",3)==0)//温度修正
  {
    FloatstrToUint(dat+4,len-4,&g_configRead.TXZ[0]);
		g_LowBatteryVoltageStartGenerator = g_configRead.TXZ[0]/10.0;
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"HXZ",3)==0)//湿度修正
  {
    FloatstrToUint(dat+4,len-4,&g_configRead.HXZ[0]);
		g_HighBatteryVoltageStopGenerator = g_configRead.HXZ[0]/10.0;
		g_iCfg_flag = 1;
  }
  //add by wjj 2015-12-01
  else if(strcmp_(dat,"T2Z",3)==0)//温度修正
  {
    FloatstrToUint(dat+4,len-4,&g_configRead.TXZ[1]);
		g_EnclosureFanTurnOffTemperature = g_configRead.TXZ[1]/10.0;
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"H2Z",3)==0)//湿度修正
  {
    FloatstrToUint(dat+4,len-4,&g_configRead.HXZ[1]);
		g_EnclosureFanTurnOnTemperature = g_configRead.HXZ[1]/10.0;
		g_iCfg_flag = 1;
  }  
	#if 0
  else if(strcmp_(dat,"DANWEI",6)==0)//单位编号
  {
			memcpy(g_configRead.Danwei,dat+7,len-7);
			g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"BUMEN",5)==0)//部门编号
  {
			memcpy(g_configRead.Depart,dat+6,len-6);
			g_iCfg_flag = 1;
  }  
	#endif
  else if(strcmp_(dat,"CLEAR=",6)==0)//清空历史记录
  {
    if(dat[6]=='1')
    {
				g_datacnt = 0;				
				g_SaveDataInfo_Pos = -1;						//标志不正�?	
				g_SaveData_Loop	= 0;					
				//SaveSaveDataInfo();
    }
  }
  else if(strcmp_(dat,"BACKCLEAR=",10)==0)//清空回传历史数据（发送错误的补发数据�?
  {
	  if(dat[10]=='1')
	  {
				g_Repair_Wr = 0;
				g_Repair_Rd = 0;
				g_RepairInfo_Pos = -1;									//标志不正�?
				g_Repair_Loop = 0;	
				//SaveRepairInfo();
				memset(g_TxBuf_Status,1,UPDATANUM);			//假定所有数据都不可�?
	  }
  }
  else if(strcmp_(dat,"SETTIME",7)==0)		//设置时间
  {
    if((len-8)>=14)
    {
       PCF_setsystime(dat+8,14,0);   				//20170406161538  
				//GPRS_Reset();
    }		
  }
	else if(strcmp_(dat,"GPRSWORK=",9)==0)			//gprs是否工作
  {
    if(dat[9]=='1')
      g_configRead.b_gprs_work=1;
    else 
      g_configRead.b_gprs_work=0; 
		g_iCfg_flag = 1;
  }
	else if(strcmp_(dat,"WIFIWORK=",9)==0)			//wifi是否工作
  {
    if(dat[9]=='1')
      g_configRead.b_wifi_work=1;
    else 
      g_configRead.b_wifi_work=0; 
		g_iCfg_flag = 1;
  }
	else if(strcmp_(dat,"RJ45WORK=",9)==0)			//rj45是否工作
  {
    if(dat[9]=='1' || dat[9]=='3')
		{
      g_configRead.b_rj45_work=1;
			if(NULL!=EtherNet_Task_Handle)
			{
					eTaskState sta = eTaskGetState(EtherNet_Task_Handle);
					if(eSuspended==sta)
					{
						vTaskResume(EtherNet_Task_Handle);
					}
			}
		}
    else 
		{
      g_configRead.b_rj45_work=0; 
		}
		g_iCfg_flag = 1;
  }	
	else if(strcmp_(dat,"LIGHTWORK=",10)==0)			//背光是否工作
  {
    if(dat[10]=='1')
      g_configRead.b_light_work=1;
    else 
      g_configRead.b_light_work=0; 
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"CH1WORK=",8)==0)//通道1是否有效
  {
    if(dat[8]=='0')//无效
      g_configRead.bCH[0]=0;
    else if(dat[8]=='1')
      g_configRead.bCH[0]=1;  //温湿度采�?
    else if(dat[8]=='2')
      g_configRead.bCH[0]=2;  //单温通道1
    else if(dat[8]=='3')
      g_configRead.bCH[0]=3;  //单温通道1和通道2都使�?
		else if(dat[8]=='4')
      g_configRead.bCH[0]=4;  //单温通道1和通道2都使�?
		else if(dat[8]=='5')
      g_configRead.bCH[0]=5;  //单温通道1和通道2都使�?
		else if(dat[8]=='6')
      g_configRead.bCH[0]=6;  //单温通道1和通道2都使�?
		else if(dat[8]=='7')
      g_configRead.bCH[0]=7;  //单温通道1和通道2都使�?    
		else
      g_configRead.bCH[0]=1;  //温湿度采�?
    //一般情况下如果使用温湿度则不可以使用单温，同样使用了单温则不可以使用温湿度
    //在使用单温情况下，两种用法，一，一个单温或两个单温,一个单温则默认接sht21的clk�?
		g_iCfg_flag = 1;
  }
  else if(strcmp_(dat,"CH2WORK=",8)==0)//通道2是否有效
  {
    if(dat[8]=='0')//无效
      g_configRead.bCH[1]=0;
    else if(dat[8]=='1')
      g_configRead.bCH[1]=1;  //温湿度采�?
    else if(dat[8]=='2')
      g_configRead.bCH[1]=2;  //单温通道1
    else if(dat[8]=='3')
      g_configRead.bCH[1]=3;  //单温通道1和通道2都使�?
		 else if(dat[8]=='4')
      g_configRead.bCH[1]=4;  //单温通道1和通道2都使�?
		else if(dat[8]=='5')
      g_configRead.bCH[1]=5;  //单温通道1和通道2都使�?
		else if(dat[8]=='6')
      g_configRead.bCH[1]=6;  //单温通道1和通道2都使�?
		else if(dat[8]=='7')
      g_configRead.bCH[1]=7;  //单温通道1和通道2都使�?    
		else
      g_configRead.bCH[1]=1;  //温湿度采�?
    //一般情况下如果使用温湿度则不可以使用单温，同样使用了单温则不可以使用温湿度
    //在使用单温情况下，两种用法，一，一个单温或两个单温,一个单温则默认接sht21的clk�?
		g_iCfg_flag = 1;
  } 
	else if(strcmp_(dat,"RESTARTSYS=",11)==0)
  {
    if(dat[11]=='1')
    {
			//pt100_reset();
			putStr("SetConfig OK!\r\n",15);
			wdt();
			send_data();
			wdt();
			wdt();
      delay_1ms(200);
			__set_FAULTMASK(1); 					
			nvic_system_reset();
    }    
  }
	else if(strcmp_(dat,"SMST=",5)==0)
	{
		g_configRead.b_Sms_Test=StrToInt(dat+5,len-5);	
		g_HighPVVoltageStopGenerator 				= g_configRead.b_Sms_Test;		
		g_iCfg_flag = 1;
	}
	else if(strcmp_(dat,"SMSTIME=",8)==0)
	{
		g_configRead.b_Sms_FxiTime=StrToInt(dat+8,len-8);			
		g_iCfg_flag = 1;		
	}
}

extern int g_tj;
void SetConfig( char* g_ch376Buf,int  fileSize)
{
        unsigned long int i;
        int n=0;
        char temp[128];
        g_iNet_flag = 0;
	      g_iCfg_flag = 0;
        for(i=0;i<fileSize;i++)
        {
            if(g_ch376Buf[i]!=',')
            {
							if(n<128)
							{
									temp[n++]=g_ch376Buf[i];
							}							
            }
            else
            {
               ConfigDataDeal(temp,n);
							 ConfigDataDeal2(temp,n);
               n=0;
            }
         }
				 
         if(g_ch376Buf[fileSize-1]!=',')		
         {
             ConfigDataDeal(temp,n);
						 ConfigDataDeal2(temp,n);
         }  
 				 if(g_iCfg_flag)
				 {
						//g_tj = 0;
						WriteConfigParaFromIICAll();			//ͨ����־�������Ƿ����д������
				 }
				 if(g_iNet_flag)
				 {
						WriteParametersToIICAll();   
				 }
				g_iNet_flag = 0;
	      g_iCfg_flag = 0;				 
}

void configinfo_analy( char* src,const int rx_len)
{
	//*000000,SM,REMOTEPORT=7107,CTIME=123#	
  if(rx_len<12)
    return;
	wdt();
  SetConfig(src+11,rx_len-12);
  putStr("SetConfig OK!\r\n",15);
	wdt();
	send_data();
}



char bfirst_udp=0;
int wsddata_package(int channel)
{
  unsigned char zhenshu,xiaoshu,fuhao;
  char real_buf[60];
  long int sendWD=0,sendSD=0;
  long int wendu2;
  unsigned char wd1[5];
  unsigned char sd1[4];
	real_buf[0]='*';
	real_buf[1]='R';
	if(bfirst_udp==0)
	{
	  //real_buf[2]='T
		real_buf[2]='H';
	  bfirst_udp=1;
	}
	else
	{
	  real_buf[2]='H';
	}
	real_buf[3]=',';
	wdt();
  PCF_getsystime();					//当前时间�?
	real_buf[4]=g_sysTime[2];	
	real_buf[5]=g_sysTime[3];	
	real_buf[6]='-';	//得到时间
	real_buf[7]=g_sysTime[4];	
	real_buf[8]=g_sysTime[5];	
	real_buf[9]='-';	//得到时间
	real_buf[10]=g_sysTime[6];	
	real_buf[11]=g_sysTime[7];	
	real_buf[12]=' ';
	real_buf[13]=g_sysTime[8];	
	real_buf[14]=g_sysTime[9];	
	real_buf[15]=':';	//得到时间
	real_buf[16]=g_sysTime[10];
	real_buf[17]=g_sysTime[11];	
	real_buf[18]=':';	//得到时间
	real_buf[19]=g_sysTime[12];
	real_buf[20]=g_sysTime[13];	
	real_buf[21]=',';
	
	
	//设备ID
	real_buf[22]=g_configRead.device_ID[0];
	real_buf[23]=g_configRead.device_ID[1];
	real_buf[24]=g_configRead.device_ID[2];
	real_buf[25]=g_configRead.device_ID[3];
	real_buf[26]=g_configRead.device_ID[4];
	real_buf[27]=g_configRead.device_ID[5];
	real_buf[28]=',';
	//温度
	

	if(channel>0x2f&&channel<0x32)
	{
			sendWD=g_wendu[channel-0x30];
			sendSD=g_shidu[channel-0x30];
	}
	else if(channel>-1&&channel<2)
	{
			sendWD=g_wendu[channel];
			sendSD=g_shidu[channel];
	}
		
	//温度
  if(sendWD!=-8888)
  {
       if(sendWD<0)
       {
            fuhao=1;
            wendu2 = sendWD*-1;
       }
       else
       {
            fuhao=0;
            wendu2 = sendWD;
       }
       zhenshu=wendu2/100%100;//整数部分取低两位
       xiaoshu=wendu2%100/10;//小数部分取高一�?
       if(fuhao==0)
			 {
           wd1[0]='+';
			 }
       else      
			 {
            wd1[0]='-';
			 }
       wd1[1]=zhenshu/10+'0';
       wd1[2]=zhenshu%10+'0';
       wd1[3]='.';
       wd1[4]=xiaoshu+'0';
  }
  else
  {
		  wd1[0]='+';
      wd1[1]='8';
      wd1[2]='8';
      wd1[3]='8';
      wd1[4]='8';
  }
       
  //湿度
  if(sendSD!=0)
  {
      if(sendSD>=10000)
			{
            sendSD=9990;
			}
      sd1[0]=((sendSD%10000)/1000) + '0';
      sd1[1]=((sendSD%1000)/100) + '0';
      sd1[2]='.';
      sd1[3]=((sendSD%100)/10) + '0';
  }
  else
  {
     sd1[0]='8';
     sd1[1]='8';
     sd1[2]='8';
     sd1[3]='8';
  }
	//温度
	memcpy(real_buf+29,wd1,5);
	real_buf[34]=',';
	
	//湿度
	memcpy(real_buf+35,sd1,4);
	real_buf[39]=',';

	//电压
	real_buf[40]=g_vol/10+'0';
	real_buf[41]='.';
	real_buf[42]=g_vol%10+'0';
	real_buf[43]=',';
        
	//充电状�?
	if(g_bCharge)
	  real_buf[44]='1';
	else
	  real_buf[44]='0';
	real_buf[45]=',';

	//通道
	real_buf[46]=channel;
	real_buf[47]=',';	
	if(channel>0x2f) 
	{
		//温度报警
		real_buf[48]='0';
		real_buf[49]='0';
		//湿度报警
		real_buf[50]='0';
		real_buf[51]='0';
		//电压
		real_buf[52]='0';
	}
	else
	{
		//温度报警
		real_buf[48]=0;
		real_buf[49]=0;
		//湿度报警
		real_buf[50]=0;
		real_buf[51]=0;
		//电压
		real_buf[52]=0;
	}	
	real_buf[53]='0';
	real_buf[54]='#';
  putStr(real_buf,55);		//发送数�?
	send_data();
	return 55;
}



//增加时间打印
void print_time(void)
{
  char TxBuf[22]={0};  	
	wdt();
	TxBuf[0]=' ';  
  TxBuf[1]='[';  
	PCF_getsystime();
  TxBuf[2]=g_sysTime[2];//'1'�?
  TxBuf[3]=g_sysTime[3];//'2'
  TxBuf[4]='-';//'-'
  TxBuf[5]=g_sysTime[4];//'0'�?
  TxBuf[6]=g_sysTime[5];//'2'
  TxBuf[7]='-';//'-'
  TxBuf[8]=g_sysTime[6];//'0'�?
  TxBuf[9]=g_sysTime[7];//'8'
  TxBuf[10]=' ';//' '
  TxBuf[11]=g_sysTime[8];//'1'�?
  TxBuf[12]=g_sysTime[9];//'0'
  TxBuf[13]=':';//':'
  TxBuf[14]=g_sysTime[10];//'2'�?
  TxBuf[15]=g_sysTime[11];//'0'
  TxBuf[16]=':';//':'
  TxBuf[17]=g_sysTime[12];//'3'�?
  TxBuf[18]=g_sysTime[13];//'4'
	TxBuf[19]=']';
	TxBuf[20]=' ';
  TxBuf[21]='\0';
	Gprs_Tcp_Log(TxBuf);
}

void print_wr_rd(void)
{
	 char prt[11]=" P:00 00\r\n";
						if(g_TxBufWr>9)
						{
								prt[3] = (g_TxBufWr/10) + '0';
								prt[4] = (g_TxBufWr%10) + '0';
						}
						else
						{
								prt[3] = '0';
								prt[4] = (g_TxBufWr%10) + '0';
						}
						
						if(g_TxBufRd>9)
						{
								prt[6] = (g_TxBufRd/10) + '0';
								prt[7] = (g_TxBufRd%10) + '0';
						}
						else
						{
								prt[6] = '0';
								prt[7] = (g_TxBufRd%10) + '0';
						}						
						Gprs_Tcp_Log(prt);													//打印位置
}



int 					g_TxBufSendCnt = 0;
unsigned int 	g_TcpSendStart=0;
unsigned char tcp_wifi_step=0;
unsigned char tcp_rx_len=0; 
unsigned char tcp_rxDat[129];

/*
if(gprs上传并且gprs是打开状�?
{
	判断是否到了数据上传时刻,如果是则产生tcp上传数据到缓冲队列中，如果缓冲队列中的数据满则直接将数据写补传中�?	OK 问题读写追平�?

  无数据发送并且发现了gprs已经断开则进行标志gprs断开�? g_gprs_discon 通过串口监测到的,但g_bGPRSConnected中认为gprs还处于连接中，并且还在操作中�?

  及时判断上传ip或port是否变化。并关闭gprs,准备重新拨号。判断条件为gprs处于连接状态，g_gprs_discon=0,数据不处于发送状态�?	OK

  if(g_TxBufRd!=g_TxBufWr)		缓冲队列中有待发送数�?  			问题?  rd=3  wr=2,  当数据正在发送，则不允许打断，保证连续性�?
	{
				if(缓冲队列中有待发送数�?数据还在发送中)							//隐含条件为处于连接中，并一切条件都具备
				{
						如果没有应答，并�?6秒的则处理，处理办法如下:	
						if(g_TxBufSendCnt>2 即第三次还发送不成功�?
						{

						}
						else
						{
								g_TxBufSendCnt++;														//发送次数增�?
								g_TcpSendStart = g_sysTick;									//新的开始发送时�?
								RS2324_Send_Data(g_TxBufTemp, 78);					//进行发送�?
						}
				}

				if(缓冲队列中有待发送数�?无数据正在发送，gprs并处于已连接)			//问题?  当拨上号后，不能立即进行上传,需要等�?秒，目的保证服务器已经准备好�?
				{
						if(无短信待发�?
						{
								数据发�?
								g_TxBufSendCnt = 1;													//笿次发�?
								g_TcpSendStart = g_sysTick;									//开始发送时�? 进行发�?
								memcpy(g_TxBufTemp,g_TxBuf[g_TxBufRd],78);  //将待发送数据取出来g_TxBufRd位置�?
								RS2324_Send_Data(g_TxBufTemp, 78);						
						}
						else
						{
								有短信待发�?则关闭gprs�?
						}
				}	

				if(缓冲队列中有待发送数�?无数据正在发�?gprs处于断开状�?
				{
						//进行处理，判断g_TxBufRd位置数据是否超时,如果是则g_TxBufRd++: 是补传的直接舍掉，正常数据的则写补传队列中�?
						if((g_sysTick - g_TxStarttime[g_TxBufRd])>GPRS_TIMEOUT)
						{
								//直接将数据写入到外存�?
								if(g_TxBuf[g_TxBufRd][33]=='B')		//如果补传数据
								{
										;
								}
								else
								{
										SaveOneRepairData(g_TxBuf[g_TxBufRd],78);		//将数据临时存储起来�?
								}
								g_tcp_err_cnt++;																//发送不成功数据条数计数�?
								g_TxBufSendCnt = 0;
								g_TxBufRd = (g_TxBufRd+1)%UPDATANUM;						//读指针移动下一个�?
						}	
				}				
	}
	else
	{
			if(缓冲队列为空，短信队列无短信，gprs处于连接状态，补传队列中有待补传数�?
			{
					取一条补传数据到缓冲中�?
			}

			if(短信队列有短信并且g_bGPRSConnected=1)
			{
					断开gprs,准备发送短�?
			}
	}
}
*/


/*
D5 00 00 00 03 FF FF C8 00 CB
d5 00 00 00 01 cb b7 ff ff 7d  ��λ9.15mm
d5 00 00 00 01 cb cb ff 00 fe  ��λ0.15mm  203��ʾ(203-200)*0.05=0.15.
d5 00 00 00 02 42 42 ff 00 fd  ��λ3.30mm
d5 00 00 00 02 51 51 ff 00 fd  ��λ4.05mm

d5      00      00      00       01    cb      b7    ff           ff      7d  	��λ9.15mm
֡ͷ   ������ ���Ÿ�  ���ŵ�    ״̬  δ֪    ȱ��  ȱ�����ֵ   ����̬  ^У��
����̬: 00��ʾ��̬��ff��ʾ��̬
״̬��01=�� 02=�� 03=ת����  04=���������ݴ���
*/
void com4_2440(char *str,int len)
{
		putStr(str,len);
		send_data();
}

//60字节buf
void Wifi_Rj45_Param( char * wifi_rxDat)
{
						if(wifi_rxDat[4]=='W' && wifi_rxDat[5]=='I' && wifi_rxDat[6]=='F' && wifi_rxDat[7]=='I' && wifi_rxDat[8]==',')			//后面增加1个字节体现是wifi还是rj45
						{       
								if(wifi_rxDat[9]=='1')																		//	读wifi参数
								{
										ReadWifiPara(1);																			//	读wifi参数。不对wifi进行重启�?
										//将wifi的所有参数读出来�?
										wifi_rxDat[0]='*';
										wifi_rxDat[1]='R';
										wifi_rxDat[2]='T';
										wifi_rxDat[3]=',';
										wifi_rxDat[4]='W';
										wifi_rxDat[5]='I';
										wifi_rxDat[6]='F';
										wifi_rxDat[7]='I';								
										wifi_rxDat[8]=',';																			
										wifi_rxDat[9]='1';																			
										memcpy(wifi_rxDat+10,gs_SaveWifiCfg.ucSelfIP,4);			//ip
										memcpy(wifi_rxDat+14,gs_SaveWifiCfg.ucSubMASK,4);			//subnet
										memcpy(wifi_rxDat+18,gs_SaveWifiCfg.ucGateWay,4);			//gateway											
										wifi_rxDat[22] = (gs_SaveWifiCfg.ucDhcp<<7)|(gs_SaveWifiCfg.ucWorkMode<<5)|(gs_SaveWifiCfg.ucEncryptMode<<2);		//工作模式 ap还是wifi节点(接点,后面ssid,mima,ip信息，dhcp是否)  ap模式则dhcp,ip信息,加密方式
										//gs_SaveWifiCfg.ucWifiState   需要3bit  wifi_rxDat[22] 只剩余2bit
										//gs_SaveWifiCfg.ucWifiSingal  需要1字节
										memcpy(wifi_rxDat+23,gs_SaveWifiCfg.ucMAC,6);					//mac										
										memcpy(wifi_rxDat+29,gs_SaveWifiCfg.ucSSID_PWD,36);		//ssid+pwd;									
										wifi_rxDat[75]='#';										
										wifi_rxDat[76]=gs_SaveWifiCfg.ucWifiState;										
										wifi_rxDat[77]=gs_SaveWifiCfg.ucWifiSingal;										
										com4_2440(wifi_rxDat,78);															//合计29个字节   增加两字节，在最后， 分别表示wifi连接状态和信号强度
									#if 0
								//		TurnOffAte(0);					wdt();
										delay_1ms(1000);  			wdt();
										GetAtCmd();							wdt();
										delay_1ms(1000);				wdt();
										AllQueryCmdPrint();			wdt();
										delay_1ms(1000);
										AllExecuteCmdPrint(1);	wdt();
										AllExecuteCmdPrint(2);	wdt();
										AllExecuteCmdPrint(3);	wdt();		
										//printf("\r\nDip is %d", dip_sta);			
										#endif
								}
								else if(wifi_rxDat[9]=='6')																//	读wifi 1,2通道的数据�?
								{
										//读wifi的通道参数
										wifi_rxDat[0]='*';
										wifi_rxDat[1]='R';
										wifi_rxDat[2]='T';
										wifi_rxDat[3]=',';
										wifi_rxDat[4]='W';
										wifi_rxDat[5]='I';
										wifi_rxDat[6]='F';
										wifi_rxDat[7]='I';								
										wifi_rxDat[8]=',';																			
										wifi_rxDat[9]='6';																			
										memcpy(wifi_rxDat+10,gs_SaveWifiCfg.TdInfo[0].ucDestIP,4);			//目标IP
										memcpy(wifi_rxDat+14,gs_SaveWifiCfg.TdInfo[0].ucLocalPort,2);		//本地port
										memcpy(wifi_rxDat+16,gs_SaveWifiCfg.TdInfo[0].ucRemotePort,2);	//远程port
										wifi_rxDat[18] = gs_SaveWifiCfg.TdInfo[0].ucProtocol;
										wifi_rxDat[19] = gs_SaveWifiCfg.TdInfo[0].ucCSMode;
									
										memcpy(wifi_rxDat+20,gs_SaveWifiCfg.TdInfo[1].ucDestIP,4);			//目标IP
										memcpy(wifi_rxDat+24,gs_SaveWifiCfg.TdInfo[1].ucLocalPort,2);		//本地port
										memcpy(wifi_rxDat+26,gs_SaveWifiCfg.TdInfo[1].ucRemotePort,2);	//远程port
										wifi_rxDat[28] = gs_SaveWifiCfg.TdInfo[1].ucProtocol;						//tcp udp协议
										wifi_rxDat[29] = gs_SaveWifiCfg.TdInfo[1].ucCSMode;							//server or client 			
										wifi_rxDat[30] = gs_SaveWifiCfg.TdInfo[1].ucConnState;					//通道1的连接状态(2022-06-21)新增
										wifi_rxDat[75]='#';										
										com4_2440(wifi_rxDat,76);																				//合计29个字节�?											
								}
								else if(wifi_rxDat[9]=='2')																					//	通过wifi读其他设备温度数�?
								{
																		
								}		
								else if(wifi_rxDat[9]=='3')																	//设置wifi基本参数	10+1+12+2=23  dhcp+节点模式+加密方式  中间\0隔开�?
								{
										gs_SaveWifiCfg.ucStartFlag = '*';
										
										memcpy(gs_SaveWifiCfg.ucSelfIP,wifi_rxDat+10,4);				//ip										
										memcpy(gs_SaveWifiCfg.ucSubMASK,wifi_rxDat+14,4);				//ip										
										memcpy(gs_SaveWifiCfg.ucGateWay,wifi_rxDat+18,4);				//ip																				
										memcpy(gs_SaveWifiCfg.ucDNS,wifi_rxDat+18,4);						//ip																				
										gs_SaveWifiCfg.ucDhcp = (wifi_rxDat[22]>>7)&0x1;				
										gs_SaveWifiCfg.ucWorkMode =(wifi_rxDat[22]>>5)&0x3;			
										gs_SaveWifiCfg.ucEncryptMode =(wifi_rxDat[22]>>2)&0x7;		
										
										memset(gs_SaveWifiCfg.ucSSID_PWD,0,37);										
										memcpy(gs_SaveWifiCfg.ucSSID_PWD,wifi_rxDat+23,36);			//									
										gs_SaveWifiCfg.ucEndFlag = '#';										
										//其中ucMAC只能读，不能写。TdInfo信息(1个通道10个字�?目前不在此处配置。ucDNS不需要配置�?
										WriteFlashWifi();			//WriteFlashWifi(&gs_SaveWifiCfg.ucStartFlag,sizeof(t_SaveWifiCfg));										
										
										wifi_rxDat[0] = 'O';
										wifi_rxDat[1] = 'K';
										com4_2440(wifi_rxDat,2);
										wdt();
										//进行配置
										CfgWifiPara();
								}
								else if(wifi_rxDat[9]=='4')				//wifi reset
								{
										//将wifi的所有参数读出来�?										
										wifi_rxDat[0] = 'O';
										wifi_rxDat[1] = 'K';
										com4_2440(wifi_rxDat,2);			//合计29个字节�?											
										wifi_reset();
								}
								else if(wifi_rxDat[9]=='5')				//system reset
								{
										//将wifi的所有参数读出来�?										
										wifi_rxDat[0] = 'O';
										wifi_rxDat[1] = 'K';
										com4_2440(wifi_rxDat,2);			//合计29个字节�?			
										delay_1ms(200);
										__set_FAULTMASK(1); 					
										nvic_system_reset();
								}																			
								//*000000,TH,10000000000000#						 
                //wsddata_package(wifi_rxDat[11]);   					//根据不同    
								//com3_wifi2(wifi_rxDat,55);											
						}          
						else if(wifi_rxDat[4]=='R' && wifi_rxDat[5]=='J' && wifi_rxDat[6]=='4' && wifi_rxDat[7]=='5' && wifi_rxDat[8]==',')			//后面增加1个字节体现是wifi还是rj45
						{       
								//*000000,TH,10000000000000#						 
                //wsddata_package(wifi_rxDat[11]);   				//根据不同    
								//com3_wifi2(wifi_rxDat,55);										
								if(wifi_rxDat[9]=='1')											//读rj45参数
								{
										wifi_rxDat[0]='*';
										wifi_rxDat[1]='R';
										wifi_rxDat[2]='T';
										wifi_rxDat[3]=',';
										wifi_rxDat[4]='R';
										wifi_rxDat[5]='J';
										wifi_rxDat[6]='4';
										wifi_rxDat[7]='5';								
										wifi_rxDat[8]=',';																			
										wifi_rxDat[9]='1';																			
										memcpy(wifi_rxDat+10,gs_SaveNetIPCfg.ucSelfIP,4);				//ip
										memcpy(wifi_rxDat+14,gs_SaveNetIPCfg.ucSubMASK,4);			//subnet
										memcpy(wifi_rxDat+18,gs_SaveNetIPCfg.ucGateWay,4);			//gateway											
										memcpy(wifi_rxDat+22,gs_SaveNetIPCfg.ucMAC,6);					//mac
										wifi_rxDat[28]='#';										
										com4_2440(wifi_rxDat,29);																//合计29个字节�?							
								}
								else if(wifi_rxDat[9]=='6')																		//读rj45 1,2通道网络参数
								{
										//读wifi的通道参数
										wifi_rxDat[0]='*';
										wifi_rxDat[1]='R';
										wifi_rxDat[2]='T';
										wifi_rxDat[3]=',';
										wifi_rxDat[4]='R';
										wifi_rxDat[5]='J';
										wifi_rxDat[6]='4';
										wifi_rxDat[7]='5';								
										wifi_rxDat[8]=',';																			
										wifi_rxDat[9]='6';																			
										memcpy(wifi_rxDat+10,UDP_DIPR,4);												//目标IP
										memcpy(wifi_rxDat+14,S2_Port,2);												//本地port
										memcpy(wifi_rxDat+16,UDP_DPORT,2);											//远程port
										wifi_rxDat[18] = 1;																			//tcp udp协议   					ucProtocol  0=tcp  1=udp
										wifi_rxDat[19] = 0;																			//server or client 				ucCSMode  0=Client  1=Server
									
										memcpy(wifi_rxDat+20,gs_SaveNetIPCfg.ucDestIP,4);				//目标IP
										memcpy(wifi_rxDat+24,gs_SaveNetIPCfg.ucSourcePort,2);		//本地port
										memcpy(wifi_rxDat+26,gs_SaveNetIPCfg.ucDestPort,2);			//远程port
										wifi_rxDat[28] = 0;																			//tcp udp协议   					ucProtocol  0=tcp  1=udp
										wifi_rxDat[29] = 0;																			//server or client 				ucCSMode  0=Client  1=Server
										
										wifi_rxDat[75]='#';										
										com4_2440(wifi_rxDat,76);																	//合计29个字节�?											
								}
								else if(wifi_rxDat[9]=='2')																		//	通过rj45读其他设备温度数�?
								{
										
								}	
								else if(wifi_rxDat[9]=='3')																	//设置rj45基本参数
								{
										memcpy(gs_SaveNetIPCfg.ucSelfIP,wifi_rxDat+10,4);				//ip
										memcpy(gs_SaveNetIPCfg.ucSubMASK,wifi_rxDat+14,4);			//subnet
										memcpy(gs_SaveNetIPCfg.ucGateWay,wifi_rxDat+18,4);			//gateway		
										if(wifi_rxDat[22]==0x00&&wifi_rxDat[23]==0x00&&wifi_rxDat[24]==0x00&&wifi_rxDat[25]==0x00&&wifi_rxDat[26]==0x00&&wifi_rxDat[27]==0x00)
										{
												;
										}
										else
										{
												memcpy(gs_SaveNetIPCfg.ucMAC,wifi_rxDat+22,6);			//mac
										}
										WriteParametersToIICAll();															//保存参数
										wifi_rxDat[0] = 'O';
										wifi_rxDat[1] = 'K';
										com4_2440(wifi_rxDat,2);
								}	
								else if(wifi_rxDat[9]=='4')				//rj45 reset
								{
										//将wifi的所有参数读出来�?
										//wifi_reset();
										wifi_rxDat[0] = 'O';
										wifi_rxDat[1] = 'K';
										com4_2440(wifi_rxDat,2);			//合计29个字节�?			
								}		
								else if(wifi_rxDat[9]=='5')				//system reset
								{
										//将wifi的所有参数读出来�?										
										wifi_rxDat[0] = 'O';
										wifi_rxDat[1] = 'K';
										com4_2440(wifi_rxDat,2);			//合计29个字节�?			
											delay_1ms(200);
										__set_FAULTMASK(1); 					
										nvic_system_reset();
								}										
						}
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++usb com1 配置接口通讯处理 start+++++++++++++++++++++++++++++++
// RS232_USART1_Send_Data    
// #define RS2321_REC_BUFF_SIZE			 512
// RS2321_buff[RS2321_REC_BUFF_SIZE]  RS2321_rec_WR   RS2321_rec_RD
void comm_process_usb(void)					//uart1
{
	 static unsigned char uart_wifi_step=0;
	 static int wifi_rx_len=0; 
	 static char wifi_rxDat[512];
	 g_cSendMode = 1;																	//数据通道为usb com1模式
	 g_cSendMode_tcp = 0;
	if(RS2321_rec_WR==RS2321_rec_RD)
	{
		if((systickCount - usb_para_tick) > 200 )	//ms
		{
			wifi_rx_len=0;
			uart_wifi_step=0;
		}
		return;
	}
	
	 while(RS2321_rec_WR!=RS2321_rec_RD)							//udpӎ˽Ƥ׃א
   {
		 	wdt();		// #=35=0x23    *=42=0x2A  简�?#判断则可能出现问�?IP�?	OK
      if(uart_wifi_step==0&&RS2321_buff[RS2321_rec_RD]=='*') 					//ߪͷ
      {
         uart_wifi_step=1;
         wifi_rx_len=0;
         wifi_rxDat[wifi_rx_len++]=RS2321_buff[RS2321_rec_RD];        
      }
      else if(uart_wifi_step==1&&RS2321_buff[RS2321_rec_RD]=='#')			//ޡ˸
      {
             wifi_rxDat[wifi_rx_len++]=RS2321_buff[RS2321_rec_RD];       
						 if(wifi_rxDat[8]=='T' && wifi_rxDat[9]=='H')
             {
                 //*000000,TH,10000000000000#	
									wsddata_package(wifi_rxDat[11]);  		//读温度命令响�?
									wifi_rx_len=0;
									uart_wifi_step=0;							 
									wdt();
             }				
             else if(wifi_rxDat[8]=='G' && wifi_rxDat[9]=='C')		//??????????
             {	//*000000,GC,100000#
                 configinfo_package(wifi_rxDat,wifi_rx_len);			//????????    
								 wifi_rx_len=0;
								 uart_wifi_step=0;							 
             }
             else if(wifi_rxDat[8]=='S' && wifi_rxDat[9]=='C')		//????????
             {	//*000000,SC,REMOTEIP=192.168.1.190#
                configinfo_analy(wifi_rxDat,wifi_rx_len);
								wifi_rx_len=0;
								uart_wifi_step=0;							 
             }    
						 else if(wifi_rx_len==60 && wifi_rxDat[1]=='R' && wifi_rxDat[3]==',' && wifi_rxDat[8]==',')
						 {	
								//设置参数格式部分:
								//rj45: PPPP=4字节IP; SSSS=4字节子网掩码; GGGG=4字节网关; MMMMMM=6字节MAC; X=若干字节\0;
								//设置格式  *RH,RJ45,3PPPPSSSSGGGGMMMMMMX#     合计60字节  3=设置参数
								//设置格式  *RH,WIFI,3PPPPSSSSGGGGHSSIDPWDX#   合计60字节	 3=设置参数
								//wifi: PPPP=4字节IP; SSSS=4字节子网掩码; GGGG=4字节网关; H=1字节,加密DHCP和工作模�?SSID=若干字节\0结束;DPWD=若干字节;X=若干字节\0;							 
							 
								//读参数格式部�? *RT,RJ45,1xx#			合计60个字�? 1=读参�?		   返回76字节
								Wifi_Rj45_Param(wifi_rxDat);			
								wifi_rx_len=0;
								uart_wifi_step=0;							 
						 }
						 else 
						 {
								if(wifi_rx_len>511)
								{
									wifi_rx_len=0;
									uart_wifi_step=0;
								}		
						 }            
      }
      else  if(uart_wifi_step==1) 				//˽ߝޓ˕ڽԌא
      {
          wifi_rxDat[wifi_rx_len++]=RS2321_buff[RS2321_rec_RD]; 
          if(wifi_rx_len>511)
          {
             wifi_rx_len=0;
             uart_wifi_step=0;
          }
      } 
      else  															//??????
      {
          uart_wifi_step = 0;
          wifi_rx_len = 0;
      }
      RS2321_rec_RD=(RS2321_rec_RD+1)%RS2321_REC_BUFF_SIZE; 
    } 		
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++usb com1 配置接口通讯处理 end+++++++++++++++++++++++++++++++








//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++pt100 com5 start+++++++++++++++++++++++++++++++
//CRC校验
#if CO2
unsigned char  GS_CrcCheck(unsigned char *pData,int nLen)
{
	unsigned char t=0;
	unsigned short crc = 0;	//设置CRC寄存器，并给其赋值FFFF(hex)
	int j;	
	for(j = 0; j < nLen; j++)
	{
			crc+=pData[j];
	}	
	t = crc&0xff;
	t = 0xff-t;
	t = t+1;
	return t;
}
#else
unsigned short  GS_CrcCheck(unsigned char *pData,int nLen)
{
	unsigned short crc = 0xffff;	//设置CRC寄存器，并给其赋值FFFF(hex)
	int i,j;
	for(i = 0; i < nLen; i++)
	{
		crc^=(unsigned short)pData[i];			//将数据的第一�?-bit字符�?6位CRC寄存器的�?位进行异或，并把结果存入CRC寄存�?
		for(j = 0; j < 8; j++)
		{
			if(crc&1)
			{
				crc>>=1;						//CRC寄存器向右移一位，MSB补零，移出并检查LSB
				crc^=0xA001;					//多项式码
			}
			else
				crc>>=1;						//如果LSB�?，重复第三步；若LSB�?，CRC寄存器与多项式码相异或�?
			//		Sleep(1);
		}
	}
	return crc;
}
#endif
//int  pt_pre_wendu1 = 0;				//上次正确的温�?
//int  pt_pre_wendu2 = 0;				//上次正确的温�?
//int  pt_error_cnt1 = 0;			//已经错误次数
//int  pt_error_cnt2 = 0;			//已经错误次数
//int  pt_cnt1=0;
//int  pt_cnt2=0;
//RS2325_rec_WR  RS2325_rec_RD
//RS2325_buff[RS2325_REC_BUFF_SIZE]  
//RS2325_REC_BUFF_SIZE  512

		
void send_data()
{
	wdt();
	#if 0
	if(g_cSendMode_tcp==0)
	{
		if(g_cSendMode==1)					//usb
		{
				//RS232_USART1_Send_Data(g_wifi_send_buf,g_wifi_send_cnt);
		}		
  }	
	#endif
	
	if(g_wifi_send_cnt<RS2321_REC_BUFF_SIZE)
	{
			memcpy(g_usbbuf,g_wifi_send_buf,g_wifi_send_cnt);	
			g_usblen = g_wifi_send_cnt;
			g_usbFlag = 1;  	
	}		
	g_wifi_send_cnt = 0;
}

void putStr(char *send_buff,unsigned int length)
{
	int i = 0;
	wdt();
	for(i=0;i<length;i++)
	{
			g_wifi_send_buf[g_wifi_send_cnt] = send_buff[i];
			g_wifi_send_cnt++;
	} 	
}

//115100设备从单位1279移到单位4418