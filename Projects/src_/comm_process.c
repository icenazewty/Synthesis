#include "comm_process.h"
#include "GlobalVar.h"
#include "gd32e503v_eval.h"
#include "W5500.h"
#include "cfg_flash.h"
#include "string.h"
#include "ads1247.h"
#include "systick.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h"
#include "wifi_ble.h"
#include "usb_para.h"
#include <stdio.h>

unsigned char  g_com_port[4] = {1,7,3,8};

extern int  Year,Month,Day,Hour,Minute,Second;
extern TaskHandle_t 	EtherNet_Task_Handle;			/* w5500任务句柄 */

unsigned char g_Ch_Param[128];		//最长128字节
unsigned char	g_Ch_Par_Len = 0;		//通道参数长度	
extern int 					uiADValue;
void ads1247_calibrate(unsigned char ch,unsigned char sel)
{
		int						data[16];
		int						temp=0;
		unsigned int  Ads1247_Tick=0,Ads1247_Cnt=0,Ads1247_Timeout=0;	
		uint32_t i,j;																											
		Ads1247_Tick = systickCount;
		AI_Channel_Select(ch);
		
		while(1)
		{
			wdt();
			Ads1247_Timeout = systickCount - Ads1247_Tick;
			if(Ads1247_Timeout>99)			//ms  100ms一次
			{
				wdt();
				if(Irq_Ads1247_Ready)
				{				
					Irq_Ads1247_Ready = 0;						//等待下次					
					Read_AI_Data(ch);									//读上次的数据
					data[Ads1247_Cnt]=uiADValue;
					Ads1247_Cnt++;										//合计采集的次数									
					wdt();													
					if(Ads1247_Cnt>15)
					{
						break;
					}
					Read_Ads1247_Mode(D_RDATA);				//启动单次读							
				}
				else
				{					
					Read_Ads1247_Mode(D_RDATA);		
				}
				Ads1247_Tick = systickCount;
			}		
		}
		
		//对数据进行排序 data  16个
		for(i=0;i<15;i++)
		{
			for(j=i+1;j<16;j++)
			{
					if(data[j]<data[i])
					{
						temp = data[j];
						data[j] = data[i];
						data[i] = temp;
					}
			}			
		}		
		temp = 0;
		for(i=0;i<10;i++)
		{
			temp+=data[3+i];
		}
		g_Ads1247_Cali.AdC[ch][sel]=temp/10;
		WriteParametersADS1247();
}

void ads1247_cali_mode(void)
{
	unsigned short 		nCRC = 0;	
	unsigned char 		ch =  0;		//0~7表示8个通道
	unsigned char			sel = 0;		//0=-6v   1=0v  2=6v
	unsigned char			success = false;	//

	unsigned short		i   = 0;
	unsigned	char		nCRC_H=0,nCRC_L=0;
	//unsigned 	char		big_endian = 0;	
	
	Read_SW_Input_State();	
	while(1)													//进入校准模式ads1247
	{
		wdt();	
		success = false;	
		//+++++++++++++modbus协议解析 将来自2440上的数据进行modbus解析 目的设置rs485的波特率和读本地温湿度+++++++++++++++++++++		
		while(Com.Usart[COM_MASTER].usRec_RD!=Com.Usart[COM_MASTER].usRec_WR)
		{
			#if COM1_TEST
			COM1_Send_Data(Com.Usart1.RX_Buf+Com.Usart1.usRec_RD,1);
			Com.Usart1.usRec_RD = (Com.Usart1.usRec_RD+1)%D_USART_REC_BUFF_SIZE;
			#endif		
			wdt();
			Com.ucActiveRd = (D_USART_REC_BUFF_SIZE + Com.Usart[COM_MASTER].usRec_WR - Com.Usart[COM_MASTER].usRec_RD)%D_USART_REC_BUFF_SIZE;		//接收到的数据长度			
			if(Com.ucActiveRd>7)						//至少8个，0xfd 03 f0 f0 00 01 crc16	合计8个数据
			{
					Com.uctmpID   = (Com.Usart[COM_MASTER].RX_Buf[Com.Usart[COM_MASTER].usRec_RD] & 0xFF);											//modbus id			
					Com.ucCommand = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD+1)%D_USART_REC_BUFF_SIZE];			//命令号
					if(Com.uctmpID == gs_SaveNetIPCfg.g_ModbusID && Com.ucCommand==0x06)
					{
							Com.usRX_Analy_Buf_Len = 8;									//完整命令长度1 2 3 4 5 6
							for(i=0;i<Com.usRX_Analy_Buf_Len;i++)
							{										
								Com.RX_Analy_Buf[i] = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD+i)%D_USART_REC_BUFF_SIZE];										
							}	
							nCRC = CrcCheck(Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len-2);		
							nCRC_H = (unsigned char)(nCRC&0xFF);	
							nCRC_L = (unsigned char)((nCRC>>8)&0xFF);										
							if(nCRC_H==Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-1] && nCRC_L==Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-2])
							{
								Com.Usart[COM_MASTER].usRec_RD = (Com.Usart[COM_MASTER].usRec_RD+8)%D_USART_REC_BUFF_SIZE;	
								success = true;
							}
							else if(nCRC_H==Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-2] && nCRC_L==Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-1])
							{
								Com.Usart[COM_MASTER].usRec_RD = (Com.Usart[COM_MASTER].usRec_RD+8)%D_USART_REC_BUFF_SIZE;	
								success = true;							
							}
							else
							{
								Com.Usart[COM_MASTER].usRec_RD = (Com.Usart[COM_MASTER].usRec_RD+1)%D_USART_REC_BUFF_SIZE;	
								break;
							}					
							Com_Send(COM_MASTER,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);
							ch =  Com.RX_Analy_Buf[3];		//通道
							sel = Com.RX_Analy_Buf[5];		//校准点
					}					
					else
					{
							Com.Usart[COM_MASTER].usRec_RD = (Com.Usart[COM_MASTER].usRec_RD+1)%D_USART_REC_BUFF_SIZE;		
							break;
					}				
      }
			else			//数据长度不足够一个命令
			{
					break;
      }
	  }	
		//+++++++++++++modbus协议解析 将来自2440上的数据进行modbus解析 目的设置rs485的波特率和读本地温湿度+++++++++++++++++++++			
		if(success==true)
		{
			if(ch<8&&sel<3)
			{
					ads1247_calibrate(ch,sel);
			}							 
		}		
	}	
}

#define TTT	500
void Clear_Uart_Buffer(void)	
{	
	#if 1
	if(Com.Usart[COM_MASTER].usRec_WR!=Com.Usart[COM_MASTER].usRec_RD)					//有数据
	{
			if(systickCount - Com.Usart[COM_MASTER].usTick > TTT)										//连续TTT ms 没有新的数据 且 缓冲中接收到的数据不为空，则清空数据 ,正常转发到主口的数据不应该清除。
			{
					Com.Usart[COM_MASTER].usRec_RD = Com.Usart[COM_MASTER].usRec_WR;		
			}		
	}
	#else
		unsigned char i = 0;
		for(i=0;i<COMn;i++)
		{
			if(Com.Usart[i].usRec_WR!=Com.Usart[i].usRec_RD)				//有数据
			{
				if(systickCount - Com.Usart[i].usTick > TTT)					//连续TTT ms 没有新的数据 且 缓冲中接收到的数据不为空，则清空数据 ,正常转发到主口的数据不应该清除。
				{
					Com.Usart[i].usRec_WR = Com.Usart[i].usRec_RD = 0;	
				}		
			}
		}		
	#endif
}

//#define										W5500_MAX_BUF					512				//每个通道数据长度
extern char								w5500_buf[4][D_USART_REC_BUFF_SIZE];		//8个通道，每个通道最多512字节
extern unsigned short int	w5500_rd[4];										//8个通道的读写指针
extern unsigned short int w5500_wr[4];										//8个通道的读写指针

//============================================================================================================================================================================================
void InitW5500Data(unsigned char ch)
{
		//Write_SOCK_Data_Buffer(2, Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);
		if((Com.usRX_Analy_Buf_Len+w5500_wr[ch])>D_USART_REC_BUFF_SIZE)
		{
			memcpy(w5500_buf[ch]+w5500_wr[ch],Com.RX_Analy_Buf,D_USART_REC_BUFF_SIZE-w5500_wr[ch]);			
			memcpy(w5500_buf[ch],Com.RX_Analy_Buf+(D_USART_REC_BUFF_SIZE-w5500_wr[ch]),Com.usRX_Analy_Buf_Len+w5500_wr[ch]-D_USART_REC_BUFF_SIZE);		//如何拷贝	 D_USART_REC_BUFF_SIZE
		}
		else
		{
			memcpy(w5500_buf[ch]+w5500_wr[ch],Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);			
		}		
		w5500_wr[ch] += Com.usRX_Analy_Buf_Len;
		w5500_wr[ch] %= D_USART_REC_BUFF_SIZE;
}

//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲
unsigned char get_sw_timeout(unsigned char ch,unsigned char mode)
{
	unsigned int tmpTick = 0;	
	if(0==Sw_com[ch].ucFlag || 0==Sw_com[ch].ucMode)	//空闲模式
	{
			Sw_com[ch].ucMode = mode;											//转发类型
			Sw_com[ch].ucFlag = 1;												//进入非空闲模式
			return 1;
	}
	else
	{
			tmpTick = systickCount;
			tmpTick = tmpTick - Sw_com[ch].uiTick[Sw_com[ch].ucMode-1];								//最后1次发送完成数据后到现在的时间间隔,ucMode表示最后一次发送数据源是谁。
			if(tmpTick>Sw_com[ch].usDelay_Rev)				//超时
			{
				Sw_com[ch].ucFlag = 1;									//进入非空闲模式
				Sw_com[ch].ucMode = mode;								//转发类型  超时则进行转发类型切换
				return 1;
			}
			else
			{
				return 0;
			}
	}
}

unsigned char g_Mppt_Cur  						= 0;		//当前轮询mppt编号
unsigned char g_Modbus_Send_Flag[4]   = {0,0,0,0};		//命令是否已发送
unsigned int  g_Modbus_Send_Tick      = 0;		//命令发送时刻
unsigned int  g_Modbus_Interval_Tick  = 0;		//最后一条命令离当前时间

void Retransmission(unsigned char com_port)
{
		wdt();
		if(com_port>0x2f && com_port < '7')
		{		
				if('1'==com_port || '3'==com_port)
				{
						//对转发进行方向控制
					if('1'==com_port)
					{
						if(get_sw_timeout(0,2))																								//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲
						{								
							Com.Usart[g_com_port[0]].usRec_RD = Com.Usart[g_com_port[0]].usRec_WR;											//清空接受缓存 准备开始新的1包数据的解析
							Com_Send(com_port-0x30,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);		//RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7				透明传输
							Sw_com[0].uiTick[1] = systickCount;																	//COM1串口屏向串口转发时刻
						//记录发送时刻和发送类型，当收到数据进行转发的时候
						}
						//否则直接放弃本次数据转发
					}
					else
					{
						if(get_sw_timeout(2,2))																								//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲
						{				
							Com.Usart[g_com_port[2]].usRec_RD = Com.Usart[g_com_port[2]].usRec_WR;											//清空接受缓存 准备开始新的1包数据的解析
							Com_Send(com_port-0x30,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);		//RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7				透明传输
							Sw_com[2].uiTick[1] = systickCount;		//COM1串口屏向串口转发时刻
							//记录发送时刻和发送类型，当收到数据进行转发的时候
						}
						//否则直接放弃本次数据转发
					}
				}
				else
				{
						Com_Send(com_port-0x30,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);			//透明传输
				}	
				
//				if('1'==com_port)
//				{
//					if(0!=Sw_Sta[1] || 0!=g_configRead.b_light_work)										//非透明传输模式
//					{
//							g_Modbus_Send_Flag = 2;																					//进行标志  表示进入
//							g_Modbus_Send_Tick = systickCount;															//新的开始时刻
//					}					
//				}				
		}		
    else if(0x39==com_port || 0x3a==com_port)																	//CH432T的两个com口
		{
				//对转发进行方向控制
				if(0x39==com_port)
				{
					if(get_sw_timeout(1,2))										//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲
					{			
						Com.Usart[g_com_port[1]].usRec_RD = Com.Usart[g_com_port[1]].usRec_WR;											//清空接受缓存 准备开始新的1包数据的解析
						Com_Send(com_port-0x32,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);		//RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7				透明传输
						Sw_com[1].uiTick[1] = systickCount;		//COM1串口屏向串口转发时刻
						//记录发送时刻和发送类型，当收到数据进行转发的时候
					}
					//否则直接放弃本次数据转发
				}
				else
				{
					if(get_sw_timeout(3,2))										//mode: 1=uiTick_Local  2=uiTick_Uart  3=uiTick_RJ45   0=空闲
					{	
						Com.Usart[g_com_port[3]].usRec_RD = Com.Usart[g_com_port[3]].usRec_WR;											//清空接受缓存 准备开始新的1包数据的解析						
						Com_Send(com_port-0x32,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);		//RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7				透明传输
						Sw_com[3].uiTick[1] = systickCount;		//COM1串口屏向串口转发时刻
						//记录发送时刻和发送类型，当收到数据进行转发的时候
					}
					//否则直接放弃本次数据转发
				}
		}
		else if(com_port=='7')									//rj45
		{
			if(g_Ch_Par_Len>0)
			{
					UDP_DPORT[0] = g_Ch_Param[1];
					UDP_DPORT[1] = g_Ch_Param[2];
						
					UDP_DIPR[0]  = g_Ch_Param[3];
					UDP_DIPR[1]  = g_Ch_Param[4];
					UDP_DIPR[2]  = g_Ch_Param[5];
					UDP_DIPR[3]  = g_Ch_Param[6];
				
					if(g_Ch_Param[0]==0)																					//TCP_CLIENT  
					{
						if(gs_SaveNetIPCfg.ucDestPort[0]==g_Ch_Param[1] && gs_SaveNetIPCfg.ucDestPort[1]==g_Ch_Param[2] && gs_SaveNetIPCfg.ucDestIP[0]==g_Ch_Param[3] && gs_SaveNetIPCfg.ucDestIP[1]==g_Ch_Param[4] && gs_SaveNetIPCfg.ucDestIP[2]==g_Ch_Param[5] && gs_SaveNetIPCfg.ucDestIP[3]==g_Ch_Param[6])
						{								
								
						}
						else
						{
								//Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);								//tcp client close		
								gs_SaveNetIPCfg.ucDestPort[0]=g_Ch_Param[1];
								gs_SaveNetIPCfg.ucDestPort[1]=g_Ch_Param[2];								
							
								gs_SaveNetIPCfg.ucDestIP[0]=g_Ch_Param[3];
								gs_SaveNetIPCfg.ucDestIP[1]=g_Ch_Param[4];
								gs_SaveNetIPCfg.ucDestIP[2]=g_Ch_Param[5];
								gs_SaveNetIPCfg.ucDestIP[3]=g_Ch_Param[6];							
								//Socket_Init(0);
								WriteParametersToIICAll();
							#if 0
								if(NULL!=EtherNet_Task_Handle)
								{										
											xTaskNotify((TaskHandle_t)EtherNet_Task_Handle,				//接收任务通知的任务句柄
													(uint32_t		)0x100,																//要触发的事件
													(eNotifyAction)eSetBits);													//设置任务通知值中的位
								}
							#endif
						}		
						if(Com.usRX_Analy_Buf_Len>0)
						{
								//Write_SOCK_Data_Buffer(0, Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);
								#if 0
								if(NULL!=EtherNet_Task_Handle)
								{							
											InitW5500Data(0);
											xTaskNotify((TaskHandle_t)EtherNet_Task_Handle,				//接收任务通知的任务句柄
													(uint32_t		)0x01,													//要触发的事件
													(eNotifyAction)eSetBits);													//设置任务通知值中的位
								}
								else
								InitW5500Data(0);
								#endif
						}
					}
					else if(g_Ch_Param[0]==1)				//TCP_SERVER
					{							
							if(gs_SaveNetIPCfg.ucMonitorPort[0]==g_Ch_Param[1] && gs_SaveNetIPCfg.ucMonitorPort[1]==g_Ch_Param[2])
							{
									
							}
							else
							{
									//修改监听端口							
									//Write_W5500_SOCK_1Byte(1,Sn_CR,CLOSE);							//tcp server close
									gs_SaveNetIPCfg.ucMonitorPort[0]=g_Ch_Param[1];
									gs_SaveNetIPCfg.ucMonitorPort[1]=g_Ch_Param[2];								
									//Socket_Init(1);
									WriteParametersToIICAll();
									#if 0
									if(NULL!=EtherNet_Task_Handle)
									{										
											xTaskNotify((TaskHandle_t)EtherNet_Task_Handle,				//接收任务通知的任务句柄
													(uint32_t		)0x200,																//要触发的事件
													(eNotifyAction)eSetBits);													//设置任务通知值中的位
									}
									#endif
							}
							if(Com.usRX_Analy_Buf_Len>0)
							{
									//Write_SOCK_Data_Buffer(1, Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);	
									#if 0	
									if(NULL!=EtherNet_Task_Handle)
									{					
											InitW5500Data(1);
											xTaskNotify((TaskHandle_t)EtherNet_Task_Handle,				//接收任务通知的任务句柄
													(uint32_t		)0x02,																//要触发的事件
													(eNotifyAction)eSetBits);													//设置任务通知值中的位
									}
									else
									InitW5500Data(1);
									#endif
							}
					}
					else if(g_Ch_Param[0]==2)				//udp  
					{		
						//带ip和端口的任意udp转发
						if(gs_SaveNetIPCfg.ucUdpDestPort[0]==g_Ch_Param[1] && gs_SaveNetIPCfg.ucUdpDestPort[1]==g_Ch_Param[2] && gs_SaveNetIPCfg.ucUdpDestIP[0]==g_Ch_Param[3] && gs_SaveNetIPCfg.ucUdpDestIP[1]==g_Ch_Param[4] && gs_SaveNetIPCfg.ucUdpDestIP[2]==g_Ch_Param[5] && gs_SaveNetIPCfg.ucUdpDestIP[3]==g_Ch_Param[6])
						{								
								
						}						
						else
						{
								gs_SaveNetIPCfg.ucUdpDestPort[0]=g_Ch_Param[1];
								gs_SaveNetIPCfg.ucUdpDestPort[1]=g_Ch_Param[2];								
							
								gs_SaveNetIPCfg.ucUdpDestIP[0]=g_Ch_Param[3];
								gs_SaveNetIPCfg.ucUdpDestIP[1]=g_Ch_Param[4];
								gs_SaveNetIPCfg.ucUdpDestIP[2]=g_Ch_Param[5];
								gs_SaveNetIPCfg.ucUdpDestIP[3]=g_Ch_Param[6];				
								WriteParametersToIICAll();	
						}
						if(Com.usRX_Analy_Buf_Len>0)
						{
								//Write_SOCK_Data_Buffer(2, Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);
								InitW5500Data(2);
								#if 0
								if(NULL!=EtherNet_Task_Handle)
								{										
											xTaskNotify((TaskHandle_t)EtherNet_Task_Handle,				//接收任务通知的任务句柄
													(uint32_t		)0x04,																//要触发的事件
													(eNotifyAction)eSetBits);													//设置任务通知值中的位
								}
								#endif
						}
					}					
			}								
		}
		else if(com_port=='8')								//lora
		{
									
		}			
		
		if(com_port>0x2f && com_port < '6' && g_Ch_Par_Len>0)		//0 1 2 3 4		串口参数设置
		{
				gs_SaveNetIPCfg.baud[com_port-0x30] 		= 	(g_Ch_Param[0]<<16)|(g_Ch_Param[1]<<8)|g_Ch_Param[2];	
				gs_SaveNetIPCfg.chekbit[com_port-0x30] 	= 	g_Ch_Param[3];			
				if(com_port=='0'&&g_Ch_Par_Len>4)							//COM1  波特率3字节（高前低后）   校验码1字节     modbus id 1 字节
				{
					gs_SaveNetIPCfg.g_ModbusID = g_Ch_Param[4];
				}			
				//else if(dip7==1 && com_port=='2' && g_Ch_Par_Len>4)
				//{
				//	gs_SaveNetIPCfg.g_ModbusID = g_Ch_Param[4];
				//}
				WriteParametersToIICAll();
		}
}

unsigned char Com_Rev_Timeout(unsigned char ch)
{
		unsigned int tmpTick = systickCount;
		unsigned int ComX_Tick = 0;
		unsigned int Res=0;
		//unsigned short TickDelay = 0;
		
		ComX_Tick = Com.Usart[ch].usTick;					//最后收到数据的时间  中断
		//TickDelay = Com.Usart[ch].usDelay_Rev;		
		Res = tmpTick - ComX_Tick;								//最后1次读到的数据到目前的时间间隔
		if(Res>Com.Usart[ch].usDelay_Rev)
		{
				return 1;															//超时，则认为一包完整的数据，可以转发
		}
		else
		{
				return 0;
		}		
}

void Com2Com(unsigned char Source_Ch ,unsigned char Dest_Ch)
{
	if(Source_Ch!=Dest_Ch)						//自收自发
	{		
		wdt();
		if(Source_Ch==WIFI_BLE)
		{
			if(Com.Usart[Source_Ch].usRec_RD!=Com.Usart[Source_Ch].usRec_WR)																					//接收到新的数据并且接收超时，则考虑转发
			{
				Com.usRX_Analy_Buf_Len = 0;
				while(Com.Usart[Source_Ch].usRec_RD!=Com.Usart[Source_Ch].usRec_WR)
				{			
					Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len] = Com.Usart[Source_Ch].RX_Buf[Com.Usart[Source_Ch].usRec_RD];				
					Com.usRX_Analy_Buf_Len++;
					Com.Usart[Source_Ch].usRec_RD = (Com.Usart[Source_Ch].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
				}
				if(Com.usRX_Analy_Buf_Len>0)
				{				
					Com_Send(Dest_Ch,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);						
				}			
			}		
		}
		else if(USB==Source_Ch)
		{
			if(Com.Usart[Source_Ch].usRec_RD!=Com.Usart[Source_Ch].usRec_WR)					//接收到新的数据并且接收超时，则考虑转发
			{
				Com.usRX_Analy_Buf_Len = 0;
				while(Com.Usart[Source_Ch].usRec_RD!=Com.Usart[Source_Ch].usRec_WR)
				{			
					Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len] = Com.Usart[Source_Ch].RX_Buf[Com.Usart[Source_Ch].usRec_RD];									
					Com.Usart[Source_Ch].usRec_RD = (Com.Usart[Source_Ch].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
					Com.usRX_Analy_Buf_Len++;
				}
				if(Com.usRX_Analy_Buf_Len>0)
				{				
					Com_Send(Dest_Ch,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);						
				}			
			}		
		}		
		else if(Com.Usart[Source_Ch].usRec_RD!=Com.Usart[Source_Ch].usRec_WR)				//有数据接收到了。
		{
			if(1==Com_Rev_Timeout(Source_Ch))																					//接收到新的数据并且接收超时，则考虑转发
			{
				Com.usRX_Analy_Buf_Len = 0;
				while(Com.Usart[Source_Ch].usRec_RD!=Com.Usart[Source_Ch].usRec_WR)
				{			
					Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len] = Com.Usart[Source_Ch].RX_Buf[Com.Usart[Source_Ch].usRec_RD];									
					Com.Usart[Source_Ch].usRec_RD = (Com.Usart[Source_Ch].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
					Com.usRX_Analy_Buf_Len++;
				}
				if(Com.usRX_Analy_Buf_Len>0)
				{
					if(RS485_2==Source_Ch)
					{
							if(3==Sw_com[0].ucMode)				//rj45
							{
								Write_SOCK_Data_Buffer(4, Com.RX_Analy_Buf, Com.usRX_Analy_Buf_Len);										
							}
							else if(2==Sw_com[0].ucMode)		//com1
							{
								Com_Send(Dest_Ch,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);						
							}
							Sw_com[1].ucFlag = 0;						//已经收到查询后的返回数据
					}
					else if(CH432T_1==Source_Ch)
					{
							if(3==Sw_com[1].ucMode)					//rj45
							{
								Write_SOCK_Data_Buffer(5, Com.RX_Analy_Buf, Com.usRX_Analy_Buf_Len);		
							}
							else if(2==Sw_com[1].ucMode)		//com1
							{
								Com_Send(Dest_Ch,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);						
							}
							Sw_com[1].ucFlag = 0;						//已经收到查询后的返回数据
					}
					else if(RS485_4==Source_Ch)
					{
							if(3==Sw_com[2].ucMode)				//rj45
							{
								Write_SOCK_Data_Buffer(6, Com.RX_Analy_Buf, Com.usRX_Analy_Buf_Len);		
							}
							else if(2==Sw_com[2].ucMode)		//com1
							{
								Com_Send(Dest_Ch,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);						
							}
							Sw_com[2].ucFlag = 0;						//已经收到查询后的返回数据
					}
					else if(CH432T_2==Source_Ch)
					{
							if(3==Sw_com[3].ucMode)					//rj45
							{
								Write_SOCK_Data_Buffer(7, Com.RX_Analy_Buf, Com.usRX_Analy_Buf_Len);		
							}
							else if(2==Sw_com[3].ucMode)		//com1
							{
								Com_Send(Dest_Ch,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);						
							}
							Sw_com[3].ucFlag = 0;						//已经收到查询后的返回数据
					}			
					else
					{						
						Com_Send(Dest_Ch,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);						
					}
				}			
			}		
		}	
	}		
}
			
void Com1_Boot(void)
{
	int len = 0;
	if(b_rd!=b_wr && boot_buf[0]=='\n')
	{			
		len = (b_wr - b_rd + 128)%128;
		if(len>19 && '\r'== boot_buf[19])		//StayInBootLoader=1  strcmp()
		{			
				if('S'  == boot_buf[1] && 't'  == boot_buf[2]
				&& '1'  == boot_buf[18]
				&& '='  == boot_buf[17]
				&& 'r'  == boot_buf[16]
				&& 'e'  == boot_buf[15]
				&& 'd'  == boot_buf[14]
				&& 'a'  == boot_buf[13]
				&& 'o'  == boot_buf[12]
				&& 'L'  == boot_buf[11]
				&& 't'  == boot_buf[10]
				&& 'o'  == boot_buf[9]
				&& 'o'  == boot_buf[8]
				&& 'B'  == boot_buf[7]
				&& 'n'  == boot_buf[6]
				&& 'I'  == boot_buf[5]
				&& 'y'  == boot_buf[4]
				&& 'a'  == boot_buf[3])
				{
//			\nStayInBootLoader=1\r
//			IWDG_Configuration();
//			__disable_irq();
//			NVIC_SystemReset();	//Software reset
					__set_FAULTMASK(1); 		//¹رՋùӐ֐¶ύ					
					nvic_system_reset();
				}			
		}
	}			
}



#if  0
24 30 30 4D 0D 24 30 30 4D 44 31 0D 00 03 27 10 00 02 CE AB 00 03 00 D2 00 02 65 E3 24 30 31 4D 0D 24 30 31 
4D 44 32 0D 01 03 27 10 00 02 CF 7A 01 83 01 80 F0 01 03 00 D2 00 02 64 32 01 03 04 40 55 00 00 FF E3 24 30 32 
4D 0D 24 30 32 4D 44 33 0D 02 03 27 10 00 02 CF 49 02 03 00 D2 00 02 64 01 24 30 33 4D 0D 24 30 33 4D 44 34 0D 
03 03 27 10 00 02 CE 98 03 03 00 D2 00 02 65 D0

01 01 00 10 00 08 3C 09 01 01 01 00 51 88 
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 
01 01 00 10 00 08 3C 09 01 01 01 00 51 88 
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 
01 01 00 10 00 08 3C 09 01 01 01 00 51 88 
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 
01 01 00 10 00 08 3C 09 01 01 01 00 51 88 
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 
01 01 00 10 00 08 3C 09 01 01 01 00 51 88 
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 
01 01 00 10 00 08 3C 09 01 01 01 00 51 88 
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 
01 01 00 10 00 08 3C 09 01 01 01 00 51 88 
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 

////////
realay

01 05 00 17 FF 00 3C 3E   7闭合
01 05 00 17 FF 00 3C 3E 

01 05 00 17 00 00 7D CE   7断开
01 05 00 17 00 00 7D CE 

01 01 00 10 00 08 3C 09 01 01 01 9F 11 E0 
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 
01 01 00 10 00 08 3C 09 01 01 01 9F 11 E0 
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 
01 01 00 10 00 08 3C 09 01 01 01 9F 11 E0 

01 01 00 10 00 08 3C 09 01 01 01 1F 10 40 01 01 00 00 00 08 3D CC 01 01 01 00 51 88 01 01 00 10 00 08 3C 09 01 01 01 1F 10 40 

#endif

