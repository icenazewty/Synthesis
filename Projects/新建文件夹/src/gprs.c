// Gprs.cpp : implementation file
//
#include "stdio.h"
#include "prj.h"
#include "gd32e503v_eval.h"
#include "gprs.h"
#include "systick.h"
#include "string.h"
#include "GlobalVar.h"
#include "FreeRTOS.h"
#include "task.h"


#define 	true	1
#define 	false	0

//GPRS_INFO		m_gprsinfo;
//gsm_data	  m_gsm_queue[COUNT_MAX_GSM];
	int			   	m_gsm_wr,m_gsm_rd;		
	int					m_tel_wr,m_tel_rd;
unsigned int 	read_ext_data_tick = 0;		

extern unsigned int g_Send; 	
int  				g_connect_ErrCnt=0 ;			//���������Ӵ������
int					g_openCommTotal = 0 ;			//�򿪴���ʧ�ܴ���
int					g_iDiscardDataCnt = 0 ;		//�ϴ�����ֱ�Ӷ�����������

/////////////////////////////////////////////////////////////////////////////

/***********************************************
* �洢������Ϣ��Ϣ
***********************************************/

#define	CGREG	1
#define	CREG	2

 char AT_CMD[]   = {"AT\r\n"};						//����AT����
 char ATE_CMD[]  = {"ATE0\r\n"};					//�رջ���
 char AT_CMGD[]  = {"AT+CMGD=1,4\r\n"};		//ɾ�����ж���
 char AT_CIMI[]  = {"AT+CIMI\r\n"};				//IMSI	MC301��MG323
 
 #if MODULE_4G
 char AT_ICCID[] = {"AT+CICCID\r\n"};			//ICCID
 #else
 char AT_ICCID[] = {"AT^ICCID?\r\n"};			//ICCID
 #endif
 
 char AT_CSQ[]	 = {"AT+CSQ\r\n"};				//CSQ
 char AT_CGREG[] = {"AT+CGREG?\r\n"};		//gprs����ע��ɹ���
 char AT_CREG[]  = {"AT+CREG?\r\n"};			//gsm����ע��ɹ���
 char AT_COPS[]  = {"AT+COPS?\r\n"};			//��ѯ��Ӫ��
 char AT_CNUM[]  = {"AT+CNUM\r\n"};			//��ѯ�洢���ֻ����еı����ֻ�����
 
 char AT_CIPSENDMODE[]    = {"AT+CIPSENDMODE=1\r\n"};		//tcp client send ack应答模式
 char AT_CIPMODE[]  			= {"AT+CIPMODE=1\r\n"};				//tcp 透传模式
 char AT_NETOPEN[]  			= {"AT+NETOPEN\r\n"};					//连接网络,获取本地ip地址
 char AT_IPADDR[]  			  = {"AT+IPADDR\r\n"};					//连接网络或,获取本地ip地址
 

 
 char g_ComBuf[512];
 int  g_res=0;

extern unsigned int  					g_sysTick ;
extern unsigned int 					Timer2_Counter;         //Timer2��ʱ����������(ms) ��������0xffffffff���Զ���Ϊ0,
//extern void delay_1ms(unsigned int d);																								//��ʱ����(ms) ��ȫ������
//extern void GPRS_Reset(void);																											
//extern void RS232_USART1_Send_Data(unsigned char *send_buff,unsigned int length);	
//extern void RS2324_Send_Data(unsigned char *send_buff,unsigned int length);				

//extern volatile unsigned char Com.Usart[G4].RX_Buf[D_USART_REC_BUFF_SIZE] ;//ԃԚޓ˕˽ߝ
//extern volatile unsigned int  Com.Usart[G4].usRec_WR;//����rs232���ջ���дָ��
//extern volatile unsigned int  Com.Usart[G4].usRec_RD;//����rs232���ջ����ָ��


//��ӡ������Ϣ
void InsertLog(char *str)
{
	#if  GPRS_DEBUG		
			unsigned int len = 0;
			if(g_configRead.b_debug_work==1)
			{
				len = strlen(str);				
				Com_Send(USB,(unsigned char*)str,len);				//Com_Send(RS485,(unsigned char*)str,len);
			}		
	#endif
}

