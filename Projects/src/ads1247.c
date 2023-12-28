#include "ads1247.h"
#include "GlobalVar.h"
#include "string.h"
#include "systick.h"
#include "gd32e50x_spi.h"

void ADS_CS(unsigned char Para_ucState)
{
	if(Para_ucState==D_HIGH)
	{
		gpio_bit_set(D_AD_CS_PORT,D_AD_CS);
	}
	else
	{
		gpio_bit_reset(D_AD_CS_PORT,D_AD_CS);
	}
}

#if 0
void ADS_DIN(unsigned char Para_ucState)
{
	if(Para_ucState==D_HIGH)
	{
		gpio_bit_set(D_AD_DIN_PORT,D_AD_DIN);
	}
	else
	{
		gpio_bit_reset(D_AD_DIN_PORT,D_AD_DIN);
	}
}


void ADS_SCK(unsigned char Para_ucState)
{
	if(Para_ucState==D_HIGH)
	{
		gpio_bit_set(D_AD_SCK_PORT,D_AD_SCK);
	}
	else
	{
		gpio_bit_reset(D_AD_SCK_PORT,D_AD_SCK);
	}
}
#endif

void _delay_us(int _us)
{
	//volatile static int a=0;
//	for(;_us>0;_us--)
//		for(a=0;a<1;a++);
	
	volatile unsigned int _dcnt;
	_dcnt=(_us*100);
	while(_dcnt-->0)
	{
		continue;
	}
	
//	while(_us--){
//		a=10;
//		while(a--);
//	}
}



//void Delay_AD(int num){//��ʱ1.25*num(us)
//	int i;
//	for(i=0;i<num;i++);
//}

unsigned char ADS_WriteByte(unsigned char byte)
{
	while(spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET) 
	{
		;
	}

  /* Send byte through the SPI1 peripheral */
  spi_i2s_data_transmit(SPI0, byte);	

  /* Wait to receive a byte */
  while(spi_i2s_flag_get(SPI0, SPI_FLAG_RBNE) == RESET) ;

  /* Return the byte read from the SPI bus */
  return spi_i2s_data_receive(SPI0);
}

unsigned char ADS_ReadByte(void)
{
	return (ADS_WriteByte(0x00));
}

//д����ʱ��д��WriteData
//������ʱ��д��0xFF������Dout���ֵ
//unsigned char ADS_WriteByte(unsigned char Para_ucWriteData){
//	unsigned char i,tucReadData=0;
//	for(i=0;i<8;i++){
//		ADS_SCK(D_HIGH);
//		_delay_us(10);
//		tucReadData<<=1;
//		if(Para_ucWriteData&bit(7)){
//			ADS_DIN(D_HIGH);
//		}else{
//			ADS_DIN(D_LOW);
//		}
//		_delay_us(10);
//		if(gpio_input_bit_get(D_AD_DOUT_PORT,D_AD_DOUT)==1){
//			tucReadData|=bit(0);
//		}else{
//			tucReadData&=~bit(0);
//		}
//		ADS_SCK(D_LOW);
//		_delay_us(10);
//		Para_ucWriteData<<=1;
//	}
//	return tucReadData;
//}


#if 0
void ADS_Reset(void)
{
	ADS_WriteByte(D_RESET);
	_delay_us(10);//delay 10us
}

void ADS_Sleep(void)
{
	ADS_WriteByte(D_SLEEP);
	_delay_us(10);//delay 10us
}

void ADS_Wakeup(void)
{
	ADS_WriteByte(D_WAKEUP);
	ADS_SCK(D_HIGH);
	_delay_us(1);//delay 1.25us
	ADS_SCK(D_LOW);
	_delay_us(10);//delay 12.5us
}

