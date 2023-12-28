#ifndef GPRS_H
#define GPRS_H

#define	  COUNT_MAX_GPRS	16				//16ͨ�� 10min 1min/��	200*92	
#define 	COUNT_MAX_GSM 	8					
#define		COUNT_MAX_TEL		8					
/*ģ���ʼ�����ؽ��*/
#define 	GPRS_ERROR_OK 0
#define 	GPRS_ERROR_COM_N -1		//���ڴ���
#define 	GPRS_ERROR_COM_BAUND -2 //ģ�鹤��������
#define 	GPRS_ERROR_COM_SIM   -3 //SIM��������
#define 	GPRS_ERROR_COM_NOREG -4 //SIMû��ע�ᵽ����
#define 	GPRS_ERROR_COM_SINGAL -8 //SIMû��ע�ᵽ����
/////////////////////////////////////////////////////////////////////////////
// CGprs window
	/*SIM����Ϣ*/

#define NOPARITY            0
#define ONESTOPBIT          0
#define CR_ 13			//�س� 
#define LF 10			//����

// �û���Ϣ���뷽ʽ
#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS2		8

// Ӧ��״̬
#define GSM_WAIT		0		// �ȴ�����ȷ��
#define GSM_OK			1		// OK 
#define GSM_ERR			-1		// ERROR


	//void CGprs(void);
	
	//bool OpenGPRSMD(char* ip_address,int port_number);
	
	//bool SendGPRS(char* gprs,int len,unsigned int timeout_sec=0);
	
	//bool SendMsgNonBlock(char* msg,int lenMsg, char* telp,unsigned int timeout_sec=0); 
	
	//void SetIpPort(char *ip,int port);
	//int  timeout_sec_default;
	
	int  CheckMG323Status(void);
	int  CmdExe(char*cmd,int mode,unsigned char Para_ucType_Length_Cal,unsigned short Para_usLength);	
	int  GPRS_Soft_OK();
	void ResetGPRS();
	int  Dailer();
	int  Dailer_Pre();
	//int ConnectServer(int workmode);
	//void StartGPRSThread();
	void GPRSThread();
	//int GetGprsIPAddress();
	void GetICCID(void);
	void GetCSQ(void);
	void GetREG(int ch);
	void GetCOPS();
	unsigned char  GetCSCA();

	//int RecvData(char * buf , int len);
	int MatchRecData(char*ack , int len);
	//void ClearReciveSocket();		
	int GprsDisConn(void);

//�Ͷ����йصı߱����ͺ���
	int  OpenGSM(void);
	int  SendSMS(char* PhoneNum,char* Conten,int len);
	void Gprs_Para_Init();	
	void InsertLog(char *str);
  void G4_Task(void* pvParameters);				/* LED2_Task任务实现 */
#endif
