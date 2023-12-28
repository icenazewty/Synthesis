#include "MQTT.h"
#include "GlobalVar.h"
#include "string.h"
#include "gprs.h"
#include "spi_flash.h"
#include "comm_process.h"
#include "W5500.h"
#include "prj.h"

//OLD_MDOE 定义在prj.h中

extern unsigned char S_State[8] ;
unsigned char  g_mqtt_work_mode = 0;
unsigned char  ucMQTT_TxBuf[1024];
unsigned short usMQTT_Tx_Index;
unsigned int   uiMQTT_Send_Tim=0;
unsigned int   uiMQTT_Con_Send_Tim=0;
unsigned int   uiMQTT_Timing=0;//授时

volatile unsigned short g_tristart_mppt_Setup_Register_Addr[5][2]={
	{0x0000,0xE000},//0
	{0x0001,0xE001},//1
	{0x0002,0xE007},//2
	{0x0003,0xE011},//3
	{0x0004,0xE012} //4
};

volatile unsigned short g_Inverter_Setup_Register_Addr[3][2]={
	{0x0000,0xE003},//0
	{0x0001,0xE004},//1
	{0x0001,0xE005},//2
};

volatile unsigned short g_Air_Setup_Register_Addr[6][2]={
	{0x0000,0x0016},//0
	{0x0001,0x0017},//1
	{0x0002,0x0028},//2
	{0x0003,0x0029},//3
	{0x0004,0x0038},//4
	{0x0005,0x0039} //5
};

//---------- W25Q32 ---------------------------------------------------------------
//1、容量
//   32M-Bit/4M-byte(4,194,304)
//2、存储结构
//   页：256-bytes,16384页(4,194,304/256)
//   扇区：4K-bytes,1024扇区(4,194,304/(4*1024))
//   块：64K-bytes,64块(4,194,304/(64*1024))
//---------------------------------------------------------------------------------
void SaveToW25QXX(unsigned int _Flash_Base,unsigned char *pstr,int num,int nStartAddressOffset){
	int i;
	unsigned char tucArray[4096];//1个扇区4K最大存储的数据的个数
	unsigned char tucBuf[256];
	if(num==0){
		return;
	}
	
	SPI_FLASH_BufferRead(tucArray,_Flash_Base,4096);
	SPI_FLASH_SectorErase(_Flash_Base);
	for(i=0;i<num;i++){
		tucArray[nStartAddressOffset+i] = *(pstr+i);
		tucArray[nStartAddressOffset+i+2048] = (~tucArray[nStartAddressOffset+i])&0x0ff;
	}
	for(i=0;i<16;i++){//1个扇区4K，每次写256个byte，写16次
		memcpy(tucBuf,&tucArray[i*256],256);
		SPI_FLASH_BufferWrite(tucBuf,_Flash_Base+i*256,256);
	}
}
unsigned char xxxxxxxxx1=0;
unsigned char yyyyyyyyy1=0;
int	ReadFromW25QXX(unsigned int _Flash_Base,unsigned char *pucOutData,int nNum,int nStartAddressOffset){
	int i;
	unsigned char tucArray[4096];//1个扇区4K最大存储的数据的个数
	if(nNum==0){
		return 0;
	}	
	
	SPI_FLASH_BufferRead(tucArray,_Flash_Base,4096);
	if( (tucArray[0]==0xFF)&&(tucArray[1]==0xFF)&&(tucArray[2]==0xFF)&&(tucArray[3]==0xFF)&&(tucArray[4]==0xFF)&&(tucArray[5]==0xFF)){
		return 0;//没写过,是新的扇区
	}
	for(i=0;i<nNum;i++){
		xxxxxxxxx1=tucArray[nStartAddressOffset+i];
		yyyyyyyyy1=(~tucArray[2048+nStartAddressOffset+i]);
//		if(tucArray[nStartAddressOffset+i] == (~tucArray[1024+nStartAddressOffset+i]) ){
		if(xxxxxxxxx1==yyyyyyyyy1){
			pucOutData[i]=tucArray[nStartAddressOffset+i];
		}else{
			return 0;
		}
	}
	return 1;
}
//-------------------- MQTT上传设备总数读写 -----------------------------------------------------------------------------------
//void Mqtt_Upload_Device_Total_SaveToW25QXX(void){
//	unsigned char buf[2];
////	g_Result.ucTotal_Upload_Device=9;
//	buf[0]=g_MQTT.ucTotal_Upload_Device;
//	SaveToW25QXX(D_W25QXX_FLASH_MQTT_UPLOAD_DEVICE_TOTAL_ADDR,buf,1,0x0000);
//}
void Mqtt_Upload_Device_Total_ReadFromW25QXX(void)
{
//	unsigned char buf[2];
//	if(ReadFromW25QXX(D_W25QXX_FLASH_MQTT_UPLOAD_DEVICE_TOTAL_ADDR,buf,1,0x0000))
//	{
//		g_MQTT.ucTotal_Upload_Device=buf[0];
//	}
//	else
//	{
//		g_MQTT.ucTotal_Upload_Device=10;
//	}
		g_MQTT.ucTotal_Upload_Device=12;
}
//-------------------- MQTT上传设备ID的读写 -----------------------------------------------------------------------------------
//void Mqtt_Upload_Device_ID_SaveToW25QXX(void){
//	unsigned char buf[32*12];
//	unsigned char i;
//	for(i=0;i<g_MQTT.ucTotal_Upload_Device;i++)
//	{
//		memcpy(&buf[i*12],g_MQTT.ucID_Upload_Device_Buf[i],12);
//	}
//	SaveToW25QXX(D_W25QXX_FLASH_MQTT_UPLOAD_DEVICE_ID_ADDR,buf,g_MQTT.ucTotal_Upload_Device*12,0x0000);
//}
void Mqtt_Upload_Device_ID_ReadFromW25QXX(void)
{
//	unsigned char buf[32*12],i;
//	if(ReadFromW25QXX(D_W25QXX_FLASH_MQTT_UPLOAD_DEVICE_ID_ADDR,buf,g_MQTT.ucTotal_Upload_Device*12,0x0000))
//	{
//		for(i=0;i<g_MQTT.ucTotal_Upload_Device;i++)
//		{
//			memcpy(g_MQTT.ucID_Upload_Device_Buf[i],&buf[i*12],12);
//		}
//	}
//	else
	{
		int i = 0;
		char tmp[12] = {0,0,0,0,0,'0','0','0','0',0,0,0};
		tmp[0] = g_configRead.device_ID[1];
		tmp[1] = g_configRead.device_ID[2];
		tmp[2] = g_configRead.device_ID[3];
		tmp[3] = g_configRead.device_ID[4];
		tmp[4] = g_configRead.device_ID[5];
//		tmp[5] = g_configRead.device_ID[5];	
//		g_Result.device_ID  			= atoi(tmp);
		tmp[9] = '0';		tmp[10] = '0';		tmp[11] = '0';		
		memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//系统控制器 		实时读数据
		
		tmp[9] = '0';		tmp[10] = '0';		tmp[11] = '1';
		memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//系统控制器		日报
		
		tmp[9] = '0';		tmp[10] = '0';		tmp[11] = '2';
		memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//系统控制器		设置参数
		
		tmp[9] = '0';
		for(int j=0;j<g_configRead.alarmDelyMinute;j++)
		{			
			tmp[10] = 8+j;
			if(tmp[10]>9)
				tmp[10] = tmp[10] - 10 + 'A';
			else
				tmp[10] = tmp[10] + '0';
			
			tmp[11] = '0';			
			memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//Tristar MPPT1 第1包
		
			tmp[11] = '1';
			memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//Tristar MPPT1 第2包
		}
		
		tmp[9] = '2';		tmp[10] = '8';		tmp[11] = '0';
		memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//空调
		
		tmp[9] = '3';		tmp[10] = '0';		tmp[11] = '0';
		memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//逆变器
		
		tmp[9] = '3';		tmp[10] = '8';		tmp[11] = '0';
		memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//整流器 第1包
		
		tmp[9] = '3';		tmp[10] = '8';		tmp[11] = '1';
		memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//整流器 第2包
		
		tmp[9] = '4';		tmp[10] = '0';		tmp[11] = '0';
		memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//电池
		
		tmp[9] = '4';		tmp[10] = '8';		tmp[11] = '0';
		memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//柴油发电机 第1包
		
		tmp[9] = '4';		tmp[10] = '8';		tmp[11] = '1';
		memcpy(g_MQTT.ucID_Upload_Device_Buf[i++],tmp,12);//柴油发电机 第2包
		
		g_MQTT.ucTotal_Upload_Device=i;
	}
}
//-------------------- 上传时间间隔的读写 -------------------------------------------------------------------------------------
//void Mqtt_Upload_Time_Interval_SaveToW25QXX(void)
//{
//	unsigned char buf[2];
//	buf[0]=(g_MQTT.usTm_Interval_Upload >> 8) & 0xFF;
//	buf[1]=g_MQTT.usTm_Interval_Upload  & 0xFF;
//	SaveToW25QXX(D_W25QXX_FLASH_MQTT_UPLOAD_TIME_INTERVAL_ADDR,buf,2,0x0000);
//}
//void Mqtt_Upload_Time_Interval_ReadFromW25QXX(void)
//{
//	unsigned char buf[2];
//	if(ReadFromW25QXX(D_W25QXX_FLASH_MQTT_UPLOAD_TIME_INTERVAL_ADDR,buf,2,0x0000))
//	{
//		g_MQTT.usTm_Interval_Upload=(buf[0]<<8) | buf[1];
//	}
//	else
//	{
//		g_MQTT.usTm_Interval_Upload=60*3;
//	}
//}
//-------------------- 心跳时间间隔的读写 -------------------------------------------------------------------------------------
//void Mqtt_PINGREQ_Time_Interval_SaveToW25QXX(void){
//	unsigned char buf[2];
//	buf[0]=(g_MQTT.usTm_Interval_PINGREQ >> 8) & 0xFF;
//	buf[1]=g_MQTT.usTm_Interval_PINGREQ  & 0xFF;
//	SaveToW25QXX(D_W25QXX_FLASH_MQTT_PINGREQ_TIME_INTERVAL_ADDR,buf,2,0x0000);
//}
void Mqtt_PINGREQ_Time_Interval_ReadFromW25QXX(void)
{
//	unsigned char buf[2];
//	if(ReadFromW25QXX(D_W25QXX_FLASH_MQTT_PINGREQ_TIME_INTERVAL_ADDR,buf,2,0x0000))
//	{
//		g_MQTT.usTm_Interval_PINGREQ=(buf[0]<<8) | buf[1];
//	}
//	else
//	{
//		g_MQTT.usTm_Interval_PINGREQ=60;
//	}
		g_MQTT.usTm_Interval_PINGREQ = 60;		//2分钟进行1次ping命令
		g_MQTT.usTm_Interval_Con     = 30;		//30秒尝试一次连接，避免频繁发送连接
		g_MQTT.uiRevTimeOutMs 			 = 5000;	//从发送到接收数据超时时间 ms  2sencond  systickCount
}
//-------------------- BCD转十进制 --------------------------------------------------------------------------------------------
unsigned char BCD_To_Dec(unsigned char Para_ucTemp){	
	return ( (Para_ucTemp/16)*10 +(Para_ucTemp&0x0F));
}
//-------------------- 延时函数 -----------------------------------------------------------------------------------------------
void DlymS(unsigned int uitd){ 
	unsigned long ult0 = systickCount;
	while(1){
		if(systickCount +(~ult0) +1 >= uitd){
			break;
		}
	}
}

void DlyS(unsigned int uitd){ 
	unsigned long ult0 = g_sysTick;
	while(1){
		if(g_sysTick +(~ult0) +1 >= uitd){
			break;
		}
	}
}
//----------------------------------------------------------------------------------------------------------------------
unsigned char OneAscii_to_Data(unsigned char ucHex){
	if(ucHex >= 'A' && ucHex <= 'F'){
		return ucHex-'A'+10;
	}else if(ucHex >= 'a' && ucHex <= 'f'){
		return ucHex-'a'+10;
	}else if (ucHex >= '0' && ucHex <= '9'){
		return ucHex-'0'+0;
	}
	return 0;
}
unsigned char TwoASCII_to_1ByteData(unsigned char ucH,unsigned char ucL){
	return (OneAscii_to_Data(ucH)<<4)	|	OneAscii_to_Data(ucL);
}

void StringToHex(char *str, unsigned char *strhex){
	uint8_t i,cnt=0;
	char *p = str;             //直针p初始化为指向str
	uint8_t len = strlen(str); //获取字符串中的字符个数
	
	while(*p != '\0') {        //结束符判断
		for (i = 0; i < len; i ++)  //循环判断当前字符是数字还是小写字符还是大写字母
		{
			if ((*p >= '0') && (*p <= '9')) //当前字符为数字0~9时
				strhex[cnt] = *p - '0' + 0x30;//转为十六进制
			
			if ((*p >= 'A') && (*p <= 'Z')) //当前字符为大写字母A~Z时
				strhex[cnt] = *p - 'A' + 0x41;//转为十六进制
			
			if ((*p >= 'a') && (*p <= 'z')) //当前字符为小写字母a~z时
				strhex[cnt] = *p - 'a' + 0x61;  //转为十六进制
		
			p ++;    //指向下一个字符
			cnt ++;  
		}
	}
}
//-------------------- MQTT的CRC ------------------------------------------------------------------------------------------------
unsigned char MQTT_CRC(unsigned char *pData,int nLen){
	unsigned short i;
	unsigned char crc=0;
	for(i=0;i<nLen;i++){
		if(i==0){
			crc = pData[i] ^ pData[++i];
		}else{
			crc = crc ^ pData[i];
		}
	}
	return crc;
}
//-------------------- 单精度浮点数和4字节互转 ----------------------------------------------------------------------------------
//void Float_to_Byte4(float f,unsigned char *byte){//将浮点数f转化为4个字节数据存放在byte[4]中
//	unsigned long longdata = 0;
//	longdata = *(unsigned long*)&f;//注意，会丢失精度
//	byte[0] = (longdata & 0xFF000000) >> 24;
//	byte[1] = (longdata & 0x00FF0000) >> 16;
//	byte[2] = (longdata & 0x0000FF00) >> 8;
//	byte[3] = (longdata & 0x000000FF);
////	return byte;
//}
//float Byte4_to_Float(unsigned char *p){//将4个字节数据byte[4]转化为浮点数存放在*f中
//	float float_data=0;
//	unsigned long longdata = 0;
//	longdata = (*p<< 24) + (*(p+1) << 16) + (*(p + 2) << 8) + (*(p + 3) << 0);
//	float_data = *(float*)&longdata;
//	return float_data;
//}
unsigned int float_to_int(float f){
	unsigned int intdata=0;
	intdata=*(unsigned long*)&f;//注意，会丢失精度
	return intdata;
}
float int_to_float(unsigned int Para_uiData){
	float float_data=0;
	float_data = *(float*)&Para_uiData;
	return (float_data*100+0.5)/100.0;
}
 