unsigned char WaitRDY(void)
{
//	ADS_CS(D_LOW);
	while(gpio_input_bit_get(D_AD_DRDY_PORT,D_AD_DRDY));
//	ADS_CS(D_HIGH);
	return 0;
}
#endif
unsigned int ReadADSConversionData(void)
{
	unsigned char i=0,data1,data2,data3;
//	unsigned char tucFlag=0;
	unsigned int tuiConvertData=0;
//	static unsigned int tsuiLastvalue=0;
//	if(gpio_input_bit_get(D_AD_DRDY_PORT,D_AD_DRDY)){//�ȴ�DRDY���ͣ��ȴ�����ת�����
//		return tsuiLastvalue;
//	}
//	ADS_CS(D_LOW);
	//WaitRDY();							//中断模式，不需要这个,查询模式开启这个
	//_delay_us(2);					//2us延时后，立即读数据，读数据时间长度为50us
	ADS_CS(D_LOW);				//cs处于低电平的时间为125us;如果去掉两个_delay_us(1)总时间为52us 其中包含两个固定10us,clk_L=650ns  CLK_H=1100ns
	_delay_us(2);					//clk默认为L
	data1  = ADS_ReadByte();
	data2  = ADS_ReadByte();
	data3  = ADS_ReadByte();
	tuiConvertData = (data1<<16)|(data2<<8)|data3;
	#if 0
	for(i=0;i<24;i++)			//_delay_us(1); 则高低clk都为2us左右
	{
		tuiConvertData=tuiConvertData << 1;	//相当于适当的延时
		ADS_SCK(D_HIGH);
		_delay_us(1);
		tuiConvertData|=gpio_input_bit_get(D_AD_DOUT_PORT,D_AD_DOUT);
		ADS_SCK(D_LOW);
		_delay_us(1);
	}
	#endif
	_delay_us(2);
	ADS_CS(D_HIGH);
	return(tuiConvertData);
}


unsigned char ADS_RREG(unsigned char Para_ucAddress,unsigned char Para_ucNumber)
{
	unsigned char tucReadData=0;
	ADS_CS(D_LOW);
	_delay_us(4);
	ADS_WriteByte(0x20+(Para_ucAddress&0x0F));
	//_delay_us(10);
	ADS_WriteByte(Para_ucNumber-1);
	//_delay_us(4);
	//ADS_DIN(D_HIGH);
	tucReadData=ADS_ReadByte();	
//	ADS_WriteByte(D_NOP);//ǿ������DOUT
	_delay_us(4);
	ADS_CS(D_HIGH);
	return(tucReadData);
}

void ADS_WREG(unsigned char Para_ucAddress,unsigned char Para_ucData)
{
//	unsigned char i=0,tucTmpAddr=0,tucTmp=0;
	ADS_CS(D_LOW);
	_delay_us(4);
	ADS_WriteByte(0x40+(Para_ucAddress&0x0F));
	//_delay_us(4);//��д�Ĵ��������д���ʱ������оƬ�޷�ʶ��
	ADS_WriteByte(0x00);
	//_delay_us(4);//��д�Ĵ��������д���ʱ������оƬ�޷�ʶ��
	ADS_WriteByte(Para_ucData);
	//_delay_us(4);//��д�Ĵ��������д���ʱ������оƬ�޷�ʶ��
	//ADS_DIN(D_HIGH);
	_delay_us(4);
	ADS_CS(D_HIGH);
//	tucTmpAddr=0x40+(Para_ucAddress&0x0F);
////	ADS_CS(D_LOW);
//	for(i=0;i<8;i++){
//		if(tucTmpAddr & 0x80){
//			ADS_DIN(D_HIGH);
//		}else{
//			ADS_DIN(D_LOW);
//		}
//		ADS_SCK(D_HIGH);
//		tucTmpAddr<<=1;
//		ADS_SCK(D_LOW);
//	}
//	Delay_AD(1);//��д�Ĵ��������д���ʱ������оƬ�޷�ʶ��
//	for(i=0;i<8;i++){
//		if(tucTmp & 0x80){
//			ADS_DIN(D_HIGH);
//		}else{
//			ADS_DIN(D_LOW);
//		}
//		ADS_SCK(D_HIGH);
//		tucTmp<<=1;
//		ADS_SCK(D_LOW);
//	}
//	Delay_AD(1);//��д�Ĵ��������д���ʱ������оƬ�޷�ʶ��
//	for(i=0;i<8;i++){
//		if(Para_ucData & 0x80){
//			ADS_DIN(D_HIGH);
//		}else{
//			ADS_DIN(D_LOW);
//		}
//		ADS_SCK(D_HIGH);
//		Para_ucData<<=1;
//		ADS_SCK(D_LOW);
//	}
//	Delay_AD(1);//��д�Ĵ��������д���ʱ������оƬ�޷�ʶ��
//	ADS_DIN(D_HIGH);
////	ADS_CS(D_HIGH);
}

int 					uiADValue=0;
float  				flADValue = 0.0;
float					flADValue4 = 0.0;
unsigned char ucReadData=0;
unsigned int ofc = 0;
unsigned int fsc = 0;