void get_pvt_para(void)
{ 	
	char tmp[8] = {0,0,0,0,0,0,0,0};
	tmp[0] = g_configRead.device_ID[0];
	tmp[1] = g_configRead.device_ID[1];
	tmp[2] = g_configRead.device_ID[2];
	tmp[3] = g_configRead.device_ID[3];
	tmp[4] = g_configRead.device_ID[4];
	tmp[5] = g_configRead.device_ID[5];	
	g_PVTPara.device_ID 			= atoi(tmp);
	g_PVTPara.collect_frq 		= g_configRead.collect_frq;
	g_PVTPara.send_frq 				= g_configRead.send_frq;
	g_PVTPara.TMax 						= g_configRead.TMax[1];
	g_PVTPara.TMin 						= g_configRead.TMin[1];
	g_PVTPara.HMax 						= g_configRead.HMax[1];
	g_PVTPara.HMin 						= g_configRead.HMin[1];
	
//	if(0==g_configRead.save_frq)
//		g_PVTPara.b_TH_work 			= 0;
//	else 
//		g_PVTPara.b_TH_work 			= 1;
	
	g_PVTPara.b_TH_work	        = g_configRead.save_frq;
	g_PVTPara.TMax2 						= g_configRead.TMax2;
	g_PVTPara.TMin2 						= g_configRead.TMin2;
	g_PVTPara.HMax2 						= g_configRead.HMax2;
	g_PVTPara.HMin2 						= g_configRead.HMin2;
	
	g_PVTPara.TXZ[0] 					= g_configRead.TXZ[0];
	g_PVTPara.HXZ[0] 					= g_configRead.HXZ[0];
	g_PVTPara.TXZ[1] 					= g_configRead.TXZ[1]; 
	g_PVTPara.HXZ[1] 					= g_configRead.HXZ[1];
	g_PVTPara.VMin 						= g_configRead.VMin;
	g_PVTPara.bCH[0] 					= g_configRead.bCH[0];
	g_PVTPara.bCH[1] 					= g_configRead.bCH[1];
	g_PVTPara.alarmDelyMinute = g_configRead.alarmDelyMinute;
	g_PVTPara.beep 						= g_configRead.beep;
	g_PVTPara.wifi_mode 			= g_configRead.wifi_mode;
	g_PVTPara.b_light_work 		= g_configRead.b_light_work;
	g_PVTPara.b_gprs_work 		= g_configRead.b_gprs_work;
	g_PVTPara.b_wifi_work 		= g_configRead.b_wifi_work;
	g_PVTPara.b_rj45_work 		= g_configRead.b_rj45_work;
	g_PVTPara.b_debug_work 		= g_configRead.b_debug_work;
	g_PVTPara.reset_time 			= g_configRead.reset_time;
	g_PVTPara.b_Sms_Test 			= g_configRead.b_Sms_Test;
	g_PVTPara.b_Sms_FxiTime 	= g_configRead.b_Sms_FxiTime;	
	memcpy(tmp,g_configRead.remotePort,g_configRead.PortLen);
	tmp[g_configRead.PortLen] = '\0';
	g_PVTPara.remotePort= atoi(tmp);
	if(strIP2ip(g_configRead.remoteIP,g_configRead.IPLen,(unsigned char*)tmp))
	{	
		g_PVTPara.remoteIP[0] = tmp[0];
		g_PVTPara.remoteIP[1] = tmp[1];
		g_PVTPara.remoteIP[2] = tmp[2];
		g_PVTPara.remoteIP[3] = tmp[3];
	}				
	else
	{
		g_PVTPara.remoteIP[0] = 0;
		g_PVTPara.remoteIP[1] = 0;
		g_PVTPara.remoteIP[2] = 0;
		g_PVTPara.remoteIP[3] = 0;
	}
}
//reg已经-0x0300
void set_pvt_para(unsigned short reg,unsigned short data)
{ 	
	int  num = 0,i = 0,abc = 0;
	char tmp[6] = {0,0,0,0,0,0};
//00	unsigned short 		device_ID;							//设备ID 占3个字节24位   最大 65535
//01  unsigned short 		collect_frq;						//采集频率 单位s		1=wifi udp和188通讯   2=rj45 upd和188通讯  rs485采集命令发送间隔  ms
//02  unsigned short 		send_frq;    						//主动发送频率   暂无  但保留 
//03	short   int     	TMax;										//通道1温度上限 实际温度*10   主负载继电器闭合高压
//04  short   int     	TMin;										//通道1温度下限			主负载继电器开路低压
//05  short   int     	HMax;										//通道1湿度上限			备负载继电器闭合高压
//06  short   int    		HMin;										//通道1湿度下限			备负载继电器开路低压
//07	short   int     	TXZ[0];
//08  short   int     	TXZ[1];									//温度修正	0=启动发电机电池电压偏低   1=外壳风扇打开高温
//09  short   int     	HXZ[0];									//湿度修正  0=停止发电机电池电压偏高   1=外壳风扇断开低温  
//10	short   int     	HXZ[1];									//湿度修正  0=停止发电机电池电压偏高   1=外壳风扇断开低温  
//11	short 	int 			VMin;       						//电压下限   -->  2014-08-14 目前修改为掉电是否（短信,电话）报警,0表示不报   本地UTC偏移（分钟）
//12  unsigned short 	int  		bCH[0];						//通道1是否工作    0=电池类型   1=开机负载连接使能
//13	unsigned short 	int  		bCH[1];						//通道1是否工作    0=电池类型   1=开机负载连接使能
//14  unsigned short 	int  		alarmDelyMinute;	//警报检测延时 单位Min 		最多255min即4个小时15min。 mppt个数
//15  unsigned short 	int  		beep;        			//1表示开启beep,0表示关闭beep  暂时无用
//16	unsigned short 	int  		wifi_mode;   			//通讯方式  15 
//17  unsigned short 	int 		b_light_work;			//硬件工作模式
//18  unsigned short 	int			b_gprs_work;			//gprs是否工作 1表示工作  0xff表示未配置，默认工作	   其他值表示不工作  不工作则设置成0
//19	unsigned short 	int			b_wifi_work;			//wifi是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0 
//20	unsigned short 	int			b_rj45_work;			//rj45是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0
//21	unsigned short 	int			b_debug_work;			//debug 是否工作,=1表示工作，否则不工作。com1打印调试信息。gprs的。
//22	unsigned short 	int			reset_time;				//负载断开等待时间  min	
//23	unsigned short 	int			b_Sms_Test;				//停止发电机PV高压
//24	unsigned short 	int     b_Sms_FxiTime;		//电池容量	
//25  unsigned short 	int			remotePort;     	//接收端端口  
//26	unsigned char  					remoteIP[2];      //GPRS接收端IP 255.255.255.255    	  g_configRead.remoteIP
//27	unsigned char  					remoteIP[2];      //GPRS接收端IP 255.255.255.255 
	wdt();
	if(reg==0)
	{
			num = (unsigned int)data;
			for(i=0;i<6;i++)
			{
				g_configRead.device_ID[5-i] = (num%10) + '0';
				num = num/10;
			}
	}
	else if(reg==1)
	{
		g_configRead.collect_frq = data;
	}
	else if(reg==2)
	{
		g_configRead.send_frq = data;
	}
	else if(reg==3)
	{
		g_configRead.TMax[1] = (short int)data;
		g_EssentialLoadRelayCloseVoltage 		= g_configRead.TMax[1]/10.0 ;		
	}	
	else if(reg==4)
	{
		g_configRead.TMin[1] = (short int)data;
		g_EssentialLoadRelayOpenVoltage 		= g_configRead.TMin[1]/10.0;
	}	
	else if(reg==5)
	{
		g_configRead.HMax[1] = (short int)data;
		g_NonEssentialLoadRelayCloseVoltage = g_configRead.HMax[1]/10.0;
	}
	else if(reg==6)
	{
		g_configRead.HMin[1] = (short int)data;
		g_NonEssentialLoadRelayOpenVoltage 	= g_configRead.HMin[1]/10.0;
	}	
	else if(reg==7)
	{
		g_configRead.TXZ[0] = (short int)data;
	}
	else if(reg==8)
	{
		g_configRead.TXZ[1] = (short int)data;
	}
	else if(reg==9)
	{
		g_configRead.HXZ[0] = (short int)data;
	}
	else if(reg==10)
	{
		g_configRead.HXZ[1] = (short int)data;
	}
	else if(reg==11)
	{
		g_configRead.VMin = (short int)data;
	}
	else if(reg==12)
	{
		g_configRead.bCH[0] = data;
	}
	else if(reg==13)
	{
		g_configRead.bCH[1] = data;
	}
	else if(reg==14)
	{
		g_configRead.alarmDelyMinute = data;
	}
	else if(reg==15)
	{
		g_configRead.beep = data;
	}
	else if(reg==16)
	{
		g_configRead.wifi_mode = data;
	}	
	else if(reg==17)
	{
		g_configRead.b_light_work = data;
	}
	else if(reg==18)
	{
		g_configRead.b_gprs_work = data;
	}
	else if(reg==19)
	{
		g_configRead.b_wifi_work = data;
	}
	else if(reg==20)
	{
		g_configRead.b_rj45_work = data;
	}
	else if(reg==21)
	{
		g_configRead.b_debug_work = data;
	}
	else if(reg==22)
	{
		g_configRead.reset_time = data;
	}	
	else if(reg==23)
	{
		g_configRead.b_Sms_Test = data;
	}
	else if(reg==24)
	{
		g_configRead.b_Sms_FxiTime = data;
	}
	else if(reg==25)		//26  g_configRead.remotePort
	{	
		 num = data;
		 sprintf(tmp,"%d",num);
		 g_configRead.PortLen = strlen(tmp);
		 memcpy(g_configRead.remotePort,tmp,g_configRead.PortLen);  
	}
	
	//unsigned char  		remoteIP[4];      //GPRS接收端IP 255.255.255.255    28
	
	else if(reg==28)
	{
		g_configRead.TMax2 = (short int)data;
		g_EssentialLoadRelayCloseVoltage_Sec 		= g_configRead.TMax2/10.0 ;
	}	
	else if(reg==29)
	{
		g_configRead.TMin2 = (short int)data;
		g_EssentialLoadRelayOpenVoltage_Sec 		= g_configRead.TMin2/10.0;
	}	
	else if(reg==30)
	{
		g_configRead.HMax2 = (short int)data;
		g_NonEssentialLoadRelayCloseVoltage_Sec = g_configRead.HMax2/10.0;
	}
	else if(reg==31)
	{
		g_configRead.HMin2 = (short int)data;
		g_NonEssentialLoadRelayOpenVoltage_Sec = g_configRead.HMin2/10.0;
	}	
	else if(reg==32)
	{
		g_configRead.save_frq = (short int)data;
	}	
	WriteConfigParaFromIICAll();
}

//对Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len] modbus协议分析
void Local_RTU(unsigned char channel)	
{					
					t_BaudCfg	ttt;
					unsigned  char    *p;
					unsigned  char		DiDo[4]={0x40,0x55,0x00,0x00};
					unsigned  char		Byte_Pos = 0,Bit_Pos=0;
					//unsigned 	char	cdi=0,cdo=0,csw=0;
					unsigned 	char		cdi=0;
					unsigned  short		cnt = 0,pos=0,pos2=0;
					unsigned  short		i = 0;
					unsigned 	char		success = 0;					
					unsigned short 		nCRC = 0;					
					unsigned	char		nCRC_H=0,nCRC_L=0;
					unsigned 	char		big_endian = 0;	
					unsigned short    regAddr = 0;	//寄存器开始地址
					unsigned short		regCnt	= 0;	//寄存器个数
					unsigned short		actualRegCnt = 0;
					unsigned short		regAddrS1 = 0xff;	//0xff表示不存在
					unsigned short		regAddrS2 = 0xff;	//0xff表示不存在
					//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++对数据合法性验证+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
					//01 读bit 	举例: 03 01 ab cd 07 d0 crc16  (03=modbus id)  01=读bit   0xabcd(读bit的start 0~0xffff)    07d0(bit数量 最大2000)			合计8字节
					//02 读bit	举例: 同上01
					//03 读16bit举例: 03 01 ab cd 00 7d crc16  (03=modbus id)  03=读bit   0xabcd(读16bit的start 0~0xffff)  007d(16bit数量 最大125)		合计8字节
					//04 读16bit举例: 同上03
					//05 写bit	举例: 03 05 ab cd 00 00 crc16 或 03 05 ab cd ff 00 crc16  0xabcd表示reg地址  0x0000释放继电器  0xff00吸合继电器				合计8字节
					//06 写16bit举例: 03 06 ab cd 12 34 crc16 向0xabcd reg中写0x1234																																	合计8字节
					//15 写bit	举例: 03 0f ab cd 00 12 03 03 ff ff crc16  表示向0xabcd reg开始连续0x12个reg写线圈 3字节数据  0x03ffff位写入的数据。	变长
					//16 写16bit举例: 03 10 ab cd 00 7b F6 ......(f6个数)	crc16	表示向0xabcd开始连续0x007b(最大)个16bit reg写数据,数据长度为F6(7B*2)  后面为要写入的数据  边长
	
					//https://wenku.baidu.com/view/3f37289104a1b0717ed5dd4e.html
					wdt();
					nCRC = CrcCheck(Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len-2);		
					nCRC_H = (unsigned char)((nCRC>>8)&0xFF);										
					nCRC_L = (unsigned char)(nCRC&0xFF);	
					if(nCRC_H==Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-1] && nCRC_L==Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-2])
					{							
							big_endian = 1;
					}
					else if(nCRC_H==Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-2] && nCRC_L==Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-1])
					{							
							big_endian = 0;
					}
					else					//crc不正确
					{
							return;
					}								
					
					Com.ucCommand = Com.RX_Analy_Buf[1];							//modbus command
					if(Com.ucCommand==0x10||Com.ucCommand==0x0f)			//该命令数据总长度
					{
							Com.ustmpInt = Com.RX_Analy_Buf[6] + 9;																									//有效数据长度
					}	
					else if(Com.ucCommand > 0 && Com.ucCommand < 7)		//该命令数据总长度
					{
							Com.ustmpInt = 8;
					}
					else	//暂时不支持该命令
					{
							return;		
							//向调试口打印
					}					
					
					//++++++++++++++接收到的数据长度是否有效判断+++++++++++++++++++++++++++
					if(Com.ustmpInt==Com.usRX_Analy_Buf_Len)												//实际接受到的数据=待分析最少数据量  协议解析
					{  
							//COM1_Send_Data(Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);	//com1  echo 测试
							//通讯协议，规定了如下数据
#if 0
1. ads1247 modbus 协议完成		Ads1247_Value[8]  32bit
//模拟量协议:  01 03 00 00 00 08 44 0c  通道0~7数据高16bit
//模拟量协议:  01 03 00 0a 00 08 64 0e  通道0~7数据低8bit
//模拟量协议:  01 03 00 c8 00 01 05 f4  模块地址
//模拟量协议:  01 03 00 c9 00 01 xx xx  模块波特率
//模拟量协议:  01 03 00 d2 00 01 xx xx  模块名称  返回 0028
//模拟量协议:  01 03 00 dc 00 01 xx xx  模块通道状态   返回 0x00ff
						
2. di  modbus协议完成
3. do modbus协议完成，有写，控制部分
4. 内部ad传感器列表
   环境温度
   蓄电池温度
   蓄电池电压
   12V电压测量 (输入变->12V)
#endif
						Com.uctmpID = Com.RX_Analy_Buf[0];													//modbus id		
						if(Com.uctmpID == gs_SaveNetIPCfg.g_ModbusID )															//第1字节	Com.uctmpID modbus id
						{	
							regAddr = (Com.RX_Analy_Buf[2]<<8)|Com.RX_Analy_Buf[3];		//reg地址
							regCnt  = (Com.RX_Analy_Buf[4]<<8)|Com.RX_Analy_Buf[5];		//regAddr个数							
							if(Com.ucCommand==0x03 || Com.ucCommand==0x04)						//第2字节	Com.ucCommand  命令号  0x03 0x04命令
							{								
								//if(regAddr<0x16)							//开始地址 ads1247高2字节地址为 [0 7] ;ads1247低1字节地址为 [10 17]
								{
									//模拟量协议:  01 03 00 00 00 08 44 0c  通道0~7数据高16bit
									//模拟量协议:  01 03 00 0a 00 08 64 0e  通道0~7数据低8bit
									if(regCnt>0x7d)							//modbus规定最大0x7d个reg
									{
										regCnt = 0x7d;																				
									}
									//准备返回数据	
									actualRegCnt = regCnt*2;
									Com.TX_Buf[0] = Com.uctmpID;
									Com.TX_Buf[1] = Com.ucCommand;
									//Com.TX_Buf[2] = (actualRegCnt>>8)&0xff;
									Com.TX_Buf[2] = actualRegCnt&0xff;											
									memset(Com.TX_Buf+3,0,actualRegCnt);		//默认全部清零		
									//j = 0;
									//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
									if(regAddr<8)								//高16bit ad数据装填
									{
										regAddrS1 = regAddr;			//有效数据开始位置ad 16bit开始位置  高16bit
										regAddrS2 = 8-regAddr;		//reg个数(最多)
										if(regAddrS2>regCnt)			//regCnt=reg最多数量
										{
												regAddrS2 = regCnt;		//实际reg数量
										}
										for(i=0;i<regAddrS2;i++)	
										{
												Com.TX_Buf[3+2*i] 	= (Ads1247_Value[regAddrS1+i]>>16)&0xff;
												Com.TX_Buf[3+2*i+1] = (Ads1247_Value[regAddrS1+i]>>8)&0xff;												
										}										
									}
									//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
									//低8bit ad数据装填											
									regAddrS1 = regAddr+regCnt-1;					//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]
									//存在的问题，当regAddr+regCnt-1  > 0xffff则会出现问题
									if(regAddrS1<10 || regAddr>17)				//有效位置[10  17]  当开始位置regAddr>17则无效；  当结束位置regAddrS1<10则无效
									{
											;
									}
									else
									{
											//部分数据落有效位置  reg[10  17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=10;i<18;i++)				//统计[10~17] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;								//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;						//[10 ~ 17] 开始位置														
														pos2 = i-regAddr;		//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}
												}
											}
											//pos=[10 ~ 17] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											for(i=0;i<cnt;i++)
											{												
												Com.TX_Buf[3+2*pos2+1+2*i] = Ads1247_Value[pos-10+i]&0xff;												
											}											
									}
									//有效数据开始位置ad 8bit开始位置   低8bit		
									//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
									//电池电压;12V实际电压;    rts温度;电路板的温度。 温度为有符号，2字节，最小单位°，分正负  ;   电压*100，有符号，整数。 10.11则发送数据为1011
									//地址0x20   依次为电池电压，12V电压，pcb温度，rts温度
									regAddrS1 = regAddr+regCnt-1;					//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]
									//存在的问题，当regAddr+regCnt-1  > 0xffff则会出现问题
									if(regAddrS1<0x20 || regAddr>0x25)				//有效位置[0x20  0x23]  当开始位置regAddr>17则无效；  当结束位置regAddrS1<10则无效
									{
											;
									}
									else
									{
											//部分数据落有效位置  reg[10  17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=0x20;i<0x26;i++)				//统计[0x20~0x23] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;									//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;						//[0x20 ~ 0x23] 开始位置														
														pos2 = i-regAddr;		//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}
												}
											}
											//pos=[0x20 ~ 0x23] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											for(i=0;i<cnt;i++)
											{												
												Com.TX_Buf[3+2*pos2+2*i] = Adc_Data[pos-0x20+i]&0xff;							//低位在前，高位在后	
												Com.TX_Buf[3+2*pos2+1+2*i] = (Adc_Data[pos-0x20+i]>>8)&0xff;			//低位在前，高位在后	
											}											
									}
									//有效数据开始位置ad 8bit开始位置   低8bit			ble+wifi=10  lora=10  4g=40  rj45=10
									
									//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
									//4055 di do模块			地址读返回 40 55 00 00			
									#if 0									
									regAddrS1 = regAddr+regCnt-1;							//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]
									//存在的问题，当regAddr+regCnt-1  > 0xffff则会出现问题
									if(regAddrS1<0xD2 || regAddr>0xD3)				//有效位置[D2  D3]  当开始位置regAddr>17则无效；  当结束位置regAddrS1<10则无效
									{
											;
									}
									else
									{
											//部分数据落有效位置  reg[10  17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=0xD2;i<0xD4;i++)				//统计[10~17] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;								//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;						//[10 ~ 17] 开始位置														
														pos2 = i-regAddr;		//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}
												}
											}
											//pos=[10 ~ 17] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											for(i=0;i<cnt;i++)
											{												
												Com.TX_Buf[3+2*pos2+2*i] 		= DiDo[pos-0xD2+2*i]&0xff;												
												Com.TX_Buf[3+2*pos2+1+2*i] 	= DiDo[pos-0xD2+2*i+1]&0xff;												
											}											
									}		
									#endif
									//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
									//串口参数 [0x00e0 0x00ed] 							8*3=24个int  合计14个reg  modbuid(1)  chebit[7](7)  baud[5](20)
									regAddrS1 = regAddr+regCnt-1;							//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]
									//存在的问题，当regAddr+regCnt-1  > 0xffff则会出现问题
									if(regAddrS1<0x00e0 || regAddr>0x00ed)				//有效位置[0x0100  0x012f]  当开始位置regAddr>17则无效；  当结束位置regAddrS1<10则无效
									{
											;
									}
									else
									{
											//部分数据落有效位置  reg[10  17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=0x00e0;i<0x00ee;i++)				//统计[10~17] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;											//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;									//[10 ~ 17] 开始位置														
														pos2 = i-regAddr;					//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}	
												}
											}
											//pos=[10 ~ 17] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											if(cnt>0)
											{
												ttt.g_ModbusID = gs_SaveNetIPCfg.g_ModbusID;
												ttt.chekbit[0] = gs_SaveNetIPCfg.chekbit[0];
												ttt.chekbit[1] = gs_SaveNetIPCfg.chekbit[1];
												ttt.chekbit[2] = gs_SaveNetIPCfg.chekbit[2];
												ttt.chekbit[3] = gs_SaveNetIPCfg.chekbit[3];
												ttt.chekbit[4] = gs_SaveNetIPCfg.chekbit[4];												
												ttt.chekbit[5] = 0;
												ttt.chekbit[6] = 0;
												
												ttt.baud[0] = gs_SaveNetIPCfg.baud[0];
												ttt.baud[1] = gs_SaveNetIPCfg.baud[1];
												ttt.baud[2] = gs_SaveNetIPCfg.baud[2];
												ttt.baud[3] = gs_SaveNetIPCfg.baud[3];
												ttt.baud[4] = gs_SaveNetIPCfg.baud[4];											
												
												p = &ttt.g_ModbusID;			//开始地址
												
												memcpy(Com.TX_Buf+3+2*pos2,p+pos-0x00e0,cnt*2);															
											}													
									}		
									
									//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
									//ad校准参数 [0x0100 0x012f] 8*3=24个int  合计48个reg
									regAddrS1 = regAddr+regCnt-1;							//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]
									//存在的问题，当regAddr+regCnt-1  > 0xffff则会出现问题
									if(regAddrS1<0x0100 || regAddr>0x012f)				//有效位置[0x0100  0x012f]  当开始位置regAddr>17则无效；  当结束位置regAddrS1<10则无效
									{
											;
									}
									else
									{
											//部分数据落有效位置  reg[10  17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=0x0100;i<0x0130;i++)				//统计[10~17] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;								//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;						//[10 ~ 17] 开始位置														
														pos2 = i-regAddr;		//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}
												}
											}
											//pos=[10 ~ 17] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											if(cnt>0)
											{
												p = &g_Ads1247_Cali.ucStartFlag;		//开始地址
												memcpy(Com.TX_Buf+3+2*pos2,p+4+pos-0x0100,cnt*2);															
											}													
									}		
									
									//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
									//rj45所有参数 [0x0140 0x0152] 		38byte  合计19个reg
									regAddrS1 = regAddr+regCnt-1;							//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]
									//存在的问题，当regAddr+regCnt-1  > 0xffff则会出现问题
									if(regAddrS1<0x0140 || regAddr>0x0152)				//有效位置[0x0100  0x012f]  当开始位置regAddr>17则无效；  当结束位置regAddrS1<10则无效
									{
											;
									}
									else
									{
											//部分数据落有效位置  reg[10  17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=0x0140;i<0x0153;i++)				//统计[10~17] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;								//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;						//[10 ~ 17] 开始位置														
														pos2 = i-regAddr;		//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}
												}
											}
											//pos=[10 ~ 17] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											if(cnt>0)
											{
												p = &gs_SaveNetIPCfg.ucStartFlag;		//开始地址
												memcpy(Com.TX_Buf+3+2*pos2,p+4+pos-0x0140,cnt*2);															
											}													
									}		
									
									//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
									//pvt-6逻辑 整个系统的状态 [0x0200 0x021e] 		62byte  合计31个reg
									regAddrS1 = regAddr+regCnt-1;									//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]
									//存在的问题，当regAddr+regCnt-1  > 0xffff则会出现问题
									if(regAddrS1<0x0200 || regAddr>0x0246)				//有效位置[0x0200  0x021e]  当开始位置regAddr>17则无效；  当结束位置regAddrS1<10则无效
									{
											;
									}
									else
									{
											//部分数据落有效位置  reg[10  17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=0x0200;i<0x0247;i++)				//统计[10~17] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;									//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;							//[10 ~ 17] 开始位置														
														pos2 = i-regAddr;			//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}
												}
											}
											//pos=[10 ~ 17] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											if(cnt>0)
											{
												char tmp[8] = {0,0,0,0,0,0,0,0};
												tmp[0] = g_configRead.device_ID[0];
												tmp[1] = g_configRead.device_ID[1];
												tmp[2] = g_configRead.device_ID[2];
												tmp[3] = g_configRead.device_ID[3];
												tmp[4] = g_configRead.device_ID[4];
												tmp[5] = g_configRead.device_ID[5];	
												g_Result.device_ID  			= atoi(tmp);
												//unsigned short			b_gprs_work;			//gprs是否工作 1表示工作  0xff表示未配置，默认工作	   其他值表示不工作  不工作则设置成0
												//unsigned short			b_wifi_work;			//wifi是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0 
												//unsigned short			b_rj45_work;			//rj45是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0
												//unsigned short 			b_TH_work;				//增加部分  bit 0控制  第1路温湿度    bit1控制第2路温湿度        bit4控制a/c  bit5控制fan
												//unsigned short 			device_ID;				//设备ID 占3个字节24位   最大 65535
												g_Result.b_gprs_work = g_configRead.b_gprs_work;
												g_Result.b_wifi_work = g_configRead.b_wifi_work;
												g_Result.b_rj45_work = g_configRead.b_rj45_work;
												g_Result.b_TH_work   = g_configRead.save_frq;													
												g_Result.alarmDelyMinute = g_configRead.alarmDelyMinute	;											
												p = &g_Result.load1_ctrl;		//开始地址
												memcpy(Com.TX_Buf+3+2*pos2,p+pos-0x0200,cnt*2);															
											}													
									}		
									
									//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
									//pvt-6 参数配置 [0x0300 0x031B] 		62byte  合计31个reg
									regAddrS1 = regAddr+regCnt-1;									//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]
									//存在的问题，当regAddr+regCnt-1  > 0xffff则会出现问题
									if(regAddrS1<0x0300 || regAddr>0x0323)				//有效位置[0x0300  0x0312]  当开始位置regAddr>17则无效；  当结束位置regAddrS1<10则无效
									{
											;
									}
									else
									{
											//部分数据落有效位置  reg[10  17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=0x0300;i<0x0324;i++)				//统计[10~17] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;											//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;									//[10 ~ 17] 开始位置														
														pos2 = i-regAddr;					//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}
												}
											}
											//pos=[10 ~ 17] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											if(cnt>0)
											{
												//进行数据转换
												
												//数据转换		
												get_pvt_para();
												//时间进行转换  Year,Month,Day,Hour,Minute,Second
												rtc_second_cal_active_date();				//时间进行转换
												g_PVTPara.sysctrl_time[0] = Year - 2000  ;												
												g_PVTPara.sysctrl_time[1] = Month  ;	
												g_PVTPara.sysctrl_time[2] = Day    ;	
												g_PVTPara.sysctrl_time[3] = Hour   ;	
												g_PVTPara.sysctrl_time[4] = Minute ;	
												g_PVTPara.sysctrl_time[5] = Second ;	
												
												p = (unsigned char*)&g_PVTPara;		//开始地址
												
												memcpy(Com.TX_Buf+3+2*pos2,p+pos-0x0300,cnt*2);															
											}													
									}		
									
																		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
									//pvt-6逻辑 整个系统的状态 [0x0200 0x021e] 		62byte  合计31个reg
									regAddrS1 = regAddr+regCnt-1;									//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]
									//存在的问题，当regAddr+regCnt-1  > 0xffff则会出现问题
									if(regAddrS1<0x0400 || regAddr>0x0465)				//有效位置[0x0200  0x021e]  当开始位置regAddr>17则无效；  当结束位置regAddrS1<10则无效
									{
											;
									}
									else
									{
											//部分数据落有效位置  reg[10  17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=0x0400;i<0x0466;i++)				//统计[10~17] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;									//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;							//[10 ~ 17] 开始位置														
														pos2 = i-regAddr;			//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}
												}
											}
											//pos=[10 ~ 17] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											if(cnt>0)
											{
												p = (unsigned char *)&g_Result2.ff_E_UIt_J_Sum[0][0];		//开始地址
												memcpy(Com.TX_Buf+3+2*pos2,p+pos-0x0400,cnt*2);															
											}													
									}	
									
									actualRegCnt = actualRegCnt+3;
									success = 1;
								}
							}
							else if(Com.ucCommand==0x01||Com.ucCommand==0x02)				//0x01 0x02命令  读bit
							{
									//01 读bit 	举例: 03 01 ab cd 07 d0 crc16  (03=modbus id)  01=读bit   0xabcd(读bit的start 0~0xffff)    07d0(bit数量 最大2000)			合计8字节
									//02 读bit	举例: 同上01
									//01 01 00 10 00 08 3C 09 01 01 01 9F 11 E0 
									//01 01 00 00 00 08 3D CC 01 01 01 00 51 88 
									//模拟量协议:  01 03 00 00 00 08 44 0c  通道0~7数据高16bit
									//模拟量协议:  01 03 00 0a 00 08 64 0e  通道0~7数据低8bit
									if(regCnt>1280)							//modbus规定最大0x07d0个bit
									{
										regCnt = 1280;																				
									}
									
									actualRegCnt = regCnt/8;
									if((regCnt%8)!=0)
									{
										actualRegCnt++;														//返回数据字节数，不足一字节则按1字节处理
									}									
									Com.TX_Buf[0] = Com.uctmpID;
									Com.TX_Buf[1] = Com.ucCommand;
									Com.TX_Buf[2] = actualRegCnt&0xff;																				
									memset(Com.TX_Buf+3,0,actualRegCnt);				//默认全部清零		
							//		j = 2;
									//DI地址0x00, DO地址0x10 , 拨码盘地址0x20		 android(0x18)		
									//bit排列格式说明   bit0=1.bit0 ... bit7=1.bit7   ; 第二字节 bit8=2.bit0 ... bit15=2.bit7;  第三字节  bit16=3.bit0 ... bit23=3.bit7; ......
									if(regAddr<8)									//高16bit ad数据装填	di
									{
										regAddrS1 = regAddr;				//有效数据开始位置
										regAddrS2 = 8-regAddr;			//reg个数(最多)
										if(regAddrS2>regCnt)				//regCnt=reg最多数量
										{
												regAddrS2 = regCnt;			//实际reg数量
										}
										cdi = 0;
										for(i=0;i<regAddrS2;i++)		//小于8bit  1字节一定够用
										{												
											cdi = cdi | (DI.bInput_State[regAddrS1+i]<<i);											
										}	
								//		j++;
										Com.TX_Buf[3] = cdi;
									}									
																		
										//do 0[x10	0x17]		 + 	android(0x18)		
										regAddrS1 = regAddr+regCnt-1;					//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]
										if(regAddrS1<16 || regAddr>24)				//有效位置[16 23]-->[16 24]  当开始位置regAddr>23则无效；  当结束位置regAddrS1<16则无效  do  android为24
										{
											;
										}
										else
										{
											//部分数据落有效位置  reg[0x10  0x17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=16;i<25;i++)				//统计[16~23] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;								//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;						//[0x10 ~ 0x17] 开始位置-->[0x10 ~ 0x18] 开始位置																												
														pos2 = i-regAddr;		//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}
												}
											}
											//pos=[10 ~ 17] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											//cdo = 0;
											for(i=0;i<cnt;i++)
											{												
													//cdo = cdo | (Do_Sta[pos-16+i]<<i);
													Byte_Pos = pos2/8;			//字节地址
													Bit_Pos  = pos2%8;			//字节内的bit位置  0 ~ 7
													if((pos-16+i)<8)
													{
														Com.TX_Buf[3+Byte_Pos] |= ((Do_Sta[pos-16+i]&0x01)<<Bit_Pos);
													}
													else
													{
														Com.TX_Buf[3+Byte_Pos] |= ((Android_Sta&0x01)<<Bit_Pos);//android
													}
													pos2++;
											}												
										}									
									
										//拨码盘sw 	地址有效	do 0[x20	0x27]	
										regAddrS1 = regAddr+regCnt-1;					//regAddr开始位置  reg结束位置regAddrS1   [regAddr , regAddrS1]	sw
										if(regAddrS1<32 || regAddr>39)				//有效位置[32 39]  当开始位置regAddr>23则无效；  当结束位置regAddrS1<16则无效
										{
											;
										}
										else
										{
											//部分数据落有效位置  reg[10  17]为目标reg   [regAddr , regAddrS1]待读的reg位置
											cnt = 0;
											for(i=32;i<40;i++)				//统计[32~39] 有多少个落在[regAddr , regAddrS1]区间(待读reg地址)
											{
												if(i>=regAddr && i<=regAddrS1)
												{
													cnt++;								//统计有效数据个数
													if(cnt==1)
													{
														pos  = i;						//[32 ~ 39] 开始位置														
														pos2 = i-regAddr;		//[regAddr , regAddrS1]对应开始位置,regAddr位置开始，到第一个匹配点的距离为pos2
													}
												}
											}
											//pos=[32 ~ 39] 开始位置;	cnt有几个reg;	pos2=在[regAddr , regAddrS1]中从regAddr开始的位置
											//csw = 0;											
											for(i=0;i<cnt;i++)
											{												
												//	csw = csw | (Sw_Sta[pos-32+i]<<i);
													Byte_Pos = pos2/8;	//字节地址
													Bit_Pos  = pos2%8;	//字节内的bit位置  0 ~ 7
													Com.TX_Buf[3+Byte_Pos] |= ((Sw_Sta[pos-32+i]&0x01)<<Bit_Pos);
													pos2++;
											}												
										}																									
									actualRegCnt = actualRegCnt+3;
									success = 1;								
							}
							else if(Com.ucCommand==0x05)						//0x05命令 写单个线圈
							{
								//01 05 00 13 FF 00 7D FF          		返回:01 05 00 13 FF 00 7D FF
								Com.TX_Buf[0] = Com.RX_Analy_Buf[0];
								Com.TX_Buf[1] = Com.RX_Analy_Buf[1];
								Com.TX_Buf[2] = Com.RX_Analy_Buf[2];
								Com.TX_Buf[3] = Com.RX_Analy_Buf[3];
								Com.TX_Buf[4] = Com.RX_Analy_Buf[4];
								Com.TX_Buf[5] = Com.RX_Analy_Buf[5];	
								Com.TX_Buf[6] = Com.RX_Analy_Buf[6];	
								Com.TX_Buf[7] = Com.RX_Analy_Buf[7];	
								
								actualRegCnt = 6;										
								
								if(regAddr>0x0f&&regAddr<0x18)				//[0x10 0x17]分别代表8个reg
								{
									if(regCnt==0xff00)
									{
										DO_Output_Ctrl(regAddr-0x10,D_ON);
									}
									else if(regCnt==0x0000)
									{
										DO_Output_Ctrl(regAddr-0x10,D_OFF);
									}										
								}
								else if(regAddr>0x1f&&regAddr<0x24)						//0x20 表示屏幕电源控制,0x21表示led1控制,0x22表示led2控制,0x23表示系统重启
								{
									if(regAddr==0x20)														//电源控制 android
									{
										if(regCnt==0xff00)
										{
											Android_Ctrl(true);
										}
										else if(regCnt==0x0000)
										{
											Android_Ctrl(false);
										}																						
									}
									else if(regAddr>0x20&&regAddr<0x23)					//led控制
									{
										if(regCnt==0xff00)
										{
											Led_Ctrl(regAddr-0x21,true);
										}
										else if(regCnt==0x0000)
										{
											Led_Ctrl(regAddr-0x21,false);
										}																				
									}																
									else if(regAddr==0x23)											//reset system
									{				
										if(regCnt==0xff00)
										{	
											Com.usRX_Analy_Buf_Len = 8;
											memcpy(Com.RX_Analy_Buf,Com.TX_Buf,8);
											Com_Send(channel,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);											//COM1_Send_Data(Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);	
											delay_1ms(600);
											__set_FAULTMASK(1); 		//¹رՋùӐ֐¶
											nvic_system_reset();
										}
									}									
								}
								else
								{
									Com.TX_Buf[1] = 0x85;
									Com.TX_Buf[2] = 0x02;
									actualRegCnt = 3;
								}							
								success = 1;								
							}
							else if(Com.ucCommand==0x06)				//0x06命令 						写16bit举例: 03 06 ab cd 12 34 crc16 向0xabcd reg中写0x1234
							{									
							//	ch =  Com.RX_Analy_Buf[3];		//通道
							//	sel = Com.RX_Analy_Buf[5];		//校准点																
								if(regAddr < 8 && regCnt < 3)  		//if(Com.RX_Analy_Buf[2]==0 && Com.RX_Analy_Buf[3]<8 && Com.RX_Analy_Buf[4]==0 && Com.RX_Analy_Buf[5]<3)  //专门用于校准
								{
									Com.usRX_Analy_Buf_Len = 8;							
									Com_Send(channel,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);
									ads1247_calibrate(Com.RX_Analy_Buf[3],Com.RX_Analy_Buf[5]);
								}		
								
								if(regAddr > 0x02ff && regAddr < 0x0321) 	//regCnt 数据
								{
									Com.usRX_Analy_Buf_Len = 8;							
									Com_Send(channel,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);
									set_pvt_para(regAddr-0x0300,regCnt);
								}		
								
								return;
							}
							else if(Com.ucCommand==0x0f)				//0x0f命令15 					写bit	举例: 03 0f ab cd 00 12 03 03 ff ff crc16  表示向0xabcd reg开始连续0x12个reg写线圈 3字节数据  0x03ffff位写入的数据。	变长
							{
							
							}
							else if(Com.ucCommand==0x10)				//0x10命令16 					写16bit举例: 03 10 ab cd 00 7b F6 ......(f6个数)	crc16	表示向0xabcd开始连续0x007b(最大)个16bit reg写数据,数据长度为F6(7B*2)  后面为要写入的数据  边长
							{
									Com.TX_Buf[0] = Com.RX_Analy_Buf[0];
									Com.TX_Buf[1] = Com.RX_Analy_Buf[1];
									Com.TX_Buf[2] = Com.RX_Analy_Buf[2];
									Com.TX_Buf[3] = Com.RX_Analy_Buf[3];
									Com.TX_Buf[4] = Com.RX_Analy_Buf[4];
									Com.TX_Buf[5] = Com.RX_Analy_Buf[5];	
										//rj45地址如下:  0x0140开始合计	   	每个reg中低在前高在后
										//unsigned char ucMAC[6];						//mac地址				3
										//unsigned char ucDhcpMode[2];			//dhcp模式			1
										//unsigned char ucSelfIP[4];				//静态ip				2
										//unsigned char ucSubMASK[4];				//静态子网掩码	2
										//unsigned char ucGateWay[4];				//静态网关			2
									if(regAddr>0x013f && regAddr<0x014a)
									{
											if((regAddr+regCnt)<=0x014a)
											{
												if(regCnt>0)
												{
													p = gs_SaveNetIPCfg.ucMAC;											//开始地址
													memcpy(p+(regAddr-0x0140)*2,Com.RX_Analy_Buf+7,regCnt*2);		
													WriteParametersToIICAll();
												}		
											}	
									}	
									
									if(regAddr==0x031a)				//远程IP地址设置
									{											
											if(regCnt>1)
											{//01 10 01 40 00 03 06 00 5F 01 02 03 04 D5 AA 
												char  						remoteIP[16];      	//GPRS接收端IP 255.255.255.255
												unsigned char  		IPLen;             	//IP地址长度
												sprintf(remoteIP,"%d.%d.%d.%d",Com.RX_Analy_Buf[7],Com.RX_Analy_Buf[8],Com.RX_Analy_Buf[9],Com.RX_Analy_Buf[10]);
												g_configRead.IPLen = strlen(remoteIP);	
												memcpy(g_configRead.remoteIP,remoteIP,g_configRead.IPLen);
												WriteParametersToIICAll();
											}														
									}	
									
									if(regAddr==0x0321)				//设置时间
									{											
											if(3==regCnt)					//01 10 01 40 00 03 06 00 5F 01 02 03 04 D5 AA 
											{
												char  						remoteIP[16];      																		//GPRS接收端IP 255.255.255.255																										
												sprintf(remoteIP,"20%02d%02d%02d%02d%02d%02d",Com.RX_Analy_Buf[7],Com.RX_Analy_Buf[8],Com.RX_Analy_Buf[9],Com.RX_Analy_Buf[10],Com.RX_Analy_Buf[11],Com.RX_Analy_Buf[12]);
												PCF_setsystime(remoteIP,14);
											}														
									}										
									actualRegCnt = 6;		
									success = 1;												
							}
							else
							{
							
							}
							
							//////////////////////////数据全部组织好了，数据准备发////////////////////////////
							if(success==1)
							{
								nCRC = CrcCheck(Com.TX_Buf,actualRegCnt);								//CRC计算										
								nCRC_H = (unsigned char)((nCRC>>8)&0xFF);										
								nCRC_L = (unsigned char)(nCRC&0xFF);				
								if(big_endian==0)
								{
									Com.TX_Buf[actualRegCnt+0] = nCRC_H ;
									Com.TX_Buf[actualRegCnt+1] = nCRC_L	;
								}
								else
								{
									Com.TX_Buf[actualRegCnt+1] = nCRC_H ;
									Com.TX_Buf[actualRegCnt+0] = nCRC_L ;
								}	
								Com.usRX_Analy_Buf_Len = actualRegCnt+2;
								memcpy(Com.RX_Analy_Buf,Com.TX_Buf,Com.usRX_Analy_Buf_Len);			//Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len]
								Com_Send(channel,Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);			//COM1_Send_Data(Com.TX_Buf,actualRegCnt+2);							//数据发送		
							}
						}
					}
					//////////////////////////数据全部组织好了，数据准备发////////////////////////////											
								
					//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++对数据合法性验证+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	
					
						
					#if 0
					nCRC = CrcCheck(Com.RX_Analy_Buf,Com.usRX_Analy_Buf_Len);		
					nCRC_H = (unsigned char)(nCRC&0xFF);	
					nCRC_L = (unsigned char)((nCRC>>8)&0xFF);						
					if(big_endian==0)
					{
							Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-2] = nCRC_H ;
							Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-1] = nCRC_L	;
					}
					else
					{
							Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-1] = nCRC_H ;
							Com.RX_Analy_Buf[Com.usRX_Analy_Buf_Len-2] = nCRC_L ;
					}																		
					#endif					
}