//-------------------- 剩余长度的编码和解码 -------------------------------------------------------------------------------------
unsigned char MQTT_Remain_Length_Encode(unsigned char *Para_ucBuf,unsigned int Para_uiLength){
	unsigned char tucCount=0;
	do{
		unsigned char tucData=Para_uiLength%128;
		Para_uiLength/=128;
		if(Para_uiLength>0){
			tucData|=0x80;
		}
		Para_ucBuf[tucCount++]=tucData;
	}while(Para_uiLength>0);
	return tucCount;
}
unsigned int MQTT_Remain_Length_Decode(unsigned char *Para_ucBuf){
	unsigned int multiplier=1;
	unsigned int value=0;
	unsigned char ucCount=0;
	do{
		value+=(Para_ucBuf[ucCount] & 127)*multiplier;
		ucCount++;
		multiplier*=128;
	}while((Para_ucBuf[ucCount-1] & 128) !=0);
	return value;
}
//-------------------- 剩余长度的编码和解码 -------------------------------------------------------------------------------------
//Para_ucMesType: 消息类型
//Para_ucDupFlag: 重发标志,0:客户端或服务端第一次请求发送这个报文；1:可能是一个早前报文请求的重发
//Para_ucQosLevel:Qos等级
//Para_ucRetain:  保留标志(设置后在订阅了该主题后马上接收到一条该主题的信息),0: 不发布保留消息；1: 发布保留消息
unsigned char GetDataFixedHead(unsigned char Para_ucMesType, unsigned char Para_ucDupFlag, unsigned char Para_ucQosLevel, unsigned char Para_ucRetain){
	unsigned char tucData = 0;
	
	tucData = (Para_ucMesType  & 0x0F) << 4;
	tucData|= (Para_ucDupFlag  & 0x01) << 3;
	tucData|= (Para_ucQosLevel & 0x03) << 1;
	tucData|= (Para_ucRetain   & 0x01);
	return tucData;
}
//-------------------- MQTT连接 -------------------------------------------------------------------------------------------------
//ucMQTT_TxBuf[usMQTT_Tx_Index] 		形成字符串
//4G send: 102900064D514973647003CA003C000C35303030313030303030303000047A65646100077A656461313233
//4G recv: 20020000
#if OLD_MDOE
unsigned char MQTT_Connect(
#else
void MQTT_Connect(
#endif
	char          *Para_cProtocol_Name,											//协议名
	unsigned char  Para_ucProtocol_Level,										//协议级别
	unsigned char  Para_ucConnect_Flag_User_Name_Bit7,  		//连接标志位用户名
	unsigned char  Para_ucConnect_Flag_Password_Bit6, 			//连接标志位密码
	unsigned char  Para_ucConnect_Flag_Will_Retain_Bit5,   	//连接标志位遗嘱保留
	unsigned char  Para_ucConnect_Flag_Will_QoS_Bit4_3,     //连接标志位遗嘱服务质量
	unsigned char  Para_ucConnect_Flag_Will_Flag_Bit2,     	//连接标志位遗嘱标志
	unsigned char  Para_ucConnect_Flag_Clean_Session_Bit1,	//连接标志位清理会话		
	unsigned short Para_usKeep_Alive,		 										//保持连接时间(心跳周期)	
	char           *Para_cClientID,                       	//客户端标识
	char           *Para_cUser_Name,                        //用户名
	char           *Para_cPassword                          //密码
	){
	unsigned char  tucBuf[1024]={0};
	unsigned short tusCount=0;
	unsigned char  tucFh_Remain_Length_Buf[4];//最长4个字节
	unsigned char  tucFh_Remain_Length_Byte_Num;//固定报头剩余长度字节数
	
	usMQTT_Tx_Index=0;
	//1、固定报头Fh
	ucMQTT_TxBuf[usMQTT_Tx_Index++]=GetDataFixedHead(E_MQTT_CONNECT,0,0,0);
	//2、可变报头Vh	
	//2.1	协议名(协议名长度+协议名)
	tucBuf[tusCount++]=(strlen(Para_cProtocol_Name)>>8) & 0xFF;//协议名长度
	tucBuf[tusCount++]=strlen(Para_cProtocol_Name) & 0xFF;
	memcpy(&tucBuf[tusCount],Para_cProtocol_Name,strlen(Para_cProtocol_Name));//协议名
	tusCount+=strlen(Para_cProtocol_Name);
	//2.2	协议级别
	tucBuf[tusCount++]=Para_ucProtocol_Level; //3
	//2.3	连接标志：要求标志位用户、 密码为 1， 服务质量为 1， 遗嘱为 0， 清理会话为 1，CA
	tucBuf[tusCount++]= 0 | (Para_ucConnect_Flag_Clean_Session_Bit1 << 1) 
												| (Para_ucConnect_Flag_Will_Flag_Bit2 << 2) 
												| (Para_ucConnect_Flag_Will_QoS_Bit4_3 << 3) 
												| (Para_ucConnect_Flag_Will_Retain_Bit5 << 5) 
												| (Para_ucConnect_Flag_Password_Bit6 << 6) 
												| (Para_ucConnect_Flag_User_Name_Bit7 << 7);	
	//2.4	保持连接时间(心跳周期)<=58秒	
	tucBuf[tusCount++]=(Para_usKeep_Alive>>8) & 0xFF;
	tucBuf[tusCount++]=Para_usKeep_Alive & 0xFF;
	//3、有效载荷
	//3.1	客户端标识(长度+客户端标识符)
	tucBuf[tusCount++]=(strlen(Para_cClientID)>>8) & 0xFF;
	tucBuf[tusCount++]=strlen(Para_cClientID) & 0xFF;
	memcpy(&tucBuf[tusCount],Para_cClientID,strlen(Para_cClientID));
	tusCount+=strlen(Para_cClientID);
	//3.2	用户名
	tucBuf[tusCount++]=(strlen(Para_cUser_Name)>>8) & 0xFF;
	tucBuf[tusCount++]=strlen(Para_cUser_Name) & 0xFF;
	memcpy(&tucBuf[tusCount],Para_cUser_Name,strlen(Para_cUser_Name));
	tusCount+=strlen(Para_cUser_Name);
	//3.3 密码
	tucBuf[tusCount++]=(strlen(Para_cPassword)>>8) & 0xFF;
	tucBuf[tusCount++]=strlen(Para_cPassword) & 0xFF;
	memcpy(&tucBuf[tusCount],Para_cPassword,strlen(Para_cPassword));
	tusCount+=strlen(Para_cPassword);
	//4、计算固定报头剩余长度
	tucFh_Remain_Length_Byte_Num=MQTT_Remain_Length_Encode(tucFh_Remain_Length_Buf,tusCount);
	memcpy(&ucMQTT_TxBuf[usMQTT_Tx_Index],tucFh_Remain_Length_Buf,tucFh_Remain_Length_Byte_Num);
	usMQTT_Tx_Index+=tucFh_Remain_Length_Byte_Num;
	memcpy(&ucMQTT_TxBuf[usMQTT_Tx_Index],tucBuf,tusCount);
	usMQTT_Tx_Index+=tusCount;
	ucMQTT_TxBuf[usMQTT_Tx_Index]= '\0';
	//5、发送	
	#if OLD_MDOE
	if(CmdExe((char *)ucMQTT_TxBuf,3,1,usMQTT_Tx_Index)==1)			//收20 02 00 00				//if(MQTT_Send(ucMQTT_TxBuf,usMQTT_Tx_Index))
	{				
		return 1;
	}
	else
	{
		return 0;
	}
	#endif
}
//-------------------- PUBLISH得到1个参数项数据 -----------------------------------------------------------------------------
unsigned short Get_Publish_Pl_One_Par_Item_Data(
		unsigned int   Para_uiPara_ID_SN,											//参数ID序号				10070000  01  0 0   01 0002 001E
		unsigned char  Para_ucData_Type,   										//数据类型      01  BYTE
		unsigned char  Para_ucType_Mark_H4Bit_RDOrWR,					//类型标记高4位，0只读、1读写、2只写 
		unsigned char  Para_ucType_Mark_L4Bit_HexOrAsciiOrUTF,//类型标记低4位，0为16进制、1为ASCII、2为UTF
		unsigned char  Para_ucData_Quality,										//数据品质，00:初始化数据、01:正常数据、02:不良数据、03:通讯故障
		unsigned short Para_usData_Length,                    //参数值长度，为几个字节    0002
		unsigned int   Para_uiData_Value,                     //参数值    001E
		char           *Para_ucStr,                           //字符串
		unsigned char  *Para_ucReturn_Data_Buf
	){
		unsigned short tusIndex=0;
		//参数ID序号
		Para_ucReturn_Data_Buf[tusIndex++]=(Para_uiPara_ID_SN>>24) &0xFF;
		Para_ucReturn_Data_Buf[tusIndex++]=(Para_uiPara_ID_SN>>16) &0xFF;
		Para_ucReturn_Data_Buf[tusIndex++]=(Para_uiPara_ID_SN>>8)  &0xFF;
		Para_ucReturn_Data_Buf[tusIndex++]= Para_uiPara_ID_SN      &0xFF;
		//数据类型
		Para_ucReturn_Data_Buf[tusIndex++]=Para_ucData_Type;
		//类型标记
		Para_ucReturn_Data_Buf[tusIndex++]=((Para_ucType_Mark_H4Bit_RDOrWR<<4)&0xF0) | (Para_ucType_Mark_L4Bit_HexOrAsciiOrUTF&0x0F);
		//数据品质
		Para_ucReturn_Data_Buf[tusIndex++]=Para_ucData_Quality;
		//参数值长度
		Para_ucReturn_Data_Buf[tusIndex++]=(Para_usData_Length>>8) &0xFF;
		Para_ucReturn_Data_Buf[tusIndex++]= Para_usData_Length     &0xFF;
		//参数值
		//目前应该只用到BYTE、DI、string、float数据类型
		if(Para_ucData_Type==E_DATA_TYPE_STRING){
			memcpy(&Para_ucReturn_Data_Buf[tusIndex],Para_ucStr,strlen(Para_ucStr));
			tusIndex+=strlen(Para_ucStr);
		}else{
			if(Para_usData_Length==1){
				Para_ucReturn_Data_Buf[tusIndex++]= Para_uiData_Value & 0xFF;
			}else if(Para_usData_Length==2){
				Para_ucReturn_Data_Buf[tusIndex++]=(Para_uiData_Value>>8)  & 0xFF;
				Para_ucReturn_Data_Buf[tusIndex++]= Para_uiData_Value      & 0xFF;
			}else if(Para_usData_Length==3){
				Para_ucReturn_Data_Buf[tusIndex++]=(Para_uiData_Value>>16) & 0xFF;
				Para_ucReturn_Data_Buf[tusIndex++]=(Para_uiData_Value>>8)  & 0xFF;
				Para_ucReturn_Data_Buf[tusIndex++]= Para_uiData_Value      & 0xFF;
			}else if(Para_usData_Length==4){
				Para_ucReturn_Data_Buf[tusIndex++]=(Para_uiData_Value>>24) & 0xFF;
				Para_ucReturn_Data_Buf[tusIndex++]=(Para_uiData_Value>>16) & 0xFF;
				Para_ucReturn_Data_Buf[tusIndex++]=(Para_uiData_Value>>8)  & 0xFF;
				Para_ucReturn_Data_Buf[tusIndex++]= Para_uiData_Value      & 0xFF;
			}
		}
		return tusIndex;
}
//-------------------- PUBLISH -------------------------------------------------------------------------------------------------
//result : ucMQTT_TxBuf[usMQTT_Tx_Index]	 string
#if OLD_MDOE
unsigned char MQTT_PUBLISH(
#else
void MQTT_PUBLISH(
#endif
		
	unsigned char  Para_ucFh_DUP,																			//固定报头的重发标志
	unsigned char  Para_ucFh_QoS,	  																	//固定报头的服务质量等级
	unsigned char  Para_ucFh_Retain,																	//固定报头的保留标志
		
	char           *Vh_Topic,       																	//可变报头的主题名
	char           *Vh_Message_Body_Type,   													//可变报头的消息体类型
	unsigned short Para_usVh_Packet_Identifier,												//可变报头的报文标识符
		
	unsigned char  Para_ucPl_Time_Year,         											//有效载荷时间的年
	unsigned char  Para_ucPl_Time_Month,        											//有效载荷时间的月
	unsigned char  Para_ucPl_Time_Day,          											//有效载荷时间的日
	unsigned char  Para_ucPl_Time_Hour,         											//有效载荷时间的时
	unsigned char  Para_ucPl_Time_Minute,       											//有效载荷时间的分
	unsigned char  Para_ucPl_Time_Second,      											 	//有效载荷时间的秒
	unsigned short Para_usPl_Message_Body_Properties_Data_Encryption,	//有效载荷的消息体属性的数据加密
	unsigned short Para_usPl_Message_Body_Properties_Data_Compress		//有效载荷的消息体属性的数据压缩
//	unsigned short Para_usPl_Para_Item_Total                          //有效载荷的参数项总数
	){
	unsigned char  tucBuf[1024]={0};
	unsigned short tusCount=0;
	unsigned short tusPos_Pl_Para_Item_Total_Num=0;
	unsigned short tusIndex_Pl_Para_Item=1;
	unsigned char  tucFh_Remain_Length_Buf[4];//最长4个字节
	unsigned char  tucFh_Remain_Length_Byte_Num;//固定报头剩余长度字节数	
	unsigned char  tucRet_Len,tucRet_Buf[50]={0};
	unsigned int   tuiSN=0;
					 char  tcStr_Buf[32]={0};
	unsigned short tusIndex_Count=0;
	int i,j;

	usMQTT_Tx_Index=0;
	//1、固定报头Fh
	ucMQTT_TxBuf[usMQTT_Tx_Index++]=GetDataFixedHead(E_MQTT_PUBLISH,Para_ucFh_DUP,Para_ucFh_QoS,Para_ucFh_Retain);
	//2、可变报头Vh	
	//2.1	主题名(主题长度+主题)
	tucBuf[tusCount++]=((strlen(Vh_Topic)+strlen(Vh_Message_Body_Type)+1)>>8) & 0xFF;//+1是/
	tucBuf[tusCount++]=(strlen(Vh_Topic)+strlen(Vh_Message_Body_Type)+1) & 0xFF;
	memcpy(&tucBuf[tusCount],Vh_Topic,strlen(Vh_Topic));
	tusCount+=strlen(Vh_Topic);
	tucBuf[tusCount++]='/';	
	memcpy(&tucBuf[tusCount],Vh_Message_Body_Type,strlen(Vh_Message_Body_Type));
	tusCount+=strlen(Vh_Message_Body_Type);
	//2.2	报文标识符
	tucBuf[tusCount++]=(Para_usVh_Packet_Identifier>>8) & 0xFF;
	tucBuf[tusCount++]=Para_usVh_Packet_Identifier      & 0xFF;
	//3、有效载荷
	//3.1 时间
	tucBuf[tusCount++]=Para_ucPl_Time_Year;
	tucBuf[tusCount++]=Para_ucPl_Time_Month;
	tucBuf[tusCount++]=Para_ucPl_Time_Day;
	tucBuf[tusCount++]=Para_ucPl_Time_Hour;
	tucBuf[tusCount++]=Para_ucPl_Time_Minute;
	tucBuf[tusCount++]=Para_ucPl_Time_Second;
	//3.2 属性：不加密、不压缩	
	tucBuf[tusCount++]=((0x00 | (Para_usPl_Message_Body_Properties_Data_Encryption<<10) | (Para_usPl_Message_Body_Properties_Data_Compress<<8)) >> 8) & 0xFF;
	tucBuf[tusCount++]= (0x00 | (Para_usPl_Message_Body_Properties_Data_Encryption<<10) | (Para_usPl_Message_Body_Properties_Data_Compress<<8))       & 0xFF;
	//3.3 参数项总数
	tusPos_Pl_Para_Item_Total_Num=tusCount;
//	tucBuf[tusCount++]=(Para_usPl_Para_Item_Total>>8) & 0xFF;
//	tucBuf[tusCount++]= Para_usPl_Para_Item_Total     & 0xFF;
	tusCount+=2;
	//3.4各项参数
	if(0 == strcmp(Vh_Message_Body_Type,"1001")){//终端注册
		//3.4.1 版本号 0001
		tucBuf[tusCount++]=0x10;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x02;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x01;
		//3.4.2 制造商 ID 0001
		tucBuf[tusCount++]=0x10;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x02;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x02;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x01;
		//3.4.3 终端型号		
		tucBuf[tusCount++]=0x10;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x03;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x02;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x01;
		//3.4.4 制造商终端 ID，002E00000011
		tucBuf[tusCount++]=0x10;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x04;
		tucBuf[tusCount++]=0x04;
		tucBuf[tusCount++]=0x02;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=(strlen(Vh_Topic)>>8) & 0xFF;
		tucBuf[tusCount++]=strlen(Vh_Topic) & 0xFF;
		memcpy(&tucBuf[tusCount],Vh_Topic,strlen(Vh_Topic));
		tusCount+=strlen(Vh_Topic);
		
		tucBuf[tusPos_Pl_Para_Item_Total_Num++]=0x00;//参数项总数为4
		tucBuf[tusPos_Pl_Para_Item_Total_Num++]=0x04;
		
	}else if(0 == strcmp(Vh_Message_Body_Type,"1006")){//请求授时
		tucBuf[tusCount++]=0x10;
		tucBuf[tusCount++]=0x06;
		tucBuf[tusCount++]=0x10;
		tucBuf[tusCount++]=0x04;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x01;
		tucBuf[tusCount++]=0x00;
		tucBuf[tusCount++]=0x00;
		
		tucBuf[tusPos_Pl_Para_Item_Total_Num++]=0x00;//参数项总数为1
		tucBuf[tusPos_Pl_Para_Item_Total_Num++]=0x01;
	}
	else if(0 == strcmp(Vh_Message_Body_Type,"1007")){//实时数据上报
		tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(0x10070000,
																								E_DATA_TYPE_BYTE,
																								E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																								E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																								E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																								E_DATA_LENGTH_2_BYTE,0,"",tucRet_Buf);//时间间隔0秒
		memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
		tusCount+=tucRet_Len;
		
		tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//1,
																								E_DATA_TYPE_DI,
																								E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																								E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																								E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																								E_DATA_LENGTH_2_BYTE,0x0100,"",tucRet_Buf);//设备状态1
		memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
		tusCount+=tucRet_Len;		
//-------------------- 系统控制器数据 -------------------------------------------------------------------------------------
		if( (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) >= 0) && (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) <= 7) ) {
			//debug
//g_Result.load1_ctrl				 =1;	//2、负载1控制    ka1  do0  do1
//g_Result.load2_ctrl				 =0;	//3、负载2控制    ka2  do2  do3
//g_Result.fan_ctrl  				 =1; 	//4、风扇控制		  ka3  do4
//g_Result.reserve_motor_ctrl=0;	//5、后备电机控制 ka4  do5
//g_Result.batt_v=49.8;						//6、电池电压
//g_Result.lem4_batt=23.4;				//7、电池电流in4	-->in6 
//g_Result.batt_p=98.7;						//8、电池功率
//g_Result.lem1_load1=12.3;				//9、负载1电流in2 
//g_Result.lem2_load2=12.4;				//10、负载2电流in1 
//g_Result.load_i=12.5;						//11、负载电流
//g_Result.load_p=35.4;						//12、负载功率
//g_Result.pv_v=67.8;							//13、光伏电压
//g_Result.pv_p=78.9;							//14、光伏功率
//g_Result.ir=34.2;								//15、太阳能光照强度 in0
//g_Result.temp[0]=45.7;					//16、2个温度,箱子内部温度和外部环境温度 in5(内部温度)X  in7(环境温度)OK
//g_Result.temp[1]=45.8;					//17、2个温度,箱子内部温度和外部环境温度 in5(内部温度)X  in7(环境温度)OK
//g_Result.mppt_temp=45.9;				//18、mppt温度
//g_Result.batt_status				=1;	//19、电池状态 电池空气开关是否都合闸 DI0 led亮为正常
//g_Result.load_status				=0;	//20、负载状态 负载1 2空气开关是否都合闸 DI1 led亮为正常	 
//g_Result.doormagnetic_status=1;	//21、门磁状态  DI2 led亮为正常	 
//g_Result.battlose_status		=0;	//22、电池防盗状态 DI3 led亮为正常
//g_Result.pvlose_status			=1; //23、pv防盗状态  DI4 led亮为正常	 	 
//g_Result.load34_status			=0;	//24、负载状态 负载3 4空气开关是否都合闸 DI5 led亮为正常	
//g_Result.load3_ctrl					=1;	//25、负载3控制    ka1  do4  do5
//g_Result.load4_ctrl					=0;	//26、负载4控制    ka2  do6  do7
//g_Result.smog_alarm					=1;	//27、烟雾报警 DI6 led亮为报警
//g_Result.waterout_alarm			=0;	//28、水浸报警 DI7 led亮为报警
//g_Result.lem_load3=34.5;				//29、负载3电流in3
//g_Result.lem_load4=34.6;				//30、负载4电流in4
//g_Result.main_carbin_T=23;			//31、温度
//g_Result.main_carbin_H=24;			//32、湿度
//g_Result.second_carbin_T=25;		//33、温度
//g_Result.second_carbin_H=26;		//34、湿度
//g_Result.pv_i=23.8;							//35、光伏电流
//g_Result.mppt_i=45.6;						//36、mppt总电流
//g_Result.mppt_p=45.7;						//37、mppt总功率
//	 
//g_Result.fVoltage_AC=223.4;			//38、逆变器交流电压               0x000A
//g_Result.fCurrent_AC=12.3;			//39、逆变器交流电流               0x000B

//g_Result.fV_RC_Distribution_Output=52.3;//40、dc输出电压  整流器
//g_Result.fI_RC_Distribution_Moudle_Output_Total=23.4;//41、所有模块输出总电流  整流器

//g_Result.slGenerator_L1L2L3_watts=456;  //42、发电机总功率
//g_Result.fGenerator_L1L2L3_current=35.4;//43、发电机总电流

//g_Result.faverage_module_voltage=48.9;  //44、电池平均电压				V		average module voltage  		.3f 	2Byte
//g_Result.ftotal_current=50.2;						//45、电池总电流					A		total current   						.2f		2Byte
//g_Result.ucaverage_SOC=56;						  //46、电池平均SOC					%		average SOC 								0			1Byte

//g_Result.b_gprs_work=1;				//47、gprs是否工作 1表示工作  0xff表示未配置，默认工作	   其他值表示不工作  不工作则设置成0
//g_Result.b_wifi_work=1;				//48、wifi是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0 
//g_Result.b_rj45_work=1;				//49、rj45是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0
//g_Result.b_TH_work=3;					//50、增加部分  bit 0控制  第1路温湿度    bit1控制第2路温湿度        bit4控制a/c  bit5控制fan
//g_Result.alarmDelyMinute=16;	//51、警报检测延时 单位Min 		最多255min即4个小时15min。  mppt 个数
//g_Result.device_ID=5;					//52、设备ID 占3个字节24位    最大 65535
//g_Result.device_ID2=6;			 	//53、4个字节表示 4294967295  合计10个数字
//g_Result.plusengertim_last=7;	//54、pv>load电流最后一次持续的时间 	单位分钟
//g_Result.genengertim=8;				//55、发电机最后1次工作持续的时间			单位分钟
//g_Result.nolowtim_last=9;			//56、独立逻辑  发电机不工作的时候， 统计持续电量低时间。	单位s
//g_Result.yeshightim_last=10;	//57、独立逻辑  发电机工作的时候，   统计持续电量高时间。	单位s
//g_Result.gen_flag=1;					//58、发电机最后进行开关状态  1=开机  0=关机
//g_Result.oil_level=85;				//59、液位
			if(Vh_Topic[11]=='0'){//系统控制器第1包数据
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.load1_ctrl & BIT(0)) << 8,"",tucRet_Buf);//负载1控制    ka1  do0  do1
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.load2_ctrl & BIT(0)) << 8,"",tucRet_Buf);//负载2控制    ka2  do2  do3
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.fan_ctrl & BIT(0)) << 8,"",tucRet_Buf);//风扇控制		 ka3  do4
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.reserve_motor_ctrl & BIT(0)) << 8,"",tucRet_Buf);//后备电机控制 ka4  do5
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.batt_v),"",tucRet_Buf);//电池电压
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.lem4_batt),"",tucRet_Buf);//电池电流in4	-->in6 
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.batt_p),"",tucRet_Buf);//电池功率
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.lem1_load1),"",tucRet_Buf);//负载1电流in2 
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//10,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.lem2_load2),"",tucRet_Buf);//负载2电流in1
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.load_i),"",tucRet_Buf);//负载电流
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.load_p),"",tucRet_Buf);//负载功率
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.pv_v),"",tucRet_Buf);//光伏电压
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.pv_p),"",tucRet_Buf);//光伏功率
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.ir),"",tucRet_Buf);//太阳能光照强度 in0
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.temp[0]),"",tucRet_Buf);//2个温度,箱子内部温度和外部环境温度 in5(内部温度)X  in7(环境温度)OK
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//17,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.temp[1]),"",tucRet_Buf);//2个温度,箱子内部温度和外部环境温度 in5(内部温度)X  in7(环境温度)OK
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//18,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.mppt_temp),"",tucRet_Buf);//mppt温度
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//19,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.batt_status & BIT(0)) << 8,"",tucRet_Buf);//电池状态 电池空气开关是否都合闸 DI0 led亮为正常
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//20,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.load_status & BIT(0)) << 8,"",tucRet_Buf);//负载状态 负载1 2空气开关是否都合闸 DI1 led亮为正常
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//21,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.doormagnetic_status & BIT(0)) << 8,"",tucRet_Buf);//门磁状态  DI2 led亮为正常	 
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.battlose_status & BIT(0)) << 8,"",tucRet_Buf);//电池防盗状态 DI3 led亮为正常
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//23,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.pvlose_status & BIT(0)) << 8,"",tucRet_Buf);//pv防盗状态  DI4 led亮为正常
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//24,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.load34_status & BIT(0)) << 8,"",tucRet_Buf);//负载状态 负载3 4空气开关是否都合闸 DI5 led亮为正常
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//25,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.load3_ctrl & BIT(0)) << 8,"",tucRet_Buf);//负载3控制    ka1  do4  do5
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//26,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.load4_ctrl & BIT(0)) << 8,"",tucRet_Buf);//负载4控制    ka2  do6  do7
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//27,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.smog_alarm & BIT(0)) << 8,"",tucRet_Buf);//烟雾报警 DI6 led亮为报警
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.waterout_alarm & BIT(0)) << 8,"",tucRet_Buf);//水浸报警 DI7 led亮为报警
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//29,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.lem_load3),"",tucRet_Buf);//负载3电流in3
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//30,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.lem_load4),"",tucRet_Buf);//负载4电流in4
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//31,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.main_carbin_T),"",tucRet_Buf);//温度
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.main_carbin_H),"",tucRet_Buf);//湿度
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//33,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.second_carbin_T),"",tucRet_Buf);//温度
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//34,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.second_carbin_H),"",tucRet_Buf);//湿度
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//35,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.pv_i),"",tucRet_Buf);//光伏电流
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//36,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.mppt_i),"",tucRet_Buf);//mppt总电流
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//37,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.mppt_p),"",tucRet_Buf);//mppt总功率
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//38,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.fVoltage_AC),"",tucRet_Buf);//逆变器交流电压               0x000A
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//39,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.fCurrent_AC),"",tucRet_Buf);//逆变器交流电流               0x000B
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//40,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.fV_RC_Distribution_Output),"",tucRet_Buf);//dc输出电压  整流器
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//41,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.fI_RC_Distribution_Moudle_Output_Total),"",tucRet_Buf);//所有模块输出总电流  整流器
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//42,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.slGenerator_L1L2L3_watts),"",tucRet_Buf);//发电机总功率
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//43,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.fGenerator_L1L2L3_current),"",tucRet_Buf);//发电机总电流
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//44,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.faverage_module_voltage),"",tucRet_Buf);//电池平均电压				V		average module voltage  		.3f 	2Byte
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//45,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.ftotal_current),"",tucRet_Buf);//电池总电流					A		total current   						.2f		2Byte
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//46,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.ucaverage_SOC),"",tucRet_Buf);//电池平均SOC					%		average SOC 								0			1Byte
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//47,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.b_gprs_work & BIT(0)) << 8,"",tucRet_Buf);//gprs是否工作 1表示工作  0xff表示未配置，默认工作	   其他值表示不工作  不工作则设置成0
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//48,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.b_wifi_work & BIT(0)) << 8,"",tucRet_Buf);//wifi是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0 
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//49,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.b_rj45_work & BIT(0)) << 8,"",tucRet_Buf);//rj45是否工作 1表示工作  0xff表示未配置，默认不工作   其他值表示不工作  不工作则设置成0
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//50,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.b_TH_work),"",tucRet_Buf);//增加部分  bit 0控制  第1路温湿度    bit1控制第2路温湿度        bit4控制a/c  bit5控制fan
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//51,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.alarmDelyMinute),"",tucRet_Buf);//警报检测延时 单位Min 		最多255min即4个小时15min。  mppt 个数
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//52,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.device_ID),"",tucRet_Buf);//设备ID 占3个字节24位    最大 65535
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//53,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.device_ID2),"",tucRet_Buf);//4个字节表示 4294967295  合计10个数字
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//54,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.plusengertim_last),"",tucRet_Buf);//pv>load电流最后一次持续的时间 	单位分钟
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//55,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.genengertim),"",tucRet_Buf);//发电机最后1次工作持续的时间			单位分钟
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//56,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.nolowtim_last),"",tucRet_Buf);//独立逻辑  发电机不工作的时候， 统计持续电量低时间。	单位s
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//57,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.yeshightim_last),"",tucRet_Buf);//独立逻辑  发电机工作的时候，   统计持续电量高时间。	单位s
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//58,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Result.gen_flag & BIT(0)) << 8,"",tucRet_Buf);//发电机最后进行开关状态  1=开机  0=关机
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//59,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Result.oil_level),"",tucRet_Buf);//液位
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
			}else if(Vh_Topic[11]=='1'){//系统控制器第2包数据：日报数据
				//debug
//				g_Result2.ff_E_UIt_J_Sum[0][0]=12.0;//2
//				g_Result2.ff_E_UIt_J_Sum[0][1]=12.1;//3
//				g_Result2.ff_E_UIt_J_Sum[0][2]=12.2;//4
//				g_Result2.ff_E_UIt_J_Sum[0][3]=12.3;//5
//				g_Result2.ff_E_UIt_J_Sum[0][4]=12.4;//6
//				
//				g_Result2.ff_E_UIt_J_Sum[1][0]=22.0;//7
//				g_Result2.ff_E_UIt_J_Sum[1][1]=22.1;//8
//				g_Result2.ff_E_UIt_J_Sum[1][2]=22.2;//9
//				g_Result2.ff_E_UIt_J_Sum[1][3]=22.3;//10
//				g_Result2.ff_E_UIt_J_Sum[1][4]=22.4;//11
//				
//				g_Result2.f_E_MaxLoadP[0][0]=32.0;//12
//				g_Result2.f_E_MaxLoadP[0][1]=32.1;//13
//				g_Result2.f_E_MaxLoadP[0][2]=32.2;//14
//				g_Result2.f_E_MaxLoadP[0][3]=32.3;//15
//				g_Result2.f_E_MaxLoadP[0][4]=32.4;//16
//				
//				g_Result2.f_E_MaxLoadP[1][0]=42.0;//17
//				g_Result2.f_E_MaxLoadP[1][1]=42.1;//18
//				g_Result2.f_E_MaxLoadP[1][2]=42.2;//19
//				g_Result2.f_E_MaxLoadP[1][3]=42.3;//20
//				g_Result2.f_E_MaxLoadP[1][4]=42.4;//21
//				
//				g_Result2.f_E_MinLoadP[0][0]=52.0;//22
//				g_Result2.f_E_MinLoadP[0][1]=52.1;//23
//				g_Result2.f_E_MinLoadP[0][2]=52.2;//24
//				g_Result2.f_E_MinLoadP[0][3]=52.3;//25
//				g_Result2.f_E_MinLoadP[0][4]=52.4;//26
//				
//				g_Result2.f_E_MinLoadP[1][0]=62.0;//27
//				g_Result2.f_E_MinLoadP[1][1]=62.1;//28
//				g_Result2.f_E_MinLoadP[1][2]=62.2;//29
//				g_Result2.f_E_MinLoadP[1][3]=62.3;//30
//				g_Result2.f_E_MinLoadP[1][4]=62.4;//31
//				
//				g_Result2.f_E_AverLoadP[0][0]=72.0;//32
//				g_Result2.f_E_AverLoadP[0][1]=72.1;//33
//				g_Result2.f_E_AverLoadP[0][2]=72.2;//34
//				g_Result2.f_E_AverLoadP[0][3]=72.3;//35
//				g_Result2.f_E_AverLoadP[0][4]=72.4;//36
//				
//				g_Result2.f_E_AverLoadP[1][0]=82.0;//37
//				g_Result2.f_E_AverLoadP[1][1]=82.1;//38
//				g_Result2.f_E_AverLoadP[1][2]=82.2;//39
//				g_Result2.f_E_AverLoadP[1][3]=82.3;//40
//				g_Result2.f_E_AverLoadP[1][4]=82.4;//41
//				
//				g_Result2.f_E_TotalDG_KWH[0]=23.1;//42
//				g_Result2.f_E_TotalDG_KWH[1]=23.2;//43
//				
//				g_Result2.f_E_TotalMPPT_KWH[0]=34.1;//44
//				g_Result2.f_E_TotalMPPT_KWH[1]=34.2;//45
//				
//				g_Result2.f_E_NetKWH[0]=56.1;//46
//				g_Result2.f_E_NetKWH[1]=56.2;//47
//				
//				g_Result2.i_DGRunTime[0]=123;//48
//				g_Result2.i_DGRunTime[1]=456;//49
//				
//				g_Result2.i_MaxBatSoc[0]=234;//50
//				g_Result2.i_MaxBatSoc[1]=567;//51
//				
//				g_Result2.i_MinBatSOC[0]=345;//52
//				g_Result2.i_MinBatSOC[1]=678;//53
//				
//				g_Result2.i_PowerAvailability[0]=432;//54
//				g_Result2.i_PowerAvailability[1]=433;//55
//				
//				g_Result2.i_PowerAvailabilityPer[0]=534;//56
//				g_Result2.i_PowerAvailabilityPer[1]=535;//57
				
				#if 1		//KWH_J			//如果为kwh												
												for(int nn =0 ; nn<2 ;nn++)
												{
													for(int mm=0;mm<5;mm++)
													{													
														g_Result3.ff_E_UIt_J_Sum[nn][mm] = g_Result2.ff_E_UIt_J_Sum[nn][mm]/3600000.0;													
														g_Result3.f_E_MaxLoadP[nn][mm]   = g_Result2.f_E_MaxLoadP[nn][mm]/1000.0;													
														g_Result3.f_E_MinLoadP[nn][mm]   = g_Result2.f_E_MinLoadP[nn][mm]/1000.0;													
														g_Result3.f_E_AverLoadP[nn][mm]  = g_Result2.f_E_AverLoadP[nn][mm]/1000.0;													
													}
													g_Result3.f_E_TotalDG_KWH[nn]  		= g_Result2.f_E_TotalDG_KWH[nn]/3600000.0;													
													g_Result3.f_E_TotalMPPT_KWH[nn]   = g_Result2.f_E_TotalMPPT_KWH[nn]/3600000.0;													
													g_Result3.f_E_NetKWH[nn]   				= g_Result2.f_E_NetKWH[nn]/3600000.0;				
													g_Result3.i_DGRunTime[nn]   			= g_Result2.i_DGRunTime[nn]/60.0;				
													g_Result3.i_PowerAvailability[nn] = g_Result2.i_PowerAvailability[nn]/60.0;				
													
													g_Result3.i_MaxBatSoc[nn] 				= g_Result2.i_MaxBatSoc[nn];				
													g_Result3.i_MinBatSOC[nn] 				= g_Result2.i_MinBatSOC[nn];				
													g_Result3.i_PowerAvailabilityPer[nn] = g_Result2.i_PowerAvailabilityPer[nn];																	
												}
				#else
						g_Result3 = g_Result2;						
				#endif
				for(i=0;i<2;i++){
					for(j=0;j<5;j++){
						tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2-11,
																												E_DATA_TYPE_FLOAT,
																												E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																												E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																												E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																												E_DATA_LENGTH_4_BYTE,
																												float_to_int(g_Result3.ff_E_UIt_J_Sum[i][j]),"",tucRet_Buf);
						memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
						tusCount+=tucRet_Len;
					}
				}
				
				for(i=0;i<2;i++){
					for(j=0;j<5;j++){
						tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12-21,
																												E_DATA_TYPE_FLOAT,
																												E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																												E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																												E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																												E_DATA_LENGTH_4_BYTE,
																												float_to_int(g_Result3.f_E_MaxLoadP[i][j]),"",tucRet_Buf);
						memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
						tusCount+=tucRet_Len;
					}
				}
				
				for(i=0;i<2;i++){
					for(j=0;j<5;j++){
						tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22-31,
																												E_DATA_TYPE_FLOAT,
																												E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																												E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																												E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																												E_DATA_LENGTH_4_BYTE,
																												float_to_int(g_Result3.f_E_MinLoadP[i][j]),"",tucRet_Buf);
						memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
						tusCount+=tucRet_Len;
					}
				}				
				
				for(i=0;i<2;i++){
					for(j=0;j<5;j++){
						tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32-41,
																												E_DATA_TYPE_FLOAT,
																												E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																												E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																												E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																												E_DATA_LENGTH_4_BYTE,
																												float_to_int(g_Result3.f_E_AverLoadP[i][j]),"",tucRet_Buf);
						memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
						tusCount+=tucRet_Len;
					}
				}
				
				for(i=0;i<2;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//42-43,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_Result3.f_E_TotalDG_KWH[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
				
				for(i=0;i<2;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//44-45,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_Result3.f_E_TotalMPPT_KWH[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
				
				for(i=0;i<2;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//46-47,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_Result3.f_E_NetKWH[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
				
				for(i=0;i<2;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//48-49,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_Result3.i_DGRunTime[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
				
				for(i=0;i<2;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//50-51,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_Result3.i_MaxBatSoc[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
				
				for(i=0;i<2;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//52-53,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_Result3.i_MinBatSOC[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
				
				for(i=0;i<2;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//54-55,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_Result3.i_PowerAvailability[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
				
				for(i=0;i<2;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//56-57,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_Result3.i_PowerAvailabilityPer[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
			}else if(Vh_Topic[11]=='2'){//系统控制器第3包数据：设置参数
				//debug
//				g_PVTPara.device_ID=1;			//2
//				g_PVTPara.collect_frq=5;		//3
//				g_PVTPara.send_frq=6;    		//4
//				g_PVTPara.TMax=789;					//5
//				g_PVTPara.TMin=123;					//6
//				g_PVTPara.HMax=456;					//7
//				g_PVTPara.HMin=234;					//8
//				g_PVTPara.TXZ[0]=345;				//9
//				g_PVTPara.TXZ[1]=346;				//10
//				g_PVTPara.HXZ[0]=567;				//11
//				g_PVTPara.HXZ[1]=568;				//12
//				g_PVTPara.VMin=389;					//13
//				g_PVTPara.bCH[0]=541;				//14
//				g_PVTPara.bCH[1]=1;					//15,DI
//				g_PVTPara.alarmDelyMinute=5;//16
//				g_PVTPara.beep=1;						//17,DI
//				g_PVTPara.wifi_mode=3;			//18
//				g_PVTPara.b_light_work=1;		//19,DI
//				g_PVTPara.b_gprs_work=1;		//20,DI
//				g_PVTPara.b_wifi_work=1;		//21,DI
//				g_PVTPara.b_rj45_work=1;		//22,DI
//				g_PVTPara.b_debug_work=1;		//23,DI
//				g_PVTPara.reset_time=6;			//24
//				g_PVTPara.b_Sms_Test=765;   //25
//				g_PVTPara.b_Sms_FxiTime=7;	//26
//				g_PVTPara.remotePort=6601;	//27
//				g_PVTPara.remoteIP[0]=123;	//28
//				g_PVTPara.remoteIP[1]=124;	//29
//				g_PVTPara.remoteIP[2]=125;	//30
//				g_PVTPara.remoteIP[3]=126;	//31
//				g_PVTPara.TMax2=367;				//32
//				g_PVTPara.TMin2=267;				//33
//				g_PVTPara.HMax2=478;				//34
//				g_PVTPara.HMin2=378;				//35
//				g_PVTPara.b_TH_work=8;			//36
//				g_PVTPara.sysctrl_time[0]=23;//37
//				g_PVTPara.sysctrl_time[1]=10;//38
//				g_PVTPara.sysctrl_time[2]=18;//39
//				g_PVTPara.sysctrl_time[3]=9; //40
//				g_PVTPara.sysctrl_time[4]=48;//41
//				g_PVTPara.sysctrl_time[5]=50;//42
				get_pvt_para();
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.device_ID),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.collect_frq),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.send_frq),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.TMax),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.TMin),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.HMax),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.HMin),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				for(i=0;i<2;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9-10,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_PVTPara.TXZ[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
				
				for(i=0;i<2;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11-12,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_PVTPara.HXZ[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.VMin),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;				
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.bCH[0]),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_PVTPara.bCH[1] & BIT(0)) << 8,"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;	
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.alarmDelyMinute),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//17,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_PVTPara.beep & BIT(0)) << 8,"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;	
								
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//18,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.wifi_mode),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//19,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_PVTPara.b_light_work & BIT(0)) << 8,"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//20,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_PVTPara.b_gprs_work & BIT(0)) << 8,"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//21,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_PVTPara.b_wifi_work & BIT(0)) << 8,"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_PVTPara.b_rj45_work & BIT(0)) << 8,"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//23,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_PVTPara.b_debug_work & BIT(0)) << 8,"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;				
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//24,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.reset_time),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//25,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.b_Sms_Test),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//26,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.b_Sms_FxiTime),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//27,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.remotePort),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				for(i=0;i<4;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28-31,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_PVTPara.remoteIP[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;				
				}
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.TMax2),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//33,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.TMin2),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//34,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.HMax2),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//35,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.HMin2),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//36,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_PVTPara.b_TH_work),"",tucRet_Buf);
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				for(i=0;i<6;i++){
					tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//37-42,
																											E_DATA_TYPE_FLOAT,
																											E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																											E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																											E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																											E_DATA_LENGTH_4_BYTE,
																											float_to_int(g_PVTPara.sysctrl_time[i]),"",tucRet_Buf);
					memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
					tusCount+=tucRet_Len;
				}
			}
		}