void ADS1247_IO_Init(void)	
{
		spi_parameter_struct spi_init_struct;
   
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_SPI0);
		rcu_periph_clock_enable(RCU_AF);

    /* SPI1_SCK(PB15), SPI1_MISO(PB13) and SPI1_MOSI(PB14) GPIO pin configuration */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    //gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    /* SPI0_CS(PA4) GPIO pin configuration */
   // gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
		
		//W5500 RST Low active   PB1
	//	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);	
	//	W5500_RST_LOW();
		
		//W5500 int Low active PC6->PB0   必须上拉
//		gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_0);		//gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
    /* chip select invalid*/
//    W5500_CS_HIGH();
//		W5500_RST_HIGH();
	
    /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_2EDGE;		
		//SPI_CK_PL_LOW_PH_2EDGE		ok
		//SPI_CK_PL_LOW_PH_1EDGE;	  不行，缺高位
		//SPI_CK_PL_HIGH_PH_2EDGE;	不行
		//SPI_CK_PL_HIGH_PH_1EDGE;	ok
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_64;										//最小SPI_PSC_16 ;	-->SPI_PSC_32		168/16=10M		最小Tsclk=488ns=2.049180327868852Mhz   
    spi_init_struct.endian               = SPI_ENDIAN_MSB;;
    spi_init(SPI0, &spi_init_struct);
		
		//spi_nss_output_disable(SPI1);
		
    /* set crc polynomial */
    //spi_crc_polynomial_set(SPI1,7);
    /* enable SPI0 */
    spi_enable(SPI0);		
}
void ADS1247_Init(void)
{
	//clk最高速度2MHz		0.5us
	//clk中的延时去掉后为<500KHz
	//int i = 0;
	//Ӳ����λ:4��tclk,tclk=1/fclk(4.096MHz)=0.244140625us,*4=0.9765625us
	gpio_bit_reset(D_AD_RST_PORT,D_AD_RST);
	_delay_us(4000);//delay 4ms
	gpio_bit_set(D_AD_RST_PORT,D_AD_RST);
	_delay_us(4000);//delay 4ms
	
	//start=1:3��tclk
	//gpio_bit_set(D_AD_START_PORT,D_AD_START);		//
	//_delay_us(10);//delay 10us
	
	//CS=0
	ADS_CS(D_HIGH);
	
	_delay_us(10);//delay 10us
		
	//Read_Ads1247_Mode(D_SDATAC);	
	
	ucReadData=ADS_RREG(D_Reg_CFC0,0x01);			//偏移校准系数1  默认0	0x04
	ofc = ucReadData;
	//Read_Ads1247_Mode(D_SDATAC);
	ucReadData=ADS_RREG(D_Reg_CFC1,0x01);			//偏移校准系数2  默认0	0x05
	ofc = ofc | (ucReadData<<8);
	//Read_Ads1247_Mode(D_SDATAC);
	ucReadData=ADS_RREG(D_Reg_CFC2,0x01);			//偏移校准系数3  默认0	0x06
	ofc = ofc | (ucReadData<<16);
	//Read_Ads1247_Mode(D_SDATAC);
	#if 0
	ADS_WriteByte(0x62);										//0110 0010 Self offset Calibration	
	_delay_us(1000);//delay 125us
	ucReadData=ADS_RREG(D_Reg_CFC0,0x01);
	ofc = ucReadData;
	ucReadData=ADS_RREG(D_Reg_CFC1,0x01);
	ofc = ofc | (ucReadData<<8);
	
	ucReadData=ADS_RREG(D_Reg_CFC2,0x01);
	ofc = ofc | (ucReadData<<16);					//0x3bff7e
	#endif
	
	ucReadData=ADS_RREG(D_Reg_MUX0,0x01);		//read default 01  电压测量  adc-= ain1  adc+=ain0
//	ADS_WREG(D_Reg_MUX0,0x01);//AIN0 as positive inout channel;AIN1 as negative inout channel;0000 0001
//	ucReadData=ADS_RREG(D_Reg_MUX0,0x01);
	
		ucReadData=ADS_RREG(D_Reg_VBias,0x01);		//默认00
//	ADS_WREG(D_Reg_VBias,0x00);//Bias voltage not enabled   模拟电压/2 加到ain0 ~ ain3
//	ucReadData=ADS_RREG(D_Reg_VBias,0x01);
	
	ucReadData=ADS_RREG(D_Reg_MUX1,0x01);
	ADS_WREG(D_Reg_MUX1,0x38); 						//Onboard reference  0x30则采用内部2.048v作为参考 ; 0x38则采用内部2.048v作为参考并且输出到refp0和refn0
	ucReadData=ADS_RREG(D_Reg_MUX1,0x01);
		#if 0
	//++++++++++++++++++++++++++++++++++++++++++++++++++  pga=1
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	ADS_WREG(D_Reg_SYS0,0x08);					//PGA=1; 电压放大倍数 ； data output rate=1000SPS
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	
	ucReadData=ADS_RREG(D_Reg_CFC0,0x01);			//满量程校准系数1  出厂根据增益进行了不同的校准参数
	fsc = ucReadData;	
	ucReadData=ADS_RREG(D_Reg_FSC1,0x01);			//满量程校准系数2  出厂根据增益进行了不同的校准参数
	fsc = fsc | (ucReadData<<8);	
	ucReadData=ADS_RREG(D_Reg_FSC2,0x01);			//满量程校准系数3  出厂根据增益进行了不同的校准参数
	fsc = fsc | (ucReadData<<16);							//0x40147e
	//++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++  pga=4
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	ADS_WREG(D_Reg_SYS0,0x28);								//PGA=4; 电压放大倍数 ； data output rate=1000SPS
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	
	ucReadData=ADS_RREG(D_Reg_CFC0,0x01);			//满量程校准系数1  出厂根据增益进行了不同的校准参数
	fsc = ucReadData;	
	ucReadData=ADS_RREG(D_Reg_FSC1,0x01);			//满量程校准系数2  出厂根据增益进行了不同的校准参数
	fsc = fsc | (ucReadData<<8);	
	ucReadData=ADS_RREG(D_Reg_FSC2,0x01);			//满量程校准系数3  出厂根据增益进行了不同的校准参数
	fsc = fsc | (ucReadData<<16);							//0x40137e
	//++++++++++++++++++++++++++++++++++++++++++++++++++
	#endif
	//++++++++++++++++++++++++++++++++++++++++++++++++++  pga=1
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	ADS_WREG(D_Reg_SYS0,ADS1247_PGA_1|ADS1247_DARATE_5);					//PGA=1; 电压放大倍数 ； data output rate=1000SPS	//ADS_WREG(D_Reg_SYS0,ADS1247_PGA_1|ADS1247_DARATE_40);					//PGA=1; 电压放大倍数 ； data output rate=1000SPS
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	_delay_us(1000);//delay 4ms
	ucReadData=ADS_RREG(D_Reg_CFC0,0x01);			//满量程校准系数1  出厂根据增益进行了不同的校准参数
	fsc = ucReadData;	
	ucReadData=ADS_RREG(D_Reg_FSC1,0x01);			//满量程校准系数2  出厂根据增益进行了不同的校准参数
	fsc = fsc | (ucReadData<<8);	
	ucReadData=ADS_RREG(D_Reg_FSC2,0x01);			//满量程校准系数3  出厂根据增益进行了不同的校准参数
	fsc = fsc | (ucReadData<<16);							//0x40137e
	//++++++++++++++++++++++++++++++++++++++++++++++++++
	
	ucReadData=ADS_RREG(D_Reg_IDAC0,0x01);				//0x90  版本=9
	ucReadData=ADS_RREG(D_Reg_IDAC1,0x01);				//0xff
	//ADS_WREG(D_Reg_IDAC0,0x00);//DOUT/DRDY pin functions only as Data Out:xxxx 0000
	//ucReadData=ADS_RREG(D_Reg_IDAC0,0x01);
	
//	ucReadData=ADS_RREG(D_Reg_MUX1,0x01);
//	ADS_WREG(D_Reg_MUX1,0x38|0x00); 						//Onboard reference  0x30则采用内部2.048v作为参考 ; 0x38则采用内部2.048v作为参考并且输出到refp0和refn0
//	ucReadData=ADS_RREG(D_Reg_MUX1,0x01);
	
//	AI_Channel_Select(D_AI_CH_0);
	_delay_us(1000);//delay 125us
	
	#if 0
	for(i=0;i<3;i++)
	{	
		ADS_WriteByte(D_RDATA);		//��1������
		_delay_us(100);
		uiADValue=ReadADSConversionData();
	}
	#endif
	
	#if 0
	ADS_WriteByte(0x62);//0110 0010 Self offset Calibration
	
	_delay_us(100);//delay 125us
	
	ucReadData=ADS_RREG(D_Reg_CFC0,0x01);
	ofc = ucReadData;
	
	ucReadData=ADS_RREG(D_Reg_CFC1,0x01);
	ofc = ofc | (ucReadData<<8);
	
	ucReadData=ADS_RREG(D_Reg_CFC2,0x01);
	ofc = ofc | (ucReadData<<16);		//097d24
	#endif
	
	#if 0
	_delay_us(1000);//delay 125us
	
	ADS_WriteByte(D_RDATA);		//��1������
	_delay_us(100);
	uiADValue=ReadADSConversionData();
	#endif
	//CS=1
//	gpio_bit_set(D_AD_CS_PORT,D_AD_CS);
	
}

