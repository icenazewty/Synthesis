/*!
    \file    gd32e503v_lcd_eval.c
    \brief   LCD driver functions

    \version 2020-09-04, V1.0.0, demo for GD32E50x
    \version 2021-03-31, V1.1.0, demo for GD32E50x
*/

/*
    Copyright (c) 2021, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "systick.h"
#include "wifi_ble.h"
#include "string.h"
#include "GlobalVar.h"
#include <stdio.h>
#include <stdlib.h>
#include "gd32e503v_eval.h"
#include "gprs.h"
#include "cfg_flash.h"
#include "FreeRTOS.h"
#include "task.h"

 char	g_wifi_return_status=0;
 char g_ComBuf_WB[512];
 int  g_res_WB=0;
 
void WifiBleCmdSend(char *cmd)
{		
		uint32_t len = strlen(cmd);		
		Com_Send(WIFI_BLE,(unsigned char*)cmd,len);				
}

int  CmdExe_WB(char*cmd,int mode)
{
	char	exit_mode = 0;
	char	data[256];
	int success = 0;
	int delay_ms = mode;							//默认等待100*20ms即2秒时间，如果不成功，如果mode=2则等待30秒，OK=1  ERROR=2  0=timeout
	int i=0;	
	int	dw  = 0;	
	int	cnt = 0;
	Com.Usart[WIFI_BLE].usRec_RD = Com.Usart[WIFI_BLE].usRec_WR;		//Com.Usart[WIFI_BLE].usRec_WR;	
	WifiBleCmdSend(cmd);	
	
	sprintf(data,"CmdWifi send: %s\r\n",cmd);
	InsertLog(data);

	g_res_WB = 0;
	memset(g_ComBuf_WB,0,sizeof(g_ComBuf_WB));	
	if(NULL!=strstr(cmd,"+++"))
	{
		exit_mode = 1;
	}
	
	for(i=0;i<delay_ms&&g_res_WB<512;i++)							//׮³¤ִА2ë,׮¶ནʕ512ז½ڊý¾ݡ£
	{
		wdt();
		delay_1ms(1);																	//等待30s最长
		dw = (Com.Usart[WIFI_BLE].usRec_WR - Com.Usart[WIFI_BLE].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;		
		if(dw>0)
		{
			while(Com.Usart[WIFI_BLE].usRec_RD!=Com.Usart[WIFI_BLE].usRec_WR)
			{
					cnt++;
					g_ComBuf_WB[g_res_WB++] = Com.Usart[WIFI_BLE].RX_Buf[Com.Usart[WIFI_BLE].usRec_RD];	
					Com.Usart[WIFI_BLE].usRec_RD = (Com.Usart[WIFI_BLE].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
					g_res_WB =g_res_WB%512;
			}
			g_ComBuf_WB[g_res_WB] = '\0';
			wdt();
			//¶Խӊյ½µĊý¾ݽøА´¦À𨰍
			if(1==mode&&strstr(g_ComBuf_WB,">"))
			{				
				success = 1;
				break;
			}
			else if(strstr(g_ComBuf_WB,"OK"))		//ƥŤOK»򅒒OR
			{				
				success = 1;
				break;
			}				
			else if(exit_mode && strstr(g_ComBuf_WB,"CLOSED"))		//CWJAP可能收到CLOSED命令
			{				
				success = 1;
				break;
			}				
			else if(strstr(g_ComBuf_WB,"ERROR"))
			{				
				success = -1;
				break;
			}
		}
	}//for
	
	if(g_res_WB)		//Ӑʽ¾ݍ
	{	
			g_ComBuf_WB[g_res_WB] = '\0';		
			sprintf(data,"CmdWifi rec:\r\n%s\r\n",g_ComBuf_WB);
			InsertLog(data);			
	}
	
	if(0==success)
	{
		return cnt;					//cnt可能为0,1,2...
	}
	else
	{
		return success;			
	}
}


//退出透明传输模式
unsigned char ExitPassthroughMode(void)
{
		int len  = 0;
		int state = 0;
		delay_1ms(100);	
		state = CmdExe_WB("+++",4000);						//等待1秒+20ms		
		delay_1ms(100);  										
		wdt();		
		state = CmdExe_WB("ATE0\r\n",100);				//等待1秒+20ms
		if(1==state)
		{
			return 1;
		}		
		state = CmdExe_WB("ATE0\r\n",100);				//等待1秒+20ms
		if(1==state)
		{
			return 1;
		}	
		return 0;
}

void CmdCheck(char* p)
{
	char *q = NULL;
	char data[128],flag = 0;
	int	i = 0, cnt = 0, j = 0;
	for(i = 0 ; i < 128 ; i++)
	{
		if(p[i]=='\r' && p[i+1]=='\n')
		{
			flag = 1;
			break;
		}		
	}	
	
	if(flag==1)
	{
		memcpy(data,p,i);
		data[i]='\r';
		data[i+1]='\n';
		data[i+2]='\0';				
		
		for(j = 0 ; j < i ; j++)
		{
			if(p[j]==',')
			{
				cnt++;
			}
		}
		
		if(cnt==5)		//5个,格式判断
		{
			p = strstr(data,",\"");	
			q = strstr(data,"\",");	
			if(q-p > 0)
			{
				memcpy(gs_WifiBleAtCmd.ucCmdName[gs_WifiBleAtCmd.ucActiveCount],p+2,q-p-2);
				gs_WifiBleAtCmd.ucCmdName[gs_WifiBleAtCmd.ucActiveCount][q-p-2] = '\0';				
				if(q[2]=='0'||q[2]=='1')
				{					
					gs_WifiBleAtCmd.ucTest[gs_WifiBleAtCmd.ucActiveCount] = q[2] - '0';		
				}
				else
				{
					return ;
				}
				
				if(q[4]=='0'||q[4]=='1')
				{					
					gs_WifiBleAtCmd.ucQuery[gs_WifiBleAtCmd.ucActiveCount] = q[4] - '0';	
				}
				else
				{
					return ;
				}
				
				if(q[6]=='0'||q[6]=='1')
				{					
					gs_WifiBleAtCmd.ucSet[gs_WifiBleAtCmd.ucActiveCount] = q[6] - '0';		
				}
				else
				{
					return ;
				}
				
				if(q[8]=='0'||q[8]=='1')
				{					
					gs_WifiBleAtCmd.ucExecute[gs_WifiBleAtCmd.ucActiveCount] = q[8] - '0';	  
				}
				else
				{
					return ;
				}				
				//printf("\r\n %03d %s",gs_WifiBleAtCmd.ucActiveCount+1,gs_WifiBleAtCmd.ucCmdName[gs_WifiBleAtCmd.ucActiveCount]);
				gs_WifiBleAtCmd.ucActiveCount++;
			}
		}				
	}	
}

void AllCmdPrint(void)
{
	int i = 0 ,j = 0;
	printf("\r\n Cmd count is %d", gs_WifiBleAtCmd.ucActiveCount);
	for(i = 0; i < gs_WifiBleAtCmd.ucActiveCount ; i++)
	{
	//	printf("\r\n %03d %s",i+1 , gs_WifiBleAtCmd.ucCmdName[i]);
	}	
	
	printf("\r\n support test commad  \r\n001\t");
	for(i = 0; i < gs_WifiBleAtCmd.ucActiveCount ; i++)
	{
		j++;
		printf("%d  ",gs_WifiBleAtCmd.ucTest[i]);
		if(j%20==0)
		{
			printf("\r\n%03d\t",j);
		}
	}	
	
	j = 0;
	printf("\r\n support query commad  \r\n001\t");
	for(i = 0; i < gs_WifiBleAtCmd.ucActiveCount ; i++)
	{
		j++;
		printf("%d  ",gs_WifiBleAtCmd.ucQuery[i]);
		if(j%20==0)
		{
			printf("\r\n%03d\t",j);
		}
	}	
	
	j = 0;
	printf("\r\n support set commad  \r\n001\t");
	for(i = 0; i < gs_WifiBleAtCmd.ucActiveCount ; i++)
	{
		j++;
		printf("%d  ",gs_WifiBleAtCmd.ucSet[i]);
		if(j%20==0)
		{
			printf("\r\n%03d\t",j);
		}
	}	
	
	j = 0;
	printf("\r\n support execute commad  \r\n001\t");
	for(i = 0; i < gs_WifiBleAtCmd.ucActiveCount ; i++)
	{
		j++;
		printf("%d  ",gs_WifiBleAtCmd.ucExecute[i]);
		if(j%20==0)
		{
			printf("\r\n%03d\t",j);
		}
	}	
}

//获取所有命令
void GetAtCmd(void)
{
		char *p = NULL;	
		int len = 0, i = 0 , flag = 0, pos = 0, start = 0;
		Com.Usart[WIFI_BLE].usRec_WR = 0;
	
		WifiBleCmdSend("AT+CMD?\r\n");		
				
		Com.Usart[WIFI_BLE].usRec_RD = 0;
    
		delay_1ms(500);
		len = Com.Usart[WIFI_BLE].usRec_WR;							//4K字节		10KB/S
		if( len > 128)
		{
			for(i=0;i<len;i++)
			{
				if(Com.Usart[WIFI_BLE].RX_Buf[i]=='O' && Com.Usart[WIFI_BLE].RX_Buf[i+1]=='K')
				{
						flag = 1;
						usart_receive_config(COM_EVAL[WIFI_BLE], USART_RECEIVE_DISABLE);
						break;
				}
			}			
		}	
		
		if(flag==0)
		{
			delay_1ms(300);
		}
		
		len = Com.Usart[WIFI_BLE].usRec_WR;							//4K字节		10KB/S
		if(flag==0 && len > 128)
		{
			for(i=0;i<len;i++)
			{
				if(Com.Usart[WIFI_BLE].RX_Buf[i]=='O' && Com.Usart[WIFI_BLE].RX_Buf[i+1]=='K')
				{
						flag = 1;
						usart_receive_config(COM_EVAL[WIFI_BLE], USART_RECEIVE_DISABLE);
						break;
				}
			}			
		}
			
		if(1==flag)
		{
				//data process
				len = Com.Usart[WIFI_BLE].usRec_WR;							//4K字节		10KB/S
				Com.Usart[WIFI_BLE].RX_Buf[len] = '\0';
				p = (char*)Com.Usart[WIFI_BLE].RX_Buf;
				//printf("%s",p);
				gs_WifiBleAtCmd.ucActiveCount = 0;
				for(i=0;i<ATCMDCOUNT;i++)
				{
						p = strstr(p,"+CMD:");
						if(p)
						{							
							p = p + 5;
							CmdCheck(p);
						}						
						else
						{
							break;
						}
				}				
				usart_receive_config(COM_EVAL[WIFI_BLE], USART_RECEIVE_ENABLE);
				AllCmdPrint();
		}		
}

void AllQueryCmdPrint(void)
{
	char data[32];
	int i = 0, j = 0, len = 0;	
	printf("\r\n+++++++++++++++ All Query Cmd Print ++++++++++++\r\n");
	for(i = 0; i < gs_WifiBleAtCmd.ucActiveCount ; i++)
	{
		if(gs_WifiBleAtCmd.ucQuery[i])
		{
			if(strcmp((char*)gs_WifiBleAtCmd.ucCmdName[i],"AT+CMD")!=0)
			{
				j++;
				strcpy(data,(char*)gs_WifiBleAtCmd.ucCmdName[i]);
				len = strlen((char*)gs_WifiBleAtCmd.ucCmdName[i]);
				data[len++] = '?';
				data[len++] = '\r';
				data[len++] = '\n';
				data[len++] = '\0';
				Com.Usart[WIFI_BLE].usRec_WR = 0;
				WifiBleCmdSend(data);
				printf("+++++++++++++++++++++++++++%03d %03d %s",j,i+1,data);
				
				if(strcmp((char*)gs_WifiBleAtCmd.ucCmdName[i],"AT+SYSFLASH")==0) 
					delay_1ms(80);
				else
					delay_1ms(8);
				
				len = Com.Usart[WIFI_BLE].usRec_WR;
				if(len>0)
				{				
					Com.Usart[WIFI_BLE].RX_Buf[len] = '\0';
					printf("%s",Com.Usart[WIFI_BLE].RX_Buf);
				}
			}
		}		
	}	
}

void AllExecuteCmdPrint(unsigned char mode)
{
	unsigned char *p = NULL;
	char data[32];
	int i = 0, j = 0, len = 0;	
	printf("\r\n+++++++++++++++ All ");
	if(mode==0)
	{
		printf("Test");
		p = gs_WifiBleAtCmd.ucTest;
	}
	else if(mode==1)
	{
		printf("Query");
		p = gs_WifiBleAtCmd.ucQuery;
	}
	else if(mode==2)
	{
		printf("Set");
		p = gs_WifiBleAtCmd.ucSet;
	}
	else if(mode==3)
	{
		printf("Execute");
		p = gs_WifiBleAtCmd.ucExecute;
	}
	
	printf(" Cmd Print ++++++++++++\r\n");
	for(i = 0; i < gs_WifiBleAtCmd.ucActiveCount ; i++)
	{
		if(p[i])
		{			
				j++;
				strcpy(data,(char*)gs_WifiBleAtCmd.ucCmdName[i]);
				len = strlen((char*)gs_WifiBleAtCmd.ucCmdName[i]);
				//data[len++] = '?';
				data[len++] = '\r';
				data[len++] = '\n';
				data[len++] = '\0';
				//Com.Usart[WIFI_BLE].usRec_WR = 0;
				//WifiBleCmdSend(data);
				printf("%03d %03d %s",j,i+1,data);			
				//delay_1ms(8);				
				//len = Com.Usart[WIFI_BLE].usRec_WR;
				//if(len>0)
				{				
				//	Com.Usart[WIFI_BLE].RX_Buf[len] = '\0';
				//	printf("%s",Com.Usart[WIFI_BLE].RX_Buf);
				}			
		}		
	}	
}
	
void TurnOffAte(unsigned char sta)
{
	delay_1ms(5);
	WifiBleCmdSend("AT\r\n");
	delay_1ms(5);
	WifiBleCmdSend("AT\r\n");
	delay_1ms(10);
	if(sta)
	{
		WifiBleCmdSend("ATE0\r\n");
	}
	else
	{
		WifiBleCmdSend("ATE1\r\n");
	}
	delay_1ms(5);
}

//dataType=1表示字符串
unsigned char getSpecData(char *str,int dataType,char*activeData,int *activeLen,char pos)	//第几个字段
{	
	char Flag = 0;
	char *p = strstr(g_ComBuf_WB,str);
	int  len = strlen(str),i = 0;	
	int  dot = 0;
	g_ComBuf_WB[g_res_WB] = '\0';	
	if(p)
	{
		p = p+len;
		if(1==dataType)	//string
		{
			if(p[0]==':')
			{
				p++;
				if(0==pos && *p=='\"')
				{
					p++;
					Flag = 1;
				}
				else	//寻找,
				{
					while(p)
					{
						if(','==*p)
						{
							dot++;
							if(dot==pos)
							{
								break;
							}
						}	
						p++;						
					}
					if(dot==pos && '\"'==p[1])
					{
						p+=2;
						Flag = 1;
					}
				}
				
				if(1==Flag)
				{
					char *q = strstr(p,"\"");
					if(q)
					{
						*activeLen = q - p;
						memcpy(activeData,p,*activeLen);
						activeData[*activeLen] = '\0';
						return 1;
					}	
				}					
			}			
		}
		else if(2==dataType)	//num
		{
			//+CIPSTATE:0,"TCP","139.129.17.114",7106,0,0
			if(p[0]==':')
			{
					p++;	
					if(0==pos)
					{
						Flag = 1;
					}
					else
					{
						while(p)
						{
							if(','==*p)
							{
								dot++;
								if(dot==pos)
								{
									p++;
									Flag = 1;
									break;
								}
							}	
							p++;						
						}						
					}
					if(Flag)
					{
						i = p - g_ComBuf_WB;
						len = 0;
						for(;i<g_res_WB;i++)
						{
							if('-'==g_ComBuf_WB[i] || (g_ComBuf_WB[i]>0x2f && g_ComBuf_WB[i]<0x3a))
							{
								activeData[len++] = g_ComBuf_WB[i];
							}
							else
							{
								*activeLen = len;
								return 1;
							}							
						}					
					}
			}//:			
		}//num
	}
	return 0;
}
	

void GetWifiMac()
{
	//+CIPSTAMAC:"84:f7:03:5f:c6:1c"

	//OK
	char data[128];
	int  datalen = 0;
	int i  = 0;
	unsigned char abc = 0;	
	unsigned char state = getSpecData("+CIPSTAMAC",1,data,&datalen,0);		
	 
	if(state)
	{
		if(datalen==17)				//001ffab55aec
		{
				for(i=0;i<6;i++)
				{
						if(data[3*i]>0x2f&&data[3*i]<0x3a)
						{
								abc = ((data[3*i] - 0x30)<<4);
						}
						else if(data[3*i]>='a'&&data[3*i]<='f')
						{
								abc = ((data[3*i] - 'a' + 10)<<4);
						}
						else if(data[3*i]>='A'&&data[3*i]<='F')
						{
								abc = ((data[3*i] - 'A' + 10)<<4);
						}
						else
						{

						}
						
						if(data[3*i+1]>0x2f&&data[3*i+1]<0x3a)
						{
								abc += ((data[3*i+1] - 0x30));
						}
						else if(data[3*i+1]>='a'&&data[3*i+1]<='f')
						{
								abc += ((data[3*i+1] - 'a' + 10));
						}
						else if(data[3*i+1]>='A'&&data[3*i+1]<='F')
						{
								abc += ((data[3*i+1] - 'A' + 10));
						}
						else
						{

						}						
						gs_SaveWifiCfg.ucMAC[i] = abc;
				}
		}		
	}		
}

unsigned int ToIp(char*data, char datalen,unsigned char*ipdata)
{
		char res[4][4];
		int i  = 0,j = 0 ,k = 0;
		unsigned char abc = 0;	
		if(datalen>6 && datalen<16)				//001ffab55aec  1.1.1.1
		{
				for(i=0;i<datalen;i++)
				{
					if(data[i]=='.')
					{
						res[j][k]='\0';
						j++;
						k = 0;
					}
					else
					{
						res[j][k++]=data[i];
					}					
				}
		}
		if(j==3)
		{
			res[j][k]='\0';
			ipdata[0] = atoi(res[0]);
			ipdata[1] = atoi(res[1]);
			ipdata[2] = atoi(res[2]);
			ipdata[3] = atoi(res[3]);			
			return 1;
		}
		return 0;
}

unsigned char GetNIP(char *str,unsigned char*ipdata)
{
//+CIPSTA:ip:"192.168.0.110"
//+CIPSTA:gateway:"192.168.0.1"
//+CIPSTA:netmask:"255.255.255.0"
	
	char res[4][4];
	char data[128];
	int  datalen = 0;
	int i  = 0,j = 0 ,k = 0;
	unsigned char abc = 0;	
	unsigned char state = getSpecData(str,1,data,&datalen,0);		
	
	if(state)
	{
		if(ToIp(data,datalen,ipdata))
			return 1;
	}		
	return 0;
}

unsigned char GetDhcp()
{
//+CWDHCP:3
//OK		
	char data[128];
	int  datalen = 0;
	int i  = 0,j = 0 ,k = 0;
	unsigned char abc = 0;	
	unsigned char state = getSpecData("+CWDHCP",2,data,&datalen,0);		
	
	if(state)
	{
		if(datalen>0)
		{
				data[datalen] = '\0';
				abc = atoi(data);
				return abc;
		}			
	}		
	return 0;
}

unsigned char GetRssi()
{
//AT+CWJAP	AT+CWJAP	连接 AP
//+CWJAP:"BST","88:25:93:8f:04:73",11,  -44,     0,       1,                 3,                0,        0
//			                           ch	 rssi   pci_en  reconn_interval    listen_interval    scan_mode   pmf
//OK
	char data[128];
	int  datalen = 0;	
	unsigned char abc = 0;		
	unsigned char state = getSpecData("+CWJAP",2,data,&datalen,3);	
	if(state)
	{
		if(datalen>0)
		{
				data[datalen] = '\0';
				datalen = atoi(data);
				if(datalen<0)
				{
					datalen*=-1;
				}
				gs_SaveWifiCfg.ucWifiSingal =  datalen&0xff;	 	
				return 1;
		}			
	}	
	else
	{
			gs_SaveWifiCfg.ucWifiSingal =  0;	 
	}
	return 0;
}

//0表示不成功  1=表示成功且ssid不为""   2=表示成功且ssid为""
unsigned char GetSsid()
{
//+CWSTATE:2,"BST"
//OK		
	char data[128];
	int  datalen = 0;
	int i  = 0,j = 0 ,k = 0;
	unsigned char abc = 0;		
	unsigned char state = getSpecData("+CWSTATE",2,data,&datalen,0);	
	
	if(state)
	{
		if(datalen>0)
		{
				data[datalen] = '\0';
				gs_SaveWifiCfg.ucWifiState = atoi(data);				
				//<state>：当前 Wi-Fi 状态
				//– 0: ESP station 尚未进行任何 Wi-Fi 连接
				//– 1: ESP station 已经连接上 AP，但尚未获取到 IPv4 地址
				//– 2: ESP station 已经连接上 AP，并已经获取到 IPv4 地址
				//– 3: ESP station 正在进行 Wi-Fi 连接或 Wi-Fi 重连
				//– 4: ESP station 处于 Wi-Fi 断开状态
		}			
	}	
	else
	{
		
	}
	
	state = getSpecData("+CWSTATE",1,data,&datalen,1);			//实际位置为3-1=2 字符串
	if(state)
	{
		if(datalen>0)
		{
				data[datalen] = '\0';
				//memset(gs_SaveWifiCfg.ucSSID_PWD,0,37);			
				memcpy(gs_SaveWifiCfg.ucSSID_PWD,data,datalen);
				gs_SaveWifiCfg.ucSSID_PWD[datalen] = '\0';
				return 1;
		}			
		else
		{
				memset(gs_SaveWifiCfg.ucSSID_PWD,0,37);				
				return 2;
		}
	}		
	return 0;
}

//ucConnState
unsigned char GetTdInfo(unsigned char ch)
{
//STATUS:3
//+CIPSTATUS:0,"TCP","139.129.17.114",7106,0,0	
	
	unsigned char ipdata[4];
	char data[128];
	int  datalen = 0;
	
	unsigned char state = getSpecData("STATUS",2,data,&datalen,0);		//最后一个2表示位置2 即ip地址
	if(state)
	{
		if(datalen>0)
		{
				data[datalen] = '\0';				
				gs_SaveWifiCfg.TdInfo[ch].ucConnState = atoi(data); 				//3  已建立 TCP、UDP 或 SSL 传输
				//• <stat>：ESP station 接口的状态
				//– 0: ESP station 为未初始化状态
				//– 1: ESP station 为已初始化状态，但还未开始 Wi-Fi 连接
				//– 2: ESP station 已连接 AP，获得 IP 地址
				//– 3: ESP station 已建立 TCP、UDP 或 SSL 传输
				//– 4: ESP 设备所有的 TCP、UDP 和 SSL 均断开
				//– 5: ESP station 开始过 Wi-Fi 连接，但尚未连接上 AP 或从 AP 断开
		}			
	}	
	
	state = getSpecData("+CIPSTATUS",1,data,&datalen,2);		//最后一个2表示位置2 即ip地址
	if(state)
	{
		if(datalen>0)
		{
				data[datalen] = '\0';
				if(ToIp(data,datalen,ipdata))
				{
						memcpy(gs_SaveWifiCfg.TdInfo[ch].ucDestIP,ipdata,4); 
				}							
		}			
	}	
	
	state = getSpecData("+CIPSTATUS",2,data,&datalen,3);		//2表示数字类型，3表示位置3
	if(state)
	{
		if(datalen>0)
		{
				data[datalen] = '\0';
				unsigned int port = atoi(data);
				gs_SaveWifiCfg.TdInfo[ch].ucRemotePort[0]	= (port>>8)&0xff;
				gs_SaveWifiCfg.TdInfo[ch].ucRemotePort[1]	=  port&0xff;				
		}			
	}	
	
	state = getSpecData("+CIPSTATUS",2,data,&datalen,4);		//2表示数字类型，4表示位置4
	if(state)
	{
		if(datalen>0)
		{
				data[datalen] = '\0';
				unsigned int port = atoi(data);
				gs_SaveWifiCfg.TdInfo[ch].ucLocalPort[0]	= (port>>8)&0xff;
				gs_SaveWifiCfg.TdInfo[ch].ucLocalPort[1]	=  port&0xff;				
			
				gs_SaveWifiCfg.TdInfo[ch].ucProtocol = 0;
				gs_SaveWifiCfg.TdInfo[ch].ucCSMode = 0;
				return 1;
		}			
	}			
	return 0;
}

int GetWifiMode()
{
	//+CWMODE:1
	//OK		
	char data[128];
	int  datalen = 0;
	int i  = 0,j = 0 ,k = 0;
	unsigned char abc = 0;	
	unsigned char state = getSpecData("+CWMODE",2,data,&datalen,0);		
	
	if(state)
	{
		if(datalen>0)
		{
				data[datalen] = '\0';
				abc = atoi(data);
				return abc;
		}			
	}		
	return -1;
}

//获取wifi工作模式  -1表获取不成功，否则成功
int WifiMode()
{	
	if(1==CmdExe_WB("AT+CWMODE?\r\n",100))							//查询/设置 Wi-Fi 模式 (Station/SoftAP/Station+SoftAP)
	{
			int i = GetWifiMode();								
			//– 0: 无 Wi-Fi 模式，并且关闭 Wi-Fi RF
			//– 1: Station 模式
			//– 2: SoftAP 模式
			//– 3: SoftAP+Station 模
			if(-1==i)		//获取失败
			{
				return -1;								//0=infra,1=adhoc,2=ap    3=ap+infra					
			}
			else
			{
				gs_SaveWifiCfg.ucWorkMode = i&0x3;						//0=infra,1=adhoc,2=ap    3=ap+infra	
				//if(1==i)
				//{
				//	gs_SaveWifiCfg.ucWorkMode = 0;							//0=infra,1=adhoc,2=ap    3=ap+infra					
				//}
				return i;
			}			
	}
	else
	{			
			return -1;		//不成功
	}			
}	

unsigned char strIP2ip(char *str,int len,unsigned char*DestIP)
{
	int port=0,j=0,i=0;
	DestIP[port] = 0;					
	for(i=0;i<len;i++)
	{
		if(str[i]=='.')
		{
				port++;
				DestIP[port] = 0;				
		}
		else if(str[i]>0x2f && str[i]<0x3a)
		{
				DestIP[port] = DestIP[port]*10 + (str[i] - 0x030);  
		}
		else
		{
			return 0;
		}
	}		
	
	if(3==port)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
//wifi参数配置
unsigned char CfgWifiPara(void)
{
	  int  num = 0;
		int  datalen = 0;
		int  sta = 0;
		unsigned char dstip[4];
		char ip[36];
		char port[8];
		char cmd[128] = {"AT+SAVETRANSLINK=1,\"139.129.17.114\",7106,\"TCP\",15\r\n"};
		unsigned int tim;
		////////////////////进入at命令模式////////////////////////////////////////////////////
		if(1==CmdExe_WB("AT\r\n",100))				//at测试
		{
										
		}
		else if(1==CmdExe_WB("AT\r\n",100))				//at测试
		{					
				
		}
		else if(1==ExitPassthroughMode())				//退出数据模式进入命令模式
		{
			
		}
		else if(1==ExitPassthroughMode())		//退出数据模式进入命令模式
		{
			
		}
		else
		{				
				return 0;
		}					
	
		
		if(1 != CmdExe_WB("ATE0\r\n",100))							//等待1秒+20ms
		{
			 CmdExe_WB("ATE0\r\n",100);
		}
		
		if(1==CmdExe_WB("AT+SYSSTORE?\r\n",100))				//参数可以保存到flash
		{
				if(1 == getSpecData("+SYSSTORE",2,cmd,&datalen,0))				
				{
						cmd[datalen] = '\0';
						sta = atoi(cmd);
						if(1!=sta)
						{
							if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
							else if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
						}
				}
		}
		else if(1==CmdExe_WB("AT+SYSSTORE?\r\n",100))				//参数可以保存到flash
		{
				if(1 == getSpecData("+SYSSTORE",2,cmd,&datalen,0))				
				{
						cmd[datalen] = '\0';
						sta = atoi(cmd);
						if(1!=sta)
						{
							if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
							else if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
						}
				}
		}
		else
		{
				return 0;
		}
		
		////////////////////进入at命令模式////////////////////////////////////////////////////
		if(1==CmdExe_WB("AT+CWMODE=1,1\r\n",100))							//配置为station模式，并且自动连接AP
		{
								
		}
		else
		{
				return 0;
		}
		
		//老的模块2M的  默认不是上电自动连接，并且貌似在静态IP模式下，CWMODE命令不能修改CWAUTOCONN的参数。而新编译的程序，默认上电自动连接
		CmdExe_WB("AT+CWAUTOCONN=1\r\n",100);										//上电自动连接AP
						
		if(gs_SaveWifiCfg.ucDhcp==0)
		{
				sta =  CmdExe_WB("AT+CWDHCP=0,1\r\n",100);
		}
		else
		{
				sta =  CmdExe_WB("AT+CWDHCP=1,1\r\n",100);
		}
		
		if(0==sta)
			return 0;
		
		if(gs_SaveWifiCfg.ucDhcp==0)		//静态IP地址，对wifi进行IP配置
		{
			//AT+CIPSTA=<"ip">[,<"gateway">,<"netmask">]
			sprintf(cmd,"AT+CIPSTA=\"%d.%d.%d.%d\",\"%d.%d.%d.%d\",\"%d.%d.%d.%d\"\r\n",gs_SaveWifiCfg.ucSelfIP[0],gs_SaveWifiCfg.ucSelfIP[1],gs_SaveWifiCfg.ucSelfIP[2],gs_SaveWifiCfg.ucSelfIP[3],			gs_SaveWifiCfg.ucGateWay[0],gs_SaveWifiCfg.ucGateWay[1],gs_SaveWifiCfg.ucGateWay[2],gs_SaveWifiCfg.ucGateWay[3],gs_SaveWifiCfg.ucSubMASK[0],gs_SaveWifiCfg.ucSubMASK[1],gs_SaveWifiCfg.ucSubMASK[2],gs_SaveWifiCfg.ucSubMASK[3]);		
			if(1!=CmdExe_WB(cmd,200))
			{
					return 0;
			}			
		}	
		
		//AT+CWJAP="BST","BST110426"				
		
		for(sta=0;sta<36;sta++)
		{
			if(gs_SaveWifiCfg.ucSSID_PWD[sta]=='\0')
			{
				if(gs_SaveWifiCfg.ucSSID_PWD[sta+1]=='0')
				{
					sprintf(cmd,"AT+CWJAP=\"%s\",",gs_SaveWifiCfg.ucSSID_PWD);		
				}
				else
				{
					sta++;
					strcpy(ip,(char *)&gs_SaveWifiCfg.ucSSID_PWD[sta]);
					sprintf(cmd,"AT+CWJAP=\"%s\",\"%s\"",gs_SaveWifiCfg.ucSSID_PWD,ip);		
				}				
				break;
			}
		}
		strcat(cmd,",,,5,,1,5\r\n");
		//AT+CWJAP="BST","BST110426","88:25:93:8f:04:73",   0,            5,               3,               1,          5,          0
		//	        ssid     pwd            bssid         pci_en  reconn_interval    listen_interval    scan_mode   jap_timeout     pmf
		//AT+CWJAP="YZZ12231","ismartdog.com",,,5,,1,10
		tim = g_sysTick;
		if(1==CmdExe_WB(cmd,5200))				//ap连接  最长10秒
		{
				tim = 	g_sysTick - tim;	
				sprintf(cmd,"ap connect success . time = %ds\r\n",tim);									
				InsertLog(cmd);			
		}
		else
		{
				tim = 	g_sysTick - tim;
				sprintf(cmd,"ap connect faild . time = %ds\r\n",tim);									
				InsertLog(cmd);			
				//return 0;
		}	
		
		if(1==CmdExe_WB("AT+CWSTATE?\r\n",100))		//查询Wi-Fi状态和Wi-Fi 信息,连接状态
		{			
				sta = GetSsid();							
				if(2==sta)				//ssid为空  不能在进行任何操作了。表示ssid从未配置过
				{
						//return 1;
				}
				else if(0==sta)		//0不成功
				{
						//return 0;
				}
				else							//1成功
				{
						
				}
		}
		else
		{					
				//return 0;	
		}		
				
		//if(2==gs_SaveWifiCfg.ucWifiState)												//wifi已连接成功,不管wifi是否连接成功，都需要执行 开机自动连接tcp
		{		
			memcpy(ip,g_configRead.remoteIP,g_configRead.IPLen);
			ip[g_configRead.IPLen] = '\0';		
			memcpy(port,g_configRead.remotePort,g_configRead.PortLen);
			port[g_configRead.PortLen] = '\0';
		
			sprintf(cmd,"AT+SAVETRANSLINK=1,\"%s\",%s,\"TCP\",60\r\n",ip,port);
			if(1==CmdExe_WB(cmd,200))							//
			{
				if(strIP2ip(ip,g_configRead.IPLen,dstip))
				{
					memcpy(gs_SaveWifiCfg.TdInfo[1].ucDestIP,dstip,4);						
					num = atoi(port);
					gs_SaveWifiCfg.TdInfo[1].ucRemotePort[0]	= (num>>8)&0xff;		
					gs_SaveWifiCfg.TdInfo[1].ucRemotePort[1]	=  num&0xff;				
				}				
				//直接连接，如果连接成功，则执行  CIPSEND。
				sta = CmdExe_WB("AT+CIPSEND\r\n",100);			
				if(1==sta)						//进入透传模式
				{
					return 1;	
				}
				else //if(-1==sta)			//error
				{
					if(1==CmdExe_WB("AT+RST\r\n",100))
					{
						 	wifi_rev_cnt = 0;
							wifi_reset_sta = 0;
							wifi_reset_tick = g_sysTick;
							return 1;	
					}
					else
						return 0;
				}
			}
		}		
		return 1;
}

//读wifi参数
unsigned char ReadWifiPara(unsigned char mode)							
{			
	  unsigned char ipdata[4];
		char 					res = 0;		
		int 					i=0;		
		//进入模式+++
		//确认是否进入	AT+
		//读如下参数:
		//AT+QMAC=?			AT+CIPSTAMAC						读MAC地址	
		//AT+NIP=?			AT+CWDHCP		AT+CIPSTA		读DHCP还是静态IP,IP,submask,gateway,dns		
		//AT+WPRT=?			AT+CWMODE								读工作模式（0=infra,1=adhoc,2=ap）
	
		//AT+ENCRY=?			安全模式 0=OPEN  1=WEP64  2=WEP128   3=WPA-PSK(TKIP)			4=WPA-PSK(CCMP/AES)	 5=WPA2-PSK(TKIP)  6=WPA2-PSK(CCMP/AES)	7=WPA1PSK/WPA2PSK(AUTO)
		//AT+SSID=?				读SSID
		//AT+KEY=?				读密码 读加密方式	+OK=1,0,"ismartdog.com"			第1个字段:0=HEX  1=ASCII;  第2个字段(密钥索引号):0=HEX  1=ASCII    
		
		//AT+ATRM=?				读透明传输网络信息	AT+ATRM=!0,0,192.168.2.2,7107,7106
		//AT+Z或AT+ENTM		重启wifi或退出透明模式	
		g_wifi_return_status = 0;
		if(1==CmdExe_WB("AT\r\n",100))				//at测试
		{
				
		}
		else if(1==CmdExe_WB("AT\r\n",100))		//at测试
		{					
				
		}
		else if(1==ExitPassthroughMode())		//退出数据模式进入命令模式
		{
			
		}
		else if(1==ExitPassthroughMode())		//退出数据模式进入命令模式
		{
			
		}
		else
		{				
				return 0;
		}		
		
		if(1 != CmdExe_WB("ATE0\r\n",100))				//等待1秒+20ms
		{
			 CmdExe_WB("ATE0\r\n",100);
		}
			
		wdt();							
		if(1==CmdExe_WB("AT+CIPSTAMAC?\r\n",100))		//mac查询
		{
				GetWifiMac();								
		}
		else
		{	
				return 0;				
		}
				
		if(1==CmdExe_WB("AT+CWDHCP?\r\n",100))				//dhcp查询
		{					
				gs_SaveWifiCfg.ucDhcp = GetDhcp()&0x01;								
		}
		else
		{						
				return 0;
		}
		
	//	for(i=0;i<4;i++)
  //  {
  //     gs_SaveWifiCfg.ucSelfIP[i] = 1;
  //      gs_SaveWifiCfg.ucSubMASK[i] = 1;
  //      gs_SaveWifiCfg.ucGateWay[i] = 1;
  //      gs_SaveWifiCfg.ucDNS[i] = 1;
  //  }		
		
		if(1==CmdExe_WB("AT+CIPSTA?\r\n",100))			//ip查询
		{
			if(1==GetNIP("ip",ipdata))
			{
				memcpy( gs_SaveWifiCfg.ucSelfIP,ipdata,4);				
			}				
			
			if(1==GetNIP("gateway",ipdata))
			{
				memcpy( gs_SaveWifiCfg.ucGateWay,ipdata,4);				
			}	
			
			if(1==GetNIP("netmask",ipdata))
			{
				memcpy( gs_SaveWifiCfg.ucSubMASK,ipdata,4);				
			}			
		}
		else
		{						
				return 0;				
		}
		
		if(1==CmdExe_WB("AT+CWSTATE?\r\n",100))		//查询Wi-Fi状态和Wi-Fi 信息,连接状态
		{			
				GetSsid();														//gs_SaveWifiCfg.ucWifiState = 2表示wifi已连接	;	0尚未进行任何 Wi-Fi 连接
		}
		else
		{						
				return 0;	
		}
		
		if(1==CmdExe_WB("AT+CWJAP?\r\n",100))			//查询Wi-Fi 连接相关信息
		{			
				GetRssi();							
		}
		else
		{						
				return 0;	
		}
		
		i = WifiMode();	
		if(-1==i)
		{
			i = WifiMode();	
		}
		
		//AT+CWSTAPROTO  b/g/n	
		gs_SaveWifiCfg.ucEncryptMode = 7;												//加密方式0-7		
		wdt();	
		
		if(1==CmdExe_WB("AT+CIPSTATUS\r\n",100))									//连接 socket  信息
		{
				GetTdInfo(1);																					//实际情况,需要读出 所有信息,如果不是tcp client模式，并且上传ip等不符合配置参数则进行 重新设置
				//gs_SaveWifiCfg.TdInfo[ch].ucConnState = 3 				  已建立 TCP、UDP 或 SSL 传输
				//• <stat>：ESP station 接口的状态
				//– 0: ESP station 为未初始化状态
				//– 1: ESP station 为已初始化状态，但还未开始 Wi-Fi 连接
				//– 2: ESP station 已连接 AP，获得 IP 地址
				//– 3: ESP station 已建立 TCP、UDP 或 SSL 传输
				//– 4: ESP 设备所有的 TCP、UDP 和 SSL 均断开
				//– 5: ESP station 开始过 Wi-Fi 连接，但尚未连接上 AP 或从 AP 断开
		}
		else
		{				
				return 0;
		}		
	
		if(1==CmdExe_WB("AT+CIPSTATE?\r\n",100))				//连接 AP  信息
		{
				
		}
		else
		{			
				return 0;
		}
		
		if(2==gs_SaveWifiCfg.ucWifiState)									//wifi已连接,wifi可能没有连接，但一会wifi准备好，就可以连接了。
		{
			//if(3==gs_SaveWifiCfg.TdInfo[1].ucConnState)		//通道已连接
			{
				int sta = CmdExe_WB("AT+CIPSEND\r\n",100);		//重新进入透传模式			
				if(-1==sta)			//error
				{
					if(1==CmdExe_WB("AT+RST\r\n",100))
					{
							wifi_rev_cnt = 0;
							wifi_reset_sta = 0;
							wifi_reset_tick = g_sysTick;
							return 1;	
					}
					else
						return 0;
				}		
			}			
		}
		return 1;
}

//开机后尝试进行1次连接。如果模块为新的则进行必要的设置，否则退出
unsigned char wifi_conn(void)
{
		unsigned char dstip[4];
		int num = 0;
		unsigned char state = 0;		
		int datalen = 0;
		int sta = 0;
		char ip[24];
		char port[8];
		char cmd[128] = {"AT+SAVETRANSLINK=1,\"139.129.17.114\",7106,\"TCP\",15\r\n"};
		if(1==CmdExe_WB("AT\r\n",100))									//at测试
		{
				
		}
		else if(1==CmdExe_WB("AT\r\n",100))							//at测试
		{
				
		}
		else
		{
				return 1;
		}
		//上电后，执行1次
		if(1 != CmdExe_WB("ATE0\r\n",100))							//等待1秒+20ms
		{
			 CmdExe_WB("ATE0\r\n",100);
		}
		
		if(1==CmdExe_WB("AT+SYSSTORE?\r\n",100))				//配置为station模式，并且自动连接AP
		{
				if(1 == getSpecData("+SYSSTORE",2,cmd,&datalen,0))				
				{
						cmd[datalen] = '\0';
						sta = atoi(cmd);
						if(1!=sta)
						{
							if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
							else if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
						}
				}
		}
		else if(1==CmdExe_WB("AT+SYSSTORE?\r\n",100))				//配置为station模式，并且自动连接AP
		{
				if(1 == getSpecData("+SYSSTORE",2,cmd,&datalen,0))				
				{
						cmd[datalen] = '\0';
						sta = atoi(cmd);
						if(1!=sta)
						{
							if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
							else if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
						}
				}
		}
		else
		{
				return 0;
		}
		
		sta = WifiMode();	
		if(-1==sta)
		{
			sta = WifiMode();	
		}
		
		if(-1!=sta)
		{
			if(sta&0x1)
			{
			
			}
			else if(1==CmdExe_WB("AT+CWMODE=1,1\r\n",100))				//配置为station模式，并且自动连接AP
			{
								
			}
			else
			{
					return 0;
			}			
		}
		else
		{
			return 0;
		}		
		
		if(1==CmdExe_WB("AT+CWSTATE?\r\n",100))		//查询Wi-Fi状态和Wi-Fi 信息,连接状态
		{			
				sta = GetSsid();							
				if(2==sta)						//2  ssid为空  不能在进行任何操作了。表示ssid从未配置过
				{
						return 1;
				}
				else if(0==sta)				//0不成功  读命令不成功，可能性没有。
				{
						return 0;
				}
				else									//1成功
				{
						
				}
		}
		else
		{						
				return 0;	
		}
					
		//wifi连接可能成功，也可能不成功，但wifi参数配置过。也许路由器一会开机，则就可以连接。
		
		//if(2==gs_SaveWifiCfg.ucWifiState)
		{		
			memcpy(ip,g_configRead.remoteIP,g_configRead.IPLen);
			ip[g_configRead.IPLen] = '\0';		
			memcpy(port,g_configRead.remotePort,g_configRead.PortLen);
			port[g_configRead.PortLen] = '\0';
		
			sprintf(cmd,"AT+SAVETRANSLINK=1,\"%s\",%s,\"TCP\",60\r\n",ip,port);
			if(1==CmdExe_WB(cmd,200))				//
			{
				if(strIP2ip(ip,g_configRead.IPLen,dstip))
				{
					memcpy(gs_SaveWifiCfg.TdInfo[1].ucDestIP,dstip,4);						
					num = atoi(port);
					gs_SaveWifiCfg.TdInfo[1].ucRemotePort[0]	= (num>>8)&0xff;		
					gs_SaveWifiCfg.TdInfo[1].ucRemotePort[1]	=  num&0xff;			
					WriteFlashWifi();					
				}					
				sta = CmdExe_WB("AT+CIPSEND\r\n",100);			
				if(1==sta)						//进入透传模式
				{					
						return 1;	
				}
				else if(-1==sta)			//error
				{
					if(1==CmdExe_WB("AT+RST\r\n",100))
					{
							wifi_rev_cnt = 0;
							wifi_reset_sta = 0;
							wifi_reset_tick = g_sysTick;
							return 1;	
					}
					else
						return 0;
				}
			}
		}
		return 1;
}

//重新进行tcp连接, 当IP+PORT中修改过一个, 则需要重新连接。
unsigned char ReConnTcp(unsigned char*ip,int port)
{
		int  datalen = 0;
		int  sta = 0;		
		char cmd[128] = {"AT+SAVETRANSLINK=1,\"139.129.17.114\",7106,\"TCP\",15\r\n"};		
		////////////////////进入at命令模式////////////////////////////////////////////////////
		if(1==CmdExe_WB("AT\r\n",100))						//at测试
		{
										
		}
		else if(1==CmdExe_WB("AT\r\n",100))				//at测试
		{					
				
		}
		else if(1==ExitPassthroughMode())				//退出数据模式进入命令模式
		{
			
		}
		else if(1==ExitPassthroughMode())		//退出数据模式进入命令模式
		{
			
		}
		else
		{				
				return 0;
		}					
	
		
		if(1 != CmdExe_WB("ATE0\r\n",100))							//等待1秒+20ms
		{
			 CmdExe_WB("ATE0\r\n",100);
		}
		
		if(1==CmdExe_WB("AT+SYSSTORE?\r\n",100))				//参数可以保存到flash
		{
				if(1 == getSpecData("+SYSSTORE",2,cmd,&datalen,0))				
				{
						cmd[datalen] = '\0';
						sta = atoi(cmd);
						if(1!=sta)
						{
							if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
							else if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
						}
				}
		}
		else if(1==CmdExe_WB("AT+SYSSTORE?\r\n",100))				//参数可以保存到flash
		{
				if(1 == getSpecData("+SYSSTORE",2,cmd,&datalen,0))				
				{
						cmd[datalen] = '\0';
						sta = atoi(cmd);
						if(1!=sta)
						{
							if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
							else if(1!=CmdExe_WB("AT+SYSSTORE=1\r\n",100))
							{
							
							}
						}
				}
		}
		else
		{
				return 0;
		}
		
		////////////////////进入at命令模式////////////////////////////////////////////////////				
		sprintf(cmd,"AT+SAVETRANSLINK=1,\"%d.%d.%d.%d\",%d,\"TCP\",60\r\n",ip[0],ip[1],ip[2],ip[3],port);					
		if(1==CmdExe_WB(cmd,200))				//
		{
				//直接连接，如果连接成功，则执行  CIPSEND。
			memcpy(gs_SaveWifiCfg.TdInfo[1].ucDestIP,ip,4);
			gs_SaveWifiCfg.TdInfo[1].ucRemotePort[0]	= (port>>8)&0xff;
			gs_SaveWifiCfg.TdInfo[1].ucRemotePort[1]	=  port&0xff;
			WriteFlashWifi();				
			if(1==CmdExe_WB("AT+RST\r\n",100))
			{
					wifi_rev_cnt = 0;
					wifi_reset_sta = 0;
					wifi_reset_tick = g_sysTick;
					return 1;	
			}
			else
				return 0;				
		}		
		return 1;
}

unsigned int	g_getiptick = 0;
//获取上传ip和port
void GetIPAndPort2()
{		
		unsigned int  		tim  = 0;
		unsigned char 		DestIP[4];
		int 							DestPort;
		int 							i=0;
		if(g_sysTick!=g_getiptick)
		{
			tim = g_sysTick - g_getiptick;					//得到时间差
			if(tim>9)																//10秒
			{
					g_getiptick = g_sysTick;						//新的开始
					tim = 0;
					for(i=0;i<g_configRead.PortLen;i++)
					{
						tim = tim*10 + (g_configRead.remotePort[i] - 0x30);  
					}
					DestPort = tim;
					
					if(strIP2ip(g_configRead.remoteIP,g_configRead.IPLen,DestIP))
					{					
							//将上传ip和port去和gs_SaveWifiCfg.TdInfo中的ip和port比较
							for(i=0;i<4;i++)
							{
								if(gs_SaveWifiCfg.TdInfo[1].ucDestIP[i]!=DestIP[i])
								{
									break;
								}
							}		
	
							if(i==4 && ((DestPort>>8)&0xff)==gs_SaveWifiCfg.TdInfo[1].ucRemotePort[0] && (DestPort&0xff)==gs_SaveWifiCfg.TdInfo[1].ucRemotePort[1])	//ip没有变化
							{
								;
							}
							else
							{
									ReConnTcp(DestIP,DestPort);			//com3_wifi2 配置参数配置成功并重启。		
							}							
					}			
			}		
		}
}

/**********************************************************************
  * @ 函数名  ： LED_Task
  * @ 功能说明： LED_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
void WIFI_Task(void* parameter)
{	
	unsigned char wifi_sta= 0;		
	g_getiptick = g_sysTick;
  while (1)
  {		   
		//if(1==g_configRead.b_wifi_work  && g_sysTick>5 && wifi_rev_cnt>50)			//g_gprs_work      && (WIFI_BLE+1)==g_configRead.wifi_mode   wifi_rev_cnt没有接收到数据则表示wifi不存在
		if(1==g_configRead.b_wifi_work)						//g_gprs_work      && (WIFI_BLE+1)==g_configRead.wifi_mode   wifi_rev_cnt没有接收到数据则表示wifi不存在
		{
			if(0==wifi_reset_sta)										//表示重启中
			{
					if(g_sysTick - wifi_reset_tick>2 && wifi_rev_cnt < 10)		//表示重启超时  即模块不存在 3秒内没有收到reday 并且 串口没有收到数据。
					{
							wifi_reset_sta = 3;
					}
			}
			else if(1==wifi_reset_sta || 2==wifi_reset_sta)								//表示wifi已经可以工作了。非透传中
			{
				//尝试连接
				if(2==wifi_reset_sta)
				{
						wifi_sta=1;
				}
				if(0==wifi_sta)
				{					
						wifi_sta = wifi_conn();
				}
				if(wifi_sta)
				{
						GetIPAndPort2();											//7秒进行ip和port的比较	wifi需要使用。当服务器IP和PORT修改的时候，则修改wifi的第2串口参数。并重启wifi模块。
				}
			}					
		}		
		vTaskDelay(100);   /* 延时100个tick */		 		        
  }
}


//过程模拟如下
//1. 开机 wifi_conn函数中  如果发at命令可以返回则表示模块为新模块，进行必要的参数设置，如果具备启动则进行启动，否则不自动连接。
//2. 写完参数后，如果具备联网条件，设备立即进入自动上传模式
//3. 读参数，只读就可以。但注意新设备读有问题。
//优化程序 并完成，同时开启wifi参数生效。
//检测tcp连接情况，如果已连接，则执行cipsend命令

//开机执行1次 读参数操作，当读参数的时候，进行自动模式设置。
//过程如下:  1.设置参数   2. 读参数   3. 开机过程    4. 中途修改了IP+Port。可以通过重启来实现或动态改变通道信息。
//严格测试。

//当wifi充当主口进行透传和控制测试。
//主口可以通过软件来配置，如何配置。工作模式参数用于强制  

//开机如果at命令发送不成功，模块透传模式中，模块不存在，硬件通讯有问题？ 开机可以监测接收信息。判断模块是否存在？
//配置参数没有问题

//增加服务器IP 或 端口改变的情况下，重新修改自动重连信息。	ok
//增加自动监控 wifi重启或reset 字符串检测功能。
//ready   命令行模式

//进入透传模式
//>
//ready