//-------------------- MPPT数据 ----------------------------------------------------------------------------------------------
		else if( (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) >= 8) && (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) <= 39) ) {
				tusIndex_Count=TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) - 8;
				if(tusIndex_Count<g_configRead.alarmDelyMinute)
				{
			//debug
////			g_Mttp[tusIndex_Count].Model=1; 		          //2、0xE0CC:0 = TSMPPT-45, 1=TSMPPT-60,MPPT Solar Charger
//			g_Mttp[tusIndex_Count].Model=1;									//2、0xE0CC:0 = TSMPPT-45, 1=TSMPPT-60,MPPT Solar Charger
//			
//			memcpy(g_Mttp[tusIndex_Count].SN,"12345678",8);	//3、0xE0C0-0xE0C3:MPPT Serial ,字符串			
//			
//			g_Mttp[tusIndex_Count].ucEmodbus_id=1; 					//4、0xE019:MPPT MODBUS ID			
//			
////			g_Mttp[tusIndex_Count].usState_Charge=0; 			//5、充电状态,0x0032
//			g_Mttp[tusIndex_Count].Reg[26]=0;								//5、充电状态,0x0032			
//			
////			g_Mttp[tusIndex_Count].uiHourmeter=1234; 		  //6、累计时间,0x002A-0x002B
//			g_Mttp[tusIndex_Count].Mppt_HM=1234; 				    //6、累计时间,0x002A-0x002B
//			
////			g_Mttp[tusIndex_Count].usState_LED=9; 				//7、LED状态,0x0031
//			g_Mttp[tusIndex_Count].led_state_31=9; 					//7、LED状态,0x0031
//			
////			g_Mttp[tusIndex_Count].fAhc_r=4321; 					//8、可重置充电Ah,0x0034-0x0035,n*0.1
//			g_Mttp[tusIndex_Count].Mppt_Ahc_r=4321; 				//8、可重置充电Ah,0x0034-0x0035,n*0.1			
//			
////			g_Mttp[tusIndex_Count].fAhc_t=5678;						//9、总充电Ah,0x0036-0x0037,n*0.1
//			g_Mttp[tusIndex_Count].Mppt_Ahc_t=5678;					//9、总充电Ah,0x0036-0x0037,n*0.1
//			
////			g_Mttp[tusIndex_Count].fKwhc_r=3456;       		//10、可重置充电Kwh,0x0038
//			g_Mttp[tusIndex_Count].kwhc_r_38=3456;       		//10、可重置充电Kwh,0x0038
//			
////			g_Mttp[tusIndex_Count].fKwhc_t=7890;       		//11、总充电Kwh,    0x0039
//			g_Mttp[tusIndex_Count].kwhc_t_39=7890;       		//11、总充电Kwh,    0x0039
//			
////			g_Mttp[tusIndex_Count].fAhc_daily=2345;    		//12、日充电Ah,0x0043,n*0.1
//			g_Mttp[tusIndex_Count].Ahc_daily=2345;    			//12、日充电Ah,0x0043,n*0.1
//			
////			g_Mttp[tusIndex_Count].fWhc_daily=6789;    		//13、日充电Wh,0x0044
//			g_Mttp[tusIndex_Count].whc_daily=6789;    			//13、日充电Wh,0x0044
//			
////			g_Mttp[tusIndex_Count].usTm_ab_daily=12; 			//14、日吸收充时间, 0x004D
//			g_Mttp[tusIndex_Count].time_ab_daily=12; 				//14、日吸收充时间, 0x004D
//			
////			g_Mttp[tusIndex_Count].usTm_eq_daily=34; 			//15、日均衡充时间, 0x004E
//			g_Mttp[tusIndex_Count].time_eq_daily=34; 				//15、日均衡充时间, 0x004E
//			
////			g_Mttp[tusIndex_Count].usTm_fl_daily=56; 			//16、日浮充充时间, 0x004F
//			g_Mttp[tusIndex_Count].Vtime_fl_daily=56; 			//16、日浮充充时间, 0x004F
//			
////			g_Mttp[tusIndex_Count].fThs=31;          			//17、散热器温度,   0x0023
//			g_Mttp[tusIndex_Count].T_hs=31;          				//17、散热器温度,   0x0023
//			
////			g_Mttp[tusIndex_Count].fTrts=45;         			//18、RTS温度,0x0024
//			g_Mttp[tusIndex_Count].T_rts=45;         				//18、RTS温度,0x0024
//			
////			g_Mttp[tusIndex_Count].usFlag_daily=0x00;			//19-23、日标志BIT0:Reset detected,0x0045
//			g_Mttp[tusIndex_Count].flags_daily=0x00;				//19-23、日标志BIT0:Reset detected,0x0045
//			
////			g_Mttp[tusIndex_Count].uiAlarm=0x00000000;   	//24-47、报警BIT0:RTS open:0x002E-0x002F
//			g_Mttp[tusIndex_Count].Mppt_alarm=0x00000000;   //24-47、报警BIT0:RTS open:0x002E-0x002F
//			
////			g_Mttp[tusIndex_Count].usFault=0x0000; 				//48-63、故障BIT0:overcurrent,0x002C 
//			g_Mttp[tusIndex_Count].Fault_all_2C=0x0000; 		//48-63、故障BIT0:overcurrent,0x002C 
//			
////			g_Mttp[tusIndex_Count].usDIP=0x0000; 					//64-71、拨码开关第1位,0x0030
//			g_Mttp[tusIndex_Count].dip_all_30=0x0000; 			//64-71、拨码开关第1位,0x0030			
//			
//			memcpy(g_Mttp[tusIndex_Count].Vendor_Name,"Morningstar Corp.",17);	//2、
//			memcpy(g_Mttp[tusIndex_Count].Product_Code,"TS-MPPT-60",10);				//3、
//			memcpy(g_Mttp[tusIndex_Count].MajorMinorRevision,"v01.01.01",9);		//4、
////			MPPT设备状态																											//5、
////			g_Mttp[tusIndex_Count].fVb_f=48.7;																//6、滤波电池电压,0x0018,n*V_PU*2-15
//			g_Mttp[tusIndex_Count].Reg[0]=48;																		//6、滤波电池电压,0x0018,n*V_PU*2-15
//			
////			g_Mttp[tusIndex_Count].fVa_f=53.4;   															//7、滤波光伏电压,0x001B,n*V_PU*2-15
//			g_Mttp[tusIndex_Count].Reg[3]=53;   																//7、滤波光伏电压,0x001B,n*V_PU*2-15
//			
////			g_Mttp[tusIndex_Count].fIa_f=35.6;   															//8、滤波光伏电流,0x001D,n*I_PU*2-15
//			g_Mttp[tusIndex_Count].Reg[5]=35;   																//8、滤波光伏电流,0x001D,n*I_PU*2-15
//			
//			g_Mttp[tusIndex_Count].fPower_input=503.4;													//9、输入功率W,0x003B,n*V_PU*I_PU*2-17			
//			
////			g_Mttp[tusIndex_Count].fIc_f=23.8;																//10、滤波充电电流,0x0027,n*I_PU*2-15
//			g_Mttp[tusIndex_Count].Reg[15]=23;																	//10、滤波充电电流,0x0027,n*I_PU*2-15
//			
//			g_Mttp[tusIndex_Count].fPower_output=405.6;  												//11、输出功率W,0x003A,n*V_PU*I_PU*2-17			
//			
////			g_Mttp[tusIndex_Count].fVb_ref=47.9;															//12、目标调节电压,0x0033,n*V_PU*2-15
//			g_Mttp[tusIndex_Count].vb_ref_33=47;																//12、目标调节电压,0x0033,n*V_PU*2-15
//			
////			g_Mttp[tusIndex_Count].fPout_max_daily=1234.5;										//13、日最大输出功率,0x0046,n*V_PU*I_PU*2-17
//			g_Mttp[tusIndex_Count].Pout_max_daily=1234.5;												//13、日最大输出功率,0x0046,n*V_PU*I_PU*2-17
//			
//			g_Mttp[tusIndex_Count].fEV_float=46.7;      												//14、浮充电压,0xE001,n·V_PU·2-15
//			g_Mttp[tusIndex_Count].fEV_absorp=52.3;     												//15、吸收充电压,0xE000,n*V_PU*2-15
//			g_Mttp[tusIndex_Count].fEV_eq=54.5;	        												//16、均衡充电压,0xE007,n·V_PU·2-15
			
			if(Vh_Topic[11]=='0'){//Tristar MPPT第1包数据
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										g_Mttp[tusIndex_Count].Model << 8,"",tucRet_Buf);//0xE0CC:0 = TSMPPT-45, 1=TSMPPT-60
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																										E_DATA_TYPE_STRING,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										strlen((char *)g_Mttp[tusIndex_Count].SN),
																										0,(char *)g_Mttp[tusIndex_Count].SN,tucRet_Buf);//0xE0C0-0xE0C3:MPPT Serial ,字符串
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[0].modbus_id+tusIndex_Count),"",tucRet_Buf);    //float_to_int(g_Mttp[tusIndex_Count].ucEmodbus_id),"",tucRet_Buf);//0xE019:MPPT MODBUS ID  chage 23-11-30
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].Reg[26]),"",tucRet_Buf);//充电状态,0x0032
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].Mppt_HM),"",tucRet_Buf);//累计时间,0x002A-0x002B
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;			
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].led_state_31),"",tucRet_Buf);//LED状态,0x0031
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].Mppt_Ahc_r),"",tucRet_Buf);//可重置充电Ah,0x0034-0x0035,n*0.1
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].Mppt_Ahc_t),"",tucRet_Buf);//总充电Ah,0x0036-0x0037,n*0.1
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//10,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].kwhc_r_38),"",tucRet_Buf);//可重置充电Kwh,0x0038
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].kwhc_t_39),"",tucRet_Buf);//总充电Kwh,    0x0039
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].Ahc_daily),"",tucRet_Buf);//日充电Ah,0x0043,n*0.1
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].whc_daily),"",tucRet_Buf);//日充电Wh,0x0044
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].time_ab_daily),"",tucRet_Buf);//日吸收充时间, 0x004D
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].time_eq_daily),"",tucRet_Buf);//日均衡充时间, 0x004E
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].Vtime_fl_daily),"",tucRet_Buf);//日浮充充时间, 0x004F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//17,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].T_hs),"",tucRet_Buf);//散热器温度,   0x0023
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//18,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].T_rts),"",tucRet_Buf);//RTS温度,0x0024
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//19,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].flags_daily & BIT(0)) << 8,"",tucRet_Buf);//日标志BIT0:Reset detected,0x0045
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//20,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].flags_daily & BIT(1)) << 7,"",tucRet_Buf);//日标志BIT1:Equalize triggered,0x0045
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//21,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].flags_daily & BIT(2)) << 6,"",tucRet_Buf);//日标志BIT2:Entered float,0x0045
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].flags_daily & BIT(3)) << 5,"",tucRet_Buf);//日标志BIT3:an alarm occurred,0x0045
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//23,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].flags_daily & BIT(4)) << 4,"",tucRet_Buf);//日标志BIT4:a fault occurred,0x0045
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//24,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(0)) << 8,"",tucRet_Buf);//报警BIT0:RTS open:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//25,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(1)) << 7,"",tucRet_Buf);//报警BIT1:RTS shorted,0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//26,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(2)) << 7,"",tucRet_Buf);//报警BIT2:RTS disconnected,0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//27,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(3)) << 5,"",tucRet_Buf);//报警BIT3:Heatsink temp sensor open:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(4)) << 4,"",tucRet_Buf);//报警BIT4:Heatsink temp sensor shorted:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//29,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(5)) << 3,"",tucRet_Buf);//报警BIT5:High temperature current limit:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//30,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(6)) << 2,"",tucRet_Buf);//报警BIT6:Current limit:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//31,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(7)) << 1,"",tucRet_Buf);//报警BIT7:Current offset:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(8)) << 0,"",tucRet_Buf);//报警BIT8:Battery sense out of range:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//33,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(9)) >> 1,"",tucRet_Buf);//报警BIT9:Battery sense disconnected:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//34,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(10)) >> 2,"",tucRet_Buf);	//报警BIT10:Uncalibrated:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//35,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(11)) >> 3,"",tucRet_Buf);//报警BIT11:RTS miswire:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//36,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(12)) >> 4,"",tucRet_Buf);//报警BIT12:High voltage disconnect:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//37,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(13)) >> 5,"",tucRet_Buf);//报警BIT13:Undefined:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//38,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(14)) >> 6,"",tucRet_Buf);//报警BIT14:system miswire:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//39,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(15)) >> 7,"",tucRet_Buf);//报警BIT15:MOSFET open:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//40,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(16)) >> 8,"",tucRet_Buf);//报警BIT16:P12 voltage off:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//41,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(17)) >> 9,"",tucRet_Buf);//报警BIT17:High input voltage current limit:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//42,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(18)) >> 10,"",tucRet_Buf);//报警BIT18:ADC input max:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//43,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(19)) >> 11,"",tucRet_Buf);//报警BIT19:Controller was reset:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//44,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(20)) >> 12,"",tucRet_Buf);//报警BIT20:Alarm 21:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//45,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(21)) >> 13,"",tucRet_Buf);//报警BIT21:Alarm 22:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//46,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(22)) >> 14,"",tucRet_Buf);//报警BIT22:Alarm 23:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//47,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Mppt_alarm & BIT(23)) >> 15,"",tucRet_Buf);//报警BIT23:Alarm 24:0x002E-0x002F
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//48,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(0)) << 8,"",tucRet_Buf);//故障BIT0:overcurrent,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
							
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//49,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(1)) << 7,"",tucRet_Buf);//故障BIT1:FETs shorted,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//50,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(2)) << 6,"",tucRet_Buf);//故障BIT2:software bug,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//51,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(3)) << 5,"",tucRet_Buf);//故障BIT3:battery HVD,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//52,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(4)) << 4,"",tucRet_Buf);//故障BIT4:array HVD,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//53,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(5)) << 3,"",tucRet_Buf);//故障BIT5:settings switch changed,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//54,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(6)) << 2,"",tucRet_Buf);//故障BIT6:RTS shorted,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//55,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(7)) << 1,"",tucRet_Buf);//故障BIT7:RTS  shorted,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//56,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(8)) << 0,"",tucRet_Buf);//故障BIT8:RTS disconnected,0x002
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//57,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(9)) >> 1,"",tucRet_Buf);//故障BIT9:EEPROM retry limit,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//58,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(10)) >> 2,"",tucRet_Buf);//故障BIT10:Reserved,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//59,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(11)) >> 3,"",tucRet_Buf);//故障BIT11:Slave Control Timeout,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//60,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(12)) >> 4,"",tucRet_Buf);//故障BIT12:Fault 13,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//61,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(13)) >> 5,"",tucRet_Buf);//故障BIT13:Fault 14,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//62,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(14)) >> 6,"",tucRet_Buf);//故障BIT14:Fault 15,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//63,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].Fault_all_2C & BIT(15)) >> 7,"",tucRet_Buf);//故障BIT15:Fault 16,0x002C
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//64,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].dip_all_30 & BIT(0)) << 8,"",tucRet_Buf);//拨码开关第1位,0x0030
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//65,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].dip_all_30 & BIT(1)) << 7,"",tucRet_Buf);//拨码开关第2位,0x0030
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//66,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].dip_all_30 & BIT(2)) << 6,"",tucRet_Buf);//拨码开关第3位,0x0030
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//67,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].dip_all_30 & BIT(3)) << 5,"",tucRet_Buf);//拨码开关第4位,0x0030
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//68,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].dip_all_30 & BIT(4)) << 4,"",tucRet_Buf);//拨码开关第5位,0x0030
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//69,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].dip_all_30 & BIT(5)) << 3,"",tucRet_Buf);//拨码开关第6位,0x0030
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//70,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].dip_all_30 & BIT(6)) << 2,"",tucRet_Buf);//拨码开关第7位,0x0030
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//71,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(g_Mttp[tusIndex_Count].dip_all_30 & BIT(7)) << 1,"",tucRet_Buf);//拨码开关第8位,0x0030
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			}else if(Vh_Topic[11]=='1'){//Tristar MPPT第2包数据
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																									E_DATA_TYPE_STRING,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									strlen((char *)g_Mttp[tusIndex_Count].Vendor_Name),
																									0,(char *)g_Mttp[tusIndex_Count].Vendor_Name,tucRet_Buf);//Vendor_Name，"Morningstar Corp."
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																										E_DATA_TYPE_STRING,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										strlen((char *)g_Mttp[tusIndex_Count].Product_Code),
																										0,(char *)g_Mttp[tusIndex_Count].Product_Code,tucRet_Buf);//Product_Code,"TS-MPPT-60"
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
																										
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																										E_DATA_TYPE_STRING,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										strlen((char *)g_Mttp[tusIndex_Count].MajorMinorRevision),
																										0,(char *)g_Mttp[tusIndex_Count].MajorMinorRevision,tucRet_Buf);//MajorMinorRevision
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
																										
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,0x0100,"",tucRet_Buf);//MPPT设备状态1
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
																										
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].fVb_f),"",tucRet_Buf);//滤波电池电压,0x0018,n*V_PU*2-15
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].fVa_f),"",tucRet_Buf);//滤波光伏电压,0x001B,n*V_PU*2-15
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].fIa_f),"",tucRet_Buf);//滤波光伏电流,0x001D,n*I_PU*2-15
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].fPower_input),"",tucRet_Buf);//输入功率W,0x003B,n*V_PU*I_PU*2-17
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//10,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].fIc_f),"",tucRet_Buf);//滤波充电电流,0x0027,n*I_PU*2-15
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].fPower_output),"",tucRet_Buf);//输出功率W,0x003A,n*V_PU*I_PU*2-17
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].vb_ref_33),"",tucRet_Buf);//目标调节电压,0x0033,n*V_PU*2-15
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].Pout_max_daily),"",tucRet_Buf);//fPout_max_daily
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].fEV_float),"",tucRet_Buf);//浮充电压,0xE001,n·V_PU·2-15
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].fEV_absorp),"",tucRet_Buf);//吸收充电压,0xE000,n*V_PU*2-15
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Mttp[tusIndex_Count].fEV_eq),"",tucRet_Buf);//均衡充电压,0xE007,n·V_PU·2-15
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			}
		  }
		}