//读模式 D_RDATAC=连续读  ;单次读=D_RDATA
void Read_Ads1247_Mode(unsigned char mode)
{
	ADS_CS(D_LOW);					//22us start
	_delay_us(2);
	ADS_WriteByte(mode);		//¶Á1´Ίý¾ݍ
	_delay_us(2);
	ADS_CS(D_HIGH);					//22us end
}

//定义采集频率	默认clk=L  mosi=L  miso=h;在cpu进行数据的读写的时候，cs拉低后miso立即同步拉低，cs拉高，则miso也拉高;
#if 0
void Read_AI_Data(void)
{
	ADS_CS(D_LOW);					//22us start
	_delay_us(2);
	ADS_WriteByte(D_RDATA);	//��1������
	_delay_us(2);
	ADS_CS(D_HIGH);					//22us end
	_delay_us(1000);
	
	uiADValue=ReadADSConversionData();
	if(uiADValue>0x7FFFFF)		//-
	{
		uiADValue = (0x1000000-uiADValue)*-1;
	}	
	flADValue = 2.048*uiADValue/8388608.0;
	//flADValue4 = 2.046*(19.927+9.987+5.091)*uiADValue/8388608.0/9.987;	
	flADValue4 = 2.048*(198.95+9.987+5.091)*uiADValue/8388608.0/9.987;	
}
#else
void ADT(unsigned char ch)
{
	int   data=0;
	float f6v=6.000,f0v=0.0,ff6v=-6.000;
	float dy = 0.0,k=0.0,b=0.0;
	
	if(uiADValue>g_Ads1247_Cali.AdC[ch][1])	//正压
	{
		k = (f6v-f0v)/(g_Ads1247_Cali.AdC[ch][2]-g_Ads1247_Cali.AdC[ch][1]);		
	}
	else		//负压
	{	
		k = (f0v-ff6v)/(g_Ads1247_Cali.AdC[ch][1]-g_Ads1247_Cali.AdC[ch][0]);		
	}
	b = f0v-k*g_Ads1247_Cali.AdC[ch][1];		
	dy = k*uiADValue+b;
	Ads_1247_Fvalue[ch] = dy;
	
//	if(0==ch)
//	{
//		printf("break");
//	}
	if(dy>0)
	{
		data = (dy*10000)/10;	
	}
	else
	{
		data = (dy*10000)/10;
	}	
	Ads1247_Value[ch] = (data<<8);	
	
	#if 1						//精确到mv 取中间16位 bit8~bit15
	data = dy*100000;	
	if(data<0)
		data = data*-1;
	data = data%100;		//最多2位mv
	Ads1247_Value[ch] = Ads1247_Value[ch]|(data&0xff);
	#endif
}
void Read_AI_Data(unsigned char ch)
{	
	uiADValue=ReadADSConversionData();			//24bit ad转换后的结果数据
	//Ads1247_Value[ch] = uiADValue;
	if(uiADValue>0x7FFFFF)		//-
	{
		uiADValue = (0x1000000-uiADValue)*-1;
	}	
	//flADValue = 2.048*uiADValue/8388608.0;
	//flADValue4 = 2.046*(19.927+9.987+5.091)*uiADValue/8388608.0/9.987;	
	//flADValue4 = 2.048*(198.95+9.987+5.091)*uiADValue/8388608.0/9.987;	
	ADT(ch);
}
#endif