#if 0
void RTU_Process_com3(void)
{	
		unsigned short		param_len = 0;	//参数长度
		unsigned short		total_len = 0 ;	//数据总长度
		unsigned char 		fix_format[8];	//固定格式判断
		unsigned 					com_port = 0;	
		unsigned short		i   = 0;

		//+++++++++++++modbus协议解析 将来自2440上的数据进行modbus解析 目的设置rs485的波特率和读本地温湿度+++++++++++++++++++++
		while(Com.Usart3.usRec_RD!=Com.Usart3.usRec_WR)
		{
			#if COM1_TEST
			COM3_Send_Data(Com.Usart3.RX_Buf+Com.Usart3.usRec_RD,1);
			Com.Usart3.usRec_RD = (Com.Usart3.usRec_RD+1)%D_USART_REC_BUFF_SIZE;
			#endif		
			
			Com.ucActiveRd = (D_USART_REC_BUFF_SIZE + Com.Usart3.usRec_WR - Com.Usart3.usRec_RD)%D_USART_REC_BUFF_SIZE;		//接收到的数据长度			
			if(Com.ucActiveRd>12)						//至少8个，0xfd 03 f0 f0 00 01 crc16	合计8个数据
			{
					fix_format[0] = Com.Usart3.RX_Buf[Com.Usart3.usRec_RD] ;															//*			[0]
					fix_format[1] = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD+1)%D_USART_REC_BUFF_SIZE];			//S			[1]
					fix_format[2] = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD+3)%D_USART_REC_BUFF_SIZE];			//|			[3]
					fix_format[3] = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD+5)%D_USART_REC_BUFF_SIZE];			//|			[5]
					if(fix_format[0]=='*' && fix_format[1]=='S' && fix_format[2] == '|' && fix_format[3] == '|')	//开始部分特征是否正确
					{
							param_len     = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD+4)%D_USART_REC_BUFF_SIZE];												//通道参数长度	[4]
							fix_format[4] = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD+5 + param_len+1)%D_USART_REC_BUFF_SIZE];					// |
							Com.usRX_Analy_Buf_Len = (Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD + 6 + param_len + 1)%D_USART_REC_BUFF_SIZE]<<8)|Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD + 6 + param_len + 2)%D_USART_REC_BUFF_SIZE];	//有效数据长度
							fix_format[5] = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD + 6 + param_len + 3)%D_USART_REC_BUFF_SIZE];			//{	
							total_len = 12 + param_len + Com.usRX_Analy_Buf_Len;																										//完整数据包总长度
						
							if(fix_format[4]=='|' && fix_format[5]=='{')
							{
								if(Com.ucActiveRd<total_len)							//数据长度不够，继续等待
								{
									break;
								}
								else
								{
									fix_format[6] = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD + total_len-2)%D_USART_REC_BUFF_SIZE];			//}		最后2字节
									fix_format[7] = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD + total_len-1)%D_USART_REC_BUFF_SIZE];			//#		最后2字节
								}
							}
							else
							{
								Com.Usart3.usRec_RD = (Com.Usart3.usRec_RD+1)%D_USART_REC_BUFF_SIZE;			//格式不正确 向后继续查找
								continue;
							}
							
							if(fix_format[6] == '}' && fix_format[7] == '#')																										//完整的格式
							{
									com_port  = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD+2)%D_USART_REC_BUFF_SIZE];										//转发通道号
									for(i=0;i<Com.usRX_Analy_Buf_Len;i++)
									{										
											Com.RX_Analy_Buf[i] = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD+i+10+param_len)%D_USART_REC_BUFF_SIZE];										
									}
									g_Ch_Par_Len = param_len;
									for(i=0;i<g_Ch_Par_Len;i++)		//通道参数
									{
										g_Ch_Param[i] = Com.Usart3.RX_Buf[(Com.Usart3.usRec_RD+i+6)%D_USART_REC_BUFF_SIZE];							//通道参数长度		最长128字节
									}		
									
									Com.Usart3.usRec_RD = (Com.Usart3.usRec_RD+total_len)%D_USART_REC_BUFF_SIZE;											//将队列中的部分数据删除							 	
									
									if(com_port=='2')					//本地数据处理
									{
											if(Com.usRX_Analy_Buf_Len>1)
											{
													Local_RTU(3);			//直接返回到1=com1  2=com2   3=com3   4=com4  5=com5
											}
											else
											{
													Retransmission(com_port);
											}
									}
									else									//转发数据处理
									{
											Retransmission(com_port);
									}									
									break;
							}
							else				//格式不完整，继续向后扫描
							{
									Com.Usart3.usRec_RD = (Com.Usart3.usRec_RD+1)%D_USART_REC_BUFF_SIZE;		
							}
					}
					else					//没有匹配到完整的数据格式，继续向后扫描
					{
							Com.Usart3.usRec_RD = (Com.Usart3.usRec_RD+1)%D_USART_REC_BUFF_SIZE;							
					}								
      }
			else			//数据长度不足够一个命令
			{
					break;
      }
	  }	
		//+++++++++++++modbus协议解析 将来自2440上的数据进行modbus解析 目的设置rs485的波特率和读本地温湿度+++++++++++++++++++++	
}
#endif

//协议必须带相关信息
//例如转 *rs485_xx|xxxx{}#  增加部分开销
//*S0|参数长度1字节|参数|2字节{}中的有效数据长度{}#
//example:	 *S0|0||08{01 03 00 00 00 01 84 0A}#     多出 12字节的开销  好处，可以任意协议，设备端不用关心
// *  S  0 |     |   |        {       									 }  #    
//2A 53 30 7C    7C  7C       7B 												 7D 23
//2A 53 30 7C 00 7C  7C 00 08 7B 01 03 00 00 00 01 84 0A 7D 23

void RTU_Process(void)
{	
		unsigned short		param_len = 0;	//参数长度
		unsigned short		total_len = 0 ;	//数据总长度
		unsigned char 		fix_format[8];	//固定格式判断
		unsigned 					com_port = 0;	
		unsigned short		i   = 0;

		//+++++++++++++modbus协议解析 将来自2440上的数据进行modbus解析 目的设置rs485的波特率和读本地温湿度+++++++++++++++++++++
		while(Com.Usart[COM_MASTER].usRec_RD!=Com.Usart[COM_MASTER].usRec_WR)			//Com.Usart[WIFI_BLE].RX_Buf[len] = '\0';
		{
			#if COM1_TEST
			COM1_Send_Data(Com.Usart[COM_MASTER].RX_Buf+Com.Usart[COM_MASTER].usRec_RD,1);
			Com.Usart[COM_MASTER].usRec_RD = (Com.Usart[COM_MASTER].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
			#endif		
			wdt();
			Com.ucActiveRd = (D_USART_REC_BUFF_SIZE + Com.Usart[COM_MASTER].usRec_WR - Com.Usart[COM_MASTER].usRec_RD)%D_USART_REC_BUFF_SIZE;		//接收到的数据长度			
			if(Com.ucActiveRd>12)						//至少8个，0xfd 03 f0 f0 00 01 crc16	合计8个数据
			{
					fix_format[0] = Com.Usart[COM_MASTER].RX_Buf[Com.Usart[COM_MASTER].usRec_RD] ;															//*			[0]
					fix_format[1] = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD+1)%D_USART_REC_BUFF_SIZE];			//S			[1]
					fix_format[2] = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD+3)%D_USART_REC_BUFF_SIZE];			//|			[3]
					fix_format[3] = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD+5)%D_USART_REC_BUFF_SIZE];			//|			[5]
					if(fix_format[0]=='*' && fix_format[1]=='S' && fix_format[2] == '|' && fix_format[3] == '|')	//开始部分特征是否正确
					{
							param_len     = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD+4)%D_USART_REC_BUFF_SIZE];												//通道参数长度	[4]
							fix_format[4] = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD+5 + param_len+1)%D_USART_REC_BUFF_SIZE];					// |
							Com.usRX_Analy_Buf_Len = (Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD + 6 + param_len + 1)%D_USART_REC_BUFF_SIZE]<<8)|Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD + 6 + param_len + 2)%D_USART_REC_BUFF_SIZE];	//有效数据长度
							fix_format[5] = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD + 6 + param_len + 3)%D_USART_REC_BUFF_SIZE];			//{	
							total_len = 12 + param_len + Com.usRX_Analy_Buf_Len;																																					//完整数据包总长度
						
							if(fix_format[4]=='|' && fix_format[5]=='{')
							{
								if(Com.ucActiveRd<total_len)							//数据长度不够，继续等待
								{
									break;
								}
								else
								{
									fix_format[6] = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD + total_len-2)%D_USART_REC_BUFF_SIZE];			//}		最后2字节
									fix_format[7] = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD + total_len-1)%D_USART_REC_BUFF_SIZE];			//#		最后2字节
								}
							}
							else
							{
								Com.Usart[COM_MASTER].usRec_RD = (Com.Usart[COM_MASTER].usRec_RD+1)%D_USART_REC_BUFF_SIZE;			//格式不正确 向后继续查找
								continue;
							}
							
							if(fix_format[6] == '}' && fix_format[7] == '#')																										//完整的格式
							{
									com_port  = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD+2)%D_USART_REC_BUFF_SIZE];										//转发通道号
									for(i=0;i<Com.usRX_Analy_Buf_Len;i++)
									{										
											Com.RX_Analy_Buf[i] = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD+i+10+param_len)%D_USART_REC_BUFF_SIZE];										
									}
									g_Ch_Par_Len = param_len;
									for(i=0;i<g_Ch_Par_Len;i++)					//通道参数
									{
										g_Ch_Param[i] = Com.Usart[COM_MASTER].RX_Buf[(Com.Usart[COM_MASTER].usRec_RD+i+6)%D_USART_REC_BUFF_SIZE];							//通道参数长度		最长128字节
									}		
									
									Com.Usart[COM_MASTER].usRec_RD = (Com.Usart[COM_MASTER].usRec_RD+total_len)%D_USART_REC_BUFF_SIZE;											//将队列中的部分数据删除							 	
									
									if(com_port-'0'==COM_MASTER)					//本地数据处理
									{
											if(Com.usRX_Analy_Buf_Len>1)			//有效数数据长度[2,~) 则进行协议识别
											{
													Local_RTU(COM_MASTER);				//直接返回到1=com1  2=com2
											}
											else															//0 或者 1则进入转发通道，可以进行通讯口参数设置。
											{
													Retransmission(com_port);
											}
									}
									else									//转发数据处理
									{
											Retransmission(com_port);
									}									
									break;
							}
							else											//格式不完整，继续向后扫描
							{
									Com.Usart[COM_MASTER].usRec_RD = (Com.Usart[COM_MASTER].usRec_RD+1)%D_USART_REC_BUFF_SIZE;		
							}
					}
					else					//没有匹配到完整的数据格式，继续向后扫描
					{
							Com.Usart[COM_MASTER].usRec_RD = (Com.Usart[COM_MASTER].usRec_RD+1)%D_USART_REC_BUFF_SIZE;							
					}								
      }
			else			//数据长度不足够一个命令
			{
					break;
      }
	  }	
		//+++++++++++++modbus协议解析 将来自2440上的数据进行modbus解析 目的设置rs485的波特率和读本地温湿度+++++++++++++++++++++	
}

//		Local_RTU(COM_MASTER);			本地数据处理  应答数据发送到  Com_Send  
//		Retransmission(com_port);		转发到其他端口   							Com_Send 