//-------------------- AC(Air conditioning )数据 -------------------------------------------------------------------------------------
		else if( (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) >= 40) && (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) <= 47) && (Vh_Topic[11]=='0') ) {
			//debug
//g_Air.DC_power_voltage=489;																	//2、6直流电源电压 /10.0
//g_Air.High_DC_voltage_alarm_value=545;											//3、0x0026直流电压告警高限 /10.0
//g_Air.High_DC_voltage_alarm=1;															//4、0x000C直流高电压告警
//g_Air.Low_DC_voltage_alarm_value=523;												//5、0x0027直流电压告警低限	/10.0
//g_Air.Low_DC_voltage_alarm=1;																//6、0x000D直流低电压告警
//g_Air.Return_air_temp=231;																	//7、4回风温度 /10.0
//g_Air.Ambient_temp=232;																			//8、5柜外温度 /10.0
//g_Air.Condenser_temp=233;																		//9、D冷凝盘管温度 /10.0
//g_Air.Return_air_humidity=957;															//10、E回风湿度 /10.0
//g_Air.Self_check_state=1;																		//11、0x000B自检状态
//g_Air.Return_air_temp_sensor_fault=1;												//12、0x000A回风温度传感器故障
//g_Air.Ambient_temp_sensor_fault=1;													//13、0x0015柜外温度探头告警
//g_Air.Condenser_temp_sensor_faul=1;													//14、0x000B冷凝盘管温度传感器故障
//g_Air.Humidity_sensor_fault=1;															//15、0x001A湿度传感器故障
//g_Air.Filter_alarm=1;																				//16、0x0023过滤网堵塞告警
//g_Air.inside_high_temperature_limit=567;										//17、0x001C高温告警温度值 /10.0
//g_Air.Inside_low_temp_alarm=1;															//18、0x000F柜内低温告警
//g_Air.inside_low_temperature_limit=423;											//19、0x001D低温告警温度值 /10.0
//g_Air.Inside_low_temp_alarm=1;															//20、0x000F柜内低温告警
//g_Air.High_humidity=1;																			//21、0x0018高湿告警
//g_Air.Machine_state=1;																			//22、0x000C机组运行状态
//g_Air.heat_exchanger_state=1;																//23、0x000E换热状态
//g_Air.Heat_exchanger_starting_temperature=456;							//24、0x0038换热启动温度 /10.0
//g_Air.Heat_exchanger_stop_return_difference_temperature=389;//25、0x0039换热停止回差值 /10.0
//g_Air.Internal_fan_speed=3500;															//26、0内风机转速
//g_Air.Internal_fan_state=1;																	//27、0x0000内风机运行状态
//g_Air.Internal_fan_alarm=1;																	//28、0x0000内风机告警
//g_Air.External_fan_speed=3600;															//29、2外风机转速
//g_Air.External_fan_state=1;																	//30、0x0002外风机运行状态
//g_Air.External_fan_alarm=1;																	//31、0x0002外风机告警
//g_Air.Cooing_state=1;																				//32、0x0004制冷运行状态
//g_Air.Compressor_starting_temperature=467;									//33、0x0028制冷启动温度 /10.0
//g_Air.Compressor_stop_return_difference_temperature=398;		//34、0x0029制冷停止回差值 /10.0
//g_Air.Compressor_fault=1;																		//35、0x0004压缩机故障
//g_Air.Heating_state=1;																			//36、0x0005制热运行状态
//g_Air.Heater_starting_temperature=476;											//37、0x0016制热启动温度 /10.0
//g_Air.Heater_stop_return_difference_temperature=298;				//38、0x0017制热停止回差值 /10.0
//g_Air.Heater_current=1234;																	//39、A加热器电流 /100.0
//g_Air.Heater_overload_alarm=1;															//40、0x0008加热器过流告警
//g_Air.Heater_underload_alarm=1;															//41、0x0009加热器欠流告警
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.DC_power_voltage/10.0),"",tucRet_Buf);//2、6直流电源电压 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.High_DC_voltage_alarm_value/10.0),"",tucRet_Buf);//0x0026直流电压告警高限 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.High_DC_voltage_alarm & BIT(0)) << 8,"",tucRet_Buf);//0x000C直流高电压告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Low_DC_voltage_alarm_value/10.0),"",tucRet_Buf);//0x0027直流电压告警低限	/10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Low_DC_voltage_alarm & BIT(0)) << 8,"",tucRet_Buf);//0x000D直流低电压告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Return_air_temp/10.0),"",tucRet_Buf);//4回风温度 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Ambient_temp/10.0),"",tucRet_Buf);//5柜外温度 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Condenser_temp/10.0),"",tucRet_Buf);//D冷凝盘管温度 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//10,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Return_air_humidity/10.0),"",tucRet_Buf);//E回风湿度 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Self_check_state & BIT(0)) << 8,"",tucRet_Buf);//0x000B自检状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Return_air_temp_sensor_fault & BIT(0)) << 8,"",tucRet_Buf);//0x000A回风温度传感器故障	
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Ambient_temp_sensor_fault & BIT(0)) << 8,"",tucRet_Buf);//0x0015柜外温度探头告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Condenser_temp_sensor_faul & BIT(0)) << 8,"",tucRet_Buf);//0x000B冷凝盘管温度传感器故障
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Humidity_sensor_fault & BIT(0)) << 8,"",tucRet_Buf);//0x001A湿度传感器故障
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Filter_alarm & BIT(0)) << 8,"",tucRet_Buf);//0x0023过滤网堵塞告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//17,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.inside_high_temperature_limit/10.0),"",tucRet_Buf);//0x001C高温告警温度值 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//18,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Inside_low_temp_alarm & BIT(0)) << 8,"",tucRet_Buf);//0x000F柜内低温告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//19,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.inside_low_temperature_limit/10.0),"",tucRet_Buf);//0x001D低温告警温度值 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//20,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Inside_low_temp_alarm & BIT(0)) << 8,"",tucRet_Buf);//0x000F柜内低温告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//21,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.High_humidity & BIT(0)) << 8,"",tucRet_Buf);//0x0018高湿告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Machine_state & BIT(0)) << 8,"",tucRet_Buf);//0x000C机组运行状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//23,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.heat_exchanger_state & BIT(0)) << 8,"",tucRet_Buf);//0x000E换热状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//24,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Heat_exchanger_starting_temperature/10.0),"",tucRet_Buf);//0x0038换热启动温度 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//25,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Heat_exchanger_stop_return_difference_temperature/10.0),"",tucRet_Buf);//0x0039换热停止回差值 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//26,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Internal_fan_speed),"",tucRet_Buf);				//0内风机转速  231202 不能10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//27,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Internal_fan_state & BIT(0)) << 8,"",tucRet_Buf);//0x0000内风机运行状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Internal_fan_alarm & BIT(0)) << 8,"",tucRet_Buf);//0x0000内风机告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//29,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.External_fan_speed),"",tucRet_Buf);//2外风机转速		231202 不能10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//30,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.External_fan_state & BIT(0)) << 8,"",tucRet_Buf);//0x0002外风机运行状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//31,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.External_fan_alarm & BIT(0)) << 8,"",tucRet_Buf);//0x0002外风机告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Cooing_state & BIT(0)) << 8,"",tucRet_Buf);//0x0004制冷运行状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//33,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Compressor_starting_temperature/10.0),"",tucRet_Buf);//0x0028制冷启动温度 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//34,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Compressor_stop_return_difference_temperature/10.0),"",tucRet_Buf);//0x0029制冷停止回差值 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//35,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Compressor_fault & BIT(0)) << 8,"",tucRet_Buf);//0x0004压缩机故障
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//36,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Heating_state & BIT(0)) << 8,"",tucRet_Buf);//0x0005制热运行状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//37,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Heater_starting_temperature/10.0),"",tucRet_Buf);//0x0016制热启动温度 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//38,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Heater_stop_return_difference_temperature/10.0),"",tucRet_Buf);//0x0017制热停止回差值 /10.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//39,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_Air.Heater_current/100.0),"",tucRet_Buf);//A加热器电流 /100.0
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//40,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Heater_overload_alarm & BIT(0)) << 8,"",tucRet_Buf);//0x0008加热器过流告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//41,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,
																									(g_Air.Heater_underload_alarm & BIT(0)) << 8,"",tucRet_Buf);//0x0009加热器欠流告警
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
		}		