//ִ������Ľ��
//����0 ��ʾʧ�ܣ�����1��ʾ�õ�OK,����2��ʾ�õ�ERROR
int  CmdExe(char*cmd,int mode)
{
	char data[256];
	int success = 0;
	int delay_ms = 100;			//默认等待100*20ms即2秒时间，如果不成功，如果mode=2则等待30秒，OK=1  ERROR=2  0=timeout
	int i=0;
	int cmd_len = strlen(cmd);	
	int	dw  = 0;
//AT+CIPOPEN=0,"TCP","139.129.17.114",7107
	
	//Com.Usart[G4].RX_Buf[Com.Usart[G4].usRec_WR] = UART4->DR;//		
	//USART1->DR=Com.Usart[G4].RX_Buf[Com.Usart[G4].usRec_WR];
	//Com.Usart[G4].usRec_WR = (Com.Usart[G4].usRec_WR+1)%D_USART_REC_BUFF_SIZE;		
	
	Com.Usart[G4].usRec_RD = Com.Usart[G4].usRec_WR;		//Com.Usart[G4].usRec_WR;	
	Com_Send(G4,(unsigned char*)cmd,cmd_len); 	 	//WriteFile(hcom,cmd,cmd_len,&dw,NULL);		//��������	
	
	sprintf(data,"Cmd4G send: %s\r\n",cmd);
	InsertLog(data);
	
	g_res = 0;
	memset(g_ComBuf,0,sizeof(g_ComBuf));	
	if(2==mode)																	//���Ͷ�����ȴ�30��
	{
			delay_ms = 1500;	
	}
	else if(3==mode)			//执行at^siso=0命令
	{
			delay_ms = 150;			//最长等3秒
	}
	else if(4==mode)			//等待5秒
	{
			delay_ms = 250;
	}
	else if(5==mode)			//等待11秒
	{
			delay_ms = 550;
	}
	for(i=0;i<delay_ms&&g_res<512;i++)							//�ִ��2��,������512�ֽ����ݡ�
	{
		wdt();
		delay_1ms(20);																//等待30s最长
		dw = (Com.Usart[G4].usRec_WR - Com.Usart[G4].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;		
		if(dw>0)
		{
			while(Com.Usart[G4].usRec_RD!=Com.Usart[G4].usRec_WR)
			{
					g_ComBuf[g_res++] = Com.Usart[G4].RX_Buf[Com.Usart[G4].usRec_RD];	
					Com.Usart[G4].usRec_RD = (Com.Usart[G4].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
					g_res = g_res%512;
			}
			g_ComBuf[g_res] = '\0';
			wdt();
			//�Խ��յ������ݽ��д�����
			if(1==mode&&strstr(g_ComBuf,">"))
			{
				delay_1ms(10);
				dw = (Com.Usart[G4].usRec_WR - Com.Usart[G4].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;	
				if(dw>0)
				{
					//g_res+=dw;
				}
				success = 1;
				break;
			}
			else if(strstr(g_ComBuf,"OK"))		//ƥ��OK��ERROR
			{
				delay_1ms(10);
				dw = (Com.Usart[G4].usRec_WR - Com.Usart[G4].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;	
				if(dw>0)
				{
					//g_res+=dw;
				}
				success = 1;
				break;
			}		
			else if(strstr(g_ComBuf,"CONNECT"))		//ƥƤOKܲERROR
			{
				delay_1ms(10);
				dw = (Com.Usart[G4].usRec_WR - Com.Usart[G4].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;	
				if(dw>0)
				{
					//g_res+=dw;
				}
				success = 1;
				break;
			}				
			else if(strstr(g_ComBuf,"ERROR"))
			{
				delay_1ms(10);
				dw = (Com.Usart[G4].usRec_WR - Com.Usart[G4].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;	
				if(dw>0)
				{
					//g_res+=dw;
				}
				success = 2;
				break;
			}
		}
	}	
  wdt();
	if(g_res)		//������
	{	
			g_ComBuf[g_res] = '\0';		
		
			sprintf(data,"Cmd4G rec: %s\r\n",g_ComBuf);
			InsertLog(data);		
	}
	return success;
}

void GetICCID(void)
{
/*
^ICCID: 89860058031270314999

OK
��':'��ʼ��������20���ǿո��ַ�	g_res������	,g_ComBuf  */
	char	*p = strstr(g_ComBuf,":");
	char    *q = g_ComBuf;
	char	data[21];
	int		start = 0 , cnt = 0, i = 0;
	
	if(p)							//����ҵ���
	{
		p++;				//����һ���ַ�
		cnt = p - q;	
		cnt = g_res - cnt ;	//ʣ���ַ�����
		while(*p&&cnt)	//����ַ���Ϊ��
		{
			cnt--;
			if(*p!=' ')
			{
				start = 1;
			}
			if(start)
			{
				data[i]	= *p;
				i++;
				if(i>19)
				{
					break;
				}
			}
			p++;
		}//end while
		//if(20==i)
		{
			data[i] = '\0';						
			memcpy(m_siminfo.iccid,data,i);		
			m_siminfo.iccid[i]='\0';			
		}
	}	
}

void GetIMSI1(void)
{
/*
460024012948674

OK
Փ':'ߪʼ۳ĦlѸ20ٶ؇ࠕٱؖػ	g_resٶ˽ߝ	,g_ComBuf  */
	char	*p = strstr(g_ComBuf,":");
	char    *q = g_ComBuf;
	char	data[21];
	int		start = 0 , cnt = 0, i = 0;
	
	if(p)							//ɧڻ֒ս
	{
		p++;				//вЂһٶؖػ
		cnt = p - q;	
		cnt = g_res - cnt ;	//ʣԠؖػ˽
		while(*p&&cnt)	//ɧڻؖػһΪࠕ
		{
			cnt--;
			if(*p!=' ')
			{
				start = 1;
			}
			if(start)
			{
				data[i]	= *p;
				i++;
				if(i>19)
				{
					break;
				}
			}
			p++;
		}//end while
		//if(20==i)
		{
			data[i] = '\0';						
			memcpy(m_siminfo.iccid,data,i);		
			m_siminfo.iccid[i]='\0';			
		}
	}	
}

void GetIMSI(void)
{
/*
460024012948674

OK
Փ':'ߪʼ۳ĦlѸ20ٶ؇ࠕٱؖػ	g_resٶ˽ߝ	,g_ComBuf  */
	char		*p = strstr(g_ComBuf,"OK");
	char    *q = g_ComBuf;	
	if(p)						
	{
		p = strstr(g_ComBuf,"\r\n");
		if(p)
		{
			q = strstr(p+2,"\r\n");	
			if(q)
			{				
				if((p - q -2)<16)		//460014651507311
				{
					*q = '\0';
					strcpy(m_siminfo.imsi,p+2);		
				}
			}
		}			
	}	
}

void GetCSQ(void)
{
/*
+CSQ: 18,99

OK
��':'��ʼ����ǿո��ַ���,֮ǰ������g_res������	,g_ComBuf  */
	char	*start = strstr(g_ComBuf,":");
	char	*end = strstr(g_ComBuf,",");
	int		data = 0;
	int		success = 0 , cnt = 0;
	if(start&&end&&start<end)							//����ҵ���
	{
		start++;					//����һ���ַ�
		cnt = end - start;	
		while(*start&&cnt)			//����ַ���Ϊ��
		{
			cnt--;
			if(*start>0x2F&&*start<0x3A)
			{
				data = data*10+(*start-0x30);
				success = 1;
			}							
			start++;
		}//end while		
	}	
	if(success)
	{
		m_siminfo.singal=data&0xff;	
	}
}

void GetREG(int ch)
{
/*
+CGREG: 0,1

OK
��':'��ʼ����ǿո��ַ���,֮ǰ������g_res������	,g_ComBuf  */
	char	*start = strstr(g_ComBuf,":");
	char	*end = strstr(g_ComBuf,",");
	int		data = 0, data1 = 0;
	int		success = 0 , cnt = 0 , success1 = 0;
	if(start&&end&&start<end)							//����ҵ���
	{
		start++;					//����һ���ַ�
		cnt = end - start;	
		while(*start&&cnt)			//����ַ���Ϊ��
		{
			cnt--;
			if(*start>0x2F&&*start<0x3A)
			{
				data = data*10+(*start-0x30);
				success = 1;
			}							
			start++;
		}//end while	
		end++;
		if(*end>0x2F&&*end<0x3A)
		{
			success1 = 1;
			data1 = *end-0x30;
		}
	}
 

	if(success)
	{
		//Ӧ��Ϊ0�������Բ��жϡ�	
	}

	if(success1)
	{
		if(1==data1||5==data1)
		{
			if(ch==CGREG)
			{
				m_siminfo.bRegNet=true;	
			}
			else if(ch==CREG)
			{
				m_siminfo.bRegGSM=true;	
			}
		}
		else if(data1==2 && ch==CGREG)			//测试情况，尝试拨号，看看什么情况。
		{
				m_siminfo.bRegNet=true;	
		}
		else
		{
			if(ch==CGREG)
			{
				m_siminfo.bRegNet=false;	
			}
			else if(ch==CREG)
			{
				m_siminfo.bRegGSM=false;	
			}			
		}
	}	
}

void GetCOPS()
{
/*
+CGREG: 0,1

OK
��':'��ʼ����ǿո��ַ���,֮ǰ������g_res������	,g_ComBuf  */
	if(strstr(g_ComBuf,"OK") && (strstr(g_ComBuf,"CHN-CUGSM")||strstr(g_ComBuf,"China Unicom")))					//China Unicom   China Unicom
	{
			m_siminfo.service=1;
	}
	else if(strstr(g_ComBuf,"OK") && (strstr(g_ComBuf,"CHINA MOBILE")||strstr(g_ComBuf,"China Mobile")))	//China Mobile
	{
			m_siminfo.service=2;
	}
	else
	{
			m_siminfo.service=0;
	}
}

unsigned char  GetCSCA()
{/*
+CSCA: "+8613800311500",145

OK
*/
	char	*start = strstr(g_ComBuf,"\"");
	char	*end;
	char	data[128];
	int		i = 0 , cnt = 0 ;
	if(start)		
	{
		start++;	//ָ�� +
		end = strstr(start,"\"");
		if(end)
		{
			cnt = end - start;			
			i = 0;
			while(start&&cnt)
			{
				data[i]  = *start;
				i++;
				cnt--;
				start++;
			}	//data�д����i������Ϊ�������ĵ绰����
			if('+'==data[0])		//if(14==i&&'+'==data[0]&&'8'==data[1]&&'6'==data[2]&&'1'==data[3])
			{
				for(cnt = 1; cnt < i ; cnt++)
				{
					m_siminfo.strCSCA[cnt-1] = data[cnt];			//ȥ��+ �����������ŵĶ������ĵ绰����
				}
				data[cnt] = '\0';
				return true;
			}			
		}		
	}	
	return false;
}

void ResetGPRS()
{
	//Ӳ����λ	
	InsertLog("Reset gprs start ...!\r\n");
	GPRS_Reset();	
	read_ext_data_tick = g_sysTick;		
	InsertLog("Reset gprs end!\r\n");	
}

/*********************************************************
* ��������int CheckMG323Status(int* nCOM,int* baund)
* ����  ������֮ǰ���GPRSģ���Ƿ���Խ�����������
* ����  ��
					 int  nCOM,             ���ں�
					 int  baund,		    ������
					 int* singalOut         ģ�鷵�ص��ź��� 0~31
* ����ֵ��>=0��ʾGPRSģ�����������Խ��в��� <0 ��ʾģ�����Ĵ�����
*********************************************************/
int CheckMG323Status(void)
{
//	DCB		commParam;
	int		res = 0;
	unsigned int	dw=0;
	
	m_siminfo.bComm=false;			//与模块是否可以通讯即CPU向mg301发命令是否有回应。 
	m_siminfo.singal=0;
	m_siminfo.bExist=false;			//手机卡是否存在
	m_siminfo.service=0;				//ԵʼۯδݬӢսՋӪʌ	
	m_siminfo.bRegGSM=false;	
	m_siminfo.bRegNet=false;	
	
	//�߱��Զ�����ϵͳ��ģ�������
	//˵��,�����ж��ɹ�����߱�����,�����κβ�������û������ġ������ڵ�����˵���ж��豸������������ġ�
	//ģ���Ƿ����ͨѶ->�ֻ����Ƿ����->�ź�����->ע������		�ϼ�4������
	
	
		//**************************1  ģ���Ƿ����ͨѶ  bComm  CommFailCnt ***********************************
		if(!CmdExe(AT_CMD,0))							//AT����		�ж�ģ���Ƿ��������ҿ���ͨѶ			
		{
			if(m_siminfo.CommFailCnt<1)			//0��1
			{
					m_siminfo.FailStartTime = g_sysTick;		//失败开始时间，第1次
			}
			m_siminfo.CommFailCnt++;				//ͨѶʧ�ܵĴ���			
			if(m_siminfo.CommFailCnt>9 && (g_sysTick-m_siminfo.FailStartTime)>900)					//连续20次通讯失败，则至少过去了15min    1次至少需要1.5min
			{
					m_siminfo.CommFailCnt = 0;	
					m_siminfo.FailStartTime = g_sysTick;					//ͨѶʧ�ܿ�ʼʱ��						
					m_siminfo.gsm_reset_reson = 2;								//gprsģʽ,20ՎӔʏһŜͨѶҢȒӬڽ60ī						
					InsertLog("ResetGPRS:AT_CMD faile & cnt > 9 & timeout > 15min \r\n");
					ResetGPRS();							
			}
			return -10;  
		}
		m_siminfo.bComm=true;						//ͨѶ�ɹ�
		m_siminfo.CommFailCnt=0;				//ͨѶʧ�ܵĴ���
		if(!CmdExe(ATE_CMD,0))					//�رջ���		�ж�ģ���Ƿ��������ҿ���ͨѶ
		{
			InsertLog("ATE command failed!\r\n");				
		}
		
		if(CmdExe(AT_CSQ,0)==1)					//хۅ׊			Ɛ׏хۅǿ׈ɧێ
		{
				GetCSQ();										//ܱȡхۅǿ׈			
				if(m_siminfo.singal<10)			//хۅǿ׈Ɛ׏
				{
								
				}			
		}
		else
		{
				InsertLog("AT_CSQ command failed!\r\n");	
				return -12;
		}
		
		//**************************2  �ֻ����Ƿ����  bExist  iccid   phonenum   *****************************	
		if(CmdExe(AT_CIMI,0)==1)			//IMSI			�ж��ֻ����Ƿ����
		{
				GetIMSI();
				m_siminfo.bExist=true;	
		}
		else
		{
				InsertLog("AT_CIMI command failed!\r\n");	
				return -11;
		}
		if(CmdExe(AT_ICCID,0)==1)		//iccid				�ж��ֻ���iccid mg323��֧��
		{
				GetICCID();							//��iccid����	��':'��ʼ��������20���ǿո��ַ�
		}
		else
		{
				InsertLog("AT_ICCID command failed!\r\n");		
		}
		//if(CmdExe(AT_CNUM,0)==1)			//�����ֻ���
		//{
			//m_siminfo.phonenum;	
		//}
		//else
		//{
		//	InsertLog("CNUM����ִ��ʧ��!\r\n");				
		//}		
		//**************************3 �ź�����  singal      *****************************	
		
		if(CmdExe(AT_COPS,0)==1)	//gprsθçעӡԉ٦	Ɛ׏θçˇرעӡԉ٦
		{
				GetCOPS();					//Ɛ׏ˇرӑޭעӡԉ٦θç,ɧڻθçעӡԉ٦ìղƐ׏ؾϱǷʌ			
		}
		else
		{
				InsertLog("AT_COPS command failed!\r\n");				
		}	
		
		//**************************4 ע������ bRegGSM  service bRegNet  ********************	
		if(CmdExe(AT_CREG,0)==1)	//gsm����ע��ɹ�		�ж������Ƿ�ע��ɹ�
		{
			GetREG(CREG);				//�ж��Ƿ��Ѿ�ע��ɹ�������,�������ע��ɹ������жϷ�������
			if(m_siminfo.bRegGSM)
			{		
					InsertLog("Reg Gsm success!\r\n");				
			}
			else
			{
				InsertLog("Reg Gsm failed!\r\n");	
				return -13;
			}
		}
		else
		{
				InsertLog("AT_CREG command failed!\r\n");	
				return -14;
		}	
		
		if(CmdExe(AT_CGREG,0)==1)	//gprs����ע��ɹ�	�ж������Ƿ�ע��ɹ�
		{
			GetREG(CGREG);				//�ж��Ƿ��Ѿ�ע��ɹ�������,�������ע��ɹ������жϷ�������
			if(m_siminfo.bRegNet)
			{
				InsertLog("Reg gprs success!\r\n");
			}
			else
			{
				InsertLog("Reg gprs failed!\r\n");
				return -1;
			}
		}
		else
		{
				InsertLog("AT_CGREG command failed!\r\n");	
				return -2;
		}	
		
		if(CmdExe(AT_CIPSENDMODE,0)==1)			//tcp client send ack应答模式
		{
			
		}
		else
		{
				InsertLog("AT_CIPSENDMODE command failed!\r\n");	
			//	return -2;
		}		
		
		if(CmdExe(AT_CIPMODE,0)==1)				//tcp 透传模式
		{
			
		}
		else
		{
				InsertLog("AT_CIPMODE command failed!\r\n");	
			//	return -2;
		}		
		
		if(CmdExe(AT_NETOPEN,0)==1)				//连接网络,获取本地ip地址
		{
			
		}
		else
		{
				InsertLog("AT_NETOPEN command failed!\r\n");	
				//return -2;
		}		
		
		if(CmdExe(AT_IPADDR,0)==1)					//连接网络后,获取本地ip地址
		{
			
		} 
		return 0;  
}

//�������GPRS״̬
//����ֵ��0��ģ������
int GPRS_Soft_OK()
{
	int res=0;
	res = CheckMG323Status();
	return res;
}

//_______________________�����йصĲ���________________________________________________________________________________________
char AT_CMGF[]  = {"AT+CMGF=0\r\n"};						//PDU����ģʽ
char AT_CNMI[]  = {"AT+CNMI=3,0,0,1,0\r\n"};		//���ö��Ŵ洢��ʽ��ע��mg301��mg323
char AT_CSCA[]  = {"AT+CSCA?\r\n"};							//�������ĵ绰����
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// �ɴ�ӡ�ַ���ת��Ϊ�ֽ�����
// �磺"C8329BFD0E01" --> {0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01}
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ������ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ�����ݳ���
int gsmString2Bytes(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	int i = 0;
	for( i=0; i<nSrcLength; i+=2)
	{
		// �����4λ
		if(*pSrc>='0' && *pSrc<='9')
		{
			*pDst = (*pSrc - '0') << 4;
		}
		else
		{
			*pDst = (*pSrc - 'A' + 10) << 4;
		}

		pSrc++;

		// �����4λ
		if(*pSrc>='0' && *pSrc<='9')
		{
			*pDst |= *pSrc - '0';
		}
		else
		{
			*pDst |= *pSrc - 'A' + 10;
		}

		pSrc++;
		pDst++;
	}

	// ����Ŀ�����ݳ���
	return nSrcLength / 2;
}

// �ֽ�����ת��Ϊ�ɴ�ӡ�ַ���
// �磺{0xC8, 0x32, 0x9B, 0xFD, 0x0E, 0x01} --> "C8329BFD0E01" 
// pSrc: Դ����ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ���ݳ���
// ����: Ŀ���ַ�������
int gsmBytes2String(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
	const char tab[]="0123456789ABCDEF";	// 0x0-0xf���ַ����ұ�
	int i  = 0;
	for( i=0; i<nSrcLength; i++)
	{
		*pDst++ = tab[*pSrc >> 4];		// �����4λ
		*pDst++ = tab[*pSrc & 0x0f];	// �����4λ
		pSrc++;
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ���ַ�������
	return nSrcLength * 2;
}

// 7bit����
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ����봮ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ����봮����
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	int nSrc;		// Դ�ַ����ļ���ֵ
	int nDst;		// Ŀ����봮�ļ���ֵ
	int nChar;		// ��ǰ���ڴ����������ַ��ֽڵ���ţ���Χ��0-7
	unsigned char nLeft;	// ��һ�ֽڲ��������

	// ����ֵ��ʼ��
	nSrc = 0;
	nDst = 0;

	// ��Դ��ÿ8���ֽڷ�Ϊһ�飬ѹ����7���ֽ�
	// ѭ���ô������̣�ֱ��Դ����������
	// ������鲻��8�ֽڣ�Ҳ����ȷ����
	while(nSrc<nSrcLength)
	{
		// ȡԴ�ַ����ļ���ֵ�����3λ
		nChar = nSrc & 7;

		// ����Դ����ÿ���ֽ�
		if(nChar == 0)
		{
			// ���ڵ�һ���ֽڣ�ֻ�Ǳ�����������������һ���ֽ�ʱʹ��
			nLeft = *pSrc;
		}
		else
		{
			// ���������ֽڣ������ұ߲��������������ӣ��õ�һ��Ŀ������ֽ�
			*pDst = (*pSrc << (8-nChar)) | nLeft;

			// �����ֽ�ʣ�µ���߲��֣���Ϊ�������ݱ�������
			nLeft = *pSrc >> nChar;

			// �޸�Ŀ�괮��ָ��ͼ���ֵ
			pDst++;
			nDst++;
		}

		// �޸�Դ����ָ��ͼ���ֵ
		pSrc++;
		nSrc++;
	}

	// ����Ŀ�괮����
	return nDst;
}

// 8bit����
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ����봮ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ����봮����
int gsmEncode8bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	// �򵥸���
	memcpy(pDst, pSrc, nSrcLength);
	return nSrcLength;
}


// UCS2����
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ����봮ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ����봮����

int gsmEncodeUcs2(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
	int i=0,nDstLength=0;								// UNICODE���ַ���Ŀ
	unsigned short  wchar[128];		// UNICODE��������
	// �ַ���-->UNICODE��
	// nDstLength = ::MultiByteToWideChar(CP_ACP, 0, pSrc, nSrcLength, wchar, 128);
	
	for(i=0;i<nSrcLength;i++)		//gb2312תunicode
	{
			if(pSrc[i]&0x80)
			{
					wchar[nDstLength] = (pSrc[i]<<8)|pSrc[i+1];
					i++;
					nDstLength++;
			}	
			else
			{
					wchar[nDstLength] = 	pSrc[i];
					nDstLength++;
			}
	}
	// �ߵ��ֽڶԵ������
	for(i=0; i<nDstLength; i++)
	{
		*pDst++ = wchar[i] >> 8;			// �������λ�ֽ�
		*pDst++ = wchar[i] & 0xff;		// �������λ�ֽ�
	}

	// ����Ŀ����봮����
	return nDstLength * 2;
}



// ����˳����ַ���ת��Ϊ�����ߵ����ַ�����������Ϊ��������'F'�ճ�ż��
// �磺"8613910424818"-->"683119404218F8"
// pSrc: Դ�ַ���ָ��
// pDst: Ŀ���ַ���ָ��
// nSrcLength: Դ�ַ�������
// ����: Ŀ���ַ�������
int gsmInvertNumbers(const char* pSrc, char* pDst, int nSrcLength)
{
	int nDstLength;		// Ŀ���ַ�������
	char ch;			// ���ڱ���һ���ַ�
	int i = 0;
	// ���ƴ�����
	nDstLength = nSrcLength;

	// �����ߵ�
	for( i=0; i<nSrcLength;i+=2)
	{
		ch = *pSrc++;		// �����ȳ��ֵ��ַ�
		*pDst++ = *pSrc++;	// ���ƺ���ֵ��ַ�
		*pDst++ = ch;		// �����ȳ��ֵ��ַ�
	}

	// Դ��������������
	if(nSrcLength & 1)
	{
		*(pDst-2) = 'F';	// ��'F'
		nDstLength++;		// Ŀ�괮���ȼ�1
	}

	// ����ַ����Ӹ�������
	*pDst = '\0';

	// ����Ŀ���ַ�������
	return nDstLength;
}

// PDU���룬���ڱ��ơ����Ͷ���Ϣ
// pSrc: ԴPDU����ָ��
// pDst: Ŀ��PDU��ָ��
// ����: Ŀ��PDU������
int gsmEncodePdu(const SM_PARAM* pSrc, char* pDst)
{
	int nLength;			// �ڲ��õĴ�����
	int nDstLength;			// Ŀ��PDU������
	unsigned char buf[256];	// �ڲ��õĻ�����

	// SMSC��ַ��Ϣ��
	nLength = strlen(pSrc->SCA);											// SMSC��ַ�ַ����ĳ���	
	buf[0] = (char)((nLength & 1) == 0 ? nLength : nLength + 1) / 2 + 1;	// SMSC��ַ��Ϣ����
	buf[1] = 0x91;															// �̶�: �ù��ʸ�ʽ����
	nDstLength = gsmBytes2String(buf, pDst, 2);								// ת��2���ֽڵ�Ŀ��PDU��
	nDstLength += gsmInvertNumbers(pSrc->SCA, &pDst[nDstLength], nLength);	// ת��SMSC���뵽Ŀ��PDU��

	// TPDU�λ���������Ŀ���ַ��
	nLength = strlen(pSrc->TPA);		// TP-DA��ַ�ַ����ĳ���
//	if(g_iSMSbackReport==0)
//	{
		buf[0] = 0x11;					// �Ƿ��Ͷ���(TP-MTI=01)��TP-VP����Ը�ʽ(TP-VPF=10)
//	}
//	else
//	{
//		buf[0] = 0x31;					//Ҫ����ŷ��ͳɹ���ִ
//	}
	buf[1] = 0;						// TP-MR=0
	buf[2] = (char)nLength;			// Ŀ���ַ���ָ���(TP-DA��ַ�ַ�����ʵ����)
//	if(((pSrc->TPA[0]=='1')&&(pSrc->TPA[1]=='0')&&(pSrc->TPA[2]=='6'))||(nLength<11))
//	{
//		buf[3]=0x81;//С��ͨ��0x81
//	}
//	else
    {
	    buf[3] = 0x91;//�ֻ���0x91												// �̶�: �ù��ʸ�ʽ����
	}
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], 4);					// ת��4���ֽڵ�Ŀ��PDU��
	nDstLength += gsmInvertNumbers(pSrc->TPA, &pDst[nDstLength], nLength);		// ת��TP-DA��Ŀ��PDU��

	// TPDU��Э���ʶ�����뷽ʽ���û���Ϣ��
	nLength = pSrc->TP_UD_Len; //nLength = strlen(pSrc->TP_UD);		// �û���Ϣ�ַ����ĳ���
	buf[0] = pSrc->TP_PID;						// Э���ʶ(TP-PID)
	buf[1] = pSrc->TP_DCS;						// �û���Ϣ���뷽ʽ(TP-DCS)
	buf[2] = 0;						// ��Ч��(TP-VP)Ϊ5����
	if(pSrc->TP_DCS == GSM_7BIT)	
	{
		// 7-bit���뷽ʽ
		buf[3] = nLength;			// ����ǰ����
		nLength = gsmEncode7bit(pSrc->TP_UD, &buf[4], nLength+1) + 4;	// ת��TP-DA��Ŀ��PDU��
	}
	else if(pSrc->TP_DCS == GSM_UCS2)
	{
		// UCS2���뷽ʽ
		//buf[3] = gsmEncodeUcs2(pSrc->TP_UD, &buf[4], nLength);	// ת��TP-DA��Ŀ��PDU��
		//nLength = buf[3] + 4;		// nLength���ڸö����ݳ���
		buf[3] = nLength;				//数据长度		
		memcpy(&buf[4],pSrc->TP_UD,nLength);
		nLength = buf[3] + 4;	
	}
	else
	{
		// 8-bit���뷽ʽ
		buf[3] = gsmEncode8bit(pSrc->TP_UD, &buf[4], nLength);	// ת��TP-DA��Ŀ��PDU��
		nLength = buf[3] + 4;		// nLength���ڸö����ݳ���
	}
	nDstLength += gsmBytes2String(buf, &pDst[nDstLength], nLength);		// ת���ö����ݵ�Ŀ��PDU��
	// ����Ŀ���ַ�������
	return nDstLength;
}


//char g_ComBuf[256];
//int  g_res=0;
//ִ������Ľ��
//����0 ��ʾʧ�ܣ�����1��ʾ�õ�OK,����2��ʾ�õ�ERROR
//int  CGprs::CmdExe(HANDLE hcom,char*cmd)

// ɾ������Ϣ
// index: ����Ϣ��ţ���1��ʼ

//�˺����Զ��ļ�⴮�ڣ���ͨ�����ڷ���ָ����Զ����豸���ӵ��Ĵ��ڣ������ش˴���
//������ɹ�����-1��
//����-1��ʾ���ڴ�ʧ��
//����-3��ʾgsmģ���ʼ��ʧ��
//����-2��ʾ�������ĺ����ȡʧ��
//����1��ʾ�������ĺ����ȡ�ɹ�
int  OpenGSM(void)
{
	//cedriver::feed_dog();
	//-----------------------------------------------------	
	int		res = 0;
	unsigned int	dw=0;		
	m_siminfo.bSmsPDU=false;
	m_siminfo.bComm=false;
	m_siminfo.singal=0;
	m_siminfo.bExist=false;
	m_siminfo.service=0;		//ԵʼۯδݬӢսՋӪʌ
	m_siminfo.bRegGSM=false;		
		
	//������
	//-9=�򿪴���ʧ��
	//-10=������GSMģ��ͨѶ		����20�β��ܺ�ģ��ͨѶ����ʱ�䳬��1min������gsmģ��
	//-11=�ֻ���������
	//-12=�ź������жϲ��ɹ�
	//-13=GSM����ע�᲻�ɹ�
	//-14=GSM����ע����ѯʧ��
	//-15=����PDU����ģʽ���ò��ɹ�
	//-16=�������ĵ绰�����ȡʧ��
	//-17=��ȡ�������ĵ绰����ִ��ʧ��	

	//˵��,�����ж��ɹ�����߱�����,�����κβ�������û������ġ������ڵ�����˵���ж��豸������������ġ�
	//ģ���Ƿ����ͨѶ->�ֻ����Ƿ����->�ź�����->ע������->�����йصĲ�������(CMGF,CNMI,CSCA)		�ϼ�5������
		
		//**************************1  ģ���Ƿ����ͨѶ  bComm  CommFailCnt ***********************************
		if(!CmdExe(AT_CMD,0))				//AT����		�ж�ģ���Ƿ��������ҿ���ͨѶ			
		{
			InsertLog("AT command failed!\r\n");	
			if(m_siminfo.CommFailCnt<1)		//0��1
			{
					m_siminfo.FailStartTime = g_sysTick;		//g_sysTick
			}
			m_siminfo.CommFailCnt++;			//ͨѶʧ�ܵĴ���
			
			if(m_siminfo.CommFailCnt>9)			//����20�β��ܺ�ģ��ͨѶ����ʱ�䳬��1min������ģ�顣
			{
				if((g_sysTick-m_siminfo.FailStartTime)>600)		//20�� 140�룬���԰���1����10�������㣬���հ���100��,10min���ٶ���������
				{
					m_siminfo.CommFailCnt = 0;	
					m_siminfo.FailStartTime = g_sysTick;
					m_siminfo.gsm_reset_reson = 1;			//����,3�����ϲ���ͨѶ���ҳ���30��					
					ResetGPRS();						
				}
			}
			return -10;  
		}
		m_siminfo.bComm=true;					//ͨѶ�ɹ�
		m_siminfo.CommFailCnt=0;				//ͨѶʧ�ܵĴ���
		if(!CmdExe(ATE_CMD,0))				//�رջ���		�ж�ģ���Ƿ��������ҿ���ͨѶ
		{
			InsertLog("ATE command failed!\r\n");				
		}

		if(CmdExe(AT_CSQ,0)==1)		//хۅ׊			Ɛ׏хۅǿ׈ɧێ
		{
			GetCSQ();										//ܱȡхۅǿ׈		
			if(m_siminfo.singal<9)			//хۅǿ׈Ɛ׏
			{			
						
			}			
		}
		else
		{
			return -12;
		}
		
		//**************************2  �ֻ����Ƿ����  bExist  iccid   phonenum   *****************************	
		if(CmdExe(AT_CIMI,0)==1)			//IMSI			�ж��ֻ����Ƿ����
		{
				GetIMSI();
				m_siminfo.bExist=true;	
		}
		else
		{
				return -11;
		}
		if(CmdExe(AT_ICCID,0)==1)		//iccid				�ж��ֻ���iccid mg323��֧��
		{
			GetICCID();		//��iccid����	��':'��ʼ��������20���ǿո��ַ�
		}
		else
		{
					
		}
		//if(CmdExe(AT_CNUM,0)==1)			//�����ֻ���
		//{
			//m_siminfo.phonenum;	
		//}
		//else
		//{
		//	InsertLog("CNUM����ִ��ʧ��!\r\n");				
		//}		
		//**************************3 �ź�����  singal      *****************************	
	
		if(CmdExe(AT_COPS,0)==1)	//ܱȡՋӪʌ
		{
			GetCOPS();					//Ɛ׏ˇرӑޭעӡԉ٦θç,ɧڻθçעӡԉ٦ìղƐ׏ؾϱǷʌ			
		}
		else
		{
			
		}	
		
		//**************************4 ע������ bRegGSM  service   ********************	
		if(CmdExe(AT_CREG,0)==1)	//gsm����ע��ɹ�		�ж������Ƿ�ע��ɹ�
		{
			GetREG(CREG);				//�ж��Ƿ��Ѿ�ע��ɹ�������,�������ע��ɹ������жϷ�������
			if(m_siminfo.bRegGSM)
			{
			
			}
			else
			{									
				return -13;
			}
		}
		else
		{					
			return -14;
		}	
					
		//**************************5 �������� bRegGSM  service   ********************	
		if(CmdExe(AT_CMGF,0)==1)		//PDU��ʽ���Ͷ���
		{
			m_siminfo.bSmsPDU=true;
		}
		else
		{
			m_siminfo.bSmsPDU=false;
			return -15;
		}	
		
		if(CmdExe(AT_CNMI,0)==1)	//���Ŵ洢ģʽ����
		{
			
		}
		else
		{
				
		}	

		if(CmdExe(AT_CSCA,0)==1)	//��ȡ�������ĵ绰����
		{
			if(GetCSCA())				//�ж��Ƿ��Ѿ�ע��ɹ�������,�������ע��ɹ������жϷ�������			
			{
				
			}
			else
			{							
				return -16;
			}
		}
		else
		{
			return -17;
		}	
		return 1;
}

int  SendSMS(char* PhoneNum,char* Content,int len)
{
		int 			iSmslen = 0;
		int				nPduLength = 0;								// PDU������			
		SM_PARAM aSMS;			
		unsigned char	nSmscLength = 0;		// SMSC������
		char cmd[64];								// ���
		char pdu[800];							// PDU��	
		InsertLog("^^^^^^^^SendSMS  start++++++++\r\n");
		
		strcpy(aSMS.SCA, m_siminfo.strCSCA);		// ����Ϣ�������ĺ���(SMSC��ַ)  16���ֽ�

		//strcpy(aSMS.TPA,"86");
		strcpy(aSMS.TPA,(char*)g_configRead.NationCode);		
		strcat(aSMS.TPA,PhoneNum);							//Ŀ��绰���� 8612345678901	
		//strcpy(aSMS.TPA, PhoneNum);						// Ŀ�����

		InsertLog("Target telephone number:");
		InsertLog(aSMS.TPA);
		InsertLog("\r\n");
	
		aSMS.TP_UD_Len = len;
		memcpy(aSMS.TP_UD, Content,len);	// ԭʼ�û���Ϣ(����ǰ�������TP-UD)
		aSMS.TP_UD[len]='\0';							// ���70���ַ�
		aSMS.TP_PID = 0;									// �û���ϢЭ���ʶ(TP-PID)--��Ե㷽ʽ
		aSMS.TP_DCS = GSM_UCS2;						// �û���Ϣ���뷽ʽ(TP-DCS)
				
		
		//��ʽת��
		nPduLength = gsmEncodePdu(&aSMS, pdu);	// ����PDU����������PDU��  �������ģ�Ŀ��绰���붼�����õ���
		strcat(pdu, "\x01a");										// ��Ctrl-Z����
		gsmString2Bytes(pdu, &nSmscLength, 2);	// ȡPDU���е�SMSC��Ϣ����
		nSmscLength++;													// ���ϳ����ֽڱ���
		
		// �����еĳ��ȣ�������SMSC��Ϣ���ȣ��������ֽڼ�
		iSmslen=nPduLength / 2 - nSmscLength;
		memset(cmd,0,sizeof(cmd));
		strcpy(cmd,"AT+CMGS=");							// ��������
		if(iSmslen>99)			//100->99
		{
				cmd[8] = ((iSmslen/100)%10) + '0';
				cmd[9] = ((iSmslen/10)%10) + '0';
				cmd[10] = ((iSmslen/1)%10) + '0';
				cmd[11] = '\r';				
		}
		else if(iSmslen>9)		//10->9 
		{
			
				cmd[8] = ((iSmslen/10)%10) + '0';
				cmd[9] = ((iSmslen/1)%10) + '0';
				cmd[10] = '\r';				
		}
		else
		{
				cmd[8] = ((iSmslen/1)%10) + '0';
				cmd[9] = '\r';		
		}
		CmdExe(cmd,1);										//����ƥ�䡮>���ɹ��񶼷�������
		delay_1ms(200);
		nPduLength = CmdExe(pdu,2);				//״̬1�ɹ�,2ʧ��,0û�л���	��ȴ�30��
		if(1==nPduLength)
		{	
			nPduLength = 1;
			InsertLog("send sms:SMS sending success!\r\n");
		}
		else if(2==nPduLength)
		{
			nPduLength = -5;
			InsertLog("send sms: SMS sending failure,return ERROR!\r\n");
		}
		else	//�ȴ�>ʧ��
		{
			nPduLength = -4;
			InsertLog("send sms: SMS sending failure,no retrun!\r\n");
		}	
		return nPduLength;	//gsmSendMessage	
}

int  DaileTel(char* PhoneNum)
{			
		unsigned int t = 0;
		int success = 0;	
		int cmd_len = 0;	
		int	dw  = 0;	
		char cmd[128]="ATD";
	
		strcat(cmd,PhoneNum);
		strcat(cmd,";\r\n");
		cmd_len = strlen(cmd);
		InsertLog("^^^^^^^^Daile Tel  start++++++++\r\n");	
		Com.Usart[G4].usRec_RD = Com.Usart[G4].usRec_WR;					//ۏ݆
		Com_Send(G4,(unsigned char*)cmd,cmd_len); 	 					//WriteFile(hcom,cmd,cmd_len,&dw,NULL);		//ע̍ļ®	
		InsertLog("CmdExe send: ");
		InsertLog(cmd);
		InsertLog("\r\n");
	
		g_res = 0;
		memset(g_ComBuf,0,sizeof(g_ComBuf));	
	
	t = g_sysTick;
	while(1)			//最长10秒。
	{
		wdt();
		delay_1ms(20);																//等待30s最长
		dw = (Com.Usart[G4].usRec_WR - Com.Usart[G4].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;		//数据长度
		if(dw>0)
		{
			cmd_len = 0;
			while(Com.Usart[G4].usRec_RD!=Com.Usart[G4].usRec_WR)
			{		
					cmd[cmd_len++] = Com.Usart[G4].RX_Buf[Com.Usart[G4].usRec_RD];
					g_ComBuf[g_res++] = Com.Usart[G4].RX_Buf[Com.Usart[G4].usRec_RD];
					Com.Usart[G4].usRec_RD = (Com.Usart[G4].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
					g_res = g_res%512;
					cmd_len = cmd_len%127;
			}
			cmd[cmd_len] = '\0';
			g_ComBuf[g_res] = '\0';
			wdt();
			InsertLog(cmd);	
			
			if(strstr(g_ComBuf,"NO DIALTONE"))		//ƥƤOKܲERROR
			{
				delay_1ms(10);				
				success = 1;
				//break;
			}			
			else if(strstr(g_ComBuf,"CONNECT"))		//ƥƤOKܲERROR
			{
				delay_1ms(10);				
				success = 2;
				//break;
			}		
			else if(strstr(g_ComBuf,"BUSY"))			//挂断 ,即不接电话，并挂断。
			{
				delay_1ms(10);				
				success = 3;
				break;
			}
			else if(strstr(g_ComBuf,"NO CARRIER"))	//手机关机或拨号时间过长，对方一直没有接电话
			{
				delay_1ms(10);				
				success = 4;
				break;
			}
			else if(strstr(g_ComBuf,"OK"))
			{
				delay_1ms(10);			
				success = 5;
				//break;
			}			
		}
		if((g_sysTick-t)>39)
		{
				break;
		}
	}	
  wdt();
	if(g_res)		//Ԑ˽ߝ
	{	
			g_ComBuf[g_res] = '\0';		
			InsertLog("CmdExe rec:");	
			InsertLog(g_ComBuf);			
			InsertLog("\r\n");			
	}		
	CmdExe("ATH\r\n",0);
	InsertLog("^^^^^^^^Daile Tel  end++++++++\r\n");	
	return success;	
}

int  Dailer_Pre()
{
//	int HardOK=GPRS_Hard_OK();		//io驱动可以打开。
	int SoftOK = GPRS_Soft_OK();		//通过AT命令的方式进行查看gsm模块的状态，具备gprs拨号上网条件就可以拨号
	int a = SoftOK;
	char temp[14] = "SOFT_OK:-12\r\n";
	if(a<0)
	{
			a = -1*a;
			temp[8] = '-';
	}
	else
	{
			temp[8] = ' ';
	}
	if(a>9)
	{
			temp[9]  = (a/10) + '0';
			temp[10] = (a%10) + '0';
	}
	else
	{
			temp[9]  = ' ';
			temp[10] = a + '0';
	}
	temp[13] = '\0';	
	InsertLog(temp);
	
	m_gprsinfo.gprs_last_faile_reason = SoftOK;			//失败原因
	
	if(0==SoftOK)				
	{
		if(m_siminfo.singal<6)										//信号强度弱，不可以拨号  9->6
		{		
				m_gprsinfo.gprs_faile++;
				m_gprsinfo.gprs_lastsuc_faile++;
				m_gprsinfo.gprs_last_faile_reason = -18;
				InsertLog("signal too weak,stop daile\r\n");		
				return 0;										
		}	
	}
	else
	{
		//不具备拨号上网的基本条件
		m_gprsinfo.gprs_faile++;
		m_gprsinfo.gprs_lastsuc_faile++;	
		return 0;
	}
	return 1;	
}

//执行透明传输命令
int ENTRANS(void)
{
	char AT_IPENTRANS[]   = {"AT^IPENTRANS=0\r\n"};			
	int success = 0;
	success = CmdExe(AT_IPENTRANS,4);				//进入透传模式  等待3秒  3秒-->5秒
	if(success==1)													//成功进入透传模式	
	{		
			InsertLog("AT_IPENTRANS command success!\r\n");
			return 1;  
	}					
	else													//超时或错误
	{
			InsertLog("ResetGPRS:AT_IPENTRANS command failed -- 5 second timeout or error !\r\n");
			ResetGPRS();						//重新再来
			return -25;							//连接失败				
	}		
}

//0=超时，1表示成功，其他则表示失败
int Dailer(void)
{
	//进行拨号操作  如果为第1次则进行 必要5个参数设置。
	//如果发现上传服务器ip或port发生变化则重新修改ip+port。
	//进行SISO命令操作。
	//如果连接成功则进行透传模式设置。
	/*
	^SISS: 0,"srvType","Socket"
	^SISS: 0,"conId","0"
	^SISS: 0,"address","socktcp://139.129.17.114:7107"
	^SISS: 0,"tcpMR","10"
	^SISS: 0,"tcpOT","6000"
	^SISS: 1,"srvType",""
	^SISS: 2,"srvType",""
	^SISS: 3,"srvType",""
	^SISS: 4,"srvType",""
	^SISS: 5,"srvType",""
	^SISS: 6,"srvType",""
	^SISS: 7,"srvType",""
	^SISS: 8,"srvType",""
	^SISS: 9,"srvType",""
	*/
	#if MODULE_4G
		int success = 0 ,i = 0,port = 0;
		//char AT_QIOPEN[]  	= {"AT+QIOPEN=1,0,\"TCP\",\"139.129.17.114\",7107,0,2\r\n     "};		//AT+QIOPEN=1,0,"TCP","121.42.176.145"",71070,0,2
		char AT_QIOPEN[]  	= {"AT+CIPOPEN=0,\"TCP\",\"139.129.17.114\",7107\r\n     "};		//AT+QIOPEN=1,0,"TCP","121.42.176.145"",71070,0,2
		char temp[32];
		char temp2[32];
		int	 len  = 0;
		int	 len2  = 0;
	#if 1
		memcpy(AT_QIOPEN+20,g_configRead.remoteIP,g_configRead.IPLen);   
   
    AT_QIOPEN[20+g_configRead.IPLen]='"';
		AT_QIOPEN[21+g_configRead.IPLen]=',';
		memcpy(AT_QIOPEN+22+g_configRead.IPLen,g_configRead.remotePort,g_configRead.PortLen);
		
		//AT_QIOPEN[23+g_configRead.IPLen+g_configRead.PortLen]=',';
    //AT_QIOPEN[24+g_configRead.IPLen+g_configRead.PortLen]='0';
		//AT_QIOPEN[25+g_configRead.IPLen+g_configRead.PortLen]=',';
    //AT_QIOPEN[26+g_configRead.IPLen+g_configRead.PortLen]='2';
		
    AT_QIOPEN[22+g_configRead.IPLen+g_configRead.PortLen]='\r';
    AT_QIOPEN[23+g_configRead.IPLen+g_configRead.PortLen]='\n';
    AT_QIOPEN[24+g_configRead.IPLen+g_configRead.PortLen]='\0';	//15+5+33
		
		//保留
		memset(m_siminfo.ip_address,0,sizeof(m_siminfo.ip_address));
		memcpy(m_siminfo.ip_address,g_configRead.remoteIP,g_configRead.IPLen);
		for(i=0;i<g_configRead.PortLen && i<5;i++)
		{
				port = (g_configRead.remotePort[i] - 0x30) + port*10; 
		}
		#else
		sprintf(temp,"%d.%d.%d.%d",gs_SaveNetIPCfg.ucDestIP[0],gs_SaveNetIPCfg.ucDestIP[1],gs_SaveNetIPCfg.ucDestIP[2],gs_SaveNetIPCfg.ucDestIP[3]);
		len = strlen(temp);
		sprintf(temp2,"%d",(gs_SaveNetIPCfg.ucDestPort[0]<<8)|gs_SaveNetIPCfg.ucDestPort[1]);
		len2 = strlen(temp2);
		
		memcpy(AT_QIOPEN+20,temp,len);      
    AT_QIOPEN[20+len]='"';
		AT_QIOPEN[21+len]=',';
	
		memcpy(AT_QIOPEN+22+len,temp2,len2);
		
		
    AT_QIOPEN[22+len+len2]='\r';
    AT_QIOPEN[23+len+len2]='\n';
    AT_QIOPEN[24+len+len2]='\0';	
		
		
		memset(m_siminfo.ip_address,0,sizeof(m_siminfo.ip_address));
		memcpy(m_siminfo.ip_address,temp,len);
		
		port = (gs_SaveNetIPCfg.ucDestPort[0]<<8)|gs_SaveNetIPCfg.ucDestPort[1];
		
		#endif
		m_siminfo.port_number = port;
		
		
		Com.Usart[G4].usRec_RD=Com.Usart[G4].usRec_WR;				//由于AT_SISO可能需要异步,所以对buf清空。准备后面的异步判断(返回超时)。
		success = CmdExe(AT_QIOPEN,4);			//等待3秒时间，OK=1  ERROR=2  0=timeout		3->4  即3秒修改为5秒				0=timeout		1=CONNECT		2=ERROR
		if(0==success)					//0=timeout
		{			
				InsertLog("AT+CIOPEN command 5 second  timeout !\r\n");
				return 0;  					//超时
		}				
		else if(1==success)			//OK
		{
				delay_1ms(5);
				InsertLog("AT+CIOPEN command OK!\r\n");
				//success = ENTRANS();	//1或-25
				return 1;
		}
		else
		{
				return -24;		//失败  ERROR
		}
		//g_ComBuf[g_res];	
		return 1;	
		
	#else	
		int success = 0 ,i = 0,port = 0;
		char AT_SICS1[]  	= {"AT^SICS=0,conType,GPRS0\r\n"};				
		char AT_SICS2[]  	= {"AT^SICS=0,apn,CMNET\r\n"};				//该命令执行失败			
		//char AT_SISS1[]  = {"AT^SISS=0,conId,0\r\n"};							
		char AT_SISS2[]  	= {"AT^SISS=0,srvType,Socket\r\n"};					
		char AT_SISS3[]  	= {"AT^SISS=0,address,\"socktcp://139.129.17.114:7107\"\r\n   "};					           
		char AT_SISO[]   	= {"AT^SISO=0\r\n"};				
		char AT_SISC[]   	= {"AT^SISC=0\r\n"};				
		char AT_IPCFL[]   = {"AT^IPCFL=10,78\r\n"};				
		

		memcpy(AT_SISS3+29,g_configRead.remoteIP,g_configRead.IPLen);
    AT_SISS3[29+g_configRead.IPLen]=':';
    memcpy(AT_SISS3+30+g_configRead.IPLen,g_configRead.remotePort,g_configRead.PortLen);
    AT_SISS3[30+g_configRead.IPLen+g_configRead.PortLen]='"';
    AT_SISS3[31+g_configRead.IPLen+g_configRead.PortLen]='\r';
    AT_SISS3[32+g_configRead.IPLen+g_configRead.PortLen]='\n';
    AT_SISS3[33+g_configRead.IPLen+g_configRead.PortLen]='\0';	//15+5+33
		
		//保留
		memset(m_siminfo.ip_address,0,sizeof(m_siminfo.ip_address));
		memcpy(m_siminfo.ip_address,g_configRead.remoteIP,g_configRead.IPLen);
		for(i=0;i<g_configRead.PortLen && i<5;i++)
		{
				port = (g_configRead.remotePort[i] - 0x30) + port*10; 
		}
		m_siminfo.port_number = port;
		
		
		//**************************1  ģࠩˇرࠉӔͨѶ  bComm  CommFailCnt ***********************************
		if(CmdExe(AT_SICS1,0)!=1)				
		{			
			InsertLog("AT_SICS1 command failed!\r\n");	
			return -20;  
		}
		delay_1ms(5);
		if(CmdExe(AT_SICS2,0)!=1)				
		{			
			InsertLog("AT_SICS2 command failed!\r\n");	
			return -21;  
		}
		delay_1ms(5);
		if(CmdExe(AT_SISS2,0)!=1)				
		{			
			InsertLog("AT_SISS2 command failed!\r\n");	
			return -22;  
		}
		delay_1ms(5);
		if(CmdExe(AT_SISS3,0)!=1)					
		{			
			InsertLog("AT_SISS3 command failed!\r\n");	
			return -23;  
		}		
		delay_1ms(5);
		//2018-10-30增加
		if(CmdExe(AT_IPCFL,0)!=1)					
		{			
			InsertLog("AT_IPCFL 10 ,78 command failed!\r\n");	
			return -23;  
		}		
		delay_1ms(5);
		AT_IPCFL[10]='4';
		AT_IPCFL[11]=',';
		AT_IPCFL[12]='1';
		AT_IPCFL[13]='\r';
		AT_IPCFL[14]='\n';
		AT_IPCFL[15]='\0';
		
		if(CmdExe(AT_IPCFL,0)!=1)					
		{			
			InsertLog("AT_IPCFL 14 ,1 command failed!\r\n");	
			return -23;  
		}		
		delay_1ms(5);
		
		//发送连接命令，并等待1min,如果等待结果成功则成功（进入透明传输模式），如果等待结果失败，则失败，否则等待超时(当等待超时，如何处理？)。
		
		Com.Usart[G4].usRec_RD=Com.Usart[G4].usRec_WR;			//由于AT_SISO可能需要异步,所以对buf清空。准备后面的异步判断(返回超时)。
		success = CmdExe(AT_SISO,4);			//等待3秒时间，OK=1  ERROR=2  0=timeout		3->4  即3秒修改为5秒
		if(0==success)					//0=timeout
		{			
				InsertLog("AT_SISO command 5 second  timeout !\r\n");
				return 0;  					//超时
		}				
		else if(1==success)			//OK
		{
				delay_1ms(5);
				InsertLog("AT_SISO command OK!\r\n");
				success = ENTRANS();	//1或-25
				return success;
		}
		else
		{
				return -24;		//失败  ERROR
		}
		//g_ComBuf[g_res];	
		return 1;	
#endif				
}

char g_tranflag=1;
//由透传模式修改为断开连接,如果执行结果不是预期，则考虑重启模块
int CloseGprs(void)
{
		int success = 0,success2=0;
	#if	MODULE_4G
		char AT_SISC[] 	= {"AT+CIPCLOSE=0\r\n"};					
	#else
		char AT_SISC[] 	= {"AT^SISC=0\r\n"};					
	#endif
		char AT_IPENTRANS_EXIT[]   = {"+++"};				
		g_tranflag = 0;															//准备发送退出gprs模式命令
		
		delay_1ms(200);			//等待,目的防止tcp server发送修改 ip或port命令，立即执行关闭gprs，而发送+++命令时间间隔过短，发送失败。
		wdt();		
		delay_1ms(200);			//2018-10-25
		wdt();		
		delay_1ms(200);			//2018-10-25
		wdt();
		delay_1ms(200);			//2018-10-25
		wdt();
		delay_1ms(200);			//2018-10-25
		wdt();

		success = CmdExe(AT_IPENTRANS_EXIT,4);			//5秒退出
		if(success==1)		//成功
		{		
				m_gprsinfo.g_bGPRSConnected = 0;
				Led_Ctrl(0,false);
				success2 = CmdExe(AT_CMD,0);			//目的为了发\r\n
				delay_1ms(20);
				success = CmdExe(AT_SISC,5);			//断开连接最长可以等10秒
				g_tranflag = 1;
				if(success==0)
				{
						InsertLog("ResetGPRS:AT_IPENTRANS_EXIT command success ,but AT_SISC command faile!\r\n");
						ResetGPRS();									//gprs								
						return 0;											//不成功  考虑重启设备
				}		
        InsertLog("ResetGPRS: success !\r\n");				
				return 1;  
		}
		else							//不成功
		{
				InsertLog("AT_IPENTRANS_EXIT command failed!\r\n");				
				m_gprsinfo.g_bGPRSConnected = 0;
				Led_Ctrl(0,false);
				success2 = CmdExe(AT_CMD,0);			//目的为了发\r\n
				delay_1ms(20);
				success = CmdExe(AT_SISC,5);			//断开连接最长可以等3秒
				g_tranflag = 1;				
				if(success==0)
				{
						InsertLog("ResetGPRS:AT_IPENTRANS_EXIT & AT_SISC command faile!\r\n");
						ResetGPRS();									//gprs				
						return 0;											//不成功  考虑重启设备
				}			
			  InsertLog("ResetGPRS: success !\r\n");					
				return 1; 
		}		
}

//*RR,15-11-18 11:30:57,1401191600# 33字节
int MatchRecData(char*ack , int len)
{
	int i = 0 , k = len-32;
	for(i=0;i<k;i++)
	{
		if(ack[i]=='*' && ack[i+1]=='R' && ack[i+2]=='R' && ack[i+32]=='#')
		{
			return i;			//最好核对一下设备id
		}													
	}
	return -1;
}

unsigned char strcmp2(unsigned char* dat,unsigned char* dat1,unsigned char len)
{
  unsigned char i;
  for(i=0;i<len;i++)
  {
    if(dat[i]!=dat1[i])
      return 1;
  }
  return 0;
}
unsigned int  g_GprsConnTimeout = 0;				//gprs超时时间
int						g_GprsWaitResultFlag = 0;			//需要gprs等待标志
int 					iNetReg = 0;									//gprs拨号的时候，gprs网络注册不成功的次数 
int 					dailerCnt=0;									//连续拨号次数	
//等待拨号结果
void WaitForResult(void)
{
	int success=0;
	int	dw  = 0;
	unsigned int temp;
	//已经接收数据g_ComBuf[g_res];	
	if(g_GprsWaitResultFlag)										//进行时间判断
	{
		temp = g_sysTick - g_GprsConnTimeout ;		//已经过去的时间
		//进行数据判断			
		dw = (Com.Usart[G4].usRec_WR - Com.Usart[G4].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;		
		if(dw>0)
		{
			while(Com.Usart[G4].usRec_RD!=Com.Usart[G4].usRec_WR)
			{
					g_ComBuf[g_res++] = Com.Usart[G4].RX_Buf[Com.Usart[G4].usRec_RD];
					Com.Usart[G4].usRec_RD = (Com.Usart[G4].usRec_RD+1)%D_USART_REC_BUFF_SIZE;
					g_res = g_res%512;
			}	
		
			if(strstr(g_ComBuf,"OK"))		//ƥƤOKܲERROR
			{
				delay_1ms(10);
				dw = (Com.Usart[G4].usRec_WR - Com.Usart[G4].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;	
				if(dw>0)
				{
						//g_res+=dw;
				}	
				//success=ENTRANS();
				success=1;
				if(success==1)
				{
					g_GprsWaitResultFlag = 0;
					g_GprsConnTimeout = g_sysTick;
					m_gprsinfo.g_bGPRSConnected = 1;	
					Led_Ctrl(0,true);
					RSGPRS_rec_WR = 0 ;
					RSGPRS_rec_RD = 0 ;
					InsertLog("WaitForResult gprs success!\r\n");
				}
				else
				{
						g_GprsWaitResultFlag = 0;						
						g_GprsConnTimeout = g_sysTick;
						m_gprsinfo.g_bGPRSConnected = 0;	
						Led_Ctrl(0,false);
					  InsertLog("WaitForResult gprs success,but enter trans failed!\r\n");
				}
				return;
			}
			#if MODULE_4G
			else if(strstr(g_ComBuf,"CONNECT"))		//ƥƤOKܲERROR
			{
				delay_1ms(10);
				dw = (Com.Usart[G4].usRec_WR - Com.Usart[G4].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;	
				if(dw>0)
				{
						//g_res+=dw;
				}					
				g_GprsWaitResultFlag = 0;
				g_GprsConnTimeout = g_sysTick;
				m_gprsinfo.g_bGPRSConnected = 1;	
				Led_Ctrl(0,true);
				RSGPRS_rec_WR = 0 ;
				RSGPRS_rec_RD = 0 ;
				InsertLog("WaitForResult 4g success!\r\n");				
				return;
			}
			#endif
			else if(strstr(g_ComBuf,"ERROR"))
			{
				delay_1ms(10);
				dw = (Com.Usart[G4].usRec_WR - Com.Usart[G4].usRec_RD + D_USART_REC_BUFF_SIZE)%D_USART_REC_BUFF_SIZE;	
				if(dw>0)
				{
						//g_res+=dw;
				}				
				g_GprsWaitResultFlag = 0;
				g_GprsConnTimeout = g_sysTick;				
				m_gprsinfo.g_bGPRSConnected = 0;		
				Led_Ctrl(0,false);				
				m_gprsinfo.gprs_last_fail_time = g_sysTick;				//GetLocalTime(&pSocket->m_gprsinfo.gprs_last_fail_time);				//连接失败的时间
				m_gprsinfo.gprs_lastsuc_faile++;			
				m_gprsinfo.gprs_faile++;								
				m_gprsinfo.gprs_last_faile_reason = -24;													
				m_gprsinfo.gprs_last_fail_time = g_sysTick;				//GetLocalTime(&pSocket->);			//连接失败的时间					
				InsertLog("WaitForResult gprs/4g failed!\r\n");						
				return;
			}
		}	
		
		if(temp>34)						//最长等待65+5(siso等5)+5(ResetGPRS)+5(其他命令)秒=80秒合计。   64->34
		{
				g_GprsWaitResultFlag = 0;
				g_GprsConnTimeout = g_sysTick;		
				InsertLog("ResetGPRS:WaitForResult faile ,wait time > 35s ...\r\n");
				ResetGPRS();																			//gprs				 执行需要5秒时间。					
				//WaitForResult
		}
	}		
}
/*
GPRSThread功能描述: gprs上传模式下(前提gprs模块允许工作),60秒尝试1次拨号，条件:无短信发送，并且m_gprsinfo.g_bGPRSConnected=false;  
拨号分两步，1. 进行预先判断，看是否具备拨号条件Dailer_Pre(),否则查看失败的原因(连续多次发送AT失败，gprs注册网络失败，gsm注册失败),如果上面三种情况，考虑重启gprs模块。
								AT_CMD  	判断		-10
								ATE				不判断	
								AT_CSQ		判断		-12
								AT_CIMI		判断		-11
								AT_ICCID	不判断
								AT_COPS		不判断
								AT_CREG		判断		-13 -14
								AT_CGREG	判断		-1  -2
								成功0

            2. 进行拨号,Dailer
								AT_SICS1	判断	-20
								AT_SICS2	判断	-21
								AT_SISS2	判断	-22
								AT_SISS3	判断	-23
								AT_SISO		判断	0(5秒超时) -24
								ENTRANS		判断	-25
								成功1	
在g_GprsWaitResultFlag=1的时候，表示GPRSThread函数还没有执行完毕。其他对gprs操作不能进行。
								
*/

void GPRSThread(void)
{	
		char temp[19] = "Daile Result:-12\r\n";
		int 	i = 0, len = 0 , a = 0;
		//int success = 0;	
		//char 	recv_buff[512];			
		int 	tmpRecvLen;		
		if((G4+1) == g_configRead.wifi_mode) 		//
		{
			//gprs工作模式,如果无短信 则允许进入gprs模式;rj45模式则直接进入   5秒读1次				
			if((g_sysTick - read_ext_data_tick)>20 )					//60秒尝试1次  即每间隔60秒执行1次。
			{
					
					//gprs拨号执行时间0~1min
					//++++++++++++++++++GPRS拨号过程++++++++++++++++++++++++++++++++++++++++++++++++++++++
					//g_gprs_discon=1表示检测到了gprs已经断开 当g_bGPRSConnected可能还=1 表示处于连接状态
					if(false==m_gprsinfo.g_bGPRSConnected && m_gsm_wr==m_gsm_rd && m_tel_wr==m_tel_rd && g_GprsWaitResultFlag==0)				//GPRS未连接		gprs模式  ,并且无短信的情况下，不处于等待AT_SISO 过程中(g_GprsWaitResultFlag)。
					{
						if(1==Dailer_Pre())													//具备拨号条件		如果为-1则表示gprs
						{	//最长1min拨号，失败则继续拨号
							#if MODULE_4G
							//强制进行1次关闭socket操作。无论是否成功,可能存在的问题。执行超时。									
							CmdExe("AT+CIPCLOSE=0\r\n",4);						//断开连接最长可以等5秒
							#endif
							iNetReg = 0;	
							InsertLog("Dailing... \r\n");							//,pSocket->m_siminfo.singal);							
							tmpRecvLen=Dailer();											//调用拨号函数，异步处理。如果失败，则失败原因和时间在Dailer中
						
							//打印daile 结果
							a = tmpRecvLen;
							if(a<0)
							{
									a = -1*a;
									temp[13] = '-';
							}
							else
							{
									temp[13] = ' ';
							}
							if(a>9)
							{
								temp[14]  = (a/10) + '0';
								temp[15] = (a%10) + '0';
							}
							else
							{
								temp[14]  = ' ';
								temp[15] = a + '0';
							}
							temp[18] = '\0';	
							InsertLog(temp);
							
	
							dailerCnt++;																			//拨号次数统计	连续失败次数						
							wdt();
							//如果拨号成功或不成功,则直接出结果。如果超时则进入下一个环节。但在拨号的过程中不允许
							if(tmpRecvLen==1)																	//正在进行拨号 拨号函数调用成功
							{
									g_GprsWaitResultFlag = 0;
									dailerCnt = 0;																//gprs连接成功														
									InsertLog("gprs connection success!\r\n");			
									m_gprsinfo.g_bGPRSConnected = 1;		
									Led_Ctrl(0,true);
									RSGPRS_rec_WR = 0 ;
									RSGPRS_rec_RD = 0 ;								
							}		
							else if(0==tmpRecvLen)														//AT_SISO 超时，需要继续判断  在WaitForResult中进行。
							{
									m_gprsinfo.g_bGPRSConnected = 0;		
									Led_Ctrl(0,false);
									g_GprsWaitResultFlag = 1;
									g_GprsConnTimeout = 	g_sysTick;							//等待异步开始时间
									//已经接收数据g_ComBuf[g_res];	
							}
							else				//失败
							{
									g_GprsWaitResultFlag = 0;
									m_gprsinfo.g_bGPRSConnected = 0;				
									Led_Ctrl(0,false);								
									m_gprsinfo.gprs_last_fail_time = g_sysTick;				//GetLocalTime(&pSocket->m_gprsinfo.gprs_last_fail_time);				//连接失败的时间
									m_gprsinfo.gprs_lastsuc_faile++;			
									m_gprsinfo.gprs_faile++;								
									m_gprsinfo.gprs_last_faile_reason = tmpRecvLen;		//最后失败原因							
									InsertLog("max 1min dial failure!\r\n");									
									m_gprsinfo.gprs_last_fail_time = g_sysTick;				//GetLocalTime(&pSocket->);			//连接失败的时间									
							}										
						}
						else		//预拨号失败,如果为gprs网络注册的问题，则连续150次则重启gsm模块，否则不做任何处理；5s后继续拨号
						{
								wdt();
								g_GprsWaitResultFlag = 0;
								m_gprsinfo.g_bGPRSConnected = 0;		
								Led_Ctrl(0,false);
								//gsm||gprs 注册失败次数之和达到10次，即15min,则gprs重启1次。
								if(-1==m_gprsinfo.gprs_last_faile_reason || -13==m_gprsinfo.gprs_last_faile_reason)			//失败原因为gprs注册网络失败-1,gsm注册失败-13   2018-01-06 问题，当gsm注册失败次数达到上限，则进行重启gprs模块。在复位的时候,gprs模块可能复位造成设备无法连接。
								{
									iNetReg++;
									
									memcpy(temp,"reg fail,cnt=00\r\n",17);
									temp[17] = '\0';
									if(iNetReg<10)
									{
											temp[15] = iNetReg + '0';
									}
									else
									{
											temp[14] = (iNetReg/10) + '0';
											temp[15] = (iNetReg%10) + '0';
									}
									InsertLog(temp);
									
									if(iNetReg>9)																//gprs预拨号,大概15分钟注册gprs网络都失败,则重启gprs模块  当手机无费，充完钱后，一般都是gprs网络注册失败，需要重启模块。
									{
											iNetReg = 0;
											InsertLog("15min,register gprs net failure,gsm reset...!\r\n");
											m_siminfo.gsm_reset_reson = 4;					//gprs预拨号,大概15分钟注册gprs网络或gsm都失败
											InsertLog("ResetGPRS:Dailer_Pre faile & cnt > 9 \r\n");
											ResetGPRS();														//gprs									
									}
								}
								else
								{
										iNetReg = 0;
								}
								m_gprsinfo.gprs_last_fail_time = g_sysTick;			//GetLocalTime(&pSocket->m_gprsinfo.gprs_last_fail_time);					//连接失败的时间							
						}						
					}//end if gprs拨号过程
					//++++++++++++++++++GPRS拨号过程++++++++++++++++++++++++++++++++++++++++++++++++++++++		
					read_ext_data_tick = g_sysTick;		
					//无论成功和失败，最多等75秒(65秒等+5秒重启模块,WaitForResult中)尝试新的连接。 开始时间0秒时刻，Dailer_Pre和Dailer不成成功情况下时间估算最大10秒。WaitForResult等待70秒。重启5秒
					//时间成本siso最长  65+5=70
					//WaitForResult不成功超时最多			  	65+5(reset gprs)=70
					//WaitForResult不成功明确命令最多 	 	65+5(进入透传模式命令)=70					
					//WaitForResult成功最多  							65+5(进入透传模式命令)=70					
					//1次拨号最长用时为: Dailer_Pre+Dailer=5秒，siso=5秒，WaitForResult=70秒，合计需要80秒。
					//read_ext_data_tick重新赋值后，最长需要70秒才可以进入在拨号状态，考虑gprs模块重启时间为12秒连接好网络。则本处空闲15秒设计。85秒。
				}//end if 30秒尝试1次
				WaitForResult();											//等待拨号结果
		}//end if  gprs模式
}
unsigned char IsDisconn()
{
	unsigned char success = 0, success2 = 0;
	char AT_IPENTRANS_EXIT[]   = {"+++"};		
	char AT_SISC[] 	= {"AT+CIPCLOSE=0\r\n"};	
	if(1==CmdExe(AT_CMD,0))
	{
		return 1;
	}	
	else if(1==CmdExe(AT_CMD,0))
	{
		return 1;
	}
	else	
	{
		//测试 +++ 命令
		success = CmdExe(AT_IPENTRANS_EXIT,4);			//5秒退出
		if(success==1)		//成功
		{		
			//	m_gprsinfo.g_bGPRSConnected = 0;
			//	Led_Ctrl(0,false);
				success2 = CmdExe(AT_CMD,0);			//目的为了发\r\n
				delay_1ms(20);
				success = CmdExe("ATO\r\n",0);							
				if(success==0)
				{
						InsertLog("ResetGPRS:AT_IPENTRANS_EXIT command success ,but ATO command faile!\r\n");
						ResetGPRS();									//gprs								
						return 1;											//不成功  考虑重启设备
				}		
        InsertLog("re conn trans: success !\r\n");				
				return 0;  
		}
		else							//不成功
		{
				InsertLog("AT_IPENTRANS_EXIT command failed!\r\n");				
				//m_gprsinfo.g_bGPRSConnected = 0;
				//Led_Ctrl(0,false);
				success2 = CmdExe(AT_CMD,0);			//目的为了发\r\n
				delay_1ms(20);
				success = CmdExe(AT_SISC,5);			//断开连接最长可以等3秒
				//g_tranflag = 1;				
				if(success==0)
				{
						InsertLog("ResetGPRS:AT_IPENTRANS_EXIT & AT_SISC command faile!\r\n");
						ResetGPRS();									//gprs				
						return 1;											//不成功  考虑重启设备
				}			
			  InsertLog("ResetGPRS: success !\r\n");					
				return 0; 
		}				
	}
}
/* 
GprsDisConn功能描述,当m_gprsinfo.g_bGPRSConnected = 1 则对串口中收到的数据进行过滤，过滤的条件为是否包含 ^SIS: 0,0,48, 字符串，如果包含则g_gprs_discon=1
注意此处没有对m_gprsinfo.g_bGPRSConnected 进行改变。
*/
int GprsDisConn(void)													//gprs是否处于断开状态 1=表示断开，0=表示其他。  修改g_gprs_discon=1,但不修改 m_gprsinfo.g_bGPRSConnected=1则WaitForResult一定不能被执行。
{
	//问题2.  gprs.c --> GprsDisConn  中需要 CLOSED\r\n  进一步判断是否断开了4G。可能上位机发送的了 CLOSED\r\n数据
	int dw = 0,i=0,cur=0,cnt=0;
	unsigned char temp[17];
	#if MODULE_4G				//NO CARRIER  ; sim7600 CLOSED
	if(m_gprsinfo.g_bGPRSConnected)							//当gprs处于连接状态  在UART4_IRQHandler中也加了该判断,当处于连接状态  m_gprsinfo.g_bGPRSConnected=1则Dailer_Pre
	{
			//进行判断^SIS: 0,0,48,			至少15个字节
			while(RSGPRS_rec_WR!=RSGPRS_rec_RD)
			{
					if(RSGPRS_buff[RSGPRS_rec_RD]=='N' || RSGPRS_buff[RSGPRS_rec_RD]=='C')		//找到第1个字符
					{
							dw = (RSGPRS_REC_BUFF_SIZE + RSGPRS_rec_WR - RSGPRS_rec_RD )%RSGPRS_REC_BUFF_SIZE;		
							if(dw>7)			//对格式进行判断9-->
							{
									for(i=0;i<10 && i < dw;i++)
									{
											cur = (RSGPRS_rec_RD+i)%RSGPRS_REC_BUFF_SIZE;
											temp[i] = RSGPRS_buff[cur];									
									}
									temp[i++]='\r';		
									temp[i++]='\n';		
									temp[i]='\0';		
									InsertLog((char*)temp);									
									
									if(strstr((char*)temp,"NO CARRIER"))				//至少1个，判断出gprs已断开
									{
											RSGPRS_rec_RD = (RSGPRS_rec_RD + dw)%RSGPRS_REC_BUFF_SIZE;
											g_gprs_discon = 1;
											Led_Ctrl(0,false);
											InsertLog("4g disconn !\r\n");
											return 1;
									}
									else if(strstr((char*)temp,"CLOSED\r\n"))			//至少1个，判断出gprs已断开
									{
										RSGPRS_rec_RD = (RSGPRS_rec_RD + dw)%RSGPRS_REC_BUFF_SIZE;
										if(1==IsDisconn())
										{											
											g_gprs_discon = 1;
											Led_Ctrl(0,false);
											InsertLog("4g disconn !\r\n");
											return 1;
										}
									}
									else
									{
											RSGPRS_rec_RD = (RSGPRS_rec_RD + 1)%RSGPRS_REC_BUFF_SIZE;
											InsertLog("error discon!\r\n");
									}
							}
							else							//bug 否则 当收到^字符并且字符个数<15,并且后面没有数据在来的情况下。则程序死机
							{
									break;
							}
					}
					else
					{
							RSGPRS_rec_RD = (RSGPRS_rec_RD+1)%RSGPRS_REC_BUFF_SIZE;
					}
			}
	}
	return 0;
	#else	
	if(m_gprsinfo.g_bGPRSConnected)				//当gprs处于连接状态  在UART4_IRQHandler中也加了该判断,当处于连接状态  m_gprsinfo.g_bGPRSConnected=1则Dailer_Pre
	{
			//进行判断^SIS: 0,0,48,			至少15个字节
			while(RSGPRS_rec_WR!=RSGPRS_rec_RD)
			{
					if(RSGPRS_buff[RSGPRS_rec_RD]=='^')		//找到第1个字符
					{
							dw = (RSGPRS_REC_BUFF_SIZE + RSGPRS_rec_WR - RSGPRS_rec_RD )%RSGPRS_REC_BUFF_SIZE;		
							if(dw>14)			//对格式进行判断
							{
									for(i=0;i<14;i++)
									{
											cur = (RSGPRS_rec_RD+i)%RSGPRS_REC_BUFF_SIZE;
											temp[i] = RSGPRS_buff[cur];											
											if(temp[i]==',')
											{
													cnt++;
											}
									}
									temp[i++]='\r';		
									temp[i++]='\n';		
									temp[i]='\0';		
									InsertLog(temp);
									if(cnt&&temp[1]=='S'&&temp[2]=='I'&&temp[3]=='S'&&temp[4]==':')			//至少1个，判断出gprs已断开
									{
											RSGPRS_rec_RD = (RSGPRS_rec_RD + dw)%RSGPRS_REC_BUFF_SIZE;
											g_gprs_discon = 1;
											InsertLog("disconn !\r\n");
											return 1;
									}
									else
									{
											RSGPRS_rec_RD = (RSGPRS_rec_RD + 1)%RSGPRS_REC_BUFF_SIZE;
											InsertLog("error discon!\r\n");
									}
							}
							else		//bug 否则 当收到^字符并且字符个数<15,并且后面没有数据在来的情况下。则程序死机
							{
									break;
							}
					}
					else
					{
							RSGPRS_rec_RD = (RSGPRS_rec_RD+1)%RSGPRS_REC_BUFF_SIZE;
					}
			}
	}
	return 0;
	#endif
}

//gprs参数初始化
void Gprs_Para_Init()										//如果first=1表示开机第1次。
{
		memset(m_siminfo.ip_address,0,sizeof(m_siminfo.ip_address));					//tcp连接成功的ip
		m_siminfo.port_number=0;							//tcp连接成功的port
//		m_siminfo.gsm_reset_cnt=0;						//gsm模块重启次数
//		m_siminfo.gsm_reset_time=0;						//模块最后重启时间，如果为0则表示系统开始工作的时间。
		m_siminfo.gsm_reset_reson=0;					//gsm模块重启原因
		m_siminfo.sms_success=0;							//成功发送短信条数
//		m_siminfo.sms_faile=0;								//发送失败短信条数
//		m_siminfo.sms_lastsuc_faile=0;				//最后成功到目前发送失败的次数;		Z
//		m_siminfo.sms_last_faile_time=0;			//最后1次失败时间					Z
		m_siminfo.sms_last_faile_reson=0;			//最后1次失败原因					Z
		
		m_siminfo.bComm=false;										//与模块是否可以通讯即CPU向mg301发命令是否有回应。 
		m_siminfo.CommFailCnt=0;									//连续通讯失败的次数
		m_siminfo.FailStartTime=0;								//失败开始时刻
		m_siminfo.singal=0;												//信号强度
		m_siminfo.bExist=false;										//手机卡是否存在
		memset(m_siminfo.imsi,0,sizeof(m_siminfo.imsi));									//最多15字节IMSI			
		memset(m_siminfo.iccid,0,sizeof(m_siminfo.iccid));								//20字节iccid		
		m_siminfo.service=0;											//服务商 0:未检测到运营商信息  1:联通  2:移动 （暂时只支持这两个）
		m_siminfo.bRegGSM=false;									//GSM网络注册成功否
		m_siminfo.bRegNet=false;									//GPRS网络注册是否成功		
		//以下与短信有关
		m_siminfo.bSmsPDU=false;																			//PDU工作模式设置成功否
		memset(m_siminfo.strCSCA,0,sizeof(m_siminfo.strCSCA));				//短信中心电话号码
		memset(m_siminfo.phonenum,0,sizeof(m_siminfo.phonenum));			//本机手机号码  暂时无用
		
		
		//gprs部分
	//	m_gprsinfo.iGprsDisConnCnt=0;						//gprs断开次数
		m_gprsinfo.g_bGPRSConnected=0;						//GPRS连接状态	
		Led_Ctrl(0,false);
		//m_gprsinfo.gprs_last_succ_time=0;			//gprs最后连接成功时间
		m_gprsinfo.gprs_last_fail_time=0;				//gprs最后连接失败时间
		m_gprsinfo.gprs_last_faile_reason=0;		//gprs最后连接失败的原因			Z
		m_gprsinfo.gprs_lastsuc_faile=0;				//gprs最后连接成功到目前失败次数	Z
//	m_gprsinfo.gprs_success=0;							//成功连接gprs的次数
		m_gprsinfo.gprs_faile=0;								//连接gprs失败的次数
		//tcp部分
			
		//数据发送部分		
			m_gprsinfo.dat_last_succ_time=0;				//最后发送成功时间
//		m_gprsinfo.dat_last_fail_time=0;				//最后发送失败时间	
//		m_gprsinfo.dat_last_fail_reson=0;				//最后发送失败的原因
//		m_gprsinfo.dat_lastsuc_faile=0;					//最后发送成功到目前失败次数
//		m_gprsinfo.dat_success=0;								//发送成功次数
//		m_gprsinfo.dat_faile=0;									//发送不成功次数
}
#if 0
int Get_ICCID(void)
{
	int		res = 0;
	unsigned int	dw=0;
	
	m_siminfo.bComm=false;			//与模块是否可以通讯即CPU向mg301发命令是否有回应。 
	m_siminfo.singal=0;
	m_siminfo.bExist=false;			//手机卡是否存在
	m_siminfo.service=0;				//ԵʼۯδݬӢսՋӪʌ	
	m_siminfo.bRegGSM=false;	
	m_siminfo.bRegNet=false;	
	
	//ߟѸؔ֯טǴϵͳۍģࠩքŜf
	//˵ķ,ձ̹Ԑּԉ٦Ӆ̣ߟѸ͵ݾ,رղɎێәطּˇûԐӢӥքcիהԚַ˔4˵ìƐ׏ʨѸڊ֏ˇԐӢӥքc
	//ģࠩˇرࠉӔͨѶ->˖ܺߨˇرզ՚->хۅ׊->עӡθç		ۏ݆4ٶҤ
	
	if(g_configRead.b_gprs_work==1)
	{
		//**************************1  ģࠩˇرࠉӔͨѶ  bComm  CommFailCnt ***********************************
		if(!CmdExe(AT_CMD,0))							//ATļ®		Ɛ׏ģࠩˇرֽӣҢȒࠉӔͨѶ			
		{
			if(m_siminfo.CommFailCnt<1)			//0ܲ1
			{
					m_siminfo.FailStartTime = g_sysTick;		//失败开始时间，第1次
			}
			m_siminfo.CommFailCnt++;				//ͨѶʧќքՎ˽			
			if(m_siminfo.CommFailCnt>9 && (g_sysTick-m_siminfo.FailStartTime)>900)					//连续20次通讯失败，则至少过去了15min    1次至少需要1.5min
			{
					m_siminfo.CommFailCnt = 0;	
					m_siminfo.FailStartTime = g_sysTick;					//ͨѶʧќߪʼʱࠌ						
					m_siminfo.gsm_reset_reson = 2;								//gprsģʽ,20ՎӔʏһŜͨѶҢȒӬڽ60ī						
					InsertLog("ResetGPRS:AT_CMD faile & cnt > 9 & timeout > 15min \r\n");
					ResetGPRS();							
			}
			return -10;  
		}
		m_siminfo.bComm=true;						//ͨѶԉ٦
		m_siminfo.CommFailCnt=0;				//ͨѶʧќքՎ˽
		if(!CmdExe(ATE_CMD,0))					//ژҕܘД		Ɛ׏ģࠩˇرֽӣҢȒࠉӔͨѶ
		{
			InsertLog("ATE command failed!\r\n");				
		}
		
		if(CmdExe(AT_CSQ,0)==1)					//хۅ׊			Ɛ׏хۅǿ׈ɧێ
		{
				GetCSQ();										//ܱȡхۅǿ׈			
				if(m_siminfo.singal<10)			//хۅǿ׈Ɛ׏
				{
								
				}			
		}
		else
		{
				InsertLog("AT_CSQ command failed!\r\n");	
				return -12;
		}
		
		//**************************2  ˖ܺߨˇرզ՚  bExist  iccid   phonenum   *****************************	
		if(CmdExe(AT_CIMI,2)==1)			//IMSI			Ɛ׏˖ܺߨˇرզ՚
		{
				GetIMSI();
				m_siminfo.bExist=true;	
		}
		else
		{
			if(CmdExe(AT_CIMI,2)==1)			//IMSI			Ɛ׏˖ܺߨˇرզ՚
			{
				GetIMSI();
				m_siminfo.bExist=true;	
			}
			else
			{
				InsertLog("AT_CIMI command failed!\r\n");	
				//return -11;
			}
		} 
		if(CmdExe(AT_ICCID,2)==1)		//iccid				Ɛ׏˖ܺքiccid mg323һ֧Ԗ
		{
				GetICCID();							//ׁiccidۅë	Փ':'ߪʼ۳ĦlѸ20ٶ؇ࠕٱؖػ
		}
		else
		{
				InsertLog("AT_ICCID command failed!\r\n");		
		}		
	}
	return 0;
}
#else
int Get_ICCID(void)
{
	if(g_configRead.b_gprs_work==1)
	{
		//**************************1  ģࠩˇرࠉӔͨѶ  bComm  CommFailCnt ***********************************
		if(CmdExe(AT_ICCID,2)==1)		//iccid				Ɛ׏˖ܺքiccid mg323һ֧Ԗ
		{
				GetICCID();							//ׁiccidۅë	Փ':'ߪʼ۳ĦlѸ20ٶ؇ࠕٱؖػ
		}
		else
		{
				InsertLog("AT_ICCID command failed!\r\n");		
		}		
	}
	return 0;
}
#endif

/**********************************************************************
  * @ 函数名  ： LED_Task
  * @ 功能说明： LED_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
void G4_Task(void* parameter)
{	
   while (1)
   {
		 if(1==g_configRead.b_gprs_work)
		 {
				if(m_gprsinfo.g_bGPRSConnected && g_gprs_discon==1)		//无数据发送并且发现了gprs已经断开则进行标志gprs断开  g_gprs_discon 通过串口监测到的,但g_bGPRSConnected中认为gprs还处于连接中，并且还在操作中〿
				{
					g_gprs_discon = 0;
					m_gprsinfo.g_bGPRSConnected = 0;						//gprs断开
					InsertLog("g_gprs_discon=1,g_TxBufSendCnt=0,g_bGPRSConnected=1 disconn!\r\n");
				}		
				if(g_sysTick>10)			//g_gprs_work    && (G4+1)==g_configRead.wifi_mode 
				{	
					//判断是否断开了连接,如果断开,则标记。(主动断开，例如服务器关闭)。
					wdt();
					GprsDisConn();						//=1表示判断出断开。通过检测gprs口 ^SIS:来判断  g_gprs_discon  中断中接收数据 作肯定回答。当m_gprsinfo.g_bGPRSConnected = 1 则对串口中收到的数据进行过滤，过滤的条件为是否包含 ^SIS: 0,0,48, 字符串，如果包含则g_gprs_discon=1
					wdt();
					GPRSThread();							//拨号过程中无需判断是否断开(通过字符,关闭数据接收开关),在拨号前需要判断,gprs_Tcp是否处于发送状态中，如果已发送完，则进行拨号，否则不能拨号。否则等待，直到发送完毕。
					wdt();
					//GPRS_Tcp();							//在连接的状态下，如果多次发送数据不成功（可能原因 服务器实际断开;服务器软件不给应答）;当g_gprs_discon=1并且gprs连接状态下并且无数据发送的时候，则标志断开gprs。		进行数据补传
			
					//wdt();
					//gprs_tcp_cfg();					//处理rj45接收到的数据 tcp数据通道  进行参数配置
					//wdt();
				}	
			}			
      vTaskDelay(100);   												/* 延时500个tick */		 		       
    }
}

//NO CARRIER  当主动断开gprs的时候。
//设置透传模式，将wifi作为主口，可以通过wifi远程控制设备。