#if 0
void NVIC_DeInit(void)
{ 
	u32 index = 0;
	 
	NVIC->ICER[0] = 0xFFFFFFFF; 
	NVIC->ICER[1] = 0x000007FF; 
	NVIC->ICPR[0] = 0xFFFFFFFF; 
	NVIC->ICPR[1] = 0x000007FF; 
	 
	for(index = 0; index < 0x0B; index++)
	{ 
		NVIC->IP[index] = 0x00000000; 
	}	
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	#if BOOT_NO
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);    //ָ���ж�������Ϊflash��ʼλ�á�
	#else	
	NVIC_DeInit();
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0xC000);
	#endif
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  //ȷ�����ȼ�����ķ��ࡣ��ռ���ȼ�4������ռ���ȼ�4����
	//��0�飺����4λ����ָ����Ӧ���ȼ�
	//��1�飺���1λ����ָ����ռʽ���ȼ������3λ����ָ����Ӧ���ȼ�
	//��2�飺���2λ����ָ����ռʽ���ȼ������2λ����ָ����Ӧ���ȼ�
	//��3�飺���3λ����ָ����ռʽ���ȼ������1λ����ָ����Ӧ���ȼ�
	//��4�飺����4λ����ָ����ռʽ���ȼ�
  /* Configure the NVIC Preemption Priority Bits */  
  /* Configure one bit for preemption priority */
  /* ���ȼ��� ˵������ռ���ȼ����õ�λ��������Ӧ���ȼ����õ�λ��   ��������0�� 4 
  0�飺  ��ռ���ȼ�ռ0λ�� ��Ӧ���ȼ�ռ4λ
  1�飺  ��ռ���ȼ�ռ1λ�� ��Ӧ���ȼ�ռ3λ
  2�飺  ��ռ���ȼ�ռ2λ�� ��Ӧ���ȼ�ռ2λ
  3�飺  ��ռ���ȼ�ռ3λ�� ��Ӧ���ȼ�ռ1λ
  4�飺  ��ռ���ȼ�ռ4λ�� ��Ӧ���ȼ�ռ0λ  
  */
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;                        //������ȼ����¿��Ʋ������ػ��ȹؼ��Ե���������
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	                     //PI�����������ݲɼ����жϴ���ͨ�ŵ����������ԡ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);		
		
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
		
	//	Enable the RTC Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =9;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
		
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	              
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
	
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;	  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;	  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 10;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
														
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;	  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 11;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 

    //NVIC_InitStructure.NVIC_IRQChannel=SysTick_IRQn;   
    //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0;
    //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 10;
    //NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    //NVIC_Init(&NVIC_InitStructure); 
		
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;	  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 12;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void IWDG_init(void)
{
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: 40KHz(LSI) / 8 = 5 KHz */
  IWDG_SetPrescaler(IWDG_Prescaler_8);

  /* Set counter reload value to 2499,500ms */
  IWDG_SetReload(2499);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

void IWDG_Feed(void)
{      
	IWDG_ReloadCounter();                            //reload              
}
#endif

//void EXTI9_5_IRQHandler(void)
//{
//	//printf("enter irq\r\n");
//	//if(gpio_input_bit_get(RADIO2_DIO1_PORT,RADIO2_DIO1_PIN))
//	//{
//		//printf("irq2\r\n");		//DIO1
//		//if(NULL!=dio1IrqCallback){
//		//	dio1IrqCallback(NULL);
//		//}
//		Irq_Ads1247_Ready = 1;
//		exti_interrupt_flag_clear(EXTI_8);		//EXTI_ClearITPendingBit(EXTI_Line8);	//ǥ³ý֐¶ϱꖾλEXTI_Line11-->EXTI_Line14
//	//}
//}




//模拟量协议:  01 03 00 00 00 08 44 0c  通道0~7数据高16bit
//模拟量协议:  01 03 00 0a 00 08 64 0e  通道0~7数据低8bit
//模拟量协议:  01 03 00 c8 00 01 05 f4  模块地址
//模拟量协议:  01 03 00 c9 00 01 xx xx  模块波特率
//模拟量协议:  01 03 00 d2 00 01 xx xx  模块名称  返回 0028
//模拟量协议:  01 03 00 dc 00 01 xx xx  模块通道状态   返回 0x00ff
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

01 01 00 10 00 08 3C 09 01 01 01 9F 11 E0 DO
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 DI
01 01 00 10 00 08 3C 09 01 01 01 9F 11 E0 DO
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 DI
01 01 00 10 00 08 3C 09 01 01 01 9F 11 E0 DO


01 01 00 10 00 08 3C 09 01 01 01 1F 10 40 01 01 00 00 00 08 3D CC 01 01 01 00 51 88 01 01 00 10 00 08 3C 09 01 01 01 1F 10 40 


#endif