//-------------------- 逆变器数据 ------------------------------------------------------------------------------------------
		else if( (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) >= 48) && (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) <= 55) && (Vh_Topic[11]=='0') ) {
			//debug
//			g_Inverter.fVoltage_AC=235;												//2、交流电压               0x000A
//			g_Inverter.fP_AC_Instantaneous=0x0141;//321 			//3、AC瞬时功率，单位:W     0x005E
//			g_Inverter.fVoltage_BAT=52;												//4、蓄电池电压             0x0008
//			g_Inverter.fCurrent_DC=5.4;												//5、直流电流               0x0009
//			g_Inverter.usPower_Rated=2500;              			//6、额定功率               0x0003
//			g_Inverter.usVoltage_Nominal_DC_Input=48;					//7、标称直流输入电压       0x0004
//			g_Inverter.usVoltage_Nominal_AC_Output=230;				//8、标称交流输出电压       0x0005
//			g_Inverter.usFrequency_Nominal_AC_Output=50;			//9、标称交流输出频率       0x0006
//			g_Inverter.ulTm_Accumulated_Work=0x7B;//123				//10、累计工作时间，单位:分钟0x0022-0x0023
//			g_Inverter.usFault=0xFF;													//11-16、故障                0x0015
//			g_Inverter.usAlarm=0x01FF;												//17-24、报警                0x0016
//			g_Inverter.usStatus_Run=1;              					//25、运行状态               0x005C
//			g_Inverter.ulTotal_Load_Electricity_Consumption=0x0234;//564 //26、负载总用电，单位:KWh   x0051-0x0052
//			g_Inverter.ucState_Boat_SW=2;            					//27、船型开关状态           0x0050
//																												//28、power设备状态
//																												//29、power DC_power
//			g_Inverter.fV_DC_LVD=47.2;                  			//30、          
//			g_Inverter.fV_DC_LVR=52;													//31、
//			g_Inverter.fV_DC_LVD_Warn=48.4;										//32、
//																												//33、网关状态
//																												//34、corporate name
//																												//35、Product model 
//																												//36、Hardware version
//																												//37、Software version
//																												//38、device serial number
//			g_Inverter.fCurrent_AC=2.3;												//、交流电流               	 0x000B
//			
//			g_Inverter.Device_INFO.COMPANY_TB[0]=11;
//			memcpy(&g_Inverter.Device_INFO.COMPANY_TB[1],"MorningStar",g_Inverter.Device_INFO.COMPANY_TB[0]);
//			g_Inverter.Device_INFO.Model[0]=17;
//			memcpy(&g_Inverter.Device_INFO.Model[1],     "SI-2500-48-120-60",g_Inverter.Device_INFO.Model[0]);
//			g_Inverter.Device_INFO.HV[0]=16;
//			memcpy(&g_Inverter.Device_INFO.HV[1],        "SureSine-2500-V6",g_Inverter.Device_INFO.HV[0]);
//			g_Inverter.Device_INFO.SV[0]=14;
//			memcpy(&g_Inverter.Device_INFO.SV[1],        "SureSine-V1.10",g_Inverter.Device_INFO.SV[0]);
//			g_Inverter.Device_INFO.SN[0]=8;
//			memcpy(&g_Inverter.Device_INFO.SN[1],        "143920130",g_Inverter.Device_INFO.SN[0]);
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.fVoltage_AC),"",tucRet_Buf);//交流电压
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.fP_AC_Instantaneous),"",tucRet_Buf);//交流功率
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.fVoltage_BAT),"",tucRet_Buf);//电池电压
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.fCurrent_DC),"",tucRet_Buf);//直流电流
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.usPower_Rated),"",tucRet_Buf);//额定功率
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.usVoltage_Nominal_DC_Input),"",tucRet_Buf);//标称DC
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.usVoltage_Nominal_AC_Output),"",tucRet_Buf);//标称AC
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.usFrequency_Nominal_AC_Output),"",tucRet_Buf);//标称频率
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//10,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.ulTm_Accumulated_Work/60.0f),"",tucRet_Buf);//累加工作时间
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usFault)&BIT(1)) << 7),"",tucRet_Buf);//交流过流,BIT(1)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usFault)&BIT(2)) << 6),"",tucRet_Buf);//交流短路,BIT(2)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usFault)&BIT(3)) << 5),"",tucRet_Buf);//HVD断开,BIT(3)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
				
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usFault)&BIT(4)) << 4),"",tucRet_Buf);//LVD断开,BIT(4)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usFault)&BIT(5)) << 3),"",tucRet_Buf);//散热器高温断开,BIT(5)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usFault)&BIT(6)) << 2),"",tucRet_Buf);//逆变器故障,BIT(6)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;			
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//17,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usAlarm)&BIT(0)) << 8),"",tucRet_Buf);//散热器温度传感器断开,BIT(0)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//18,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usAlarm)&BIT(1)) << 7),"",tucRet_Buf);//散热器温度传感器短路,BIT(1)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//19,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usAlarm)&BIT(2)) << 6),"",tucRet_Buf);//环境温度传感器断开,BIT(2)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//20,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usAlarm)&BIT(3)) << 5),"",tucRet_Buf);//环境温度传感器短路,BIT(3)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//21,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usAlarm)&BIT(4)) << 4),"",tucRet_Buf);//HVD高压报警,BIT(4)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usAlarm)&BIT(5)) << 3),"",tucRet_Buf);//LVD低压报警,BIT(5)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//23,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,((((unsigned short)g_Inverter.usAlarm)&BIT(7)) << 1),"",tucRet_Buf);//逆变器交流过流,BIT(7)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//24,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,g_Inverter.usAlarm&BIT(8),"",tucRet_Buf);//散热器温度太高,BIT(8)
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//25,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.usStatus_Run),"",tucRet_Buf);//运行状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//26,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.ulTotal_Load_Electricity_Consumption),"",tucRet_Buf);//总负载功率消耗HIword（KWH）
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//27,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.ucState_Boat_SW),"",tucRet_Buf);//船型开关状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28,
																									E_DATA_TYPE_DI,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_2_BYTE,0x0100,"",tucRet_Buf);//power设备状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//29,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.fVoltage_BAT*g_Inverter.fCurrent_DC),"",tucRet_Buf);//直流功率
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//30,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.fV_DC_LVD),"",tucRet_Buf);//LVD
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//31,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.fV_DC_LVR),"",tucRet_Buf);//LVR
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_RD_AND_WR,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(g_Inverter.fV_DC_LVD_Warn),"",tucRet_Buf);//LVD_Warn
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//33,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,float_to_int(1),"",tucRet_Buf);//网关状态
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			memset(tcStr_Buf,0,sizeof(tcStr_Buf));
			memcpy(tcStr_Buf,&g_Inverter.Device_INFO.COMPANY_TB[1],g_Inverter.Device_INFO.COMPANY_TB[0]);
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//34,
																									E_DATA_TYPE_STRING,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									strlen(tcStr_Buf),0,tcStr_Buf,tucRet_Buf);//设备信息.公司名
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			memset(tcStr_Buf,0,sizeof(tcStr_Buf));
			memcpy(tcStr_Buf,&g_Inverter.Device_INFO.Model[1],g_Inverter.Device_INFO.Model[0]);
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//35,
																									E_DATA_TYPE_STRING,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									strlen(tcStr_Buf),0,tcStr_Buf,tucRet_Buf);//设备信息.产品型号
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			memset(tcStr_Buf,0,sizeof(tcStr_Buf));
			memcpy(tcStr_Buf,&g_Inverter.Device_INFO.HV[1],g_Inverter.Device_INFO.HV[0]);
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//36,
																									E_DATA_TYPE_STRING,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									strlen(tcStr_Buf),0,tcStr_Buf,tucRet_Buf);//设备信息.硬件版本
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			memset(tcStr_Buf,0,sizeof(tcStr_Buf));
			memcpy(tcStr_Buf,&g_Inverter.Device_INFO.SV[1],g_Inverter.Device_INFO.SV[0]);
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//37,
																									E_DATA_TYPE_STRING,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									strlen(tcStr_Buf),0,tcStr_Buf,tucRet_Buf);//设备信息.软件版本
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			memset(tcStr_Buf,0,sizeof(tcStr_Buf));
			memcpy(tcStr_Buf,&g_Inverter.Device_INFO.SN[1],g_Inverter.Device_INFO.SN[0]);
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//38,
																									E_DATA_TYPE_STRING,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									strlen(tcStr_Buf),0,tcStr_Buf,tucRet_Buf);//设备信息.序列号
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			tucBuf[tusPos_Pl_Para_Item_Total_Num++]= tusIndex_Pl_Para_Item     & 0xFF;
		}
