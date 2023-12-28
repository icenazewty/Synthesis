#ifndef GPRS_H
#define GPRS_H

#define	  COUNT_MAX_GPRS	16				//16通道 10min 1min/条	200*92	
#define 	COUNT_MAX_GSM 	8					
#define		COUNT_MAX_TEL		8					
/*模块初始化返回结果*/
#define 	GPRS_ERROR_OK 0
#define 	GPRS_ERROR_COM_N -1		//串口错误
#define 	GPRS_ERROR_COM_BAUND -2 //模块工作不正常
#define 	GPRS_ERROR_COM_SIM   -3 //SIM卡不存在
#define 	GPRS_ERROR_COM_NOREG -4 //SIM没有注册到网络
#define 	GPRS_ERROR_COM_SINGAL -8 //SIM没有注册到网络
/////////////////////////////////////////////////////////////////////////////
// CGprs window
	/*SIM卡信息*/

#define NOPARITY            0
#define ONESTOPBIT          0
#define CR_ 13			//回车 
#define LF 10			//换行

// 用户信息编码方式
#define GSM_7BIT		0
#define GSM_8BIT		4
#define GSM_UCS2		8

// 应答状态
#define GSM_WAIT		0		// 等待，不确定
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

//和短信有关的边变量和函数
	int  OpenGSM(void);
	int  SendSMS(char* PhoneNum,char* Conten,int len);
	void Gprs_Para_Init();	
	void InsertLog(char *str);
  void G4_Task(void* pvParameters);				/* LED2_Task浠诲姟瀹炵幇 */
#endif