//独立工作,串口号，id,cmd,reg,regcnt
void Send_Modbus(unsigned char com_port,unsigned char modbus_id,unsigned char cmd,unsigned short int addr,unsigned char  reg_cnt)
{
	unsigned char 		data[8];
	unsigned short 		nCRC = 0;	
	unsigned char			nCRC_H=0,nCRC_L=0;
	
	data[0] = modbus_id;
	data[1] = cmd;
	
	data[2] = (addr>>8)&0xff;
	data[3] = addr&0xff;
	
	data[4] = 0x00;
	wdt();
	if(0x2b==cmd)
	{
		nCRC = CrcCheck(data,5);		
		nCRC_H = (unsigned char)((nCRC>>8)&0xFF);										
		nCRC_L = (unsigned char)(nCRC&0xFF);	
	
		data[5] = nCRC_L ;
		data[6] = nCRC_H ;	
		Com_Send(com_port,data,7);		//02 2B 0E 01 00 34 77  返回  02 2B 0E 01 01 00 00 03 00 11 4D 6F 72 6E 69 6E 67 73 74 61 72 20 43 6F 72 70 2E 01 0A 54 53 2D 4D 50 50 54 2D 36 30 02 09 76 30 31 2E 30 31 2E 33 32 FD EC
		//02 2B 0E 01 01 
		//00 00 
		//03 
		//00 11 4D 6F 72 6E 69 6E 67 73 74 61 72 20 43 6F 72 70 2E 	Morningstar Corp.
		//01 0A 54 53 2D 4D 50 50 54 2D 36 30 	TS-MPPT-60
		//02 09 76 30 31 2E 30 31 2E 33 32 			v01.01.32
		//FD EC
		Com.Usart[com_port].usRec_RD = Com.Usart[com_port].usRec_WR;
	}
	else
	{
		data[5] = reg_cnt;
		nCRC = CrcCheck(data,6);		
		nCRC_H = (unsigned char)((nCRC>>8)&0xFF);										
		nCRC_L = (unsigned char)(nCRC&0xFF);	
	
		data[6] = nCRC_L ;
		data[7] = nCRC_H ;	
		Com_Send(com_port,data,8);
		Com.Usart[com_port].usRec_RD = Com.Usart[com_port].usRec_WR;
	}		
}
#define   MPPT_BASE_ADD		0													//实际发行为64 或 0
//对协议解析
void Modbus_Process(unsigned char com_port,unsigned char modbus_id,unsigned char cmd,unsigned short int addr,unsigned char  reg_cnt,unsigned char type)
{
	int  						i=0, recCnt = 0, ucActiveRd = 0;	//返回数据的个数
	unsigned char 	data[128];	
	unsigned short  nCRC = 0; 
	unsigned char   nCRC_H,nCRC_L;	
	if(Com.Usart[com_port].usRec_RD!=Com.Usart[com_port].usRec_WR)															//Com.Usart[WIFI_BLE].RX_Buf[len] = '\0';
	{
			wdt();
			#if COM1_TEST
			COM1_Send_Data(Com.Usart[COM_MASTER].RX_Buf+Com.Usart[COM_MASTER].usRec_RD,1);
			Com.Usart[COM_MASTER].usRec_RD = (Com.Usart[COM_MASTER].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
			#endif			
			recCnt = 3 + reg_cnt*2 + 2 ;	//01 03 12
			ucActiveRd = (D_USART_REC_BUFF_SIZE + Com.Usart[com_port].usRec_WR - Com.Usart[com_port].usRec_RD)%D_USART_REC_BUFF_SIZE;		//接收到的数据长度		
			if(ucActiveRd>=recCnt)						//至少8个，0xfd 03 f0 f0 00 01 crc16	合计8个数据
			{
				 if(modbus_id == Com.Usart[com_port].RX_Buf[Com.Usart[com_port].usRec_RD])
				 {
						if(cmd == Com.Usart[com_port].RX_Buf[(Com.Usart[com_port].usRec_RD+1)%D_USART_REC_BUFF_SIZE])
						{
							if(reg_cnt*2 == Com.Usart[com_port].RX_Buf[(Com.Usart[com_port].usRec_RD+2)%D_USART_REC_BUFF_SIZE])
							{
									for(i=0;i<recCnt&&i<128;i++)									
									{
											data[i] = Com.Usart[com_port].RX_Buf[(Com.Usart[com_port].usRec_RD+i) % D_USART_REC_BUFF_SIZE];												
									}
									nCRC   = CrcCheck(data,recCnt-2);		
									nCRC_H = (unsigned char)((nCRC>>8)&0xFF);										
									nCRC_L = (unsigned char)(nCRC&0xFF);	
									if(nCRC_L==data[recCnt-2] && nCRC_H==data[recCnt-1])		//成功
									{							
											Com.Usart[com_port].usRec_RD = (Com.Usart[com_port].usRec_RD+recCnt)%D_USART_REC_BUFF_SIZE;
											g_Modbus_Send_Flag[0] = 0;
											Sw_com[0].ucFlag = 0;	
											if(0==type)							//0x0000  5
											{												
													g_Mttp[modbus_id-1-MPPT_BASE_ADD].V_PU_Hi    =  (data[3]<<8)|data[4]  ;		//01 03 06
													g_Mttp[modbus_id-1-MPPT_BASE_ADD].V_PU_Lo    =  (data[5]<<8)|data[6]  ;		//01 03 06
												
													g_Mttp[modbus_id-1-MPPT_BASE_ADD].I_PU_Hi    =  (data[7]<<8)|data[8] ;		//01 03 06
													g_Mttp[modbus_id-1-MPPT_BASE_ADD].I_PU_Lo    =  (data[9]<<8)|data[10] ;		//01 03 06
												
													g_Mttp[modbus_id-1-MPPT_BASE_ADD].SoftVer	   =  (data[11]<<8)|data[12];					
													g_Mttp[modbus_id-1-MPPT_BASE_ADD].Mask |=1;
											}
											else if(1==type)				//0xE0C0	14			E0CD = Hardware version
											{
													memcpy(g_Mttp[modbus_id-1-MPPT_BASE_ADD].SN,data+3,8);				//sn			
													g_Mttp[modbus_id-1-MPPT_BASE_ADD].Model      =  (data[27]<<8)|data[28];							
													g_Mttp[modbus_id-1-MPPT_BASE_ADD].HardVer	   =  (data[29]<<8)|data[30];							
													g_Mttp[modbus_id-1-MPPT_BASE_ADD].Mask |=2;
											}
											else if(2==type)				//0x0018  25
											{
													for(i=0;i<27;i++)			
													{	
															g_Mttp[modbus_id-1-MPPT_BASE_ADD].Reg[i]	=  (data[3+2*i]<<8)|data[4+2*i];								
													}
													g_Mttp[modbus_id-1-MPPT_BASE_ADD].Mask |=4;
											}
									}	
									else
									{
											Com.Usart[com_port].usRec_RD = (Com.Usart[com_port].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
									}
							}		
							else
							{
								Com.Usart[com_port].usRec_RD = (Com.Usart[com_port].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
							}
						}
						else
						{
								Com.Usart[com_port].usRec_RD = (Com.Usart[com_port].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
						}
				 }
				 else
				 {
						Com.Usart[com_port].usRec_RD = (Com.Usart[com_port].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
				 }
			}
			else					//对时间进行判断，当超过1s,则直接丢弃
			{
					//systickCount;
			}
	}					
}



void Get_Mppt_Data()																		//获取mppt数据		独立工作
{		
		static  unsigned char       type = 0;
		static  unsigned char 			cnt = 0;
	  //static  unsigned char 		com_port  = RS485_2;	//通讯端口
		static	unsigned char 			modbus_id = 0x01;					//modbus_id id
		static  unsigned char 			cmd  = 0x03;						//modbus cmd
		static  unsigned short int 	addr = 0xE0C0;					//开始地址 [E0C0 E0CD]
		static  unsigned char  			reg_cnt  = 14;					//reg个数
					
	  // 01 06 e0 19 00 02 EE 0C	  设置modbus id 地址  01-->02  重启设备02生效
	  // 01 03 e0 19 00 01 62 0D 		读地址 01
	  // 02 03 E0 19 00 01 62 3E    读地址 02 
	
		// 0xE019	57370	  Emodbus_id	MODBUS slave address	-		1-247	MODBUS从站地址   开机扫描 如果扫描成功，则进行必要参数读取
		// 0xE01A	57371	  Emeterbus_id	MeterBus address	  -		1-15	MeterBus地址
	
		// 0xE0C0–0xE0C3	57537 – 57540	Eserial	Serial Number	-		4个一起ascii码串号	看下边说明
		// 0xE0CC	57548	Emodel	Model: 0 = 45A, 1=60A version	-		0 or 1	
		// 0xE0CD	57549	Ehw_version	Hardware version, vMajor.Minor	-		none	
	
		// mppt读 0x0000~0x0004开机读1次  				
		// [0x0018 0x0031]  重复读取 并取出对应数据
			
		// 当数据全部采集完毕，进行1次数据转换。转换成目标数据。
				
		if(g_configRead.alarmDelyMinute>0)																					//mppt个数
		{
			g_Modbus_Interval_Tick = systickCount;																		//systickCount++;				//1ms				g_sysTick++;					//1s
			g_Modbus_Interval_Tick = g_Modbus_Interval_Tick - g_Modbus_Send_Tick;
			
			if(g_Modbus_Interval_Tick>g_configRead.collect_frq)					//每间隔多长时间发送1次命令
			{
				if(get_sw_timeout(0,1))																		//查询是否其他通道需要向该通道发送数据,空闲则进行发送
				{				
					g_Modbus_Send_Tick = systickCount;											//新的开始时刻
//				if(g_Modbus_Send_Flag==0)																												//通讯ok
//				{
//					
//				}	
//				else																										//通讯超时
//				{
//					
//				}		
				
					if(0 == cnt)
					{						
						if(g_Mttp[g_Mppt_Cur].Mask&1)
						{
							cnt = 1;
						}
						else
						{									
							modbus_id = g_Mppt_Cur+1+MPPT_BASE_ADD;
							cmd  = 0x03;
							addr = 0x0000;
							reg_cnt = 5;
							type = 0;
							Send_Modbus(RS485_2,modbus_id,cmd,addr,5);					//com2, 地址01,命令03, reg地址 0xe0c0,读reg个数
							g_Modbus_Send_Flag[0] = 1;   													//表示命令已发送，等待返回信息										
						}
					}		
				
					if(1 == cnt)
					{
						if(g_Mttp[g_Mppt_Cur].Mask&2)
						{
							cnt = 2;
						}
						else
						{
							modbus_id = g_Mppt_Cur+1+MPPT_BASE_ADD;
							cmd  = 0x03;
							addr = 0xE0C0;
							reg_cnt = 14;					
							type = 1;
							Send_Modbus(RS485_2,modbus_id,cmd,addr,reg_cnt);		//com2, 地址01,命令03, reg地址 0xe0c0,读reg个数
							g_Modbus_Send_Flag[0] = 1;   													//表示命令已发送，等待返回信息										
						}
					}
				
//				else if(2 == cnt)
//				{
//					modbus_id = g_Mppt_Cur+1;
//					cmd  = 0x2B;
//					addr = 0x0E01;
//					reg_cnt = 14;	
//					type = 2;
//					Send_Modbus(RS485_2,modbus_id,cmd,addr,reg_cnt);		//com2, 地址01,命令03, reg地址 0xe0c0,读reg个数
//					g_Modbus_Send_Flag = 1; 
//				}
				
					if(2 == cnt) 
					{
						modbus_id = g_Mppt_Cur+1+MPPT_BASE_ADD;
						cmd  			= 0x03;
						addr 			= 0x0018;			//18  19   1A
						reg_cnt 	= 27;			
						type = 2;		
						Send_Modbus(RS485_2,modbus_id,cmd,addr,reg_cnt);			//com2, 地址01,命令03, reg地址 0xe0c0,读reg个数
						g_Modbus_Send_Flag[0] = 1;   														//表示命令已发送，等待返回信息
						g_Mppt_Cur++ ;
						g_Mppt_Cur = g_Mppt_Cur%g_configRead.alarmDelyMinute;
					}	
				
					Sw_com[0].uiTick[0] = systickCount;											//本地向串口发送时刻				
					cnt++;
					cnt = cnt%3 ;
				}
			}	
			else if(1==g_Modbus_Send_Flag[0])															//进行数据解析
			{
					//将解析好的数据写入到对应变量中			
					if(1==Sw_com[0].ucMode)																	//处于本地查询模式下
					{
							Modbus_Process(RS485_2,modbus_id,cmd,addr,reg_cnt,type);	//
					}
			}
		}		
}

void Hex1_To_Ascii2(unsigned char *pucBuffer,unsigned char ucD)		//1个Hex转换成2个ASCII
{
	unsigned char ucDa;
	ucDa = ucD;
	ucDa >>= 4;
	ucDa &= 0x0f;
	pucBuffer[0] = (ucDa >= 10)?('A'+ucDa-10):ucDa+'0';
	ucDa = ucD;
	ucDa &= 0x0f;
	pucBuffer[1] = (ucDa >= 10)?('A'+ucDa-10):ucDa+'0';	
	pucBuffer[2] = 0;
}

unsigned short int CheckSum_Cal(unsigned char *pData,unsigned short int nLen)
{
	unsigned short int i,tusReturn_Val=0;
	for(i=0;i<nLen;i++)
	{
		tusReturn_Val+=(unsigned short int)pData[i+1];
	}
	tusReturn_Val=~tusReturn_Val+1;
	return tusReturn_Val;
}

unsigned short int Length_Cal(unsigned short int Para_usLen_INFO_ASCII_Data)
{
	unsigned char tucVal[5]={0};
	unsigned short int usReturn_Val=0;
	//D11D10D9D8+D7D6D5D4+D3D2D1D0
	//0  0  0 0 +0 0 0 0 +0 0 1 0
	tucVal[0]=Para_usLen_INFO_ASCII_Data & 0x000F;     //D3D2D1D0
	tucVal[1]=(Para_usLen_INFO_ASCII_Data>>4) & 0x000F;//D7D6D5D4
	tucVal[2]=(Para_usLen_INFO_ASCII_Data>>8) & 0x000F;//D11D10D9D8
	tucVal[3]=tucVal[0]+tucVal[1]+tucVal[2];
	//取反+1
	tucVal[4]=(~tucVal[3]+1) & 0x0F;
	usReturn_Val=((((unsigned short int)tucVal[4])<<12) & 0xF000) + Para_usLen_INFO_ASCII_Data;//CLHKSUM为D15D14D13D12
	return usReturn_Val;
}

unsigned char Ascii_Hex(unsigned char ucData){
	if( (ucData>=0x30) && (ucData<=0x39) ){
    ucData -= 0x30;
	}else if( (ucData>=0x41) && (ucData<=0x46) ){//大写字母
    ucData -= 0x37;
	}else if( (ucData>=0x61) && (ucData<=0x66) ){//小写字母
    ucData -= 0x57;
	}else{
		ucData = 0xff;
	}
	return ucData;
}

unsigned char Ascii2_To_Hex1(unsigned char Para_ucAscii1,unsigned char Para_ucAscii2){
	unsigned char uccData;
  uccData = (Ascii_Hex(Para_ucAscii1) << 4) | Ascii_Hex(Para_ucAscii2);
  return(uccData);
}

unsigned short int Ascii4_To_Hex2(unsigned char Para_ucAscii1,unsigned char Para_ucAscii2,unsigned char Para_ucAscii3,unsigned char Para_ucAscii4){
	unsigned short int tusData;
  tusData = (((unsigned short int)(Ascii_Hex(Para_ucAscii1))) << 12)
          | (((unsigned short int)(Ascii_Hex(Para_ucAscii2))) << 8)
          | (((unsigned short int)(Ascii_Hex(Para_ucAscii3))) << 4) 
          | (((unsigned short int)(Ascii_Hex(Para_ucAscii4))) << 0);
  return(tusData);
}

void DZ_Uart_Get_Data(unsigned char Para_ucVER,unsigned char Para_ucADR,unsigned char Para_ucCID1,unsigned char Para_ucCID2,unsigned char Para_ucCom)	//DZ:dian zong:电总
{	
	unsigned char tmpdat[48] = {0x2A, 0x53, 0x30 ,0x7C ,0x00 ,0x7C ,0x7C ,0x00 ,0x08 ,0x7B};	
	unsigned char i;
	unsigned char tucBuf[14];		//当LENID=0时共6字节；当LENID=2时共7字节；当设定时间(LENID=14)时共13字节，所以最长为13字节
	unsigned char tucDZ_Buf[36];	//最长(13*2)+1(SOI)+4(CHKSUM)+1(EOI)=32字节
	unsigned char tucArray[4];  
	//unsigned char tucSend_Buf[128]={0};
	unsigned short int nCRC;
	unsigned char LENID=0;
	unsigned char INFO=0;
	unsigned char cnt=0;
//~   21  01  41   41   0000   FDB2   \r
//SOI	VER ADR CID1 CID2 LENGTH CHKSUM EOI
//0   1  2  3  4  5  6  7  8  9  0   1  2  3  4  5  6  7  8  9  
//~   32 31 30 31 34 31 34 31 30 30 30 30 46 44 42 32 \r
//SOI	VER   ADR   CID1  CID2   LENGTH     CHKSUM EOI	
//---------- 通用命令 ----------------------------------------------------------------------------------------
	if(Para_ucCID2==0x4D){//获取时间命令
		LENID=0;
	}else if(Para_ucCID2==0x4E){//设定时间
		LENID=0x0E;
	}else if(Para_ucCID2==0x4D){//获取协议版本号
		LENID=0;
	}else if(Para_ucCID2==0x50){//获取设备(SM)地址
		LENID=0;
	}else if(Para_ucCID2==0x51){//获取设备(SM)厂商信息
		LENID=0;
	}
//---------- 获取系统模拟量量化数据（浮点数） -----------------------------------------------------------------
	else if(Para_ucCID2==0x41){
		if(Para_ucCID1==0x40){//开关电源系统（交流配电）
			LENID=0x02;
			INFO=0x01;//获取所有交流屏的数据
		}else if(Para_ucCID1==0x41){//开关电源系统(整流器)
			LENID=0;
		}else if(Para_ucCID1==0x42){//开关电源系统（直流配电）
			LENID=0x02;
			INFO=0x01;//获取所有直流屏的数据
		}
	}
//---------- 获取模拟量数据（定点数） ------------------------------------------------------------------------
	else if(Para_ucCID2==0x42){
		if(Para_ucCID1==0x46){//PYLON电池
			LENID=0x02;
			INFO=Para_ucADR;//INFO is 1 byte = ADR
		}
	}
//---------- 获取系统开关输入状态 ----------------------------------------------------------------------------
	else if(Para_ucCID2==0x43){
		if(Para_ucCID1==0x40){//开关电源系统（交流配电）
			LENID=0x02;
			INFO=0x01;//获取所有交流屏的运行数据
		}else if(Para_ucCID1==0x41){//开关电源系统（整流配电）
			LENID=0x00;
		}
	}
//---------- 获取告警状态 ------------------------------------------------------------------------------------
	else if(Para_ucCID2==0x44){
		if(Para_ucCID1==0x40){//开关电源系统（交流配电）
			LENID=0x02;
			INFO=0xFF;//获取所有交流屏的告警数据
		}else if(Para_ucCID1==0x41){//开关电源系统(整流配电)
			LENID=0;
		}else if(Para_ucCID1==0x42){//开关电源系统（直流配电）
			LENID=0x02;
			INFO=0x01;//获取所有直流屏的告警数据
		}else if(Para_ucCID1==0x46){//PYLON电池
			LENID=0x02;
			INFO=Para_ucADR;//INFO is 1 byte = ADR
		}
	}
//---------- 获取参数(浮点数) ------------------------------------------------------------------------------------
	else if(Para_ucCID2==0x46){
		if(Para_ucCID1==0x42){//开关电源系统（直流配电）
			LENID=0x00;
		}
	}
//---------- 获取系统参数 ----------------------------------------------------------------------------------------
	else if(Para_ucCID2==0x47){
		if(Para_ucCID1==0x46){//PYLON电池
			LENID=0x00;
		}
	}
//---------- 获取自定义参数 --------------------------------------------------------------------------------------
	else if(Para_ucCID2==0x80){
		if(Para_ucCID1==0x40){//开关电源系统（交流配电）
			LENID=0x00;
		}
	}
//---------- 获取充放电管理信息 ----------------------------------------------------------------------------------
	else if(Para_ucCID2==0x92){
		if(Para_ucCID1==0x46){//PYLON电池
			LENID=0x02;
			INFO=Para_ucADR;//INFO is 1 byte = ADR
		}
	}
//---------- 获取序列号 ------------------------------------------------------------------------------------------
	else if(Para_ucCID2==0x93){
		if(Para_ucCID1==0x46){//PYLON电池
			LENID=0x02;
			INFO=Para_ucADR;//INFO is 1 byte = ADR
		}
	}
//---------- 获取直流电量数据（定点数） --------------------------------------------------------------------------
	else if(Para_ucCID2==0xA9){
		if(Para_ucCID1==0x42){//开关电源系统（直流配电）
			LENID=0;
		}
	}
//------------------------------------------------------------------------------------------------------------	
	tucBuf[cnt++]=Para_ucVER;
	tucBuf[cnt++]=Para_ucADR;
	tucBuf[cnt++]=Para_ucCID1;
	tucBuf[cnt++]=Para_ucCID2;
	tucBuf[cnt++]=(Length_Cal(LENID)>>8)&0xFF;
	tucBuf[cnt++]=Length_Cal(LENID)&0xFF;
	if(LENID!=0){
		if(Para_ucCID2==0x4E){//设定时间
			
		}else{
			tucBuf[cnt++]=INFO;
		}
	}
	//转换成ASCII码
	memset(tucDZ_Buf,0,sizeof(tucDZ_Buf));
	tucDZ_Buf[0]='~';//0x7E,起始位标志,Start bit mark
	for(i=0;i<cnt;i++){
		Hex1_To_Ascii2(tucArray,tucBuf[i]);
		strcat((char *)tucDZ_Buf,(char *)tucArray);
	}
	nCRC=CheckSum_Cal(tucDZ_Buf,cnt*2);
	Hex1_To_Ascii2(tucArray,(nCRC>>8)&0xFF);
	strcat((char *)tucDZ_Buf,(char *)tucArray);
	Hex1_To_Ascii2(tucArray,nCRC&0xFF);
	strcat((char *)tucDZ_Buf,(char *)tucArray);
	
	strcat((char *)tucDZ_Buf,"\r");
	#if 1	
	if(Para_ucCom<0x35)
		Para_ucCom-=0x30;		
	else
		Para_ucCom-=0x32;
	cnt = strlen((char *)tucDZ_Buf);			//RS485_2=0x31(1)			CH432T_1=0x39(7)	 	RS485_4=0x33(3)			CH432T_2=0x3a(8)
	Com_Send(Para_ucCom,tucDZ_Buf,cnt);		//uart_send_string(COM4,tucDZ_Buf,strlen((char *)tucDZ_Buf));  
	#else	
	cnt = strlen((char *)tucDZ_Buf);
	strcat((char *)(tmpdat+10),(char *)tucDZ_Buf);
	tmpdat[2] = Para_ucCom;
	tmpdat[8] = cnt;
	tmpdat[cnt+10]=0x7D;
	tmpdat[cnt+11]=0x23;	
	uart_send_string(COM4,tmpdat,cnt+12);		
	#endif
	if(1==Para_ucCom)
	{
		g_modbus_id[0] = Para_ucADR;		
	}
	else if(3==Para_ucCom)
	{
		g_modbus_id[2] = Para_ucADR;		
	}
	else if(7==Para_ucCom)
	{
		g_modbus_id[1] = Para_ucADR;		
	}
	else if(8==Para_ucCom)
	{
		g_modbus_id[3] = Para_ucADR;		
	}	
		
	                     // 0      1    2      3    4      5     6     7    8     9     10    11    12     13   14    15    16     17   18     19
//	unsigned char      tmpdat[20] = {0x2A, 0x53, 0x30 ,0x7C ,0x00 ,0x7C ,0x7C ,0x00 ,0x08 ,0x7B ,0x01 ,0x01 ,0x00 ,0x00 ,0x00 ,0x07, 0x05 ,0xBA ,0x7D ,0x23};		
//	tucSend_Buf[0]=0x2A;
//	tucSend_Buf[1]=0x53;
//	tucSend_Buf[2]=Para_ucCom;
//	tucSend_Buf[3]=0x7C;
//	tucSend_Buf[4]=0x00;
//	tucSend_Buf[5]=0x7C;
//	tucSend_Buf[6]=0x7C;
//	tucSend_Buf[7]=0x00;
//	tucSend_Buf[8]=strlen(tucDZ_Buf);//电总数据的长度
//	tucSend_Buf[9]=0x7B;
//	for(i=0;i<strlen(tucDZ_Buf);i++){
//		tucSend_Buf[10+i]=tucDZ_Buf[i];
//	}
//	tucSend_Buf[10+strlen(tucDZ_Buf)]=0x7D;
//	tucSend_Buf[11+strlen(tucDZ_Buf)]=0x23;	
//	uart_send_string(COM4,tucSend_Buf,12+strlen((char *)tucDZ_Buf));
//	g_modbus_id=Para_ucADR;	
}

void uart_get_data(unsigned char modbus_id,unsigned short int addr, unsigned short int num,unsigned char cmd,unsigned char com)
{		
	#if 0
		RS485_2 	= 1,  	//rs485
		WIFI_BLE 	= 2,		//wifi & ble
		RS485_4 	= 3,  	//rs485
	  METER 		= 4,		//meter
		G4				= 5,		//4g
		USB				= 6,		//USB
	  CH432T_1  = 7,		//ch432T com1
	  CH432T_2  = 8			//ch432T com2
	#endif
	if(com<0x35)
		com-=0x30;		
	else
		com-=0x32;
	Send_Modbus(com,modbus_id,cmd,addr,num);			//RS485_2=0x31(1)			CH432T_1=0x39(7)	 	RS485_4=0x33(3)			CH432T_2=0x3a(8)
	if(1==com)
	{
		g_modbus_id[0] = modbus_id;		
	}
	else if(3==com)
	{
		g_modbus_id[2] = modbus_id;		
	}
	else if(7==com)
	{
		g_modbus_id[1] = modbus_id;		
	}
	else if(8==com)
	{
		g_modbus_id[3] = modbus_id;		
	}	
}


unsigned char 	CID2;
int  	g_cur_pos = 0;
//#define D_USART_REC_BUFF_SIZE				1024
unsigned char  g_data[D_USART_REC_BUFF_SIZE];

//		RS485 		= 0,		//rs485 master
//    RS485_2 	= 1,  	//rs485
//		WIFI_BLE 	= 2,		//wifi & ble
//		RS485_4 	= 3,  	//rs485
//	  METER 		= 4,		//meter
//		G4				= 5,		//4g
//		USB				= 6,		//USB
//	  CH432T_1  = 7,		//ch432T com1
//	  CH432T_2  = 8		//ch432T com2  A	
void get_all_device_info(void)
{
			 static unsigned short flag[4] ={0,0,0,0};
			 static unsigned short rev_mode[4] ={0,0,0,0};
			 unsigned int   			i = 0;		      		   	     
			 unsigned int   			cnt = 0, cur = 0;
			 unsigned int   			tmp = 0;			 
		 	
//			 g_Mttp[0].ComPort											 = 0x31;		//MPPT的转发通讯口
//			 g_PYLON_BAT.ComPort[D_PYLON_BAT_SETS_1] = 0x39;		//电池组1的转发通讯口				
//			 g_Rectifiter.ComPort 									 = 0x33;		//整流器的转发通讯口
//			 g_Inverter.ComPort				               = 0x33;		//逆变器转发通讯口
//			 g_Air.ComPort				                 	 = 0x33;		//空调的转发通讯口			 
//			 g_Fan.ComPort				              		 = 0x33;		//风扇转发通讯口  
//			 g_Diesel_Generator.ComPort              = 0x33;		//柴油发电机转发通讯口																															
//			 g_PYLON_BAT.ComPort[D_PYLON_BAT_SETS_2] = 0x3a;		//电池组2的转发通讯口
//			 
//			 g_Mttp[0].modbus_id										 = 0x01;		//mppt的modbus id		//g_Mttp.modbus_id				 0x41;	或  1
//			 g_Air.modbus_id				                 = 0x01;		//空调modbus id			 
//			 g_Inverter.modbus_id				             = 0x02;		//逆变器modbus id			 
//			 g_Fan.modbus_id				            		 = 0x03;		//风扇modbus id
//			 g_Diesel_Generator.modbus_id            = 0x0A;    //柴油发电机modbus id																											 
			 
	g_Modbus_Interval_Tick = systickCount;																		//systickCount++;				//1ms				g_sysTick++;					//1s
	g_Modbus_Interval_Tick = g_Modbus_Interval_Tick - g_Modbus_Send_Tick;
	if(g_Modbus_Interval_Tick>g_configRead.collect_frq)																								//每间隔多长时间发送1次命令
	{		
		wdt();
		///////////////////////////////mppt处理  start////////////////////////////////////
		if(g_configRead.alarmDelyMinute>0 && g_configRead.alarmDelyMinute < 17)													//mppt个数
		{			
			if(get_sw_timeout(0,1))																																				//查询是否其他通道需要向该通道发送数据,空闲则进行发送
			{	
				Com.Usart[g_com_port[0]].usRec_RD = Com.Usart[g_com_port[0]].usRec_WR;				//清空接受缓存 准备开始新的1包数据的解析
				if(flag[0]>1)
				{
						flag[0] = 0;
						g_cur_pos++;
						if(g_cur_pos>=g_configRead.alarmDelyMinute)
						{
							g_cur_pos = 0;
						}						
				}					
				if(0==flag[0])							//if(0==g_Mttp[g_Mppt_Cur].Mask)
				{											
						uart_get_data(g_cur_pos+g_Mttp[0].modbus_id,0x0000,80,3,g_Mttp[0].ComPort);				//mppt  g_cur_pos=modbus id	;add=0x0000;  num=5; cmd=3; 	0x31转发口; v_pu i_pu ver_sw
						rev_mode[0] = 4;										
						g_Modbus_Send_Flag[0] = 1;   													//表示命令已发送，等待返回信息										
				}
				else if(1==flag[0])	//else if(1==g_Mttp[g_Mppt_Cur].Mask)
				{										
						uart_get_data(g_cur_pos+g_Mttp[0].modbus_id,0xE0C0,14,3,g_Mttp[0].ComPort);			//mppt  g_cur_pos=modbus id	;add=0xe0c0;  num=14; cmd=3; 	0x31转发口; serial num,model,hardware_ver;
						rev_mode[0] = 5;
						g_Modbus_Send_Flag[0] = 1;   													//表示命令已发送，等待返回信息										
				}
//				else if(2==flag[0])	//else if(3==g_Mttp[g_Mppt_Cur].Mask)
//				{						
//						uart_get_data(g_cur_pos+g_Mttp[0].modbus_id,0x0018,27,3,g_Mttp[0].ComPort);			//mppt  g_cur_pos=modbus id	;add=0x001a;  num=25; cmd=3; 	0x31转发口; 0x001a开始连续25个reg存储在 g_Mttp.Reg[25]中	 unsigned short  Reg[25];		
//						rev_mode[0] = 6;
//						g_Modbus_Send_Flag[0] = 1;   													//表示命令已发送，等待返回信息										
//				}					
				flag[0]++;				
				Sw_com[0].uiTick[0] = systickCount;													//本地向串口发送时刻				
			}
		}			
		///////////////////////////////mppt处理  end  ////////////////////////////////////
		
		///////////////////////////////bat处理  start////////////////////////////////////  2个通道,可以接2组电池
		//问题，如果不成功，则可能导致程序不运行。															//进行命令输出					需要读状态，报警，交流，直流		
    for(i=0;i<2;i++) 															
		{	
			ucPYLON_BAT_SETS = i;						//	g_PVTPara.bCH[0] 	= g_configRead.bCH[0];
			if((g_configRead.bCH[0]&(1<<i)))							//查询是否其他通道需要向该通道发送数据,空闲则进行发送)										//暂时用位来表示	g_PVTPara.bCH[0]  	bit0=表示第1路工作   bit1=表示第2路工作
			{
				if(get_sw_timeout(2*i+1,1))
				{
				Com.Usart[g_com_port[2*i+1]].usRec_RD = Com.Usart[g_com_port[2*i+1]].usRec_WR;				//清空接受缓存 准备开始新的1包数据的解析
				if(flag[1+2*i]>3)
				{
						flag[1+2*i] = 0;						
				}				
				if(0==flag[1+2*i])					//if(!(g_PYLON_BAT.Mask&1))								
				{			
					//电池组1获取系统基本信息		4.1 系统基本信息/Basic system information					
					CID2=0x60;
					DZ_Uart_Get_Data(0x20,0x02,0x46,CID2,g_PYLON_BAT.ComPort[ucPYLON_BAT_SETS]);
					rev_mode[1+2*i]=D_REV_MODE_PYLON_BAT_GET_SYSTEM_BASIC_INFO_133;		
					g_Modbus_Send_Flag[1+2*i] = 1;   																		//表示命令已发送，等待返回信息															
				}
				#if 0
				else if(!(g_PYLON_BAT.Mask&2))
				{		
					//电池组1获取模拟量数据，定点数
					//ucPYLON_BAT_SETS=D_PYLON_BAT_SETS_1;			//电池组1			D_PYLON_BAT_SETS_2=电池组2
					CID2=0x42;
					DZ_Uart_Get_Data(0x20,0x02,0x46,CID2,g_PYLON_BAT.ComPort[ucPYLON_BAT_SETS]);
					rev_mode=D_REV_MODE_PYLON_BAT_GET_ANALOG_DATA_130;							
				}
				else if(!(g_PYLON_BAT.Mask&0x4))
				{						
						//电池组1获取系统参数：注意：只获取主机保护参数    多个电池										
						CID2=0x47;
						DZ_Uart_Get_Data(0x20,0x02,0x46,CID2,g_PYLON_BAT.ComPort[ucPYLON_BAT_SETS]);
						rev_mode=D_REV_MODE_PYLON_BAT_GET_SYSTEM_PARA_135;
				}		
				else if(!(g_PYLON_BAT.Mask&8))
				{
						//电池组1获取报警信息										
						CID2=0x44;
						DZ_Uart_Get_Data(0x20,0x02,0x46,CID2,g_PYLON_BAT.ComPort[ucPYLON_BAT_SETS]);
						rev_mode=D_REV_MODE_PYLON_BAT_GET_Alarm_INFO_131;
				}
				else if(!(g_PYLON_BAT.Mask&0x10))
				{
						//电池组1获取SN										
						CID2=0x93;
						DZ_Uart_Get_Data(0x20,0x02,0x46,CID2,g_PYLON_BAT.ComPort[ucPYLON_BAT_SETS]);
						rev_mode=D_REV_MODE_PYLON_BAT_GET_SN_132;
				}
				#endif
				else if(1==flag[1+2*i])			//else if(!(g_PYLON_BAT.Mask&0x20))
				{
						//电池组1获取电池充放电管理信息			-->4.4 系统交互信息 / system recommend info										
						CID2=0x63;		//CID2=0x92;-->63
						DZ_Uart_Get_Data(0x20,0x02,0x46,CID2,g_PYLON_BAT.ComPort[ucPYLON_BAT_SETS]);
						rev_mode[1+2*i]=D_REV_MODE_PYLON_BAT_GET_BAT_CHARGE_DISCHARGE_MI_134;	
						g_Modbus_Send_Flag[1+2*i] = 1;   																		//表示命令已发送，等待返回信息															
				}
				else if(2==flag[1+2*i])			//else if(!(g_PYLON_BAT.Mask&0x40))
				{
						//电池组1获取电池充放电管理信息  	获取系统模拟量（电池）	4.2 系统模拟量/Analog data of system 
						//~200246610000FDAB
						//~200246008062C2BF0000550008000864640CFC00020CFB00020BAB0BAC00020BA900020BA80BA800020BA800020BB00BB000020BB00002E8CC										
						CID2=0x61;	//第1路的soc，电压  充放电电流 用于主界面													
						DZ_Uart_Get_Data(0x20,0x02,0x46,CID2,g_PYLON_BAT.ComPort[ucPYLON_BAT_SETS]);
						rev_mode[1+2*i]=D_REV_MODE_PYLON_BAT_GET_SYSTEM_ANALOG_136;		
						g_Modbus_Send_Flag[1+2*i] = 1;   																		//表示命令已发送，等待返回信息																				
						//电池组1获取电池充放电管理信息  	获取系统模拟量（电池）	4.2 系统模拟量/Analog data of system 
						//~200246610000FDAB
						//~200246008062C2BF0000550008000864640CFC00020CFB00020BAB0BAC00020BA900020BA80BA800020BA800020BB00BB000020BB00002E8CC
						//ucPYLON_BAT_SETS=D_PYLON_BAT_SETS_1;//电池组1												
				}
				else if(3==flag[1+2*i])			//else if(!(g_PYLON_BAT.Mask&0x80))
				{
						//4.3 系统告警保护/alarm & protection info of system
						CID2=0x62;
						DZ_Uart_Get_Data(0x20,0x02,0x46,CID2,g_PYLON_BAT.ComPort[ucPYLON_BAT_SETS]);
						rev_mode[1+2*i]=D_REV_MODE_PYLON_BAT_GET_SYSTEM_ALARM_137;
						g_Modbus_Send_Flag[1+2*i] = 1;   																		//表示命令已发送，等待返回信息															
				}	
				flag[1+2*i]++;				
				Sw_com[1+2*i].uiTick[0] = systickCount;											//本地向串口发送时刻	
			}				
			}				
		}											
		///////////////////////////////bat处理  end  ////////////////////////////////////
		
		///////////////////////////////整流器 逆变器 风扇 空调  发电机等 处理  start////////////////////////////////////  
		if(get_sw_timeout(2,1))
		{
			Com.Usart[g_com_port[2]].usRec_RD = Com.Usart[g_com_port[2]].usRec_WR;				//清空接受缓存 准备开始新的1包数据的解析
			{			//rectifier状态界面  问题，如果不成功，则可能导致程序不运行。														//进行命令输出					需要读状态，报警，交流，直流								
				if(0==flag[2])				//if(!(g_Rectifiter.Mask&1))								
				{							
					//获取系统模拟量量化数据（浮点数）  整流  整流模块输出电压和电流及模块数量	3.3.1、获取系统模拟量量化数据(浮点数)
						CID2=0x41;
						DZ_Uart_Get_Data(0x21,0x01,0x41,CID2,g_Rectifiter.ComPort);
						rev_mode[2]=D_REV_MODE_RECTIFIER_RC_DISTRIBUTION_GET_ANALOG_DATA_100;				//获取系统模拟量量化数据（浮点数）  整流器  计算整流器总电流和功率及功
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
				else if(1==flag[2])		//else if(!(g_Rectifiter.Mask&2))
				{		
						//获取系统模拟量量化数据（浮点数）	交流电压
						CID2=0x41;
						DZ_Uart_Get_Data(0x21,0x01,0x40,CID2,g_Rectifiter.ComPort);
						rev_mode[2]=D_REV_MODE_RECTIFIER_AC_DISTRIBUTION_GET_ANALOG_DATA_103;
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
				else if(2==flag[2])		//else if(!(g_Rectifiter.Mask&0x4))
				{						
						//获取告警状态				3.3.3、获取模块告警状态   整流
						CID2=0x44;
						DZ_Uart_Get_Data(0x21,0x01,0x41,CID2,g_Rectifiter.ComPort);
						rev_mode[2]=D_REV_MODE_RECTIFIER_RC_DISTRIBUTION_GET_ALARM_STATE_102;
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}		
				else if(3==flag[2])		//else if(!(g_Rectifiter.Mask&8))
				{
						//获取系统开关输入状态	3.3.2、获取系统开关输入状态  整流。
						CID2=0x43;
						DZ_Uart_Get_Data(0x21,0x01,0x41,CID2,g_Rectifiter.ComPort);
						rev_mode[2]=D_REV_MODE_RECTIFIER_RC_DISTRIBUTION_GET_SYSTEM_SWITCH_INPUT_STATE_101;
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
				else if(4==flag[2])		//else if(!(g_Rectifiter.Mask&0x10))
				{
						//获取参数(浮点数)		3.4.3、获取/设定参数(浮点数)   直流 
						CID2=0x46;
						DZ_Uart_Get_Data(0x21,0x01,0x42,CID2,g_Rectifiter.ComPort);
						rev_mode[2]=D_REV_MODE_RECTIFIER_DC_DISTRIBUTION_GET_PARA_108;	
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
			}
				
			{							//发电机
				if(5==flag[2])		//if(!(g_Diesel_Generator.Mask&1))
				{
						uart_get_data(g_Diesel_Generator.modbus_id,1024,66,3,g_Diesel_Generator.ComPort);			//66个寄存器
						rev_mode[2] = D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_1024_1089_40;										//发电机总功率和总电流
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
				else if(6==flag[2])		//else if(!(g_Diesel_Generator.Mask&2))
				{
						uart_get_data(g_Diesel_Generator.modbus_id,1536,46,3,g_Diesel_Generator.ComPort);			//46个寄存器
						rev_mode[2] = D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_1536_1581_41;
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
				else if(7==flag[2])		//else if(!(g_Diesel_Generator.Mask&4))
				{
						uart_get_data(g_Diesel_Generator.modbus_id,1798,12,3,g_Diesel_Generator.ComPort);			//12个寄存器
						rev_mode[2] = D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_1798_1809_42;
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
				else if(8==flag[2])		//else if(!(g_Diesel_Generator.Mask&8))
				{
						uart_get_data(g_Diesel_Generator.modbus_id,39425,18,3,g_Diesel_Generator.ComPort);		//18个寄存器
						rev_mode[2] = D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_39425_39442_43;
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
				#if 0
									else if(!(g_Diesel_Generator.Mask&0x10))
									{
										uart_get_data(g_Diesel_Generator.modbus_id,43873,1,3,g_Diesel_Generator.ComPort);			//1个寄存器
										rev_mode = D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_43873_44;
									}
									else if(!(g_Diesel_Generator.Mask&0x20))
									{
										uart_get_data(g_Diesel_Generator.modbus_id,43520,16,3,g_Diesel_Generator.ComPort);		//16个寄存器
										rev_mode = D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_43520_43535_45;
									}
									else if(!(g_Diesel_Generator.Mask&0x40))
									{
										uart_get_data(g_Diesel_Generator.modbus_id,43925,25,3,g_Diesel_Generator.ComPort);		//25个寄存器
										rev_mode = D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_43925_43949_46;
									}
				#endif
				else if(9==flag[2])		//else if(!(g_Diesel_Generator.Mask&0x80))
				{
						uart_get_data(g_Diesel_Generator.modbus_id,48640,22,3,g_Diesel_Generator.ComPort);		//22个寄存器
						rev_mode[2] = D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_48640_48661_47;
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}									
			}
		
			{ 					//读逆变器数据				
				//系统控制器运行状态	140		 0x0030+92=		8C		uart_get_data(Modbus_ID,0x0200,0x0030,3,Local_ComPort);			rev_mode = 0;
				//获取系统控制器参数						 	uart_get_data(Modbus_ID,0x0300,36,3,Local_ComPort);												rev_mode = 1
				//读dip  do di 等的状态																																											rev_mode = 2;										
				//读ad值																																																		rev_mode = 3;									
				//系统控制器daily			 92=0x5C			uart_get_data(Modbus_ID,0x0400,0x005C,3,Local_ComPort);									rev_mode = 10;
				if(10==flag[2])		//
				{
					uart_get_data(g_Inverter.modbus_id,0x0000,0x005F-0x0000+1,3,g_Inverter.ComPort);						//从0x0000开始读 01 03 00 00 00 60 45 E2       96
					rev_mode[2]=D_REV_MODE_INVERTER_RAM_AREA_20;		
					g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
			}				
			
			//g_PVTPara.b_TH_work	        = g_configRead.save_frq;
			//g_state_5004 |= ((g_PVTPara.b_TH_work&0x10)<<6);			//空调  bit4  a/c   -->bit10
			//g_state_5004 |= ((g_PVTPara.b_TH_work&0x20)<<6);			//风扇  bit5  Fan   -->bit11
			cur = 0;
			if(g_configRead.save_frq&0x20)													// 风扇  界面75			 
			{
					cur = 1;
					if(11==flag[2])		//
					{
						uart_get_data(g_Fan.modbus_id	,0x0000,0x0031-0x0000+1,3,g_Fan.ComPort);						//从0x0000开始读 01 03 00 00 00 32 C4 1F						50
						rev_mode[2]=D_REV_MODE_FAN_RAM_AREA_30;		
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息																					
					}
			}		
			
			tmp = 0;
			if(g_configRead.save_frq&0x10)		//空调状态  界面75
			{		
				cnt = 11+cur;
				tmp = 4;
				if(cnt==flag[2])		//if(!(g_Air.Mask&1))						//空调状态位读取
				{											
						uart_get_data(g_Air.modbus_id,0x0000,16,1,g_Air.ComPort);				
						rev_mode[2] = D_REV_MODE_DBS_AIR_STATE_1;																
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
				else if((cnt+1)==flag[2])		//else if(!(g_Air.Mask&2))	//空调报警位读取
				{										
						uart_get_data(g_Air.modbus_id,0x0000,40,2,g_Air.ComPort);			
						rev_mode[2] = D_REV_MODE_DBS_AIR_ALARM_2;						
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}
				else if((cnt+2)==flag[2])		//else if(!(g_Air.Mask&4))	//空调传感器读取
				{						
						uart_get_data(g_Air.modbus_id,0x0000,15,4,g_Air.ComPort);			
						rev_mode[2] = D_REV_MODE_DBS_AIR_SENSOR_4;			
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}			
				else if((cnt+3)==flag[2])		//else if(!(g_Air.Mask&8))	//空调参数读取
				{						
						uart_get_data(g_Air.modbus_id,0x0016,36,3,g_Air.ComPort);			
						rev_mode[2] = D_REV_MODE_DBS_AIR_PARAMETER_3;						
						g_Modbus_Send_Flag[2] = 1;   																		//表示命令已发送，等待返回信息															
				}																																		
			}		
			
			flag[2]++;
			if(flag[2] > (10+tmp+cur))
			{
					flag[2] = 0;				
			}
			Sw_com[2].uiTick[0] = systickCount;											//本地向串口发送时刻				
		}
		///////////////////////////////整流器 逆变器 风扇 空调  发电机等 处理  end  ////////////////////////////////////			 
		g_Modbus_Send_Tick = systickCount;		
	}
	else
	{		
		for(i=0;i<4;i++)						//数据解析
		{
			if(1==g_Modbus_Send_Flag[i])															//进行数据解析
			{					
				if(1==Sw_com[i].ucMode)																	//处于本地查询模式下
				{
					wdt();
					//增加从发送命令到命令返回最长时间的判断。避免空转
					{
						cnt = (D_USART_REC_BUFF_SIZE + Com.Usart[g_com_port[i]].usRec_WR - Com.Usart[g_com_port[i]].usRec_RD)%D_USART_REC_BUFF_SIZE;		//接收到的数据长度	
						if(cnt > 5)
						{
							cnt = 0;
							cur = Com.Usart[g_com_port[i]].usRec_RD;
							while(Com.Usart[g_com_port[i]].usRec_WR!=cur && cnt<D_USART_REC_BUFF_SIZE)
							{									
									g_data[cnt] = Com.Usart[g_com_port[i]].RX_Buf[cur];
									cnt++;
									cur = (Com.Usart[g_com_port[i]].usRec_RD + cnt)%D_USART_REC_BUFF_SIZE;
							}
							if(cnt)		//对g_data[]  中的 cnt个数据进行处理
							{
								//Modbus_Process(RS485_2,modbus_id,cmd,addr,reg_cnt,type);	//
								g_Result_Status = 0;								
								deal_rev_data(g_data,cnt,rev_mode[i],g_modbus_id[i]);					//将解析好的数据写入到对应变量中	
								if(1==g_Result_Status)
								{
									if(1==Sw_com[i].ucMode)
									{
											Sw_com[i].ucMode  = 0; 
									}										
									g_Modbus_Send_Flag[i] = 0;
									Com.Usart[g_com_port[i]].usRec_RD = Com.Usart[g_com_port[i]].usRec_WR;
								}
							}
						}
					}
				}							
			}			
		}		
	}
}


/*************************************************************************************************************
函数名：deal_uart_data(unsigned char* Arr_rece,unsigned short int* Data_len,unsigned char Uart_num,bit Response,bit Crc_ck)
参数：Arr_rece为串口接收数组，Data_len接收数据长度地址，Uart_num串口号，Response是否带应答，Crc_ck是否带校验
作者：cuijia
日期：20220526
*************************************************************************************************************/
void  deal_rev_data(unsigned char* Arr_rece,unsigned int Data_len,unsigned char mode,unsigned char modbus_id)
{
			unsigned short int N=0,i=0,j=0;
	    unsigned short int nCRC=0;	
	    unsigned char Flog=1;
	    unsigned char  cnt = 0;	    
	    unsigned char  tmp = 0;
			unsigned short int Chksum=0;
			unsigned char  tucDZ_Ascii_To_Hex_Buf[1024];//最长13+5*16块=93byte
	    unsigned char  tucDZ_Active_Data_Buf[1024];//DZ:电总的拼音首字母
			unsigned char  VER,ADR,CID1,RTN,LCHKSUM,b,c;
		  unsigned short int lchksum,LENGTH,LENID;
			float *pabc;
	    unsigned char Exchange_Buf[5];			
//			data unsigned char tttt[80];
//			g_dd = *Data_len;
//			for(i=0;i<80;i++)
//			{
//				tttt[i] = Arr_rece[i];
//			}
			while(Flog && N < Data_len)
			{		
					wdt();
					if(mode>99)					//电总协议数据
					{
						if(Arr_rece[N]=='\r')
						{
							if(Arr_rece[0]=='~')
							{
								Data_len = N+1;
								nCRC=CheckSum_Cal(Arr_rece,Data_len-6);
								Chksum=Ascii4_To_Hex2(Arr_rece[Data_len-5],Arr_rece[Data_len-4],Arr_rece[Data_len-3],Arr_rece[Data_len-2]);
								if(nCRC!=Chksum)
								{
									return;
								}
								tucDZ_Ascii_To_Hex_Buf[0]=Arr_rece[0];//SOI
								for(i=0;i<((Data_len-2)/2);i++)		//-2：减去SOI、EOI
								{
									tucDZ_Ascii_To_Hex_Buf[1+i]=Ascii2_To_Hex1(Arr_rece[1+2*i],Arr_rece[2+2*i]);
								}
								tucDZ_Ascii_To_Hex_Buf[1+((Data_len-2)/2)]=Arr_rece[N];//EOI
								VER =tucDZ_Ascii_To_Hex_Buf[1];
								ADR =tucDZ_Ascii_To_Hex_Buf[2];
								CID1=tucDZ_Ascii_To_Hex_Buf[3];
								RTN =tucDZ_Ascii_To_Hex_Buf[4];
								LENGTH=(tucDZ_Ascii_To_Hex_Buf[5]<<8) | tucDZ_Ascii_To_Hex_Buf[6];
								
								lchksum=(LENGTH>>12)&0x0F;//D12-D16
								LENID=(LENGTH & 0xFFF)/2;//D0-D11
								LCHKSUM=LENGTH&0xf;
								b=(LENGTH&0xF0)>>4;
								c=(LENGTH&0xf00)>>8;
								
								LCHKSUM = LCHKSUM + b + c;
								LCHKSUM = LCHKSUM%16;
								LCHKSUM = (0x0F - LCHKSUM + 1)%16;
								if(LCHKSUM!=lchksum)
								{
									return;
								}
								if(RTN==0)
								{
									if( tucDZ_Ascii_To_Hex_Buf[2] !=modbus_id  )//判断ID
									{
										return;
									}
									else
									{
										Flog=0;
										memcpy(tucDZ_Active_Data_Buf,tucDZ_Ascii_To_Hex_Buf+7,LENID);
										if( LENID==0 && CID2!=0x4F && CID2!=0x50 )
										{
										
										}
										else
										{
//-------------------- 获取时间 --------------------------------------------------------------------------------------
											if(CID2==0x4D){//~2101404E200E 07E5 01 1C 10 3A 16   FA96
											
											}
//-------------------- 协议版本号 ------------------------------------------------------------------------------------
											else if(CID2==0x4F){
											
											}
//-------------------- 设备地址 --------------------------------------------------------------------------------------
											else if(CID2==0x50){
											
											}
//-------------------- 厂商信息 --------------------------------------------------------------------------------------
											else if(CID2==0x51){
											
											}
//-------------------- 获取系统模拟量量化数据（浮点数）----------------------------------------------------------------
											else if(CID2==0x41)
											{
												if(       (CID1==0x40) && (mode==D_REV_MODE_RECTIFIER_AC_DISTRIBUTION_GET_ANALOG_DATA_103) ) //交流配电
												{
													g_Result_Status	= 1;
													g_Rectifiter.Mask |=2;
//                0  1  2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
//~210140 00 F03E 11 01 06006643                00001644 00             9A999D42                F370
//                      A相电压	B相电压 C相电压 频率		 自定义遥测数量	用户自定义字节									
//                      				a1b1c1d1e1f1g1h1
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+2,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[2+i]=Exchange_Buf[3-i];
													}
													pabc=(float*)(tucDZ_Active_Data_Buf+2);
													g_Rectifiter.fV_AC_Distribution_A_Phase=*pabc;
													
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+14,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[14+i]=Exchange_Buf[3-i];
													}
													pabc=(float*)(tucDZ_Active_Data_Buf+14);
													g_Rectifiter.fHz_AC_Distribution=*pabc;
												}
												else if( (CID1==0x41) && (mode==D_REV_MODE_RECTIFIER_RC_DISTRIBUTION_GET_ANALOG_DATA_100) )//整流配电
												{
													g_Result_Status	= 1;
													g_Rectifiter.Mask |=1;
//                   1                   2                    3         4         5
//0 12 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  012345678901234567890123456789
//~21014100402A11BC7043420317B7D1380017B7D138003901CD3D00F4B5
//                                   1                              2
//0   1   2   3    4   5 6    7  8 9 0 1  2        3 4 5 6   7  8 9 0 1   2  3 4 5 6   7  8 9
//~   21  01  41   00  402A   11 BC704342 03       17B7D138  00 17B7D138  00 3901CD3D  00 F4B5    \r  //这组数据非定长，随着模块的数量变化
//SOI VER ADR CID1 RTN LENGTH    输出电压 模块数量 模块1电流    模块2电流    模块3电流    CHKSUM  EOI
//0   1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9
//~   32 31 30 31 34 31 30 30 34 30 32 41 31 31 42 43 37 30 34 33 34 32 30 33 
//SOI VER   ADR   CID1  RTN   LENGTH            输     出    电      压 数量
													g_Rectifiter.ucNum_RC_Distribution_Monitor_Module=tucDZ_Active_Data_Buf[5];																									
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf,5);
													for(i=1;i<5;i++)
													{
														tucDZ_Active_Data_Buf[i]=Exchange_Buf[5-i];
													}
													pabc=(float*)(tucDZ_Active_Data_Buf+1);
													g_Rectifiter.fV_RC_Distribution_Output=*pabc;
													g_Result.fV_RC_Distribution_Output = g_Rectifiter.fV_RC_Distribution_Output;
//													g_Rectifiter.fV_Output=0;
//													g_Rectifiter.fV_Output=Float_Cal(tucDZ_Active_Data_Buf[1],tucDZ_Active_Data_Buf[2],tucDZ_Active_Data_Buf[3],tucDZ_Active_Data_Buf[4]);
													g_Rectifiter.fI_RC_Distribution_Moudle_Output_Total = 0.0;
													for(i=0;i<g_Rectifiter.ucNum_RC_Distribution_Monitor_Module;i++)
													{
														memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+6+i*5,4);
														for(j=0;j<4;j++)
														{
															tucDZ_Active_Data_Buf[j+6+i*5]=Exchange_Buf[3-j];
														}
														pabc=(float*)(tucDZ_Active_Data_Buf+6+i*5);
														g_Rectifiter.fI_RC_Distribution_Moudle_Output[i]=*pabc;
														g_Rectifiter.fI_RC_Distribution_Moudle_Output_Total += g_Rectifiter.fI_RC_Distribution_Moudle_Output[i];
													}
													g_Result.fI_RC_Distribution_Moudle_Output_Total = g_Rectifiter.fI_RC_Distribution_Moudle_Output_Total;
												}
												else if( (CID1==0x42) && (mode==D_REV_MODE_RECTIFIER_DC_DISTRIBUTION_GET_ANALOG_DATA_106) )		//直流配电
												{

//               0  1 2 3 4  5
//~21014200 5038 11 4C334342 3901CD3D   01                 17B7D138                   00030D00F0420D00F04203009643F1F1
//                  输出电压 总负载电流 监测蓄电池电流路数 第一路蓄电池组充、放电电流													
													g_Result_Status	= 1;
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+1,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[1+i]=Exchange_Buf[3-i];
													}
													pabc=(float*)(tucDZ_Active_Data_Buf+1);
													g_Rectifiter.fV_DC_Distribution_Output=*pabc;
													
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+5,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[5+i]=Exchange_Buf[3-i];
													}
													pabc=(float*)(tucDZ_Active_Data_Buf+5);
													g_Rectifiter.fI_DC_Distribution_Total_Load=*pabc;
												}
											}
//-------------------- 获取系统模拟量量化数据（定点数）----------------------------------------------------------------
											else if(CID2==0x42)
											{
												if((CID1==0x46) && (mode==D_REV_MODE_PYLON_BAT_GET_ANALOG_DATA_130) ) //PYLON电池
												{
													g_Result_Status	= 1;
													g_PYLON_BAT.Mask |=2;	
//7E 32 30 30 32 34 36 30 30 46 30 37 41 31 31 30 32 30 46 30 43 36 36 30 43 36 38 30 43 36 36 30 43 36 37 30 43 36 34 30 43 36 30 30 43 36 41 30 43 36 36 30 43 36 34 30 43 36 31 30 43 36 35 30 43 36 38 30 43 36 34 30 43 36 36 30 43 35 46 30 35 30 43 30 38 30 42 45 44 30 42 46 31 30 42 45 46 30 43 33 46 46 45 41 34 42 39 45 41 46 46 46 46 30 34 46 46 46 46 30 30 30 31 30 30 32 41 46 38 30 31 38 36 41 30 45 32 33 38 0D
//                                    															 1                                       2                                           3                                          4                                         5                                                                       6               
//						   0  1             2        3 4     5 6     7 8     9 0     1 2     3 4     5 6     7 8     9 0     1 2      3 4      5 6      7 8      9 0      1 2      3           4 5  6 7  8 9  0 1  2 3  4 5  6 7      8 9               0          1 2                      3 4          5 6 7              8 9 0                     1 2 3 4 5 6 7 8 9
//~20024600 F07A 11 02            0F       0C66    0C68    0C66    0C67    0C64    0C60    0C6A    0C66    0C64    0C61     0C65     0C68     0C64     0C66     0C5F     05          0C08 0BED 0BF1 0BEF 0C3F FEA4 B9EA     FFFF              04         FFFF                     0001         002AF8             0186A0                    E238
//~20024600 F07A 11 02            0F       0D09    0D0B    0D0B    0D0A    0D0B    0D0B    0D0B    0D0A    0D0A    0D0B     0D09     0D0B     0D0A     0D0A     0D09     05          0BC3 0BAF 0BAF 0BAF 0BAF 0000 C39A     FFFF              04         FFFF                     0015         00FE60             012110                    E23F
//		              Command value	电芯节数 cell1_V cell2_V cell3_V cell4_V cell5_V cell6_V cell7_V cell8_V cell9_V cell10_V cell11_V cell12_V cell13_V cell14_V cell15_V 温度点数量5 T1   T2   T3   T4   T5   电流 模块电压 Remain capacity 1 自定义个数 Module total capacity 1  Cycle number Remain capacity 2  Module total capacity 2
													for(i=0;i<5;i++)
													{
														g_PYLON_BAT.Get_Analog_Data[ucPYLON_BAT_SETS].fT[ADR-2][i]                =(((float)((((unsigned short int)tucDZ_Active_Data_Buf[34+2*i])<<8) | tucDZ_Active_Data_Buf[35+2*i]))-2731)/10.0f;//原数放大了10倍
													}
													g_PYLON_BAT.Get_Analog_Data[ucPYLON_BAT_SETS].fI[ADR-2]                     =((((unsigned short int)tucDZ_Active_Data_Buf[44])<<8)      | tucDZ_Active_Data_Buf[45])/10.0f;//原数放大了10倍
													
													g_PYLON_BAT.Get_Analog_Data[ucPYLON_BAT_SETS].fV_Module[ADR-2]              =((((unsigned short int)tucDZ_Active_Data_Buf[46])<<8)      | tucDZ_Active_Data_Buf[47])/1000.0f;//原数放大了1000倍
													
													g_PYLON_BAT.Get_Analog_Data[ucPYLON_BAT_SETS].fRemain_Capacity_2[ADR-2]     =((((unsigned int)tucDZ_Active_Data_Buf[55])<<16)     | (((unsigned int)tucDZ_Active_Data_Buf[56])<<8) | tucDZ_Active_Data_Buf[57])/1000.0f;//原数放大了1000倍
													
													g_PYLON_BAT.Get_Analog_Data[ucPYLON_BAT_SETS].fModule_Total_Capacity_2[ADR-2]=((((unsigned int)tucDZ_Active_Data_Buf[58])<<16)    | (((unsigned int)tucDZ_Active_Data_Buf[59])<<8) | tucDZ_Active_Data_Buf[60])/1000.0f;//原数放大了1000倍
												}
											}
//-------------------- 获取系统开关输入状态 ---------------------------------------------------------------------------
											else if(CID2==0x43)
											{
												if(       (CID1==0x40) && (mode==D_REV_MODE_RECTIFIER_AC_DISTRIBUTION_GET_SYSTME_SWITCH_INPUT_STATE_104) )//交流配电
												{
//               0  1            2               3            4 5
//~21014000 8008 11 00           01              00           FC25
//                  输出开关数量 自定义状态数量p 自定义字节p*1		
												}
												else if( (CID1==0x41) && (mode==D_REV_MODE_RECTIFIER_RC_DISTRIBUTION_GET_SYSTEM_SWITCH_INPUT_STATE_101) )//整流配电
												{
													g_Result_Status	= 1;
													g_Rectifiter.Mask |=8;
//               0  1  2         3           4              5                     6 7 8 9  0 1 2 3 4 5
//~21014100 301C 11 03 00        01          00             00                    00010000 00010000 F858
//                     开机/关机 限流/不限流 浮充/均充/测试 自定义运行状态数量p=0
//状态字节描述：
//a)开机/关机
//—00H:开机
//—01H:关机
//—E1H:效能待机
//—FOH:模块屏蔽
//b)限流/不限流
//—00H:限流
//—01H:不限流
//—FOH:模块屏蔽
//c)浮充/均充/测试
//—00H:浮充
//—01H:均充
//—02H:测试
//—FOH:模块屏蔽
//—80H~EFH:用户自定义。
//注：用户自定义运行状态数量p=0。													
													g_Rectifiter.ucNum_RC_Distribution_Monitor_Module=tucDZ_Active_Data_Buf[1];
													for(i=0;i<g_Rectifiter.ucNum_RC_Distribution_Monitor_Module;i++)
													{
														for(j=0;j<3;j++)
														{
															g_Rectifiter.ucState_RC_Distribution_System_Input_Switch[i][j]=tucDZ_Active_Data_Buf[(4*i)+(2+j)];
														}
													}
												}
											}
//-------------------- 获取告警状态 ------------------------------------------------------------------------------------
											else if(CID2==0x44)
											{
												if(       (CID1==0x40) && (mode==D_REV_MODE_RECTIFIER_AC_DISTRIBUTION_GET_ALARM_STATE_105) )//交流配电
												{
//               0  1          2 3 4 5    6                 7           8                9                0 1 2 3 4
//~21014000 501A 11 01         00         01                00          01               00                     F97C
//~21014000 501A 11 01         00abcdef   01                00          01               00               hijklmF97C
//								  交流屏数量 ABC相加空1	检测熔丝/开关数量	1#熔丝/开关	自定义告警数量p 用户自定义字节P*1
												}
												else if( (CID1==0x41) && (mode==D_REV_MODE_RECTIFIER_RC_DISTRIBUTION_GET_ALARM_STATE_102) )//整流配电
												{
													g_Result_Status	= 1;
													g_Rectifiter.Mask |=4;
//~21014100 C022   11 03       00    03                   000000           0103000100     0103000100     F72E
//                             告警  自定义运行状态数量p  自定义字节p*1
//          LENGTH    模块数量 模块1告警内容                               模块2告警内容  模块3告警内容
													
//整流模块告警内容及传送顺序
//序号| 内容                  |字节|
//1   |整流模块告警           | 1  |
//2   |用户自定义运行状态数量p| 1  |
//3   |用户自定义字节         | p*1|
//运行状态字节描述:00H:正常；01H:告警；F0H:模块屏蔽
													
//—80H~EFH：用户自定义。
//表 3.3.3.5 自定义字节
//序号|P=2| 内容 	 |字节|
//1   |   |高压关机|1   |
//2   |   |通讯故障|1   |
//3   |   |风扇故障|1   |
//运行状态字节描述:00H:正常；01H:告警；F0H:模块屏蔽
													g_Rectifiter.ucNum_RC_Distribution_Monitor_Module=tucDZ_Active_Data_Buf[1];
													for(i=0;i<g_Rectifiter.ucNum_RC_Distribution_Monitor_Module;i++)
													{
														g_Rectifiter.ucAlarm_State_RC_Distribution_Moudle[i][0]=tucDZ_Active_Data_Buf[2+i*5];//告警
														for(j=0;j<3;j++)
														{
															g_Rectifiter.ucAlarm_State_RC_Distribution_Moudle[i][j+1]=tucDZ_Active_Data_Buf[j+4+i*5];//自定义告警
														}
													}
													
												}
												else if( (CID1==0x42) && (mode==D_REV_MODE_RECTIFIER_DC_DISTRIBUTION_GET_ALARM_STATE_107) )//直流配电
												{
//                                                                                                  1                   2                   3                   4                   5
//               0  1        2                 3              4              5              6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
//~21014200 9070 11 00       02                00             00             32             0000040000000000000000000000000000000000000000000000000000000000000001010000000000000000000000000000E897
//                  直流电压 直流熔丝/开关数量 直流熔丝/开关1 直流熔丝/开关2 自定义告警数量p p*1字节
//告警字节描述：
//— 00H:正常；
//—01H:低于下限；
//—02H:高于上限；
//— 03H:熔丝断；
//— 04H:开关打开；
//—80H~EFH:用户自定义；
//—FOH:其他告警。

												}else if( (CID1==0x46) && (mode==D_REV_MODE_PYLON_BAT_GET_Alarm_INFO_131) ) {//PYLON电池
													g_Result_Status	= 1;
													g_PYLON_BAT.Mask |=8;	
//                                                                    1                                     2                                                3
//               0          1           2        3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8          9  0  1  2  3   4        5        6        7  8  9  0  1     2 3 4 5 6 7 8 9
//~20024600 C040 00         02          0F       00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 05         00 00 00 00 00  00       00       00       00 0E 00 00 00    F169//US3000
//~20034600 A042 11         03          0F       00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 05         00 00 00 00 00  00       00       00       00 0E 00 00 00 00 F105//UP500
//               DATAFLAG   Cmd value   电芯节数 15接电芯电压告警状态                         温度点数量 5个温度告警状态 充电电流 模块电压 放电电流 Status 1----5
													for(i=0;i<5;i++){
														g_PYLON_BAT.Get_Alarm_INFO[ucPYLON_BAT_SETS].ucStatus[ADR-2][i]=tucDZ_Active_Data_Buf[i+27];
													}
												}
											}
//-------------------- 获取参数(浮点数) ------------------------------------------------------------------------------------
											else if(CID2==0x46)
											{
												if( (CID1==0x42) && (mode==D_REV_MODE_RECTIFIER_DC_DISTRIBUTION_GET_PARA_108) )//直流配电
												{
													g_Result_Status	= 1;
													g_Rectifiter.Mask |=0x10;
//               0 1 2 3   4 5 6 7   8               9 0 1 2        3 4 5 6        7 8 9 0    1 2 3 4  5 6 7
//~21014200 303A 1A006A42  1A003C42  05              1A007042       1A003442       1A006C42   1A004842 4D334342 F19F
//							 直流V上限 直流V下限 自定义状态数量p 电池高温告警值 环境高温告警值 高压关机值 均充电压 浮充电压
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[i]=Exchange_Buf[3-i];
													}
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+4,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[4+i]=Exchange_Buf[3-i];
													}
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+9,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[9+i]=Exchange_Buf[3-i];
													}
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+13,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[13+i]=Exchange_Buf[3-i];
													}
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+17,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[17+i]=Exchange_Buf[3-i];
													}
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+21,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[21+i]=Exchange_Buf[3-i];
													}
													memcpy(Exchange_Buf,tucDZ_Active_Data_Buf+25,4);
													for(i=0;i<4;i++)
													{
														tucDZ_Active_Data_Buf[25+i]=Exchange_Buf[3-i];
													}													
													g_Rectifiter.fPara_DC_Distribution_Get[0]=*((float*)(tucDZ_Active_Data_Buf));   //直流电压上限
													g_Rectifiter.fPara_DC_Distribution_Get[1]=*((float*)(tucDZ_Active_Data_Buf+4)); //直流电压下限
													g_Rectifiter.fPara_DC_Distribution_Get[2]=*((float*)(tucDZ_Active_Data_Buf+9)); //电池高温告警值
													g_Rectifiter.fPara_DC_Distribution_Get[3]=*((float*)(tucDZ_Active_Data_Buf+13));//环境高温告警值
													g_Rectifiter.fPara_DC_Distribution_Get[4]=*((float*)(tucDZ_Active_Data_Buf+17));//高压关机值
													g_Rectifiter.fPara_DC_Distribution_Get[5]=*((float*)(tucDZ_Active_Data_Buf+21));//均充电压
													g_Rectifiter.fPara_DC_Distribution_Get[6]=*((float*)(tucDZ_Active_Data_Buf+25));//浮充电压
												}
											}
//-------------------- 获取系统参数 ---------------------------------------------------------------------------------------
											else if(CID2==0x47)
											{
												if( (CID1==0x46) && (mode==D_REV_MODE_PYLON_BAT_GET_SYSTEM_PARA_135) ) //PYLON电池
												{
													g_Result_Status	= 1;
													g_PYLON_BAT.Mask |=4;	
//                                                            1                                                   2
//               0  1 2          3 4      5 6      7 8      9 0     1 2          3 4          5 6      7 8      9 0        1 2   3 4          5 6 7 8 9
//~20024600 B032 11 0E42         0BEA     0AF0     0D03     0A47    0384         D2F0         B3B0     A9EC     0D03       0A47  FC7C         F272
//                  单芯过压上限 单芯低压	单芯欠压 充电温度上、下限 充电电流限值 模块过压上限 模块低压 模块欠压 放电温度上、下限 放电电流限值
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fSingle_Cell_High_Voltage_Limit  =((((unsigned short int)tucDZ_Active_Data_Buf[1])<<8)  | tucDZ_Active_Data_Buf[2])/1000.0f;//原数放大了1000倍													
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fSingle_Cell_Low_Voltage_Limit   =((((unsigned short int)tucDZ_Active_Data_Buf[3])<<8)  | tucDZ_Active_Data_Buf[4])/1000.0f;//原数放大了1000倍
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fSingle_Cell_Under_Voltage_Limit =((((unsigned short int)tucDZ_Active_Data_Buf[5])<<8)  | tucDZ_Active_Data_Buf[6])/1000.0f;//原数放大了1000倍
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fCharge_High_Temperature_Limit   =(((float)((((unsigned short int)tucDZ_Active_Data_Buf[7])<<8) | tucDZ_Active_Data_Buf[8]))-2731)/10.0f;//原数放大了10倍
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fCharge_Low_Temperature_Limit    =(((float)((((unsigned short int)tucDZ_Active_Data_Buf[9])<<8) | tucDZ_Active_Data_Buf[10]))-2731)/10.0f;//原数放大了10倍
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fCharge_Current_Limit            =((((unsigned short int)tucDZ_Active_Data_Buf[11])<<8) | tucDZ_Active_Data_Buf[12])/10.0f;//原数放大了10倍
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fModule_High_Voltage_Limit       =((((unsigned short int)tucDZ_Active_Data_Buf[13])<<8) | tucDZ_Active_Data_Buf[14])/1000.0f;//原数放大了1000倍
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fModule_Low_Voltage_Limit        =((((unsigned short int)tucDZ_Active_Data_Buf[15])<<8) | tucDZ_Active_Data_Buf[16])/1000.0f;//原数放大了1000倍
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fModule_Under_Voltage_Limit      =((((unsigned short int)tucDZ_Active_Data_Buf[17])<<8) | tucDZ_Active_Data_Buf[18])/1000.0f;//原数放大了1000倍
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fDischarge_High_Temperature_Limit=(((float)((((unsigned short int)tucDZ_Active_Data_Buf[19])<<8) | tucDZ_Active_Data_Buf[20]))-2731)/10.0f;//原数放大了10倍
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fDischarge_Low_Temperature_Limit =(((float)((((unsigned short int)tucDZ_Active_Data_Buf[21])<<8) | tucDZ_Active_Data_Buf[22]))-2731)/10.0f;//原数放大了10倍
													g_PYLON_BAT.Get_Host_System_PARA[ucPYLON_BAT_SETS].fDischarge_Current_Limit         =((((unsigned short int)tucDZ_Active_Data_Buf[23])<<8) | tucDZ_Active_Data_Buf[24])/1000.0f;//原数放大了1000倍
												}
											}
//-------------------- 获取系统基本信息 ------------------------------------------------------------------------------------
											else if(CID2==0x60)
											{
												if( (CID1==0x46) && (mode==D_REV_MODE_PYLON_BAT_GET_SYSTEM_BASIC_INFO_133) ) //PYLON电池
												{                               
													g_Result_Status	= 1;
													g_PYLON_BAT.Mask =1;					
//               0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1         2        3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8     9 0
//~20024600 8062 55 50 35 30 30 30 00 00 00 00 50 79 6C 6F 6E 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 02 01        02       50 50 54 41 41 30 31 31 30 31 33 31 30 32 34 34    EA69
//               主机设备名称                  主机厂商名称                                                主机软件版本 电池数量 主机SN
													g_PYLON_BAT.Get_System_Basic_INFO[ucPYLON_BAT_SETS].ucBAT_Num_Same_Group=tucDZ_Active_Data_Buf[32];
													for(i=0;i<16;i++)
													{
														g_PYLON_BAT.Get_System_Basic_INFO[ucPYLON_BAT_SETS].ucSN_Master_Moudle[i]=tucDZ_Active_Data_Buf[i+33];
													}
													g_PYLON_BAT.Get_System_Basic_INFO[ucPYLON_BAT_SETS].ucSN_Master_Moudle[16]= 0 ;
													g_PYLON_BAT.Get_System_Basic_INFO[ucPYLON_BAT_SETS].ucSN_Master_Moudle[17]= 0;
												}												
											}
//-------------------- 获取自定义参数 ------------------------------------------------------------------------------------
											else if(CID2==0x80)
											{
												if( (CID1==0x40) && (mode==D_REV_MODE_RECTIFIER_AC_DISTRIBUTION_GET_CUSTOM_PARA1_109) )	//交流配电
												{
//                                                                                                                                                                                                                                       1                                                                                                                                                                                                              2                                                                                        
//                                   1                   2                   3                    4                   5                   6                    7                    8                    9                               0                    1                   2                    3                   4                     5                   6                    7                   8                    9                    0                    1                   2                   3                   4                     5   
//               0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5  6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2  3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8  9 0 1 2 3 4 5 6 7  8 9 0 1             2 3 4 5 6 7 8 9 0 1 2 3 4 5 6  7 8 9 0 1 2 3 4 5 6 7 8 9 0  1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1  2 3 4 5 6 7 8 9  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6  7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6  7 8 9 0 1 2 3 4 5 6  7 8 9 0 1 2 3 4 5 6  7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5  6 7 8 9  0 1 2 3 4 5 6 7 8 9 
//~21014000 C1F4 06000122009A20202020202020202020202020202020200122009A202020202020202020 0320202020024E0064202020202020202020202020202020202020 002D2020003C202000024901D6192000 01012C009601E801F4 001E010A            0001B801EA0101CC01EA2020202020 01000601E03C001E002C070B0A37 0101032C040B0A3705140103140101E50105000020 010A0A0A0A01B80F 0020000000000000000000000000000000 0001070001D1021E170514091A5A412820202001 0100CD02000002009600 20202020202020202020 000000000000000000000000000020202020202020202020202020202020200000000000000000 000000B4 9AD0
//               交流参数36字节                                                           模块参数27字节                                         告警参数16字节                   电池参数9个字节    温度补偿参数4个字节 LVDS参数15字节                 电池测试参数14字节           均充参数21字节                             充电限流参数8B   通讯参数17B                        效能管理参数20B                          其它参数10B          扩展板参数10B        电池测试记录39B                                                                预留4B
													g_Rectifiter.AC_Distribution_Get_Custom_PARA1_Alarm_PARA.ucValue_Charge_Overcurrent                           =tucDZ_Active_Data_Buf[76];
													g_Rectifiter.AC_Distribution_Get_Custom_PARA1_BAT_PARA.ucNum_Pack                                             =tucDZ_Active_Data_Buf[79];
													g_Rectifiter.AC_Distribution_Get_Custom_PARA1_BAT_PARA.usCapacity_Rated                                       =(tucDZ_Active_Data_Buf[80]<<8) | tucDZ_Active_Data_Buf[81];
													g_Rectifiter.AC_Distribution_Get_Custom_PARA1_BAT_PARA.usRange_Current_Full                                   =(tucDZ_Active_Data_Buf[82]<<8) | tucDZ_Active_Data_Buf[83];
													g_Rectifiter.AC_Distribution_Get_Custom_PARA1_TC_PARA.ucSwitch                                                =tucDZ_Active_Data_Buf[88];
													g_Rectifiter.AC_Distribution_Get_Custom_PARA1_LVDS_PARA.ucMode_BAT_LVDS                                       =tucDZ_Active_Data_Buf[92];
													g_Rectifiter.AC_Distribution_Get_Custom_PARA1_EQ_Charge_PARA.ucFunction_Switch                                =tucDZ_Active_Data_Buf[121];
													g_Rectifiter.AC_Distribution_Get_Custom_PARA1_EQ_Charge_PARA.ucMode                                           =tucDZ_Active_Data_Buf[122];
													g_Rectifiter.AC_Distribution_Get_Custom_PARA1_EQ_Charge_PARA.ucPerioid                                        =tucDZ_Active_Data_Buf[123];
													g_Rectifiter.AC_Distribution_Get_Custom_PARA1_Charge_Current_Limit_PARA.ucValue_Constant_Voltage_Current_Limit=tucDZ_Active_Data_Buf[146];
												}
											}
//-------------------- 4.2 系统模拟量/Analog data of system  --------------------------------------------------------------------------------
											else if(CID2==0x61)
											{
												if(       (CID1==0x46) && (mode==D_REV_MODE_PYLON_BAT_GET_SYSTEM_ANALOG_136) ) //PYLON电池
												{
													g_Result_Status	= 1;
													g_PYLON_BAT.Mask |=0x40;	
													//	float faverage_module_voltage;			//平均电压				V		average module voltage  		.3f 	2Byte
													//	float ftotal_current;								//总电流					A		total current   						.2f		2Byte
													//	unsigned char 	ucaverage_SOC;									//平均SOC					%		average SOC 								0			1Byte
													//	unsigned short int 	ucaverage_cycle_number;				//平均循环次数				average cycle number				0			2Byte
													//	unsigned char    ucaverage_SOH;								//平均 SOH				%		average SOH 								0			1Byte
													//	float	faverage_cell_temperature; 		//单芯平均温度		K		average cell temperature  	.1f 	2Byte
													//	float	faverage_MOSFET_temperature;	//MOSFET 平均温度	K		average MOSFET temperature	.1f 	2Byte
													//	float	faverage_BMS_temperature;			//BMS 平均温度		K		average BMS temperature			.1 		2Byte
													g_PYLON_BAT.Get_System_Analog[ucPYLON_BAT_SETS].faverage_module_voltage   	=((((unsigned short int)tucDZ_Active_Data_Buf[0])<<8) | tucDZ_Active_Data_Buf[1])/1000.0f;
													Chksum = (((unsigned short int)tucDZ_Active_Data_Buf[2])<<8) | tucDZ_Active_Data_Buf[3];
													if(Chksum&0x8000)
													{
														Chksum = 0xffff-Chksum+1;
														g_PYLON_BAT.Get_System_Analog[ucPYLON_BAT_SETS].ftotal_current = Chksum/-100.0f;
													}
													else
													{
														g_PYLON_BAT.Get_System_Analog[ucPYLON_BAT_SETS].ftotal_current = Chksum/100.0f;
													}																				
													g_PYLON_BAT.Get_System_Analog[ucPYLON_BAT_SETS].ucaverage_SOC         			=tucDZ_Active_Data_Buf[4];
													g_PYLON_BAT.Get_System_Analog[ucPYLON_BAT_SETS].ucaverage_cycle_number    	=(((unsigned short int)tucDZ_Active_Data_Buf[5])<<8) | tucDZ_Active_Data_Buf[6];
													g_PYLON_BAT.Get_System_Analog[ucPYLON_BAT_SETS].ucaverage_SOH     					=tucDZ_Active_Data_Buf[9];													
													g_PYLON_BAT.Get_System_Analog[ucPYLON_BAT_SETS].faverage_cell_temperature 	=(((((unsigned short int)tucDZ_Active_Data_Buf[19])<<8) | tucDZ_Active_Data_Buf[20])-2731)/10.0f;	
													g_PYLON_BAT.Get_System_Analog[ucPYLON_BAT_SETS].faverage_MOSFET_temperature =(((((unsigned short int)tucDZ_Active_Data_Buf[29])<<8) | tucDZ_Active_Data_Buf[30])-2731)/10.0f;	
													g_PYLON_BAT.Get_System_Analog[ucPYLON_BAT_SETS].faverage_BMS_temperature    =(((((unsigned short int)tucDZ_Active_Data_Buf[39])<<8) | tucDZ_Active_Data_Buf[40])-2731)/10.0f;									
												}
											}													
//-------------------- 4.3 系统告警保护/alarm & protection info of system  --------------------------------------------------------------------------------
											else if(CID2==0x62)
											{
												if(       (CID1==0x46) && (mode==D_REV_MODE_PYLON_BAT_GET_SYSTEM_ALARM_137) ) //PYLON电池
												{
													g_Result_Status	= 1;
													g_PYLON_BAT.Mask |=0x80;	
													//	float faverage_module_voltage;			//平均电压				V		average module voltage  		.3f 	2Byte
													//	float ftotal_current;								//总电流					A		total current   						.2f		2Byte
													//	unsigned char 	ucaverage_SOC;									//平均SOC					%		average SOC 								0			1Byte
													//	unsigned short int 	ucaverage_cycle_number;				//平均循环次数				average cycle number				0			2Byte
													//	unsigned char    ucaverage_SOH;								//平均 SOH				%		average SOH 								0			1Byte
													//	float	faverage_cell_temperature; 		//单芯平均温度		K		average cell temperature  	.1f 	2Byte
													//	float	faverage_MOSFET_temperature;	//MOSFET 平均温度	K		average MOSFET temperature	.1f 	2Byte
													//	float	faverage_BMS_temperature;			//BMS 平均温度		K		average BMS temperature			.1 		2Byte
													g_PYLON_BAT.ucSystem_Alarm[ucPYLON_BAT_SETS][0] = tucDZ_Active_Data_Buf[0];														
													g_PYLON_BAT.ucSystem_Alarm[ucPYLON_BAT_SETS][1] = tucDZ_Active_Data_Buf[1];														
													g_PYLON_BAT.ucSystem_Alarm[ucPYLON_BAT_SETS][2] = tucDZ_Active_Data_Buf[2];														
													g_PYLON_BAT.ucSystem_Alarm[ucPYLON_BAT_SETS][3] = tucDZ_Active_Data_Buf[3];														
												}
											}					
//-------------------- 获取电池充放电管理信息-->系统交互信息 / system recommend info -----------------------------------------------------------------------
											else if(CID2==0x63)		//else if(CID2==0x92)
											{
												if(       (CID1==0x46) && (mode==D_REV_MODE_PYLON_BAT_GET_BAT_CHARGE_DISCHARGE_MI_134) ) //PYLON电池
												{
													g_Result_Status	= 1;
													g_PYLON_BAT.Mask |=0x20;	
//               0         1 2              3 4              5 6          7 8	         9 											
//~20024600 B014 02        D002             AFC8             00C8         FE0C         C0          F915
//               Cmd_Value 充电电压建议上限	放电电压建议下限 最大充电电流	最大放电电流 充放电状态		
													g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[ucPYLON_BAT_SETS].fCharge_Voltage_Upper_Limit   =((((unsigned short int)tucDZ_Active_Data_Buf[0])<<8) | tucDZ_Active_Data_Buf[1])/1000.0f;//原数放大了1000倍
													g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[ucPYLON_BAT_SETS].fDischarge_Voltage_Lower_Limit=((((unsigned short int)tucDZ_Active_Data_Buf[2])<<8) | tucDZ_Active_Data_Buf[3])/1000.0f;//原数放大了1000倍
													g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[ucPYLON_BAT_SETS].fCharge_Current_Limit         =((((unsigned short int)tucDZ_Active_Data_Buf[4])<<8) | tucDZ_Active_Data_Buf[5])/10.0f;  //原数放大了1000倍
													nCRC = (unsigned short int)(tucDZ_Active_Data_Buf[6]<<8 | tucDZ_Active_Data_Buf[7]);
													if(nCRC&0x8000)
													{
														nCRC = 0xffff-nCRC+1;
														g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[ucPYLON_BAT_SETS].fDischarge_Current_Limit      =nCRC/-10.0f;
													}
													else
													{
														g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[ucPYLON_BAT_SETS].fDischarge_Current_Limit      =nCRC/10.0f;
													}													
													g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[ucPYLON_BAT_SETS].ucCharge_Discharge_Status     =tucDZ_Active_Data_Buf[8];
												}
											}
//-------------------- 获取SN ------------------------------------------------------------------------------------------------
											else if(CID2==0x93)
											{
												if(       (CID1==0x46) && (mode==D_REV_MODE_PYLON_BAT_GET_SN_132) ) //PYLON电池
												{
													g_Result_Status	= 1;
													g_PYLON_BAT.Mask |=0x10;	
//               0						 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6  7 8
//~20024600 C022 02            50505441483032343031343436303035 F6DA
//               Command value P P T A H 0 2 4 0 1 4 4 6 0 0 5
													for(i=0;i<16;i++)
													{
														g_PYLON_BAT.ucSN[ucPYLON_BAT_SETS][i+ADR-2]=tucDZ_Active_Data_Buf[i+1];
													}													
												}
											}
//----------------------------------------------------------------------------------------------------------------------------
											CID2=0;											
										}
									}
								}			
								return;			
							}
							else			//add by wjj 2023.5.30
							{
								break;
							}
						}
						else
						{
							N++;
						}
					}
					else
					{
						//01 03 38 39 30 B8 0B 2C 01 E0 01 CC 01 E0 01 CC 01 CC 01 90 01 44 02 5E 01 E0 01 02 00 01 00 02 00 01 00 00 00 01 00 00 00 00 00 00 00 00 00 1E 00 64 00 C8 00 C3 1B C0 A8 00 6B FB D8
						if( Arr_rece[N] == modbus_id )   						//5A A5 07 82 1000 0001 0002      = Modbus_ID;
						{	     
							if(Arr_rece[N+1]>0 && Arr_rece[N+1]<5)			//if(Arr_rece[N+1]==03 || ( Arr_rece[N+1]==01 && 2==mode ))
							{								
									 cnt  = Arr_rece[N+2];									//有效数据个数		  01 03 02 00 00 crc														
									 if((cnt+5+N) < Data_len)							//if((cnt+5) < *Data_len)						//判断数据总个数
									 {
											return;
									 }
									 
									 nCRC = CrcCheck(Arr_rece+N,cnt+3);																					//crc校验	0a41												
									 if(Arr_rece[N+3+cnt]==(nCRC&0xff)  && Arr_rece[N+4+cnt]==((nCRC>>8)&0xff))     //crc正确
									 {	   
											Flog=0;  										
											if(4==mode && 160==cnt)							//0x0000  5
											{						
														unsigned int    tmp_data = 0;														
																									
														g_Mttp[g_cur_pos].V_PU_Hi    =  (Arr_rece[N+3]<<8)|Arr_rece[N+4]  ;		//01 03 06
														g_Mttp[g_cur_pos].V_PU_Lo    =  (Arr_rece[N+5]<<8)|Arr_rece[N+6]  ;		//01 03 06
													
														g_Mttp[g_cur_pos].I_PU_Hi    =  (Arr_rece[N+7]<<8)|Arr_rece[N+8] ;		//01 03 06
														g_Mttp[g_cur_pos].I_PU_Lo    =  (Arr_rece[N+9]<<8)|Arr_rece[N+10] ;		//01 03 06
													
														g_Mttp[g_cur_pos].SoftVer	   =  (Arr_rece[N+11]<<8)|Arr_rece[N+12];				
															//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
														for(i=0;i<27;i++)								//0x0018=24=i  N+3+i*2		
														{	
																g_Mttp[g_cur_pos].Reg[i]	=  (Arr_rece[N+51+2*i]<<8)|Arr_rece[N+52+2*i];								
														}
													
														g_Mttp[g_cur_pos].T_hs = (Arr_rece[N+3+70]<<8) | Arr_rece[N+4+70];			//0x0018  0x0023	Heatsink temperature
														g_Mttp[g_cur_pos].T_rts = (Arr_rece[N+3+72]<<8) | Arr_rece[N+4+72];		//0x0024					rts温度
														//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
														//[2a
														g_Mttp[g_cur_pos].Mppt_HM    			= 	(Arr_rece[N+3+84]<<24)|(Arr_rece[N+3+85]<<16)|(Arr_rece[N+3+86]<<8)|Arr_rece[N+3+87];			//2a=42 84
                            g_Mttp[g_cur_pos].Fault_all_2C     =   (Arr_rece[N+3+88]<<8)|Arr_rece[N+3+89];                                                             
														g_Mttp[g_cur_pos].Mppt_alarm 			=   (Arr_rece[N+3+8+84]<<24)|(Arr_rece[N+3+9+84]<<16)|(Arr_rece[N+3+10+84]<<8)|Arr_rece[N+3+11+84];
                            g_Mttp[g_cur_pos].dip_all_30       =   (Arr_rece[N+3+12+84]<<8)|Arr_rece[N+3+13+84];
                            g_Mttp[g_cur_pos].led_state_31     =   (Arr_rece[N+3+14+84]<<8)|Arr_rece[N+3+15+84];
                            g_Mttp[g_cur_pos].vb_ref_33        =   ((Arr_rece[N+3+18+84]<<8)|Arr_rece[N+3+19+84])*(g_Mttp[g_cur_pos].V_PU_Hi + g_Mttp[g_cur_pos].V_PU_Lo/65536.0)/32768.0;
												
														tmp_data                =   (Arr_rece[N+3+20+84]<<24)|(Arr_rece[N+3+21+84]<<16)|(Arr_rece[N+3+22+84]<<8)|Arr_rece[N+3+23+84];                                
														g_Mttp[g_cur_pos].Mppt_Ahc_r 			= 	tmp_data*0.1f;		//Ah charge – resetable
												
														tmp_data                =   (Arr_rece[N+3+24+84]<<24)|(Arr_rece[N+3+25+84]<<16)|(Arr_rece[N+3+26+84]<<8)|Arr_rece[N+3+27+84];
                            g_Mttp[g_cur_pos].Mppt_Ahc_t 			= 	tmp_data*0.1f;	 	//Ah charge – total
												
                            g_Mttp[g_cur_pos].kwhc_r_38        =   (Arr_rece[N+3+28+84]<<8)|Arr_rece[N+3+29+84];
                            g_Mttp[g_cur_pos].kwhc_t_39        =   (Arr_rece[N+3+30+84]<<8)|Arr_rece[N+3+31+84];       
														//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
														//0  1  2  34 56 78 910 1112 1314 1516 1718 1920    4b-43=8+1
                            //01 03 24 43 44 45 46   47   48    49  4a   4b   
														g_Mttp[g_cur_pos].Ahc_daily            =   (Arr_rece[N+3+0+132]<<8) | Arr_rece[N+3+1+132];				//0x0043 Total Ah charge daily *0.1    
                            g_Mttp[g_cur_pos].whc_daily            =   (Arr_rece[N+3+2+132]<<8) | Arr_rece[N+3+3+132];				//0x0044 Total Wh charge daily
                            g_Mttp[g_cur_pos].flags_daily          =   (Arr_rece[N+3+4+132]<<8) | Arr_rece[N+3+5+132];				//0x0045 Daily flags bitfield
                            g_Mttp[g_cur_pos].Pout_max_daily       =   ((Arr_rece[N+3+6+132]<<8) | Arr_rece[N+3+7+132])*(g_Mttp[g_cur_pos].I_PU_Hi + g_Mttp[g_cur_pos].I_PU_Lo/65536.0)*(g_Mttp[g_cur_pos].V_PU_Hi + g_Mttp[g_cur_pos].V_PU_Lo/65536.0)/131072.0;				//0x0046 Max. Power Out, daily n·V_PU·I_PU·2(-17)
                            g_Mttp[g_cur_pos].alarm_daily_HI       =   (Arr_rece[N+3+16+132]<<8) | Arr_rece[N+3+17+132];			//  4b
                            g_Mttp[g_cur_pos].alarm_daily_LO       =   (Arr_rece[N+3+18+132]<<8) | Arr_rece[N+3+19+132];			//  4c
                            //g_Mttp.alarm_daily        =   (Arr_rece[N+3+16]<<8) | Arr_rece[N+3+17];//0x004B  0x004C                                          
                            g_Mttp[g_cur_pos].time_ab_daily        =   (Arr_rece[N+3+20+132]<<8) | Arr_rece[N+3+21+132];			//0x004D cumulative time in absorption, daily
                            g_Mttp[g_cur_pos].time_eq_daily        =   (Arr_rece[N+3+22+132]<<8) | Arr_rece[N+3+23+132];			//0x004E cumulative time in equalize, daily
                            g_Mttp[g_cur_pos].Vtime_fl_daily       =   (Arr_rece[N+3+24+132]<<8) | Arr_rece[N+3+25+132];			//0x004F cumulative time in float,daily
														
														g_Mttp[g_cur_pos].Mask |=1;
														g_Result_Status = 1;
														return;
											}											
											if(5==mode && 28==cnt)				//0xE0C0	14			E0CD = Hardware version
											{
														memcpy(g_Mttp[g_cur_pos].SN,Arr_rece+3+N,8);				//sn		
														tmp = g_Mttp[g_cur_pos].SN[0] ;   g_Mttp[g_cur_pos].SN[0] = g_Mttp[g_cur_pos].SN[1] ;  g_Mttp[g_cur_pos].SN[1] = tmp ;
														tmp = g_Mttp[g_cur_pos].SN[2] ;   g_Mttp[g_cur_pos].SN[2] = g_Mttp[g_cur_pos].SN[1] ;  g_Mttp[g_cur_pos].SN[3] = tmp ;
														tmp = g_Mttp[g_cur_pos].SN[4] ;   g_Mttp[g_cur_pos].SN[4] = g_Mttp[g_cur_pos].SN[1] ;  g_Mttp[g_cur_pos].SN[5] = tmp ;
														tmp = g_Mttp[g_cur_pos].SN[6] ;   g_Mttp[g_cur_pos].SN[6] = g_Mttp[g_cur_pos].SN[1] ;  g_Mttp[g_cur_pos].SN[7] = tmp ;
												
														g_Mttp[g_cur_pos].Model      =  (Arr_rece[N+27]<<8)|Arr_rece[N+28];							
														g_Mttp[g_cur_pos].HardVer	   =  (Arr_rece[N+29]<<8)|Arr_rece[N+30];							
														g_Mttp[g_cur_pos].Mask |=2;
														g_Result_Status	= 1;	
														return;												
											}
//											if(6==mode)				//0x0018  27
//											{
//													for(i=0;i<27;i++)			
//													{	
//																g_Mttp[g_cur_pos].Reg[i]	=  (Arr_rece[N+3+2*i]<<8)|Arr_rece[N+4+2*i];								
//													}
//													g_Mttp[g_cur_pos].Mask |=4;
//													g_Result_Status	= 1;				
//													return;
//											}																	
//											if(7==mode)
//											{
//												g_Mttp.ucEmodbus_id=(Arr_rece[N+3]<<8)|Arr_rece[N+4];
//												g_Mttp.Mask |=8;
//												g_Result_Status	= 1;				
//												return;
//											}
//-------------------- 柴油发电机diesel generator ---------------------------------------------------------------------
											if(D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_1024_1089_40==mode)
											{
												g_Diesel_Generator.Value_Display.Engine_Para.usOil_pressure													=         (Arr_rece[N+3+(1024-1024)*2]<<8) + Arr_rece[N+4+(1024-1024)*2];																																							//发动机油压,						1024				R	0	       		10000	    	1	    Kpa	  	16
												g_Diesel_Generator.Value_Display.Engine_Para.fCoolant_temperature										=         (Arr_rece[N+3+(1025-1024)*2]<<8) + Arr_rece[N+4+(1025-1024)*2];																																							//冷却液温度,						1025				R -50	     		200	      	1	    ℃	  	16 S
												g_Diesel_Generator.Value_Display.Engine_Para.usFule_level														=         (Arr_rece[N+3+(1027-1024)*2]<<8) + Arr_rece[N+4+(1027-1024)*2];																																			//           						1027				R 0	         	130	      	1	    %	    	16
												g_Diesel_Generator.Value_Display.Engine_Para.fCharge_alternator_voltage							=((float)((Arr_rece[N+3+(1028-1024)*2]<<8) + Arr_rece[N+4+(1028-1024)*2]))*0.1f;																																			//充电发动机电压,				1028				R	0	          40	        0.1	  V				16
												g_Diesel_Generator.Value_Display.Engine_Para.fEngine_battery_voltage								=((float)((Arr_rece[N+3+(1029-1024)*2]<<8) + Arr_rece[N+4+(1029-1024)*2]))*0.1f;																																			//发动机电池电压,				1029				R	0	          40	        0.1		V				16
												g_Diesel_Generator.Value_Display.Engine_Para.usEngine_speed    											=         (Arr_rece[N+3+(1030-1024)*2]<<8) + Arr_rece[N+4+(1030-1024)*2];																																							//发动机转速,						1030				R	0	          6000	      1			RPM			16
												
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_frequency								=((float)((Arr_rece[N+3+(1031-1024)*2]<<8) + Arr_rece[N+4+(1031-1024)*2]))*0.1f;																																			//发电机频率      			1031				R	0						70					0.1		Hz			16
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1_N_voltage							=((float)((Arr_rece[N+3+(1032-1024)*2]<<24)+(Arr_rece[N+4+(1032-1024)*2]<<16) + (Arr_rece[N+5+(1032-1024)*2]<<8) + Arr_rece[N+6+(1032-1024)*2]))*0.1f;//L1-N相电压						1032和1033	R	0						18,000			0.1		V				32
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L2_N_voltage							=(((float)(Arr_rece[N+3+(1034-1024)*2]<<24)+(Arr_rece[N+4+(1034-1024)*2]<<16) + (Arr_rece[N+5+(1034-1024)*2]<<8) + Arr_rece[N+6+(1034-1024)*2]))*0.1f;//L2-N相电压						1034和1035	R	0						18,000			0.1		V				32
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L3_N_voltage							=((float)((Arr_rece[N+3+(1036-1024)*2]<<24)+(Arr_rece[N+4+(1036-1024)*2]<<16) + (Arr_rece[N+5+(1036-1024)*2]<<8) + Arr_rece[N+6+(1036-1024)*2]))*0.1f;//L3-N相电压						1036和1037	R	0						18,000			0.1		V				32
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1_L2_voltage						=((float)((Arr_rece[N+3+(1038-1024)*2]<<24)+(Arr_rece[N+4+(1038-1024)*2]<<16) + (Arr_rece[N+5+(1038-1024)*2]<<8) + Arr_rece[N+6+(1038-1024)*2]))*0.1f;//L1-L2线电压						1038和1039	R	0						30,000			0.1		V				32
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L2_L3_voltage						=((float)((Arr_rece[N+3+(1040-1024)*2]<<24)+(Arr_rece[N+4+(1040-1024)*2]<<16) + (Arr_rece[N+5+(1040-1024)*2]<<8) + Arr_rece[N+6+(1040-1024)*2]))*0.1f;//L2-L3线电压						1040和1041	R	0						30,000			0.1		V				32
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L3_L1_voltage						=((float)((Arr_rece[N+3+(1042-1024)*2]<<24)+(Arr_rece[N+4+(1042-1024)*2]<<16) + (Arr_rece[N+5+(1042-1024)*2]<<8) + Arr_rece[N+6+(1042-1024)*2]))*0.1f;//L3-L1线电压						1042和1043	R	0						30,000			0.1		V				32
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1_current								=((float)((Arr_rece[N+3+(1044-1024)*2]<<24)+(Arr_rece[N+4+(1044-1024)*2]<<16) + (Arr_rece[N+5+(1044-1024)*2]<<8) + Arr_rece[N+6+(1044-1024)*2]))*0.1f;//L1相电流							1044和1045	R	0						99,999.90		0.1		A				32
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L2_current								=((float)((Arr_rece[N+3+(1046-1024)*2]<<24)+(Arr_rece[N+4+(1046-1024)*2]<<16) + (Arr_rece[N+5+(1046-1024)*2]<<8) + Arr_rece[N+6+(1046-1024)*2]))*0.1f;//L2相电流							1046和1047	R	0						99,999.90		0.1		A				32
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L3_current								=((float)((Arr_rece[N+3+(1048-1024)*2]<<24)+(Arr_rece[N+4+(1048-1024)*2]<<16) + (Arr_rece[N+5+(1048-1024)*2]<<8) + Arr_rece[N+6+(1048-1024)*2]))*0.1f;//L3相电流							1048和1049	R	0						99,999.90		0.1		A				32
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_earth_current						=((float)((Arr_rece[N+3+(1050-1024)*2]<<24)+(Arr_rece[N+4+(1050-1024)*2]<<16) + (Arr_rece[N+5+(1050-1024)*2]<<8) + Arr_rece[N+6+(1050-1024)*2]))*0.1f;//接地电流							1050和1051	R	0						99,999.90		0.1		A				32
												g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L1_watts								=         (Arr_rece[N+3+(1052-1024)*2]<<24)+(Arr_rece[N+4+(1052-1024)*2]<<16) + (Arr_rece[N+5+(1052-1024)*2]<<8) + Arr_rece[N+6+(1052-1024)*2];				//L1相有功功率					1052和1053	R	-99,999,999	99,999,999	1			W				32 S
												g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L2_watts								=         (Arr_rece[N+3+(1054-1024)*2]<<24)+(Arr_rece[N+4+(1054-1024)*2]<<16) + (Arr_rece[N+5+(1054-1024)*2]<<8) + Arr_rece[N+6+(1054-1024)*2];				//L2相有功功率					1054和1055	R	-99,999,999	99,999,999	1			W				32 S
												g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L3_watts								=         (Arr_rece[N+3+(1056-1024)*2]<<24)+(Arr_rece[N+4+(1056-1024)*2]<<16) + (Arr_rece[N+5+(1056-1024)*2]<<8) + Arr_rece[N+6+(1056-1024)*2];				//L3相有功功率					1056和1057	R	-99,999,999	99,999,999	1			W				32 S
												g_Diesel_Generator.Value_Display.Generator_Para.ssGenerator_current_lag_lead				=					(Arr_rece[N+3+(1058-1024)*2]<<8) + Arr_rece[N+4+(1058-1024)*2];																																							//电流超前/滞后					1058				R	-180				180					1			度			16 S
												g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L1L2L3_watts = g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L1_watts+g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L2_watts+g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L3_watts;
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1L2L3_current	  = g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1_current	+ g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L2_current	+ g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L3_current	;
												g_Result.slGenerator_L1L2L3_watts  = g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L1L2L3_watts;
												g_Result.fGenerator_L1L2L3_current = g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1L2L3_current;
												
#if 0  
												//市电参数	
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_frequency							=((float)((Arr_rece[N+3+(1059-1024)*2]<<8) + Arr_rece[N+4+(1059-1024)*2]))*0.1f;																																			//市电频率							1059				R	0						70					0.1		Hz			16
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_L1_N_voltage					=((float)((Arr_rece[N+3+(1060-1024)*2]<<24)+(Arr_rece[N+4+(1060-1024)*2]<<16) + (Arr_rece[N+5+(1060-1024)*2]<<8) + Arr_rece[N+6+(1060-1024)*2]))*0.1f;//L1-N相电压						1060和1061	R	0						18000				0.1		V				32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_L2_N_voltage					=((float)((Arr_rece[N+3+(1062-1024)*2]<<24)+(Arr_rece[N+4+(1062-1024)*2]<<16) + (Arr_rece[N+5+(1062-1024)*2]<<8) + Arr_rece[N+6+(1062-1024)*2]))*0.1f;//L2-N相电压						1062和1063	R	0						18000				0.1		V				32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_L3_N_voltage					=((float)((Arr_rece[N+3+(1064-1024)*2]<<24)+(Arr_rece[N+4+(1064-1024)*2]<<16) + (Arr_rece[N+5+(1064-1024)*2]<<8) + Arr_rece[N+6+(1064-1024)*2]))*0.1f;//L3-N相电压						1064和1065	R	0						18000				0.1		V				32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_L1_L2_voltage					=((float)((Arr_rece[N+3+(1066-1024)*2]<<24)+(Arr_rece[N+4+(1066-1024)*2]<<16) + (Arr_rece[N+5+(1066-1024)*2]<<8) + Arr_rece[N+6+(1066-1024)*2]))*0.1f;//L1-L2线电压						1066和1067	R	0						30000				0.1		V				32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_L2_L3_voltage					=((float)((Arr_rece[N+3+(1068-1024)*2]<<24)+(Arr_rece[N+4+(1068-1024)*2]<<16) + (Arr_rece[N+5+(1068-1024)*2]<<8) + Arr_rece[N+6+(1068-1024)*2]))*0.1f;//L2-L3线电压						1068和1069	R	0						30000				0.1		V				32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_L3_L1_voltage					=((float)((Arr_rece[N+3+(1070-1024)*2]<<24)+(Arr_rece[N+4+(1070-1024)*2]<<16) + (Arr_rece[N+5+(1070-1024)*2]<<8) + Arr_rece[N+6+(1070-1024)*2]))*0.1f;//L3-L1线电压						1070和1071	R	0						30000				0.1		V				32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.ssMains_voltage_phase_lag_lead=				(Arr_rece[N+3+(1072-1024)*2]<<8) + Arr_rece[N+4+(1072-1024)*2];																																							//市电相序滞后/超前			1072				R	-180				180					1			度			16 S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.usMains_phase_rotation				=					(Arr_rece[N+3+(1074-1024)*2]<<8) + Arr_rece[N+4+(1074-1024)*2];																																							//市电相序顺序					1074				R	0						3													16
												g_Diesel_Generator.Value_Display.City_Electricity_Para.ssMains_current_lag_lead			=					(Arr_rece[N+3+(1075-1024)*2]<<8) + Arr_rece[N+4+(1075-1024)*2];																																							//电流滞后/超前					1075				R	-180				180					1			度			16 S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_L1_current						=((float)((Arr_rece[N+3+(1076-1024)*2]<<24)+(Arr_rece[N+4+(1076-1024)*2]<<16) + (Arr_rece[N+5+(1076-1024)*2]<<8) + Arr_rece[N+6+(1076-1024)*2]))*0.1f;//L1相电流							1076和1077	R	0						99999.9			0.1		A				32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_L2_current						=((float)((Arr_rece[N+3+(1078-1024)*2]<<24)+(Arr_rece[N+4+(1078-1024)*2]<<16) + (Arr_rece[N+5+(1078-1024)*2]<<8) + Arr_rece[N+6+(1078-1024)*2]))*0.1f;//L2相电流							1078和1079	R	0						99999.9			0.1		A				32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_L3_current						=((float)((Arr_rece[N+3+(1080-1024)*2]<<24)+(Arr_rece[N+4+(1080-1024)*2]<<16) + (Arr_rece[N+5+(1080-1024)*2]<<8) + Arr_rece[N+6+(1080-1024)*2]))*0.1f;//L3相电流							1080和1081	R	0						99999.9			0.1		A				32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_earth_current					=((float)((Arr_rece[N+3+(1082-1024)*2]<<24)+(Arr_rece[N+4+(1082-1024)*2]<<16) + (Arr_rece[N+5+(1082-1024)*2]<<8) + Arr_rece[N+6+(1082-1024)*2]))*0.1f;//接地电流							1082和1083	R	0						99999.9			0.1		A				32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.slMains_L1_watts							=					(Arr_rece[N+3+(1084-1024)*2]<<24)+(Arr_rece[N+4+(1084-1024)*2]<<16) + (Arr_rece[N+5+(1084-1024)*2]<<8) + Arr_rece[N+6+(1084-1024)*2];				//L1相有功功率					1084和1085	R	-99999999		99999999		1			W				32 S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.slMains_L2_watts							=					(Arr_rece[N+3+(1086-1024)*2]<<24)+(Arr_rece[N+4+(1086-1024)*2]<<16) + (Arr_rece[N+5+(1086-1024)*2]<<8) + Arr_rece[N+6+(1086-1024)*2];				//L2相有功功率					1086和1087	R	-99999999		99999999		1			W				32 S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.slMains_L3_watts							=					(Arr_rece[N+3+(1088-1024)*2]<<24)+(Arr_rece[N+4+(1088-1024)*2]<<16) + (Arr_rece[N+5+(1088-1024)*2]<<8) + Arr_rece[N+6+(1088-1024)*2];				//L3相有功功率					1088和1089	R	-99999999		99999999		1			W				32 S
#endif												
												g_Diesel_Generator.Mask |=1;
												g_Result_Status	= 1;
											}
											else if(D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_1536_1581_41==mode)
											{
												g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_total_watts							=					(Arr_rece[N+3+(1536-1536)*2]<<24)+(Arr_rece[N+4+(1536-1536)*2]<<16) + (Arr_rece[N+5+(1536-1536)*2]<<8) + Arr_rece[N+6+(1536-1536)*2];				//总的有功功率				1536和1537	R	-99,999,999	99,999,999	1			W				32S
												g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_L1_VA										=					(Arr_rece[N+3+(1538-1536)*2]<<24)+(Arr_rece[N+4+(1538-1536)*2]<<16) + (Arr_rece[N+5+(1538-1536)*2]<<8) + Arr_rece[N+6+(1538-1536)*2];				//L1相视在功率				1538和1539	R	0						99,999,999	1			VA			32
												g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_L2_VA										=					(Arr_rece[N+3+(1540-1536)*2]<<24)+(Arr_rece[N+4+(1540-1536)*2]<<16) + (Arr_rece[N+5+(1540-1536)*2]<<8) + Arr_rece[N+6+(1540-1536)*2];				//L2相视在功率				1540和1541	R	0						99,999,999	1			VA			32
												g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_L3_VA										=					(Arr_rece[N+3+(1542-1536)*2]<<24)+(Arr_rece[N+4+(1542-1536)*2]<<16) + (Arr_rece[N+5+(1542-1536)*2]<<8) + Arr_rece[N+6+(1542-1536)*2];				//L3相视在功率				1542和1543	R	0						99,999,999	1			VA			32
												g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_total_VA								=					(Arr_rece[N+3+(1544-1536)*2]<<24)+(Arr_rece[N+4+(1544-1536)*2]<<16) + (Arr_rece[N+5+(1544-1536)*2]<<8) + Arr_rece[N+6+(1544-1536)*2];				//总视在功率					1544和1545	R	0						99,999,999	1			VA			32S
												g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L1_Var									=					(Arr_rece[N+3+(1546-1536)*2]<<24)+(Arr_rece[N+4+(1546-1536)*2]<<16) + (Arr_rece[N+5+(1546-1536)*2]<<8) + Arr_rece[N+6+(1546-1536)*2];				//L1相无功功率				1546和1547	R	-99,999,999	99,999,999	1			Var			32S
												g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L2_Var									=					(Arr_rece[N+3+(1548-1536)*2]<<24)+(Arr_rece[N+4+(1548-1536)*2]<<16) + (Arr_rece[N+5+(1548-1536)*2]<<8) + Arr_rece[N+6+(1548-1536)*2];				//L2相无功功率				1548和1549	R	-99,999,999	99,999,999	1			Var			32S
												g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L3_Var									=					(Arr_rece[N+3+(1550-1536)*2]<<24)+(Arr_rece[N+4+(1550-1536)*2]<<16) + (Arr_rece[N+5+(1550-1536)*2]<<8) + Arr_rece[N+6+(1550-1536)*2];				//L3相无功功率				1550和1551	R	-99,999,999	99,999,999	1			Var			32S
												g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_total_Var								=					(Arr_rece[N+3+(1552-1536)*2]<<24)+(Arr_rece[N+4+(1552-1536)*2]<<16) + (Arr_rece[N+5+(1552-1536)*2]<<8) + Arr_rece[N+6+(1552-1536)*2];				//总无功功率					1552和1553	R	-99,999,999	99,999,999	1	   	Var			32S
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_power_factor_L1					=((float)((Arr_rece[N+3+(1554-1536)*2]<<8) + Arr_rece[N+4+(1554-1536)*2]))*0.01f;																																			//L1相功率因素				1554				R	-1	        1	          0.01					16S
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_power_factor_L2					=((float)((Arr_rece[N+3+(1555-1536)*2]<<8) + Arr_rece[N+4+(1555-1536)*2]))*0.01f;																																			//L2相功率因素				1555				R	-1					1						0.01					16S
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_power_factor_L3					=((float)((Arr_rece[N+3+(1556-1536)*2]<<8) + Arr_rece[N+4+(1556-1536)*2]))*0.01f;																																			//L3相功率因素				1556				R	-1					1						0.01					16S
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_average_power_factor			=((float)((Arr_rece[N+3+(1557-1536)*2]<<8) + Arr_rece[N+4+(1557-1536)*2]))*0.01f;																																			//平均功率因素				1557				R	-1					1						0.01					16S
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_percentage_of_full_power	=((float)((Arr_rece[N+3+(1558-1536)*2]<<8) + Arr_rece[N+4+(1558-1536)*2]))*0.1f;																																			//总功率的百分比			1558				R	-999.9			999.9				0.1		%				16S
												g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_percentage_of_full_Var		=((float)((Arr_rece[N+3+(1559-1536)*2]<<8) + Arr_rece[N+4+(1559-1536)*2]))*0.1f;																																			//总无功功率的百分比	1559				R	-999.9			999.9				0.1		%				16S
												#if 0
												g_Diesel_Generator.Value_Display.City_Electricity_Para.slMains_total_watts					=					(Arr_rece[N+3+(1560-1536)*2]<<24)+(Arr_rece[N+4+(1560-1536)*2]<<16) + (Arr_rece[N+5+(1560-1536)*2]<<8) + Arr_rece[N+6+(1560-1536)*2];				//市电总有功功率			1560和1561	R	-99999999		999999999		1			W				32S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.ulMains_L1_VA								=					(Arr_rece[N+3+(1562-1536)*2]<<24)+(Arr_rece[N+4+(1562-1536)*2]<<16) + (Arr_rece[N+5+(1562-1536)*2]<<8) + Arr_rece[N+6+(1562-1536)*2];				//L1相视在功率				1562和1563	R	0						99999999		1			VA			32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.ulMains_L2_VA								=					(Arr_rece[N+3+(1564-1536)*2]<<24)+(Arr_rece[N+4+(1564-1536)*2]<<16) + (Arr_rece[N+5+(1564-1536)*2]<<8) + Arr_rece[N+6+(1564-1536)*2];				//L2相视在功率				1564和1565	R	0						99999999		1			VA			32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.ulMains_L3_VA								=					(Arr_rece[N+3+(1566-1536)*2]<<24)+(Arr_rece[N+4+(1566-1536)*2]<<16) + (Arr_rece[N+5+(1566-1536)*2]<<8) + Arr_rece[N+6+(1566-1536)*2];				//L3相视在功率				1566和1567	R	0						99999999		1			VA			32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.ulMains_total_VA							=					(Arr_rece[N+3+(1568-1536)*2]<<24)+(Arr_rece[N+4+(1568-1536)*2]<<16) + (Arr_rece[N+5+(1568-1536)*2]<<8) + Arr_rece[N+6+(1568-1536)*2];				//总视在功率					1568和1569	R	0						999999999		1			VA			32
												g_Diesel_Generator.Value_Display.City_Electricity_Para.slMains_L1_Var								=					(Arr_rece[N+3+(1570-1536)*2]<<24)+(Arr_rece[N+4+(1570-1536)*2]<<16) + (Arr_rece[N+5+(1570-1536)*2]<<8) + Arr_rece[N+6+(1570-1536)*2];				//L1相无功功率				1570和1571	R	-99999999		99999999		1			Var			32S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.slMains_L2_Var								=					(Arr_rece[N+3+(1572-1536)*2]<<24)+(Arr_rece[N+4+(1572-1536)*2]<<16) + (Arr_rece[N+5+(1572-1536)*2]<<8) + Arr_rece[N+6+(1572-1536)*2];				//L2相无功功率				1572和1573	R	-99999999		99999999		1			Var			32S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.slMains_L3_Var								=					(Arr_rece[N+3+(1574-1536)*2]<<24)+(Arr_rece[N+4+(1574-1536)*2]<<16) + (Arr_rece[N+5+(1574-1536)*2]<<8) + Arr_rece[N+6+(1574-1536)*2];				//L3相无功功率				1574和1575	R	-99999999		99999999		1			Var			32S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.slMains_total_Var						=					(Arr_rece[N+3+(1576-1536)*2]<<24)+(Arr_rece[N+4+(1576-1536)*2]<<16) + (Arr_rece[N+5+(1576-1536)*2]<<8) + Arr_rece[N+6+(1576-1536)*2];				//总无功功率					1576和1577	R	-999999999	999999999		1			Var			32S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_power_factor_L1				=((float)((Arr_rece[N+3+(1578-1536)*2]<<8) + Arr_rece[N+4+(1578-1536)*2]))*0.01f;																																			//L1相功率因素				1578				R	-1					1						0.01					16S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_power_factor_L2				=((float)((Arr_rece[N+3+(1579-1536)*2]<<8) + Arr_rece[N+4+(1579-1536)*2]))*0.01f;																																			//L2相功率因素				1579				R	-1					1						0.01					16S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_power_factor_L3				=((float)((Arr_rece[N+3+(1580-1536)*2]<<8) + Arr_rece[N+4+(1580-1536)*2]))*0.01f;																																			//L3相功率因素				1580				R	-1					1						0.01					16S
												g_Diesel_Generator.Value_Display.City_Electricity_Para.fMains_average_power_factor	=((float)((Arr_rece[N+3+(1581-1536)*2]<<8) + Arr_rece[N+4+(1581-1536)*2]))*0.01f;																																			//平均功率因素				1581				R	-1					1						0.01					16S
												#endif
												g_Diesel_Generator.Mask |=2;
												g_Result_Status	= 1;
											}
											else if(D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_1798_1809_42==mode)
											{
												g_Diesel_Generator.Value_Display.Engine_Para.ulEngine_run_time											=         (Arr_rece[N+3+(1798-1798)*2]<<24)+(Arr_rece[N+4+(1798-1798)*2]<<16) + (Arr_rece[N+5+(1798-1798)*2]<<8) + Arr_rece[N+6+(1798-1798)*2];				//发动机运行时间,			1798和1799	R	0	       		4.29 x109		1			秒			32
												g_Diesel_Generator.Value_Display.Engine_Para.ulNumber_of_starts											=         (Arr_rece[N+3+(1808-1798)*2]<<24)+(Arr_rece[N+4+(1808-1798)*2]<<16) + (Arr_rece[N+5+(1808-1798)*2]<<8) + Arr_rece[N+6+(1808-1798)*2];				//启动次数,						1808和1809	R	0	       		99999											32
												
												g_Diesel_Generator.Mask |=4;
												g_Result_Status	= 1;
											}
											else if(D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_39425_39442_43==mode)		// 报警状态
											{
												g_Diesel_Generator.Alarm.usReg_Addr_39425=(Arr_rece[N+3+(39425-39425)*2]<<8) + Arr_rece[N+4+(39425-39425)*2];//39425
												g_Diesel_Generator.Alarm.usReg_Addr_39426=(Arr_rece[N+3+(39426-39425)*2]<<8) + Arr_rece[N+4+(39426-39425)*2];//39426
												g_Diesel_Generator.Alarm.usReg_Addr_39427=(Arr_rece[N+3+(39427-39425)*2]<<8) + Arr_rece[N+4+(39427-39425)*2];//39427
												g_Diesel_Generator.Alarm.usReg_Addr_39428=(Arr_rece[N+3+(39428-39425)*2]<<8) + Arr_rece[N+4+(39428-39425)*2];//39428
												g_Diesel_Generator.Alarm.usReg_Addr_39429=(Arr_rece[N+3+(39429-39425)*2]<<8) + Arr_rece[N+4+(39429-39425)*2];//39429
												g_Diesel_Generator.Alarm.usReg_Addr_39430=(Arr_rece[N+3+(39430-39425)*2]<<8) + Arr_rece[N+4+(39430-39425)*2];//39430
												g_Diesel_Generator.Alarm.usReg_Addr_39431=(Arr_rece[N+3+(39431-39425)*2]<<8) + Arr_rece[N+4+(39431-39425)*2];//39431
												g_Diesel_Generator.Alarm.usReg_Addr_39432=(Arr_rece[N+3+(39432-39425)*2]<<8) + Arr_rece[N+4+(39432-39425)*2];//39432
												g_Diesel_Generator.Alarm.usReg_Addr_39433=(Arr_rece[N+3+(39433-39425)*2]<<8) + Arr_rece[N+4+(39433-39425)*2];//39433
												g_Diesel_Generator.Alarm.usReg_Addr_39434=(Arr_rece[N+3+(39434-39425)*2]<<8) + Arr_rece[N+4+(39434-39425)*2];//39434
												g_Diesel_Generator.Alarm.usReg_Addr_39435=(Arr_rece[N+3+(39435-39425)*2]<<8) + Arr_rece[N+4+(39435-39425)*2];//39435
												g_Diesel_Generator.Alarm.usReg_Addr_39436=(Arr_rece[N+3+(39436-39425)*2]<<8) + Arr_rece[N+4+(39436-39425)*2];//39436
												g_Diesel_Generator.Alarm.usReg_Addr_39442=(Arr_rece[N+3+(39442-39425)*2]<<8) + Arr_rece[N+4+(39442-39425)*2];//39442									
												
												g_Diesel_Generator.Mask |=8;
												g_Result_Status	= 1;
											}
											else if(D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_43873_44==mode)				//X  不用
											{
												g_Diesel_Generator.Value_Display.Engine_Para.usInternal_flexible_sender_A_analogue_input_reading=(Arr_rece[N+3]<<8) + Arr_rece[N+4];//43873
												
												g_Diesel_Generator.Mask |=0x10;
												g_Result_Status	= 1;
											}
											else if(D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_43520_43535_45==mode)		//X  io  input
											{
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_A_raw_status			=Arr_rece[N+4+(43520-43520)*2];//数字输入A状态			R	0	1	16	43520
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_A_processed_status=Arr_rece[N+4+(43521-43520)*2];//数字输入A处理状态	R	0	1	16  43521
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_B_raw_status			=Arr_rece[N+4+(43522-43520)*2];//数字输入B状态			R	0	1	16	43522
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_B_processed_status=Arr_rece[N+4+(43523-43520)*2];//数字输入B处理状态	R	0	1	16 	43523
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_C_raw_status			=Arr_rece[N+4+(43524-43520)*2];//数字输入C状态			R	0	1	16	43524
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_C_processed_status=Arr_rece[N+4+(43525-43520)*2];//数字输入C处理状态	R	0	1	16	43525
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_D_raw_status			=Arr_rece[N+4+(43526-43520)*2];//数字输入D状态			R	0	1	16	43526
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_D_processed_status=Arr_rece[N+4+(43527-43520)*2];//数字输入D处理状态	R	0	1	16	43527
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_E_raw_status			=Arr_rece[N+4+(43528-43520)*2];//数字输入E态				R	0	1	16	43528
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_E_processed_status=Arr_rece[N+4+(43529-43520)*2];//数字输入E处理状态	R	0	1	16	43529
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_F_raw_status			=Arr_rece[N+4+(43530-43520)*2];//数字输入F状态			R	0	1	16	43530
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_F_processed_status=Arr_rece[N+4+(43531-43520)*2];//数字输入F处理状态	R	0	1	16	43531
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_G_raw_status			=Arr_rece[N+4+(43532-43520)*2];//数字输入G状态			R	0	1	16	43532
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_G_processed_status=Arr_rece[N+4+(43533-43520)*2];//数字输入G处理状态	R	0	1	16 	43533
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_H_raw_status			=Arr_rece[N+4+(43534-43520)*2];//数字输入H状态			R	0	1	16	43534
												g_Diesel_Generator.IO_Point.Digital_Input.ucDigital_input_H_processed_status=Arr_rece[N+4+(43535-43520)*2];//数字输入H处理状态	R	0	1	16	43535
												
												g_Diesel_Generator.Mask |=0x20;
												g_Result_Status	= 1;
											}
											else if(D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_43925_43949_46==mode)		//X
											{
												g_Diesel_Generator.Value_Display.Engine_Para.usInternal_flexible_sender_B_analogue_input_reading=(Arr_rece[N+3+(43925-43925)*2]<<8) + Arr_rece[N+4+(43925-43925)*2];//43925
												g_Diesel_Generator.Value_Display.Engine_Para.usInternal_flexible_sender_C_analogue_input_reading=(Arr_rece[N+3+(43931-43925)*2]<<8) + Arr_rece[N+4+(43931-43925)*2];//43931
												g_Diesel_Generator.Value_Display.Engine_Para.usInternal_flexible_sender_D_analogue_input_reading=(Arr_rece[N+3+(43937-43925)*2]<<8) + Arr_rece[N+4+(43937-43925)*2];//43937
												g_Diesel_Generator.Value_Display.Engine_Para.usInternal_flexible_sender_E_analogue_input_reading=(Arr_rece[N+3+(43943-43925)*2]<<8) + Arr_rece[N+4+(43943-43925)*2];//43943
												g_Diesel_Generator.Value_Display.Engine_Para.usInternal_flexible_sender_F_analogue_input_reading=(Arr_rece[N+3+(43949-43925)*2]<<8) + Arr_rece[N+4+(43949-43925)*2];//43949
												
												g_Diesel_Generator.Mask |=0x40;
												g_Result_Status	= 1;
											}
											else if(D_REV_MODE_DIESEL_GENERATOR_REGISTER_ADDR_48640_48661_47==mode)	
											{
												g_Diesel_Generator.IO_Point.Digital_Output.ucFuel_Relay				=Arr_rece[N+4+(48640-48640)*2];//燃油输出							R	0	1	16	48640
												g_Diesel_Generator.IO_Point.Digital_Output.ucStart_Relay			=Arr_rece[N+4+(48641-48640)*2];//启动输出							R	0	1	16	48641
												g_Diesel_Generator.IO_Point.Digital_Output.ucDigital_Output_C	=Arr_rece[N+4+(48647-48640)*2];//自定义输出C（无源）	R	0	1	16	48647
												g_Diesel_Generator.IO_Point.Digital_Output.ucDigital_Output_D	=Arr_rece[N+4+(48646-48640)*2];//自定义输出D（无源）	R	0	1	16	48646
												g_Diesel_Generator.IO_Point.Digital_Output.ucDigital_Output_E	=Arr_rece[N+4+(48642-48640)*2];//自定义输出E					R	0	1	16	48642
												g_Diesel_Generator.IO_Point.Digital_Output.ucDigital_Output_F	=Arr_rece[N+4+(48643-48640)*2];//自定义输出F					R	0	1	16	48643
												g_Diesel_Generator.IO_Point.Digital_Output.ucDigital_Output_G	=Arr_rece[N+4+(48644-48640)*2];//自定义输出G					R	0	1	16	48644
												g_Diesel_Generator.IO_Point.Digital_Output.ucDigital_Output_H	=Arr_rece[N+4+(48645-48640)*2];//自定义输出H					R	0	1	16	48645
												g_Diesel_Generator.IO_Point.Digital_Output.ucDigital_Output_I	=Arr_rece[N+4+(48660-48640)*2];//自定义输出I					R	0	1	16	48660
												g_Diesel_Generator.IO_Point.Digital_Output.ucDigital_Output_J	=Arr_rece[N+4+(48661-48640)*2];//自定义输出J					R	0	1	16	48661
												
												g_Diesel_Generator.LED_Status.ucSTOP_LED_status								=Arr_rece[N+4+(48648-48640)*2];//停止状态							R	0	1	16	48648
												g_Diesel_Generator.LED_Status.ucMANUAL_LED_status							=Arr_rece[N+4+(48649-48640)*2];//手动状态							R	0	1	16	48649
												g_Diesel_Generator.LED_Status.ucTEST_LED_status								=Arr_rece[N+4+(48650-48640)*2];//测试状态							R	0	1	16	48650
												g_Diesel_Generator.LED_Status.ucAUTO_LED_status								=Arr_rece[N+4+(48651-48640)*2];//自动状态							R	0	1	16	48651
												g_Diesel_Generator.LED_Status.ucGEN_LED_status								=Arr_rece[N+4+(48655-48640)*2];//机组有效							R	0	1	16	48655
												g_Diesel_Generator.LED_Status.ucGEN_BREAKER_LED_status				=Arr_rece[N+4+(48654-48640)*2];//机组断路器合闸				R	0	1	16	48654
												g_Diesel_Generator.LED_Status.ucMAINS_LED_status							=Arr_rece[N+4+(48652-48640)*2];//市电有效 						R	0	1	16	48652
												g_Diesel_Generator.LED_Status.ucMAINS_BREAKER_LED_status			=Arr_rece[N+4+(48653-48640)*2];//市电断路器合闸 			R	0	1	16	48653
												g_Diesel_Generator.LED_Status.ucUSER_LED_1_status							=Arr_rece[N+4+(48656-48640)*2];//用户自定义LED1状态 	R	0	1	16	48656
												g_Diesel_Generator.LED_Status.ucUSER_LED_2_status							=Arr_rece[N+4+(48657-48640)*2];//用户自定义LED2状态		R	0	1	16	48657
												g_Diesel_Generator.LED_Status.ucUSER_LED_3_status							=Arr_rece[N+4+(48658-48640)*2];//用户自定义LED3状态		R	0	1	16	48658
												g_Diesel_Generator.LED_Status.ucUSER_LED_4_status							=Arr_rece[N+4+(48659-48640)*2];//用户自定义LED4状态		R	0	1	16	48659
												
												g_Diesel_Generator.Mask |=0x80;
												g_Result_Status	= 1;
											}
											
											
											
											if(D_REV_MODE_DBS_AIR_STATE_1==mode)				//空调状态位读取	uart_get_data(g_Air.modbus_id,0x0000,16,1,g_Air.ComPort);		
											{
													if(2==cnt)
													{
														i	=  (Arr_rece[N+3]<<8)|Arr_rece[N+4];				
//----------------3.状态数据 Coil Register  15  cmd=1---------------------------------
//	unsigned char	Internal_fan_state;							//0x0000内风机运行状态
//	unsigned char	External_fan_state;							//0x0002外风机运行状态
//	unsigned char	Cooing_state;										//0x0004制冷运行状态
//	unsigned char	Heating_state;									//0x0005制热运行状态
//	unsigned char	Self_check_state;								//0x000B自检状态
//	unsigned char	Machine_state;									//0x000C机组运行状态
//	unsigned char	heat_exchanger_state;						//0x000E换热状态
														g_Air.Internal_fan_state 	= (i>>0x00)&0x1;		
														g_Air.External_fan_state 	= (i>>0x02)&0x1;		
														g_Air.Cooing_state 				= (i>>0x04)&0x1;		
														g_Air.Heating_state 			= (i>>0x05)&0x1;		
														g_Air.Self_check_state 		= (i>>0x0B)&0x1;		
														g_Air.Machine_state 			= (i>>0x0C)&0x1;		
														g_Air.heat_exchanger_state= (i>>0x0E)&0x1;																		
														g_Air.Mask |=1;
														g_Result_Status	= 1;				
													}																										
													return;
											}		
											if(D_REV_MODE_DBS_AIR_ALARM_2==mode)					//空调报警位读取	uart_get_data(g_Air.modbus_id,0x0000,40,2,g_Air.ComPort);	
											{
													if(5==cnt)			
													{	
														i	=  (Arr_rece[N+6]<<8)|Arr_rece[N+7];								
															//----------------4.告警数据 Discrete Input Register  15 cmd=2----------------------
//	1  Internal_fan_alarm;								//0x0000内风机告警
//	2  External_fan_alarm;								//0x0002外风机告警 	
//	3  Compressor_fault;									//0x0004压缩机故障
//	4  Heater_overload_alarm;							//0x0008加热器过流告警						
//	5  Heater_underload_alarm;						//0x0009加热器欠流告警						
//	6  Return_air_temp_sensor_fault;			//0x000A回风温度传感器故障				
//	7  Condenser_temp_sensor_faul;				//0x000B冷凝盘管温度传感器故障		
//	8  High_DC_voltage_alarm; 						//0x000C直流高电压告警
//	9  Low_DC_voltage_alarm; 							//0x000D直流低电压告警
//	10 Inside_high_temp_alarm;						//0x000E柜内高温告警
//	11 Inside_low_temp_alarm;							//0x000F柜内低温告警
//	12 Ambient_temp_sensor_fault;					//0x0015柜外温度探头告警
//	13 High_humidity;											//0x0018高湿告警
//	14 Humidity_sensor_fault;							//0x001A湿度传感器故障
//	15 Filter_alarm;											//0x0023过滤网堵塞告警
														g_Air.Internal_fan_alarm 						= (i>>0x00)&0x1;		
														g_Air.External_fan_alarm 						= (i>>0x02)&0x1;		
														g_Air.Compressor_fault 							= (i>>0x04)&0x1;		
														g_Air.Heater_overload_alarm 				= (i>>0x08)&0x1;		
														g_Air.Heater_underload_alarm 				= (i>>0x09)&0x1;		
														g_Air.Return_air_temp_sensor_fault 	= (i>>0x0A)&0x1;		
														g_Air.Condenser_temp_sensor_faul		= (i>>0x0B)&0x1;															
														g_Air.Condenser_temp_sensor_faul		= (i>>0x0C)&0x1;															
														g_Air.Low_DC_voltage_alarm 					= (i>>0x0D)&0x1;		
														g_Air.Inside_high_temp_alarm 				= (i>>0x0E)&0x1;		
														g_Air.Inside_low_temp_alarm 				= (i>>0x0F)&0x1;		
														
														i	=  (Arr_rece[N+4]<<8)|Arr_rece[N+5];		
														g_Air.Ambient_temp_sensor_fault 		= (i>>0x05)&0x1;		
														g_Air.High_humidity 								= (i>>0x08)&0x1;		
														g_Air.Humidity_sensor_fault 				= (i>>0x0A)&0x1;		
														
														i	=  Arr_rece[N+3];		
														g_Air.Filter_alarm									= (i>>0x03)&0x1;																													
														
														g_Air.Mask |=2;
														g_Result_Status	= 1;				
													}													
													return;
											}			
											if(D_REV_MODE_DBS_AIR_SENSOR_4==mode)					//空调传感器读取	uart_get_data(g_Air.modbus_id,0x0000,15,4,g_Air.ComPort);
											{
													if(30==cnt)				
													{	
														g_Air.Internal_fan_speed		=  (Arr_rece[N+3+2*0]<<8)|Arr_rece[N+4+2*0];								
														g_Air.External_fan_speed		=  (Arr_rece[N+3+2*1]<<8)|Arr_rece[N+4+2*1];								
														g_Air.Return_air_temp				=  (Arr_rece[N+3+2*4]<<8)|Arr_rece[N+4+2*4];								
														g_Air.Ambient_temp					=  (Arr_rece[N+3+2*5]<<8)|Arr_rece[N+4+2*5];								
														g_Air.DC_power_voltage			=  (Arr_rece[N+3+2*6]<<8)|Arr_rece[N+4+2*6];								
														g_Air.Heater_current				=  (Arr_rece[N+3+2*0x0a]<<8)|Arr_rece[N+4+2*0x0a];								
														g_Air.Condenser_temp				=  (Arr_rece[N+3+2*0x0d]<<8)|Arr_rece[N+4+2*0x0d];								
														g_Air.Return_air_humidity		=  (Arr_rece[N+3+2*0x0e]<<8)|Arr_rece[N+4+2*0x0e];								
//-----------------5.传感器数据 Input Register 8	cmd=4-----------------------------
//	unsigned short int Internal_fan_speed;									//0内风机转速
//	unsigned short int External_fan_speed;									//2外风机转速
//	s16	Return_air_temp;										//4回风温度					/10.0
//	s16	Ambient_temp;												//5柜外温度					/10.0
//	unsigned short int DC_power_voltage;										//6直流电源电压			/10.0				 
//	unsigned short int Heater_current;											//A加热器电流				/100.0
//	s16 Condenser_temp;											//D冷凝盘管温度			/10.0
//	s16 Return_air_humidity;								//E回风湿度					/10.0			
														g_Air.Mask |=4;														
														g_Result_Status	= 1;				
													}													
													return;
											}			
											if(D_REV_MODE_DBS_AIR_PARAMETER_3==mode)				//空调参数读取	uart_get_data(g_Air.modbus_id,0x0016,36,3,g_Air.ComPort);
											{
													if(72==cnt)		
													{	
														
//------------6.系统参数 Holding Register 10  cmd=3--------------------------------
//	s16 Heater_starting_temperature;													//0x0016制热启动温度		/10.0
//	s16 Heater_stop_return_difference_temperature;						//0x0017制热停止回差值		/10.0
//	s16 inside_high_temperature_limit;												//0x001C高温告警温度值		/10.0
//	s16 inside_low_temperature_limit;													//0x001D低温告警温度值		/10.0
//	unsigned short int High_DC_voltage_alarm_value;													//0x0026直流电压告警高限		/10.0
//	unsigned short int Low_DC_voltage_alarm_value;														//0x0027直流电压告警低限		/10.0
//	s16 Compressor_starting_temperature;											//0x0028制冷启动温度		/10.0
//	s16 Compressor_stop_return_difference_temperature;				//0x0029制冷停止回差值		/10.0
//	s16 Heat_exchanger_starting_temperature;									//0x0038换热启动温度		/10.0
//	s16 Heat_exchanger_stop_return_difference_temperature;		//0x0039换热停止回差值		/10.0		
														g_Air.Heater_starting_temperature												=  (Arr_rece[N+3+2*(0x16-0x16)]<<8)|Arr_rece[N+4+2*(0x16-0x16)];								
														g_Air.Heater_stop_return_difference_temperature					=  (Arr_rece[N+3+2*(0x17-0x16)]<<8)|Arr_rece[N+4+2*(0x17-0x16)];							
														g_Air.inside_high_temperature_limit											=  (Arr_rece[N+3+2*(0x1c-0x16)]<<8)|Arr_rece[N+4+2*(0x1c-0x16)];														
														g_Air.inside_low_temperature_limit											=  (Arr_rece[N+3+2*(0x1d-0x16)]<<8)|Arr_rece[N+4+2*(0x1d-0x16)];															
														g_Air.High_DC_voltage_alarm_value												=  (Arr_rece[N+3+2*(0x26-0x16)]<<8)|Arr_rece[N+4+2*(0x26-0x16)];													
														g_Air.Low_DC_voltage_alarm_value												=  (Arr_rece[N+3+2*(0x27-0x16)]<<8)|Arr_rece[N+4+2*(0x27-0x16)];											
														g_Air.Compressor_starting_temperature										=  (Arr_rece[N+3+2*(0x28-0x16)]<<8)|Arr_rece[N+4+2*(0x28-0x16)];														
														g_Air.Compressor_stop_return_difference_temperature			=  (Arr_rece[N+3+2*(0x29-0x16)]<<8)|Arr_rece[N+4+2*(0x29-0x16)];																									
														g_Air.Heat_exchanger_starting_temperature								=  (Arr_rece[N+3+2*(0x38-0x16)]<<8)|Arr_rece[N+4+2*(0x38-0x16)];													
														g_Air.Heat_exchanger_stop_return_difference_temperature	=  (Arr_rece[N+3+2*(0x39-0x16)]<<8)|Arr_rece[N+4+2*(0x39-0x16)];											
														g_Air.Mask |=8;
														g_Result_Status	= 1;				
													}													
													return;
											}		
											//-------------------- 逆变器 ------------------------------------------------------------------------------------------------
											if(D_REV_MODE_INVERTER_RAM_AREA_20==mode)
											{
//0  1  2  3  4  5  6  7  8  9
//01 03 C0 27 0F 00 00 00 00 27 10 12 C0 04 F6 00 32 00 00 13 3C 00 05 00 00 00 00 00 1D 00 00 00 20 00 C2 00 02 00 00 00 00 00 00 00 00 00 00 00 00 05 1D 01 E4 01 48 00 03 00 C8 00 00 00 00 00 00 00 00 13 3C 13 3F 00 00 01 0F 00 00 00 FD 00 00 72 55 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 25 5D 4D 04 20 00 00 01 F6 C0 A8 01 FD FF FF FF 00 C0 A8 01 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03 00 00 00 FD 00 00 C0 A8 01 C3 13 88 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 91 62
												if(192==cnt)			//96*2
												{
													g_Inverter.usPower_Rated                       =((((unsigned short int)Arr_rece[3+0x0003*2])<<8) | Arr_rece[4+0x0003*2])/10;//原数放大了10倍
												
													g_Inverter.usVoltage_Nominal_DC_Input          =((((unsigned short int)Arr_rece[3+0x0004*2])<<8) | Arr_rece[4+0x0004*2])/100;//原数放大了100倍
												
													g_Inverter.usVoltage_Nominal_AC_Output         =((((unsigned short int)Arr_rece[3+0x0005*2])<<8) | Arr_rece[4+0x0005*2])/10;//原数放大了10倍
												
													g_Inverter.usFrequency_Nominal_AC_Output       = (((unsigned short int)Arr_rece[3+0x0006*2])<<8) | Arr_rece[4+0x0006*2];
												
													g_Inverter.fVoltage_BAT                        =((((unsigned short int)Arr_rece[3+0x0008*2])<<8) | Arr_rece[4+0x0008*2])/100.0;//原数放大了100倍
												
													g_Inverter.fCurrent_DC                         =((((unsigned short int)Arr_rece[3+0x0009*2])<<8) | Arr_rece[4+0x0009*2])/100.0;//原数放大了100倍
												
													g_Inverter.fVoltage_AC                         =((((unsigned short int)Arr_rece[3+0x000A*2])<<8) | Arr_rece[4+0x000A*2])/10.0;//原数放大了10倍;
													g_Result.fVoltage_AC = g_Inverter.fVoltage_AC;
												
													g_Inverter.fCurrent_AC                       	 =((((unsigned short int)Arr_rece[3+0x000B*2])<<8) | Arr_rece[4+0x000B*2])/100.0;//原数放大了100倍
													g_Result.fCurrent_AC = g_Inverter.fCurrent_AC;
													
													g_Inverter.usFault                             =(((unsigned short int)Arr_rece[3+0x0015*2])<<8) | Arr_rece[4+0x0015*2];
												
													g_Inverter.usAlarm                             =(((unsigned short int)Arr_rece[3+0x0016*2])<<8) | Arr_rece[4+0x0016*2];
												
													g_Inverter.ulTm_Accumulated_Work               =(((unsigned int)Arr_rece[3+0x0022*2])<<24) | (((unsigned int)Arr_rece[4+0x0022*2])<<16) | (((unsigned int)Arr_rece[5+0x0022*2])<<8) | Arr_rece[6+0x0022*2];
												
													g_Inverter.ulTotal_Load_Electricity_Consumption=(((unsigned int)Arr_rece[3+0x0051*2])<<24) | (((unsigned int)Arr_rece[4+0x0051*2])<<16) | (((unsigned int)Arr_rece[5+0x0051*2])<<8) | Arr_rece[6+0x0051*2];
												
													g_Inverter.usStatus_Run                        =(Arr_rece[3+0x005C*2]<<8) | Arr_rece[4+0x005C*2];			
													g_Inverter.Mask =1;
													g_Result_Status	= 1;	
												}		
												return;												
											}
//-------------------- 风机 ------------------------------------------------------------------------------------------------
											if(D_REV_MODE_FAN_RAM_AREA_30==mode)
											{
//01 03 64 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 E1 10 E2 10 E3 10 E4 10 E5 10 E6 10 E7 10 E8 00 00 00 01 00 00 00 01 00 00 00 01 00 00 00 01 00 0A 00 14 00 1E 00 28 00 01 00 02 00 01 00 02 00 01 00 01 00 01 00 02 00 03 00 04 00 05 00 06 00 07 00 08 00 08 00 07 00 06 00 05 00 04 00 03 00 02 00 01 79 55
												if(100==cnt)
												{
													for(i=0;i<8;i++)
													{
														g_Fan.usSpeed[i]=(((unsigned short int)Arr_rece[3+0x0008*2+2*i])<<8) | Arr_rece[4+0x0008*2+2*i];
														//usValue_Test=g_Fan.usSpeed[i];
													}
												
													for(i=0;i<8;i++)
													{
														g_Fan.ucState_Run[i]=Arr_rece[4+0x0010*2+2*i];
														//ucValue_Test[i]=g_Fan.ucState_Run[i];
													}
												
													for(i=0;i<4;i++)
													{
														g_Fan.ssT_RTS[i]=Arr_rece[4+0x0018*2+2*i];
														//scValue_Test[i]=g_Fan.ssT_RTS[i];
													}
												
													for(i=0;i<4;i++)
													{
														g_Fan.ucState_RTS_Connect[i]=Arr_rece[4+0x001C*2+2*i];
														//ucValue_Test[i]=g_Fan.ucState_RTS_Connect[i];
													}
											
													g_Fan.ucMode_Drive=Arr_rece[4+0x0020*2];
													//ucValue_Test[0]=g_Fan.ucMode_Drive;
												
													for(i=0;i<8;i++)
													{
														g_Fan.ucAlarm[i]=Arr_rece[4+0x0022*2+2*i];
														//ucValue_Test[i]=g_Fan.ucAlarm[i];
													}
												
													for(i=0;i<8;i++)
													{
														g_Fan.ucFault[i]=Arr_rece[4+0x002A*2+2*i];
														//ucValue_Test[i]=g_Fan.ucFault[i];
													}
													g_Fan.Mask =1;
													g_Result_Status	= 1;	
												}
												return;
											}
											return;
									 }							   
									 else
									 {
											N++;
									 }
								}
								else
								{
									N++;
								}
						}		
						else
						{
							N++;
						}
					}
			}//while
}	