//-------------------- RECT数据 ----------------------------------------------------------------------------------------------
		else if( (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) >= 56) && (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) <= 63) ) {
			//debug
//g_Rectifier.ucNum_RC_Distribution_Monitor_Module=16;								//2、CID1=41H,CID2=41H
//g_Rectifier.fV_RC_Distribution_Output=48.5;													//3、dc输出电压 CID1=41H,CID2=41H
//g_Rectifier.fV_AC_Distribution_A_Phase=231;													//4、AC Input voltage CID1=40H,CID2=41H
//g_Rectifier.fPara_DC_Distribution_Get[0]=54.5;											//5、直流电压上限CID1=42H,CID2=46H
//g_Rectifier.fPara_DC_Distribution_Get[1]=46.5;											//6、直流电压下限CID1=42H,CID2=46H
//g_Rectifier.fPara_DC_Distribution_Get[4]=57.8;											//7、高压关机值CID1=42H,CID2=46H
//g_Rectifier.fPara_DC_Distribution_Get[5]=51.3;											//8、均充电压CID1=42H,CID2=46H
//g_Rectifier.fPara_DC_Distribution_Get[6]=50.4;											//9、浮充电压CID1=42H,CID2=46H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[0]=1.1;								//10、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[1]=1.2;								//11、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[2]=1.3;								//12、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[3]=1.4;								//13、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[4]=1.5;								//14、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[5]=1.6;								//15、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[6]=1.7;								//16、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[7]=1.8;								//17、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[8]=1.9;								//18、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[9]=2.0;								//19、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[10]=2.1;								//20、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[11]=2.2;								//21、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[12]=2.3;								//22、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[13]=2.4;								//23、CID1=41H,CID2=41H
//g_Rectifier.fI_RC_Distribution_Moudle_Output[14]=2.5;								//24、CID1=41H,CID2=41H

//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[0][0] =1;					//25、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[1][0] =1;					//26、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[2][0] =1;					//27、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[3][0] =1;					//28、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[4][0] =1;					//29、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[5][0] =1;					//30、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[6][0] =1;					//31、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[7][0] =1;					//32、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[8][0] =1;					//33、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[9][0] =1;					//34、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[10][0]=1;					//35、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[11][0]=1;					//36、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[12][0]=1;					//37、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[13][0]=1;					//38、CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[14][0]=1;					//39、CID1=41H,CID2=44H

//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[0][0] =1;  	//40、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[1][0] =1;  	//41、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[2][0] =1;  	//42、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[3][0] =1;  	//43、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[4][0] =1;  	//44、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[5][0] =1;  	//45、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[6][0] =1;  	//46、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[7][0] =1;  	//47、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[8][0] =1;  	//48、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[9][0] =1;  	//49、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[10][0]=1; 	//50、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[11][0]=1; 	//51、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[12][0]=1; 	//52、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[13][0]=1; 	//53、Power On/Off CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[14][0]=1; 	//54、Power On/Off CID1=41H,CID2=43H

//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[0][1] =1;  	//55、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[1][1] =1;  	//56、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[2][1] =1;  	//57、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[3][1] =1;  	//58、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[4][1] =1;  	//59、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[5][1] =1;  	//60、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[6][1] =1;  	//61、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[7][1] =1;  	//62、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[8][1] =1;  	//63、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[9][1] =1;  	//64、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[10][1]=1; 	//65、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[11][1]=1; 	//66、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[12][1]=1; 	//67、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[13][1]=1; 	//68、Current limit CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[14][1]=1; 	//69、Current limit CID1=41H,CID2=43H


//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[0][1] =1;  //2、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[1][1] =1;  //3、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[2][1] =1;  //4、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[3][1] =1;  //5、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[4][1] =1;  //6、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[5][1] =1;  //7、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[6][1] =1;  //8、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[7][1] =1;  //9、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[8][1] =1;  //10、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[9][1] =1;  //11、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[10][1]=1;  //12、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[11][1]=1;  //13、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[12][1]=1;  //14、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[13][1]=1;  //15、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[14][1]=1;  //16、高压关机,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[0][2] =1;  //17、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[1][2] =1;  //18、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[2][2] =1;  //19、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[3][2] =1;  //20、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[4][2] =1;  //21、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[5][2] =1;  //22、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[6][2] =1;  //23、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[7][2] =1;  //24、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[8][2] =1;  //25、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[9][2] =1;  //26、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[10][2]=1;  //27、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[11][2]=1;  //28、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[12][2]=1;  //29、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[13][2]=1;  //30、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[14][2]=1;  //31、通讯故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[0][3] =1;  //32、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[1][3] =1;  //33、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[2][3] =1;  //34、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[3][3] =1;  //35、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[4][3] =1;  //36、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[5][3] =1;  //37、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[6][3] =1;  //38、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[7][3] =1;  //39、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[8][3] =1;  //40、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[9][3] =1;  //41、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[10][3]=1;  //42、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[11][3]=1;  //43、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[12][3]=1;  //44、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[13][3]=1;  //45、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[14][3]=1;  //46、风扇故障,CID1=41H,CID2=44H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[0][2] =1;  	//47、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[1][2] =1;  	//48、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[2][2] =1;  	//49、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[3][2] =1;  	//50、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[4][2] =1;  	//51、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[5][2] =1;  	//52、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[6][2] =1;  	//53、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[7][2] =1;  	//54、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[8][2] =1;  	//55、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[9][2] =1;  	//56、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[10][2]=1; 	//57、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[11][2]=1; 	//58、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[12][2]=1; 	//59、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[13][2]=1; 	//60、Charge state CID1=41H,CID2=43H
//g_Rectifier.ucState_RC_Distribution_System_Input_Switch[14][2]=1; 	//61、Charge state CID1=41H,CID2=43H
			
			if(Vh_Topic[11]=='0'){//整流器第1包数据
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucNum_RC_Distribution_Monitor_Module),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fV_RC_Distribution_Output),"",tucRet_Buf);//dc输出电压 CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fV_AC_Distribution_A_Phase),"",tucRet_Buf);//AC Input voltage CID1=40H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fPara_DC_Distribution_Get[0]),"",tucRet_Buf);//直流电压上限CID1=42H,CID2=46H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fPara_DC_Distribution_Get[1]),"",tucRet_Buf);//直流电压下限CID1=42H,CID2=46H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fPara_DC_Distribution_Get[4]),"",tucRet_Buf);//高压关机值CID1=42H,CID2=46H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fPara_DC_Distribution_Get[5]),"",tucRet_Buf);//均充电压CID1=42H,CID2=46H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fPara_DC_Distribution_Get[6]),"",tucRet_Buf);//浮充电压CID1=42H,CID2=46H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//10,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[0]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[1]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[2]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[3]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[4]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[5]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[6]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//17,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[7]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//18,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[8]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//19,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[9]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//20,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[10]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//21,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[11]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[12]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//23,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[13]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//24,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.fI_RC_Distribution_Moudle_Output[14]),"",tucRet_Buf);//CID1=41H,CID2=41H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;			
							
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//25,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[0][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//26,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[1][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//27,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[2][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[3][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//29,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[4][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//30,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[5][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//31,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[6][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[7][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//33,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[8][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//34,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[9][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//35,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[10][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//36,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[11][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//37,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[12][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//38,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[13][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//39,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[14][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;			
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//40,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[0][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//41,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[1][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//42,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[2][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//43,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[3][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//44,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[4][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//45,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[5][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//46,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[6][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//47,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[7][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//48,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[8][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//49,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[9][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//50,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[10][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//51,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[11][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//52,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[12][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//53,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[13][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//54,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[14][0]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;			
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//55,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[0][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//56,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[1][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//57,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[2][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//58,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[3][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//59,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[4][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//60,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[5][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//61,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[6][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//62,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[7][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//63,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[8][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//64,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[9][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//65,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[10][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//66,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[11][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//67,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[12][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//68,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[13][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//69,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[14][1]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			}else if(Vh_Topic[11]=='1'){//整流器第2包数据
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[0][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[1][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[2][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[3][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[4][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[5][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[6][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[7][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//10,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[8][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[9][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[10][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[11][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[12][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[13][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[14][1]),"",tucRet_Buf);//高压关机,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//17,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[0][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//18,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[1][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//19,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[2][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//20,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[3][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//21,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[4][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[5][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//23,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[6][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//24,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[7][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//25,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[8][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//26,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[9][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//27,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[10][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[11][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//29,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[12][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//30,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[13][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//31,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[14][2]),"",tucRet_Buf);//通讯故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[0][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//33,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[1][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//34,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[2][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//35,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[3][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//36,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[4][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//37,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[5][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//38,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[6][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//39,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[7][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//40,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[8][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//41,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[9][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//42,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[10][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//43,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[11][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//44,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[12][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//45,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[13][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//46,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucAlarm_State_RC_Distribution_Moudle[14][3]),"",tucRet_Buf);//风扇故障,CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//47,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[0][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//48,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[1][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//49,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[2][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//50,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[3][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//51,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[4][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//52,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[5][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//53,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[6][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//54,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[7][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//55,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[8][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//56,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[9][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//57,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[10][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//58,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[11][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//59,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[12][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//60,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[13][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//61,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Rectifier.ucState_RC_Distribution_System_Input_Switch[14][2]),"",tucRet_Buf);//CID1=41H,CID2=44H
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			}
		}
//-------------------- 电池数据 ----------------------------------------------------------------------------------------------
		else if( (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) >= 64) && (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) <= 71) ) {
			tusIndex_Count=TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) - 64;
			//debug
//			g_PYLON_BAT.Get_System_Basic_INFO[0].ucBAT_Num_Same_Group=5;																	//2、
//			memcpy(g_PYLON_BAT.Get_System_Basic_INFO[0].ucSN_Master_Moudle,"ABCDEFGHIJKLMNOP",16);					//3、
//			g_PYLON_BAT.Get_System_Analog[tusIndex_Count].faverage_module_voltage=52.3;										//4、
//			g_PYLON_BAT.Get_System_Analog[tusIndex_Count].ftotal_current=35.6;														//5、
//			g_PYLON_BAT.Get_System_Analog[tusIndex_Count].ucaverage_SOC=4;																//6、
//			g_PYLON_BAT.Get_System_Analog[tusIndex_Count].ucaverage_cycle_number=13;											//7、
//			g_PYLON_BAT.Get_System_Analog[tusIndex_Count].ucaverage_SOH=5;																//8、
//			g_PYLON_BAT.Get_System_Analog[tusIndex_Count].faverage_cell_temperature=23.1;									//9、
//			g_PYLON_BAT.Get_System_Analog[tusIndex_Count].faverage_MOSFET_temperature=25.6;								//10、
//			g_PYLON_BAT.Get_System_Analog[tusIndex_Count].faverage_BMS_temperature=24.3;									//11、
//			g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][0]=0xFF;																						//12-20、
//			g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][1]=0xFF;																						//21-23、
//			g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][2]=0xFF;																						//24-30、
//			g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][3]=0xFF;																						//31-33、
//			g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].fCharge_Voltage_Upper_Limit=58.7;			//34、充电电压上限
//			g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].fDischarge_Voltage_Lower_Limit=48.2;	//35、放电电压下限
//			g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].fCharge_Current_Limit=65;							//36、最大充电电流
//			g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].fDischarge_Current_Limit=63;					//37、最大放电电流
//			g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].ucCharge_Discharge_Status=0xFF;				//38-40、

			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_System_Basic_INFO[0].ucBAT_Num_Same_Group),"",tucRet_Buf);
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																									E_DATA_TYPE_STRING,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									strlen((char *)g_PYLON_BAT.Get_System_Basic_INFO[0].ucSN_Master_Moudle),
																									0,(char *)g_PYLON_BAT.Get_System_Basic_INFO[0].ucSN_Master_Moudle,tucRet_Buf);//16位
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_System_Analog[tusIndex_Count].faverage_module_voltage),"",tucRet_Buf);
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_System_Analog[tusIndex_Count].ftotal_current),"",tucRet_Buf);
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_System_Analog[tusIndex_Count].ucaverage_SOC),"",tucRet_Buf);
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_System_Analog[tusIndex_Count].ucaverage_cycle_number),"",tucRet_Buf);
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_System_Analog[tusIndex_Count].ucaverage_SOH),"",tucRet_Buf);
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_System_Analog[tusIndex_Count].faverage_cell_temperature),"",tucRet_Buf);
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//10,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_System_Analog[tusIndex_Count].faverage_MOSFET_temperature),"",tucRet_Buf);
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_System_Analog[tusIndex_Count].faverage_BMS_temperature),"",tucRet_Buf);
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][0] & BIT(7)) >> 7),"",tucRet_Buf);//模块总压高压 / module high voltage 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][0] & BIT(6)) >> 6),"",tucRet_Buf);//模块总压低压 / module low voltage 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][0] & BIT(5)) >> 5),"",tucRet_Buf);//单芯电压高压 / cell high voltage 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int( (g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][0] & BIT(4)) >> 4),"",tucRet_Buf);//单芯电压低压 / cell low voltage 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][0] & BIT(3)) >> 3),"",tucRet_Buf);//单芯温度高温 / cell high temperature 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//17,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][0] & BIT(2)) >> 2),"",tucRet_Buf);//单芯温度低温 / cell low temperature 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//18,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][0] & BIT(0)),"",tucRet_Buf);//单芯电压一致性告警 / high voltage difference between cells 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//19,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][1] & BIT(7)) >> 7),"",tucRet_Buf);//单芯温度一致性告警 / high temperature difference between cells 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//20,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][0] & BIT(1)) >> 1),"",tucRet_Buf);//MOSFET 高温 / MOSFET high temperature 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//21,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][1] & BIT(4)) >> 4),"",tucRet_Buf);//内部通信错误 / internal communication failure, salve battery offline 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][1] & BIT(6)) >> 6),"",tucRet_Buf);//充电过流告警 / charge high current 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//23,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][1] & BIT(5)) >> 5),"",tucRet_Buf);//放电过流告警 / discharge high current 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//24,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][2] & BIT(7)) >> 7),"",tucRet_Buf);//模块总压过压 / module over voltage 0: normal； 1: trigge
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//25,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][2] & BIT(6)) >> 6),"",tucRet_Buf);//模块总压欠压 / module under voltage 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//26,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][2] & BIT(5)) >> 5),"",tucRet_Buf);//单芯电压过压 / cell over voltage 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//27,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][2] & BIT(4)) >> 4),"",tucRet_Buf);//单芯电压欠压 / cell under voltage 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][2] & BIT(3)) >> 3),"",tucRet_Buf);//单芯温度过温 / cell over temperature 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//29,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][2] & BIT(2)) >> 2),"",tucRet_Buf);//单芯温度欠温 / cell under temperature 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//30,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][2] & BIT(1)) >> 1),"",tucRet_Buf);//MOSFET 过温 / MOSFET over temperature 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//31,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][3] & BIT(6)) >> 6),"",tucRet_Buf);//充电过流保护 / charge over current 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][3] & BIT(5)) >> 5),"",tucRet_Buf);//放电过流保护 / discharge over current 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//33,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.ucSystem_Alarm[tusIndex_Count][3] & BIT(3)) >> 3),"",tucRet_Buf);//系统故障保护 / BMS error 0: normal； 1: trigger
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//34,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].fCharge_Voltage_Upper_Limit),"",tucRet_Buf);//充电电压上限
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//35,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].fDischarge_Voltage_Lower_Limit),"",tucRet_Buf);//放电电压下限
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//36,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].fCharge_Current_Limit),"",tucRet_Buf);//最大充电电流
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//37,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int(g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].fDischarge_Current_Limit),"",tucRet_Buf);//最大放电电流
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//38,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].ucCharge_Discharge_Status & BIT(7)) >> 7 ),"",tucRet_Buf);//Charge enable
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);																						
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//39,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].ucCharge_Discharge_Status & BIT(6)) >> 6),"",tucRet_Buf);//Discharge enable
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
			
			tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//40,
																									E_DATA_TYPE_FLOAT,
																									E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																									E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																									E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																									E_DATA_LENGTH_4_BYTE,
																									float_to_int((g_PYLON_BAT.Get_BAT_Charge_Discharge_MI[tusIndex_Count].ucCharge_Discharge_Status & BIT(5)) >> 5),"",tucRet_Buf);//charge immediately
			memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
			tusCount+=tucRet_Len;
		}
//-------------------- 柴油发电机数据 ----------------------------------------------------------------------------------------------
		else if( (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) >= 72) && (TwoASCII_to_1ByteData(Vh_Topic[9],Vh_Topic[10]) <= 79) ) {
			//debug   
//			g_Diesel_Generator.IO_Point.Digital_Output.ucFuel_Relay=1;										//2、燃油输出						R	0	1	16	48640
//			g_Diesel_Generator.IO_Point.Digital_Output.ucStart_Relay=1;									//3、启动输出						R	0	1	16	48641
//			g_Diesel_Generator.LED_Status.ucSTOP_LED_status=1;														//4、停止状态
//			g_Diesel_Generator.LED_Status.ucMANUAL_LED_status=1;													//5、手动状态
//			g_Diesel_Generator.LED_Status.ucTEST_LED_status=1;														//6、测试状态
//			g_Diesel_Generator.LED_Status.ucAUTO_LED_status=1;														//7、自动状态
//			g_Diesel_Generator.LED_Status.ucGEN_LED_status=1;														//8、机组有效							R	0	1	16	48655
//			g_Diesel_Generator.LED_Status.ucGEN_BREAKER_LED_status=1;										//9、机组断路器合闸				R	0	1	16	48654
//			g_Diesel_Generator.LED_Status.ucMAINS_LED_status=1;													//10、市电有效 							R	0	1	16	48652
//			g_Diesel_Generator.LED_Status.ucMAINS_BREAKER_LED_status=1;									//11、市电断路器合闸 				R	0	1	16	48653
//			g_Diesel_Generator.Value_Display.Engine_Para.usOil_pressure=56;							//12、发动机油压
//			g_Diesel_Generator.Value_Display.Engine_Para.fCoolant_temperature=78;				//13、冷却液温度
//			g_Diesel_Generator.Value_Display.Engine_Para.usFule_level=90;								//14、油量
//			g_Diesel_Generator.Value_Display.Engine_Para.fCharge_alternator_voltage=54.5;//15、充电发动机电压,			1028				R	0	       		40	        0.1	  V			16
//			g_Diesel_Generator.Value_Display.Engine_Para.fEngine_battery_voltage=48.9;	  //16、发动机电池电压,			1029				R	0	       		40	        0.1		V			16
//			g_Diesel_Generator.Value_Display.Engine_Para.usEngine_speed=5678;						//17、发动机转速
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_frequency=50;			//18、发电机频率
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1_N_voltage=221;	//19、L1-N相电压
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L2_N_voltage=222;	//20、L2-N相电压
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L3_N_voltage=223;	//21、L3-N相电压
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1_L2_voltage=381;//22、L1-L2线电压					1038和1039	R	0						30,000			0.1		V				32
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L2_L3_voltage=382;//23、L2-L3线电压					1040和1041	R	0						30,000			0.1		V				32
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L3_L1_voltage=383;//24、L3-L1线电压					1042和1043	R	0						30,000			0.1		V				32
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1_current=12;		//25、L1相电流
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L2_current=13;		//26、L2相电流
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L3_current=14;		//27、L3相电流
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_earth_current=78;	//28、接地电流
//			g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L1_watts=67;			//29、L1相有功功率
//			g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L2_watts=68;			//30、L2相有功功率
//			g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L3_watts=69;			//31、L3相有功功率
//			g_Diesel_Generator.Value_Display.Generator_Para.ssGenerator_current_lag_lead=123;//32、电流超前/滞后				1058				R	-180				180					1			度			16 S
//			g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_total_watts=321;	//33、总的有功功率
//			g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_L1_VA=881;				//34、L1相视在功率				1538和1539	R	0						99,999,999	1			VA			32
//			g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_L2_VA=882;				//35、L2相视在功率				1540和1541	R	0						99,999,999	1			VA			32
//			g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_L3_VA=883;				//36、L3相视在功率				1542和1543	R	0						99,999,999	1			VA			32
//			g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_total_VA=4567;		//37、总视在功率					1544和1545	R	0						99,999,999	1			VA			32S
//			g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L1_Var=771;			//38、L1相无功功率				1546和1547	R	-99,999,999	99,999,999	1			Var			32S
//			g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L2_Var=772;			//39、L2相无功功率				1548和1549	R	-99,999,999	99,999,999	1			Var			32S
//			g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L3_Var=773;			//40、L3相无功功率				1550和1551	R	-99,999,999	99,999,999	1			Var			32S
//			g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_total_Var=5678;	//41、总无功功率					1552和1553	R	-99,999,999	99,999,999	1	   	Var			32S
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_power_factor_L1=0.3;						//42、L1相功率因素				1554				R	-1	        1	          0.01					16S
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_power_factor_L2=0.4;						//43、L2相功率因素				1555				R	-1					1						0.01					16S
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_power_factor_L3=0.5;						//44、L3相功率因素				1556				R	-1					1						0.01					16S
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_average_power_factor=0.6;				//45、平均功率因素				1557				R	-1					1						0.01					16S
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_percentage_of_full_power=-123.4;//46、总功率的百分比			1558				R	-999.9			999.9				0.1		%				16S
//			g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_percentage_of_full_Var=345.6;	 	//47、总无功功率的百分比	1559				R	-999.9			999.9				0.1		%				16S
//			g_Diesel_Generator.Value_Display.Engine_Para.ulEngine_run_time=1234;					//48、发动机运行时间,generator Engine run time (min)
//			g_Diesel_Generator.Value_Display.Engine_Para.ulNumber_of_starts=9876;				//49、启动次数
//			g_Diesel_Generator.Value_Display.Generator_Para.ssGenerator_current_lag_lead=123;//50、电流超前/滞后				1058				R	-180				180					1			度			16 S
//			

//			g_Diesel_Generator.Alarm.usReg_Addr_39425=0xFFFF;	//2-5、		发机机低水温				Low coolant temperature  							R 0 15 	1/16-4/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39425;				//     		发动机高水温				high coolant temperature 							R 0 15	5/16-8/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39425;				//     		发动机低油压				Low oil pressure 											R 0 15 	9/16-12/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39425;       //     		急停       	 				Emergency stop           							R 0 15 	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39426=0xFFFF;	//6-9、		过频        				Generator Over frequency 							R 0 15 	1/16-4/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39426;      	//				低频        				Generator Under frequency 						R 0 15 	5/16-8/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39426;				//				超速        				Over speed                						R 0 15 	9/16-12/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39426;				//				低速        				Under speed               						R 0 15 	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39427=0xFFFF;	//10-13、	蓄电池高电压  			Battery high voltage    							R 0 15 	1/16-4/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39427;				//				蓄电池低电压  			Battery low voltage     							R 0 15 	5/16-8/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39427;				//				过压         				Generator high voltage  							R 0 15 	9/16-12/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39427;				//				低压          			Generator low  voltage  							R 0 15 	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39428=0xFFFF;	//14-17、	发电机合闸失败			Generator fail to close 							R 0 15 	1/16-4/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39428;				//				停止失败      			Fail to stop            							R 0 15 	5/16-8/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39428;				//				启动失败      			Fail to start           							R 0 15 	9/16-12/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39428;				//				充电发电机失败			Charge alternator failure 						R 0 15 	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39429=0xFFFF;	//18-21、	转速传感器开路			Magnetic pick up open circuit					R	0	15	1/16-4/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39429;				//				转速传感器信号丢失	Loss of magnetic pick up							R	0	15	5/16-8/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39429;				//				油压传感器故障			Oil pressure sender fault							R	0	15	9/16-12/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39429;				//				市电合闸失败				Mains fail to close										R	0	15	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39430=0xFFFF;	//22-24、	CAN ECU报警					CAN ECU Warning												R	0	15	1/16-4/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39430;				//				低油位							Low fuel level			 									R	0	15	5/16-8/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39430=0xFFFF;//				过流 								Generator high current								R	0	15	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39431=0xFFFF;	//25-28、	高水温开关量				High temperature switch								R	0	15	1/16-4/16 
////			g_Diesel_Generator.Alarm.usReg_Addr_39431;				//				机油油位低开关量		Low oil level switch									R	0	15	5/16-8/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39431;				//				CAN ECU数据通讯失败	CAN ECU Data fail											R	0	15	9/16-12/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39431;				//				CAN ECU停机					CAN ECU Shutdown											R	0	15	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39432=0xFFFF;	//29-32、	三相电流不平衡报警	Negative phase sequence current alarm	R	0	15	1/16-4/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39432;				//				超载报警						kW overload alarm											R	0	15	5/16-8/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39432;				//				扩展模块看门狗报警	Expansion unit watchdog alarm					R	0	15	9/16-12/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39432;				//				低燃油低开关量			Low fuel level switch									R	0	15	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39433=0xFFFF;	//33-36、	维护保养报警				Maintenance alarm											R	0	15	1/16-4/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39433;				//				自动电压检测失败		Auto Voltage Sense Fail								R	0	15	5/16-8/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39433;				//				相序报警						Generator phase rotation alarm				R	0	15	9/16-12/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39433;				//				接地故障报警				Earth fault trip alarm								R	0	15	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39434=0xFFFF;  //37-38、	带载电压报警				Loading voltage alarm									R	0	15	9/16-12/16 
////			g_Diesel_Generator.Alarm.usReg_Addr_39434;				//				带载频率报警				Loading frequency alarm								R	0	15	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39435=0xFFFF;	//39-40、	市电过流						Mains High Current										R	0	15	1/16-4/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39435;				//				发电机短路					Generator Short Circuit								R	0	15	5/16-8/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39436=0xFFFF;	//41-43、	ECU保护							ECU protect														R	0	15	5/16-8/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39436;				//				市电短路						Mains Short Circuit										R	0	15	9/16-12/16
////			g_Diesel_Generator.Alarm.usReg_Addr_39436;				//				市电接地故障				Mains Earth Fault											R	0	15	13/16-16/16
//			g_Diesel_Generator.Alarm.usReg_Addr_39442=0xFFFF;	//44、		水温传感器开路报警	Coolant sensor open circuit						R	0	15	13/16-16/16

			if(Vh_Topic[11]=='0'){//整流器第1包数据
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(((unsigned short)g_Diesel_Generator.IO_Point.Digital_Output.ucFuel_Relay) <<8),"",tucRet_Buf);//燃油输出						R	0	1	16	48640
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(((unsigned short)g_Diesel_Generator.IO_Point.Digital_Output.ucStart_Relay) <<8),"",tucRet_Buf);//启动输出						R	0	1	16	48641
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(((unsigned short)g_Diesel_Generator.LED_Status.ucSTOP_LED_status) <<8),"",tucRet_Buf);//停止状态
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(((unsigned short)g_Diesel_Generator.LED_Status.ucMANUAL_LED_status) <<8),"",tucRet_Buf);//手动状态
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(((unsigned short)g_Diesel_Generator.LED_Status.ucTEST_LED_status) <<8),"",tucRet_Buf);//测试状态
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(((unsigned short)g_Diesel_Generator.LED_Status.ucAUTO_LED_status) <<8),"",tucRet_Buf);//自动状态
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(((unsigned short)g_Diesel_Generator.LED_Status.ucGEN_LED_status) <<8),"",tucRet_Buf);//机组有效							R	0	1	16	48655
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(((unsigned short)g_Diesel_Generator.LED_Status.ucGEN_BREAKER_LED_status) <<8),"",tucRet_Buf);//机组断路器合闸				R	0	1	16	48654
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//10,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(((unsigned short)g_Diesel_Generator.LED_Status.ucMAINS_LED_status) <<8),"",tucRet_Buf);//市电有效 							R	0	1	16	48652
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11,
																										E_DATA_TYPE_DI,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_2_BYTE,
																										(((unsigned short)g_Diesel_Generator.LED_Status.ucMAINS_BREAKER_LED_status) <<8),"",tucRet_Buf);//市电断路器合闸 				R	0	1	16	48653
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Engine_Para.usOil_pressure),"",tucRet_Buf);//发动机油压
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Engine_Para.fCoolant_temperature),"",tucRet_Buf);//冷却液温度
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Engine_Para.usFule_level),"",tucRet_Buf);//油量
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Engine_Para.fCharge_alternator_voltage),"",tucRet_Buf);//充电发动机电压,			1028				R	0	       		40	        0.1	  V			16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Engine_Para.fEngine_battery_voltage),"",tucRet_Buf);//发动机电池电压,			1029				R	0	       		40	        0.1		V			16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//17,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Engine_Para.usEngine_speed),"",tucRet_Buf);//发动机转速
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//18,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_frequency),"",tucRet_Buf);//发电机频率
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//19,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1_N_voltage),"",tucRet_Buf);//L1-N相电压
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//20,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L2_N_voltage),"",tucRet_Buf);//L2-N相电压
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//21,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L3_N_voltage),"",tucRet_Buf);//L3-N相电压
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1_L2_voltage),"",tucRet_Buf);//L1-L2线电压					1038和1039	R	0						30,000			0.1		V				32
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//23,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L2_L3_voltage),"",tucRet_Buf);//L2-L3线电压					1040和1041	R	0						30,000			0.1		V				32
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//24,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L3_L1_voltage),"",tucRet_Buf);//L3-L1线电压					1042和1043	R	0						30,000			0.1		V				32
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//25,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L1_current),"",tucRet_Buf);//L1相电流
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//26,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L2_current),"",tucRet_Buf);//L2相电流
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//27,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_L3_current),"",tucRet_Buf);//L3相电流
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_earth_current),"",tucRet_Buf);//接地电流
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//29,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L1_watts),"",tucRet_Buf);//L1相有功功率
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//30,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L2_watts),"",tucRet_Buf);//L2相有功功率
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//31,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L3_watts),"",tucRet_Buf);//L3相有功功率
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.ssGenerator_current_lag_lead),"",tucRet_Buf);//电流超前/滞后				1058				R	-180				180					1			度			16 S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//33,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_total_watts),"",tucRet_Buf);//总的有功功率
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//34,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_L1_VA),"",tucRet_Buf);//L1相视在功率				1538和1539	R	0						99,999,999	1			VA			32
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//35,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_L2_VA),"",tucRet_Buf);//L2相视在功率				1538和1539	R	0						99,999,999	1			VA			32
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//36,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_L3_VA),"",tucRet_Buf);//L3相视在功率				1538和1539	R	0						99,999,999	1			VA			32
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//37,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.ulGenerator_total_VA),"",tucRet_Buf);//总视在功率					1544和1545	R	0						99,999,999	1			VA			32S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//38,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L1_Var),"",tucRet_Buf);//L1相无功功率				1546和1547	R	-99,999,999	99,999,999	1			Var			32S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//39,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L2_Var),"",tucRet_Buf);//L2相无功功率				1546和1547	R	-99,999,999	99,999,999	1			Var			32S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//40,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_L3_Var),"",tucRet_Buf);//L3相无功功率				1546和1547	R	-99,999,999	99,999,999	1			Var			32S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//41,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.slGenerator_total_Var),"",tucRet_Buf);//总无功功率					1552和1553	R	-99,999,999	99,999,999	1	   	Var			32S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//42,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_power_factor_L1),"",tucRet_Buf);//L1相功率因素				1554				R	-1	        1	          0.01					16S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//43,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_power_factor_L2),"",tucRet_Buf);//L2相功率因素				1554				R	-1	        1	          0.01					16S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//44,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_power_factor_L3),"",tucRet_Buf);//L3相功率因素				1554				R	-1	        1	          0.01					16S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//45,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_average_power_factor),"",tucRet_Buf);//平均功率因素				1557				R	-1					1						0.01					16S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//46,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_percentage_of_full_power),"",tucRet_Buf);//总功率的百分比			1558				R	-999.9			999.9				0.1		%				16S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//47,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.fGenerator_percentage_of_full_Var),"",tucRet_Buf);//总无功功率的百分比	1559				R	-999.9			999.9				0.1		%				16S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//48,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Engine_Para.ulEngine_run_time),"",tucRet_Buf);//发动机运行时间,generator Engine run time (min)
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;			
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//49,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Engine_Para.ulNumber_of_starts),"",tucRet_Buf);//启动次数
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//50,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Value_Display.Generator_Para.ssGenerator_current_lag_lead),"",tucRet_Buf);//电流超前/滞后				1058				R	-180				180					1			度			16 S
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			}else if(Vh_Topic[11]=='1'){//整流器第2包数据
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//2,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Alarm.usReg_Addr_39425 & 0x000F),"",tucRet_Buf);//发机机低水温Low coolant temperature  R 0 15 1/16-4/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//3,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39425 & 0x00F0) >> 4),"",tucRet_Buf);//发动机高水温high coolant temperature R 0 15 5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//4,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39425 & 0x0F00) >> 8),"",tucRet_Buf);//发动机低油压high coolant temperature R 0 15 9/16-12/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//5,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39425 & 0xF000) >> 12),"",tucRet_Buf);//急停        Emergency stop           R 0 15 13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//6,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Alarm.usReg_Addr_39426 & 0x000F),"",tucRet_Buf);//过频        Generator Over frequency R 0 15 1/16-4/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//7,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39426 & 0x00F0) >> 4),"",tucRet_Buf);//低频        Generator Under frequency R 0 15 5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//8,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39426 & 0x0F00) >> 8),"",tucRet_Buf);//超速        Over speed                R 0 15 9/16-12/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//9,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39426 & 0xF000) >> 12),"",tucRet_Buf);//低速        Under speed               R 0 15 13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//10,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Alarm.usReg_Addr_39427 & 0x000F),"",tucRet_Buf);//蓄电池高电压  Battery high voltage    R 0 15 1/16-4/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//11,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39427 & 0x00F0) >> 4),"",tucRet_Buf);//蓄电池低电压  Battery low voltage     R 0 15 5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//12,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39427 & 0x0F00) >> 8),"",tucRet_Buf);//过压          Generator high voltage  R 0 15 9/16-12/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//13,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39427 & 0xF000) >> 12),"",tucRet_Buf);//低压          Generator low  voltage  R 0 15 12/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//14,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Alarm.usReg_Addr_39428 & 0x000F),"",tucRet_Buf);//发电机合闸失败			Generator fail to close 							R 0 15 	1/16-4/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//15,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39428 & 0x00F0) >> 4),"",tucRet_Buf);//停止失败      			Fail to stop            							R 0 15 	5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//16,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39428 & 0x0F00) >> 8),"",tucRet_Buf);//启动失败      			Fail to start           							R 0 15 	9/16-12/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//17,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39428 & 0xF000) >> 12),"",tucRet_Buf);//充电发电机失败			Charge alternator failure 						R 0 15 	13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//18,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Alarm.usReg_Addr_39429 & 0x000F),"",tucRet_Buf);//转速传感器开路			Magnetic pick up open circuit					R	0	15	1/16-4/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//19,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39429 & 0x00F0) >> 4),"",tucRet_Buf);//转速传感器信号丢失	Loss of magnetic pick up							R	0	15	5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//20,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39429 & 0x0F00) >> 8),"",tucRet_Buf);//油压传感器故障			Oil pressure sender fault							R	0	15	9/16-12/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//21,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39429 & 0xF000) >> 12),"",tucRet_Buf);//Mains fail to close										R	0	15	13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//22,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Alarm.usReg_Addr_39430 & 0x000F),"",tucRet_Buf);//CAN ECU报警					CAN ECU Warning												R	0	15	1/16-4/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//23,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39430 & 0x00F0) >> 4),"",tucRet_Buf);//低油位							Low fuel level			 									R	0	15	5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//24,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39430 & 0xF000) >> 12),"",tucRet_Buf);//过流 								Generator high current								R	0	15	13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//25,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Alarm.usReg_Addr_39431 & 0x000F),"",tucRet_Buf);//高水温开关量				High temperature switch								R	0	15	1/16-4/16 
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//26,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39431 & 0x00F0) >> 4),"",tucRet_Buf);//机油油位低开关量		Low oil level switch									R	0	15	5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39431 & 0x0F00) >> 8),"",tucRet_Buf);//CAN ECU数据通讯失败	CAN ECU Data fail											R	0	15	9/16-12/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//28,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39431 & 0xF000) >> 12),"",tucRet_Buf);//CAN ECU停机					CAN ECU Shutdown											R	0	15	13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//29,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Alarm.usReg_Addr_39432 & 0x000F),"",tucRet_Buf);//三相电流不平衡报警	Negative phase sequence current alarm	R	0	15	1/16-4/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//30,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39432 & 0x00F0) >> 4),"",tucRet_Buf);//超载报警						kW overload alarm											R	0	15	5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//31,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39432 & 0x0F00) >> 8),"",tucRet_Buf);//扩展模块看门狗报警	Expansion unit watchdog alarm					R	0	15	9/16-12/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//32,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39432 & 0xF000) >> 12),"",tucRet_Buf);//低燃油低开关量			Low fuel level switch									R	0	15	13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//33,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Alarm.usReg_Addr_39433 & 0x000F),"",tucRet_Buf);//维护保养报警				Maintenance alarm											R	0	15	1/16-4/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//34,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39433 & 0x00F0) >> 4),"",tucRet_Buf);//自动电压检测失败		Auto Voltage Sense Fail								R	0	15	5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//35,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39433 & 0x0F00) >> 8),"",tucRet_Buf);////				相序报警						Generator phase rotation alarm				R	0	15	9/16-12/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//36,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39433 & 0xF000) >> 12),"",tucRet_Buf);//接地故障报警				Earth fault trip alarm								R	0	15	13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//37,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39434 & 0x0F00) >> 8),"",tucRet_Buf);//带载电压报警				Loading voltage alarm									R	0	15	9/16-12/16 
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//38,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39434 & 0xF000) >> 12),"",tucRet_Buf);//带载频率报警				Loading frequency alarm								R	0	15	13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//39,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int(g_Diesel_Generator.Alarm.usReg_Addr_39435 & 0x000F),"",tucRet_Buf);//市电过流						Mains High Current										R	0	15	1/16-4/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//40,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39435 & 0x00F0) >> 4),"",tucRet_Buf);//发电机短路					Generator Short Circuit								R	0	15	5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//41,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39436 & 0x00F0) >> 4),"",tucRet_Buf);//ECU保护							ECU protect														R	0	15	5/16-8/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//42,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39436 & 0x0F00) >> 8),"",tucRet_Buf);//市电短路						Mains Short Circuit										R	0	15	9/16-12/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//43,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39436 & 0xF000) >> 12),"",tucRet_Buf);//市电接地故障				Mains Earth Fault											R	0	15	13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
				
				tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(tusIndex_Pl_Para_Item++,//44,
																										E_DATA_TYPE_FLOAT,
																										E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																										E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																										E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																										E_DATA_LENGTH_4_BYTE,
																										float_to_int((g_Diesel_Generator.Alarm.usReg_Addr_39442 & 0xF000) >> 12),"",tucRet_Buf);//水温传感器开路报警	Coolant sensor open circuit						R	0	15	13/16-16/16
				memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
				tusCount+=tucRet_Len;
			}
		}
		tucRet_Len=Get_Publish_Pl_One_Par_Item_Data(0x0000F000,
																								E_DATA_TYPE_FLOAT,
																								E_PUBLISH_TYPE_MARK_H4BIT_DATA_ONLY_RD,
																								E_PUBLISH_TYPE_MARK_L4BIT_HEX_DATA,
																								E_PUBLISH_DATA_QUALITY_NORMAL_DATA,
																								E_DATA_LENGTH_4_BYTE,float_to_int(1),"",tucRet_Buf);//充电发电机失败Charge alternator failure R 0 15 13/16-16/16
		memcpy(&tucBuf[tusCount],tucRet_Buf,tucRet_Len);
		tusCount+=tucRet_Len;
		
		//补参数项总数
		tusIndex_Pl_Para_Item++;
		tucBuf[tusPos_Pl_Para_Item_Total_Num++]=(tusIndex_Pl_Para_Item>>8) & 0xFF;
		tucBuf[tusPos_Pl_Para_Item_Total_Num++]= tusIndex_Pl_Para_Item     & 0xFF;
	}
	//5、计算固定报头的剩余长度
	tucFh_Remain_Length_Byte_Num=MQTT_Remain_Length_Encode(tucFh_Remain_Length_Buf,tusCount+1);//+1是包括校验码一位
	memcpy(&ucMQTT_TxBuf[usMQTT_Tx_Index],tucFh_Remain_Length_Buf,tucFh_Remain_Length_Byte_Num);
	usMQTT_Tx_Index+=tucFh_Remain_Length_Byte_Num;
	memcpy(&ucMQTT_TxBuf[usMQTT_Tx_Index],tucBuf,tusCount);
	usMQTT_Tx_Index+=tusCount;
	//6、校验
	ucMQTT_TxBuf[usMQTT_Tx_Index]=MQTT_CRC(ucMQTT_TxBuf,usMQTT_Tx_Index);
	usMQTT_Tx_Index++;
	ucMQTT_TxBuf[usMQTT_Tx_Index]= '\0';
	//7、发送
#if OLD_MDOE	
	if(CmdExe((char *)ucMQTT_TxBuf,3,1,usMQTT_Tx_Index)==1)				//收20 02 00 00		if(MQTT_Send(ucMQTT_TxBuf,usMQTT_Tx_Index))
	{
		return 1;
	}
	else
	{
		return 0;
	}
	#endif
}

#if OLD_MDOE
void MQTT_Run(unsigned char work_mode)
{
	char data[256];
	static unsigned char flag = 0;
	unsigned char i;
	g_mqtt_work_mode = work_mode;
//-------------------- 连接、注册 -----------------------------------------------------------------------------------------
	for(i=0;i<g_MQTT.ucTotal_Upload_Device;i++)										//连接注册条数
	{
		if((g_MQTT.uiFlag_Connect_Register&BIT(i))==0)							//没有注册
		{
			if(strlen((char *)g_MQTT.ucID_Upload_Device_Buf[i])!=0)		//DEVICE id 不为空
			{
				//1、连接
				sprintf(data,"MQTT_Connect:ch=%d*******************\r\n",i+1);		
				InsertLog(data);	
				delay_1ms(10);			
				if(1==MQTT_Connect("MQIsdp",																			//协议名
													 E_CONNECT_PPROTOCAL_LEVEL_3,         					//协议级别
													 E_CONNECT_FLAG_USER_NAME_BIT7_1,								//连接标志位用户名
													 E_CONNECT_FLAG_PASSWORD_BIT6_1,								//连接标志位密码
													 E_CONNECT_FLAG_WILL_RETAIN_BIT5_0,							//连接标志位遗嘱保留
													 E_CONNECT_FLAG_WILL_QoS_BIT4_AND_3_1,					//连接标志位遗嘱服务质量
													 E_CONNECT_FLAG_WILL_FLAG_BIT2_0,								//连接标志位遗嘱标志
													 E_CONNECT_FLAG_CLEAN_SESSION_BIT1_1,						//连接标志位清理会话
													 E_CONNECT_KEEP_ALIVE_60,												//保持连接时间(心跳周期)60秒
													 (char *)g_MQTT.ucID_Upload_Device_Buf[i],			//设备ID 
													 "zeda",																				//用户名   zeda      morningstar  
													 "zeda123")																			//密码   zeda123   yg#76@]5sV[>ZFE#VB>g]Xzs 
													 //ingestion.dev.pxt.ai(3.11.213.49)   					8883
				 )
				{
					sprintf(data,"MQTT_Auth:ch=%d--------------------\r\n",i+1);		
					InsertLog(data);					
					delay_1ms(10);
					//2、终端注册				返回值  gprs.c CmdExe
					if(1==MQTT_PUBLISH(E_PUBLISH_FH_DUP0,E_PUBLISH_FH_QoS1,E_PUBLISH_FH_RETAIN0,//Fh
														 (char *)g_MQTT.ucID_Upload_Device_Buf[i],"1001",0x0001,	//Vh
														 0x00,0x00,0x00,0x00,0x00,0x00,													  //Pl的时间
														 E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_ENCRYPTION_0,	//PL的消息体属性数据不加密
														 E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_COMPRESS_0)  	//PL的消息体属性数据不压缩
					)
					{
						g_MQTT.uiFlag_Connect_Register|=BIT(i);
						delay_1ms(10);
					}
				}
			}
		}
	}
//-------------------- 上传数据 -------------------------------------------------------------------------------------------------
	if(g_sysTick - uiMQTT_Send_Tim	>= g_configRead.send_frq)							//if(g_sysTick - uiMQTT_Send_Tim	>= g_MQTT.usTm_Interval_Upload)
	{
		uiMQTT_Send_Tim=g_sysTick;
		for(i=0;i<g_MQTT.ucTotal_Upload_Device;i++)
		{
			if(g_MQTT.uiFlag_Connect_Register & BIT(i))
			{
				sprintf(data,"MQTT_PUBLISH:ch=%d--------------------\r\n",i+1);		
				InsertLog(data);
				delay_1ms(10);
				MQTT_PUBLISH(E_PUBLISH_FH_DUP0,E_PUBLISH_FH_QoS1,E_PUBLISH_FH_RETAIN0,//Fh
										 (char *)g_MQTT.ucID_Upload_Device_Buf[i],"1007",0x0000,	//Vh
										 0x00,0x00,0x00,0x00,0x00,0x00,													  //Pl的时间
										 E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_ENCRYPTION_0,	//PL的消息体属性数据不加密
										 E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_COMPRESS_0);  	//PL的消息体属性数据不压缩
				delay_1ms(10);
			}
		}
	}

//-------------------- 请求授时 -------------------------------------------------------------------------------------------------
	if(flag)
	{
		if(g_sysTick-uiMQTT_Timing>3600)
		{
			uiMQTT_Timing=g_sysTick;
			if(g_MQTT.uiFlag_Connect_Register & BIT(0))
			{
				sprintf(data,"MQTT_PUBLISH:time getted #######################\r\n");		
				InsertLog(data);
				MQTT_PUBLISH(E_PUBLISH_FH_DUP0,E_PUBLISH_FH_QoS1,E_PUBLISH_FH_RETAIN0,//Fh
										(char *)g_MQTT.ucID_Upload_Device_Buf[0],"1006",0x0000,	//Vh
										0x00,0x00,0x00,0x00,0x00,0x00,													  //Pl的时间
										E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_ENCRYPTION_0,	//PL的消息体属性数据不加密
										E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_COMPRESS_0);  	//PL的消息体属性数据不压缩
			}
		}
	}
	else
	{
		if(g_sysTick-uiMQTT_Timing>90)
		{			
			uiMQTT_Timing=g_sysTick;
			if(g_MQTT.uiFlag_Connect_Register & BIT(0))
			{
				flag  =1;
				sprintf(data,"MQTT_PUBLISH:time request #######################\r\n");		
				InsertLog(data);
				delay_1ms(10);
				MQTT_PUBLISH(E_PUBLISH_FH_DUP0,E_PUBLISH_FH_QoS1,E_PUBLISH_FH_RETAIN0,//Fh
										(char *)g_MQTT.ucID_Upload_Device_Buf[0],"1006",0x0000,		//Vh
										0x00,0x00,0x00,0x00,0x00,0x00,													  //Pl的时间
										E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_ENCRYPTION_0,		//PL的消息体属性数据不加密
										E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_COMPRESS_0);  	//PL的消息体属性数据不压缩
			}
		}
	}
	
	
//-------------------- 发送心跳 -------------------------------------------------------------------------------------------------
	if(g_MQTT.usTm_PINGREQ_Count>=g_MQTT.usTm_Interval_PINGREQ)
	{
		ucMQTT_TxBuf[0]=0xC0;
		ucMQTT_TxBuf[1]=0x00;
		if(!CmdExe((char *)ucMQTT_TxBuf,3,1,2))					
		{
			sprintf(data,"MQTT_PUBLISH:Ping push ^^^^^^^^^^^^^^^^^\r\n");		
			InsertLog(data);
			delay_1ms(10);
			g_MQTT.usTm_PINGREQ_Count=0;
			g_MQTT.ucCount_PINGREQ_Fail++;
			if(g_MQTT.ucCount_PINGREQ_Fail>4)
			{
				g_MQTT.ucCount_PINGREQ_Fail=0;
				m_gprsinfo.g_bGPRSConnected=false;
				ResetGPRS();
			}
		}
		else
		{
			g_MQTT.ucCount_PINGREQ_Fail=0;
		}
	}
//		//disconnect
//		ucMQTT_TxBuf[0]=0xE0;
//		ucMQTT_TxBuf[1]=0x00;
//		CmdExe((char *)ucMQTT_TxBuf,3,1,2);
//		DlyS(2);
//		DlyS(2);
//		DlyS(2);
//	}
}
#else
void clear_flag(unsigned char mode)
{
	unsigned char i  = 0;
	for(i=0;i<64;i++)
		g_MQTT.ucID_Upload_Flag[i][mode] = 0;
}


void MQTT_Run(unsigned char work_mode)
{
	char data[256];
	static unsigned char flag  = 0;
	//static unsigned char flag2 = 0 ;  //0=空闲   1=上传数据     2=正在连接   3=ping   4=授时
	unsigned char i=0,j=0;
	g_mqtt_work_mode = work_mode;
	
	//---------------------超时判断-------------------------------------------------------------------
	if(g_MQTT.ucFlag2)																												//等待返回数据
	{
		if((systickCount - g_MQTT.uiSendTickStart) > g_MQTT.uiRevTimeOutMs)			//表示已超时  不管什么命令
		{	
				if(3==g_MQTT.ucFlag2)																								//1=data  2=con   3=ping  4=time  5=auth  		命令发送类型
				{
						g_MQTT.ucCount_PINGREQ_Fail++;
						if(g_MQTT.ucCount_PINGREQ_Fail>4)
						{
							g_MQTT.ucCount_PINGREQ_Fail=0;
							g_MQTT.usTm_PINGREQ_Count = g_sysTick;						
							if((G4+1)==g_mqtt_work_mode)											//4g
							{		
								m_gprsinfo.g_bGPRSConnected=false;
								ResetGPRS();
							}
							else if((ETH1+1)==g_mqtt_work_mode)								//rj451
							{		
			
							}							
						}
				}				
				#if  GPRS_DEBUG					
				if(g_configRead.b_debug_work==1)
				{
					//1=data  2=con   3=ping  4=time  5=auth  	命令发送类型
					if(1==g_MQTT.ucFlag2)									
						sprintf(data,"timeout:[data] ch=%d  \r\n",g_MQTT.ucNum+1);		
					else if(2==g_MQTT.ucFlag2)									
						sprintf(data,"timeout:[con]  ch=%d  \r\n",g_MQTT.ucNum+1);		
					else if(3==g_MQTT.ucFlag2)									
						sprintf(data,"timeout:[ping]  \r\n");		
					else if(4==g_MQTT.ucFlag2)									
						sprintf(data,"timeout:[time]  \r\n");		
					else if(5==g_MQTT.ucFlag2)									
						sprintf(data,"timeout:[auth] ch=%d  \r\n",g_MQTT.ucNum+1);							
					InsertLog(data);	
					delay_1ms(2);
				}		
				#endif		
				g_MQTT.ucFlag2 = 0;																			//超时判断							
		}
	}	
	//-------------------- 连接、注册 -----------------------------------------------------------------------------------------
	if((g_sysTick - uiMQTT_Con_Send_Tim)	>= g_MQTT.usTm_Interval_Con && 0==g_MQTT.ucFlag2)				
	{	
		for(i=0;i<g_MQTT.ucTotal_Upload_Device;i++)																						//连接注册条数
		{
			if(0==(g_MQTT.uiFlag_Connect&BIT(i)) && 0==g_MQTT.ucID_Upload_Flag[i][0])						//没有连接并且没有发送则进行连接
			{
				g_MQTT.ucID_Upload_Flag[i][0] = 1;																								//连接已发送
				if(strlen((char *)g_MQTT.ucID_Upload_Device_Buf[i])!=0)														//DEVICE id 不为空则进行连接
				{				
					//1、连接
					sprintf(data,"MQTT_Connect:ch=%d       *************************\r\n",i+1);		
					InsertLog(data);	
					g_MQTT.uiPacketID	= 0x20020000;															//4G recv:20020000
					MQTT_Connect("MQIsdp",																			//协议名
											E_CONNECT_PPROTOCAL_LEVEL_3,         						//协议级别
											E_CONNECT_FLAG_USER_NAME_BIT7_1,								//连接标志位用户名
											E_CONNECT_FLAG_PASSWORD_BIT6_1,									//连接标志位密码
											E_CONNECT_FLAG_WILL_RETAIN_BIT5_0,							//连接标志位遗嘱保留
											E_CONNECT_FLAG_WILL_QoS_BIT4_AND_3_1,						//连接标志位遗嘱服务质量
											E_CONNECT_FLAG_WILL_FLAG_BIT2_0,								//连接标志位遗嘱标志
											E_CONNECT_FLAG_CLEAN_SESSION_BIT1_1,						//连接标志位清理会话
											E_CONNECT_KEEP_ALIVE_60,												//保持连接时间(心跳周期)60秒
											(char *)g_MQTT.ucID_Upload_Device_Buf[i],				//设备ID 
											"zeda",																					//用户名   zeda      morningstar  
											"zeda123");																			//密码   zeda123   yg#76@]5sV[>ZFE#VB>g]Xzs 
											//ingestion.dev.pxt.ai(3.11.213.49)   8883
					g_MQTT.ucFlag2 = 2;																					//1=data  2=con   3=ping  4=time  5=auth  		命令发送类型										
					MQTT_Send(ucMQTT_TxBuf,usMQTT_Tx_Index);										//result : ucMQTT_TxBuf[usMQTT_Tx_Index]	 string			MQTT_Send
					g_MQTT.ucNum = i;
					return ;				
				}
				else
				{
					g_MQTT.uiFlag_Register &=~(1<<i);			//注册=0，连接也为0					
					g_MQTT.ucID_Upload_Flag[i][1] = 1;		//注册已发送
				}
			}
			else if(0==(g_MQTT.uiFlag_Register&BIT(i)) && 0==g_MQTT.ucID_Upload_Flag[i][1])													//已经连接，但没有注册，并且没有发送，进行注册
			{
				g_MQTT.ucID_Upload_Flag[i][0] = 1;				//连接已发送
				g_MQTT.ucID_Upload_Flag[i][1] = 1;				//注册已发送
				if(strlen((char *)g_MQTT.ucID_Upload_Device_Buf[i])!=0)						//DEVICE id 不为空则进行连接
				{				
					//2、注册		
					sprintf(data,"MQTT_Auth:ch=%d        ----------------------------\r\n",i+1);		
					InsertLog(data);			
					//2、终端注册				返回值  gprs.c CmdExe
					g_MQTT.uiPacketID = 0x40020001;
					MQTT_PUBLISH(E_PUBLISH_FH_DUP0,E_PUBLISH_FH_QoS1,E_PUBLISH_FH_RETAIN0,//Fh
														 (char *)g_MQTT.ucID_Upload_Device_Buf[i],"1001",0x0001,	//Vh
														 0x00,0x00,0x00,0x00,0x00,0x00,													  //Pl的时间
														 E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_ENCRYPTION_0,	//PL的消息体属性数据不加密
														 E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_COMPRESS_0);  	//PL的消息体属性数据不压缩
          g_MQTT.ucFlag2 = 5;																													//1=data  2=con   3=ping  4=time  5=auth  		命令发送类型										
					MQTT_Send(ucMQTT_TxBuf,usMQTT_Tx_Index);																		//result : ucMQTT_TxBuf[usMQTT_Tx_Index]	 string			MQTT_Send
					g_MQTT.ucNum = i;
					if((i+1)==g_MQTT.ucTotal_Upload_Device)																			//表示最后1个 
					{
						clear_flag(0);
						clear_flag(1);
						uiMQTT_Con_Send_Tim = g_sysTick;
					}
					return;
				}				
			}	
			else
			{
				g_MQTT.ucID_Upload_Flag[i][0] = 1;				//假定连接已发送
				g_MQTT.ucID_Upload_Flag[i][1] = 1;				//假定注册已发送
			}
		
			if((i+1)==g_MQTT.ucTotal_Upload_Device)																	//表示最后1个 
			{
				clear_flag(0);
				clear_flag(1);
				uiMQTT_Con_Send_Tim = g_sysTick;
			}
		}
	}
	
	//-------------------- 上传数据 -------------------------------------------------------------------------------------------------
	if((g_sysTick - uiMQTT_Send_Tim) >= g_configRead.send_frq && g_configRead.send_frq>14 && 0==g_MQTT.ucFlag2)					//if(g_sysTick - uiMQTT_Send_Tim	>= g_MQTT.usTm_Interval_Upload)
	{		
		for(i=0;i<g_MQTT.ucTotal_Upload_Device;i++)
		{
			if(g_MQTT.uiFlag_Connect & BIT(i) && g_MQTT.uiFlag_Register & BIT(i) && 0==g_MQTT.ucID_Upload_Flag[i][2])			//该设备已经连接并且已经注册并且已经发送过
			{
				//3. 数据上传
				sprintf(data,"MQTT_PUBLISH:ch=%d     &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\r\n",i+1);		
				InsertLog(data);			
				g_MQTT.uiPacketID = 0x40020000;				
				MQTT_PUBLISH(E_PUBLISH_FH_DUP0,E_PUBLISH_FH_QoS1,E_PUBLISH_FH_RETAIN0,									//Fh
										 (char *)g_MQTT.ucID_Upload_Device_Buf[i],"1007",0x0000,										//Vh
										 0x00,0x00,0x00,0x00,0x00,0x00,													  									//Pl的时间
										 E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_ENCRYPTION_0,										//PL的消息体属性数据不加密
										 E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_COMPRESS_0);  										//PL的消息体属性数据不压缩
				g_MQTT.ucFlag2 = 1;																																			//1=data  2=con   3=ping  4=time  5=auth  		命令发送类型
				MQTT_Send(ucMQTT_TxBuf,usMQTT_Tx_Index);																								//result : ucMQTT_TxBuf[usMQTT_Tx_Index]	 string			MQTT_Send						 
				g_MQTT.ucNum = i;
				g_MQTT.ucID_Upload_Flag[i][2]	= 1; 																		//数据已发送
				
				//clean g_MQTT.ucID_Upload_Flag
				if((i+1)==g_MQTT.ucTotal_Upload_Device)																//表示最后1个
				{
						//memset(g_MQTT.ucID_Upload_Flag,0,192);														//64*3=192个标志，全部清0 准备重新开始
						clear_flag(2);
						uiMQTT_Send_Tim=g_sysTick;
				}
				return;
			}
			else
			{
				g_MQTT.ucID_Upload_Flag[i][2]	= 1; 																		//不需要发送，则跳过
			}
			
			if((i+1)==g_MQTT.ucTotal_Upload_Device)																	//表示最后1个 
			{
					//memset(g_MQTT.ucID_Upload_Flag,0,192);															//64*3=192个标志，全部清0 准备重新开始
					clear_flag(2);
					uiMQTT_Send_Tim=g_sysTick;
			}
		}
	}	

//-------------------- 请求授时 -------------------------------------------------------------------------------------------------
	if(flag)
	{
		if(g_sysTick-uiMQTT_Timing>3600 && 0==g_MQTT.ucFlag2)
		{
			uiMQTT_Timing=g_sysTick;
			if(g_MQTT.uiFlag_Connect & BIT(0) && g_MQTT.uiFlag_Register & BIT(0))
			{
				sprintf(data,"MQTT_PUBLISH:time getted #######################\r\n");		
				InsertLog(data);
				g_MQTT.uiPacketID = 0x40020000;				
				MQTT_PUBLISH(E_PUBLISH_FH_DUP0,E_PUBLISH_FH_QoS1,E_PUBLISH_FH_RETAIN0,//Fh
										(char *)g_MQTT.ucID_Upload_Device_Buf[0],"1006",0x0000,		//Vh
										0x00,0x00,0x00,0x00,0x00,0x00,													  //Pl的时间
										E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_ENCRYPTION_0,		//PL的消息体属性数据不加密
										E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_COMPRESS_0);  	//PL的消息体属性数据不压缩
				g_MQTT.ucFlag2 = 4;																										//1=data  2=con   3=ping  4=time  5=auth  		命令发送类型										
				MQTT_Send(ucMQTT_TxBuf,usMQTT_Tx_Index);															//result : ucMQTT_TxBuf[usMQTT_Tx_Index]	 string			MQTT_Send
				return;
			}
		}
	}
	else
	{
		if(g_sysTick-uiMQTT_Timing>90 && 0==g_MQTT.ucFlag2)
		{			
			uiMQTT_Timing=g_sysTick;
			if(g_MQTT.uiFlag_Connect & BIT(0) && g_MQTT.uiFlag_Register & BIT(0))
			{
				flag  =1;
				sprintf(data,"MQTT_PUBLISH:time request #######################\r\n");		
				InsertLog(data);			
				g_MQTT.uiPacketID = 0x40020000;				
				MQTT_PUBLISH(E_PUBLISH_FH_DUP0,E_PUBLISH_FH_QoS1,E_PUBLISH_FH_RETAIN0,	//Fh
										(char *)g_MQTT.ucID_Upload_Device_Buf[0],"1006",0x0000,		//Vh
										0x00,0x00,0x00,0x00,0x00,0x00,													  									//Pl的时间
										E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_ENCRYPTION_0,											//PL的消息体属性数据不加密
										E_PUBLISH_PL_MESSAGE_BODY_PROPERTIES_DATA_COMPRESS_0);  										//PL的消息体属性数据不压缩
				g_MQTT.ucFlag2 = 4;																																			//1=data  2=con   3=ping  4=time  5=auth  		命令发送类型										
				MQTT_Send(ucMQTT_TxBuf,usMQTT_Tx_Index);																								//result : ucMQTT_TxBuf[usMQTT_Tx_Index]	 string			MQTT_Send
				return;
			}
		}
	}
	
	
//-------------------- 发送心跳 -------------------------------------------------------------------------------------------------
	if((g_sysTick - g_MQTT.usTm_PINGREQ_Count) > g_MQTT.usTm_Interval_PINGREQ && 0==g_MQTT.ucFlag2)			//g_MQTT.usTm_PINGREQ_Coun
	{		
		ucMQTT_TxBuf[0]=0xC0;			
		ucMQTT_TxBuf[1]=0x00;
		sprintf(data,"MQTT_PUBLISH:Ping push ^^^^^^^^^^^^^^^^^^^^^^^^^^\r\n");		
		InsertLog(data);
		g_MQTT.ucFlag2 = 3;																										//1=data  2=con   3=ping  4=time  5=auth  		命令发送类型										
		g_MQTT.uiPacketID = 0xD000;																						//response data : 0xd000
		MQTT_Send(ucMQTT_TxBuf,2);																						//if(!CmdExe((char *)ucMQTT_TxBuf,3,1,2))		
		g_MQTT.usTm_PINGREQ_Count = g_sysTick;																//准备下次发送,当其他数据收到，则进行重新赋值
		return;
		#if 0				//失败处理代码
			g_MQTT.usTm_PINGREQ_Count=0;
			g_MQTT.ucCount_PINGREQ_Fail++;
			if(g_MQTT.ucCount_PINGREQ_Fail>4)
			{
				g_MQTT.ucCount_PINGREQ_Fail=0;
				m_gprsinfo.g_bGPRSConnected=false;
				ResetGPRS();
			}
		#endif		
//		else
//		{
//			g_MQTT.ucCount_PINGREQ_Fail=0;
//		}
	}
//		//disconnect
//		ucMQTT_TxBuf[0]=0xE0;
//		ucMQTT_TxBuf[1]=0x00;
//		CmdExe((char *)ucMQTT_TxBuf,3,1,2);
//		DlyS(2);
//		DlyS(2);
//		DlyS(2);
//	}
}
#endif

unsigned char MQTT_Send(unsigned char*senddata,int len)
{
	#if  GPRS_DEBUG			
	if(g_configRead.b_debug_work==1)
	{								
			char data[300];
			int i=0;
			int	dw  = 0;
			delay_1ms(2);
			if((G4+1)==g_mqtt_work_mode)				
				sprintf(data,"4G send:%04d  ",len);
			else if((ETH1+1)==g_mqtt_work_mode)									//rj451
				sprintf(data,"E1 send:%04d  ",len);
			
			dw = 7;
			for(i=0;i<len ;i++)
			{
				sprintf(data+2*dw,"%02X",senddata[i]);
				if(0==((dw+1)%128))
				{
					Com_Send(USB,(unsigned char*)data,256);	
					dw = 0;
					delay_1ms(2);
				}
				else
				{
					dw++;
				}
			}			
			if(dw)
			{
				sprintf(data+2*dw,"\r\n");				 
			}
			else
			{
				sprintf(data,"\r\n");	
			}
			Com_Send(USB,(unsigned char*)data,2*(dw+1));						
			delay_1ms(2);
	}
	#endif
	
	if((G4+1)==g_mqtt_work_mode)											//4g
	{
			//CmdExe((char *)data,3,1,len);
			Com_Send(G4,senddata,len);		
			//Com_Send(G4,(unsigned char*)cmd,cmd_len); 	 					//WriteFile(hcom,cmd,cmd_len,&dw,NULL);		//��������	
			Com.Usart[G4].usRec_RD = Com.Usart[G4].usRec_WR;				//Com.Usart[G4].usRec_WR;	
			g_MQTT.uiSendTickStart	= systickCount;									//等待返回结果开始时刻
	}
	else if((ETH1+1)==g_mqtt_work_mode)									//rj451
	{		
			Write_SOCK_Data_Buffer(0, senddata, len);			
			g_MQTT.uiSendTickStart	= systickCount;					//等待返回结果开始时刻		
	}
	else		//没有发送怎么办
	{
			g_MQTT.uiSendTickStart	= systickCount;					//等待返回结果开始时刻	
	}
	//wifi等。	
	return 1;
}
//算法描述:  1. MQTT_Connect 和 MQTT_PUBLISH  只进行字符串的产生。			OK
//           2. MQTT_Send负责发送,发送成功后，则不允许继续发送     			OK
//					 3. 对发送的结果进行解析。并设定超时。结果正确或超时则继续发送下一条数据。
//					 4. 发送顺序: 所有数据依次发送，如果被发送对象，不具备发送条件，则进行下一条发送。直到发送完所有数据。重新排队。
//           5. 合计三种类型数据 MQTT_Connect  MQTT_PUBLISH  Ping  其中 Ping优先发送，特殊处理。到点就发送   
//					 flag = 0表示空闲   flag=1 ping发送类型或者授时类型，暂时不实现。    flag=2表示MQTT_Connect或MQTT_PUBLISH   ;每发送完成了1条则flag2++,发送完成了所有flag2=0进行新的1轮发送，对是否已经发送标志，进行清0，顺次发送，判断标志。如果标志=0，则发送，对标志进行写1
//           实际算法和上面的描述有一定的差异。
//					 6. 增加调试信息的打印，可以看到整个过程。
//           问题: 如果在MQTT_Send中 data定义为512 则发送数据 没有回应。修改为256则可用  bug  ,原因没有找到。


