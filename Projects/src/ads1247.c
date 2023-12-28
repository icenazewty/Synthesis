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



//void Delay_AD(int num){//ÑÓÊ±1.25*num(us)
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

//Ğ´²Ù×÷Ê±£¬Ğ´ÈëWriteData
//¶Á²Ù×÷Ê±£¬Ğ´Èë0xFF£¬¶Á³öDoutÊä³öÖµ
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
//	if(gpio_input_bit_get(D_AD_DRDY_PORT,D_AD_DRDY)){//µÈ´ıDRDYÀ­µÍ£¬µÈ´ıÊı¾İ×ª»»Íê³É
//		return tsuiLastvalue;
//	}
//	ADS_CS(D_LOW);
	//WaitRDY();							//ä¸­æ–­æ¨¡å¼ï¼Œä¸éœ€è¦è¿™ä¸ª,æŸ¥è¯¢æ¨¡å¼å¼€å¯è¿™ä¸ª
	//_delay_us(2);					//2uså»¶æ—¶åï¼Œç«‹å³è¯»æ•°æ®ï¼Œè¯»æ•°æ®æ—¶é—´é•¿åº¦ä¸º50us
	ADS_CS(D_LOW);				//cså¤„äºä½ç”µå¹³çš„æ—¶é—´ä¸º125us;å¦‚æœå»æ‰ä¸¤ä¸ª_delay_us(1)æ€»æ—¶é—´ä¸º52us å…¶ä¸­åŒ…å«ä¸¤ä¸ªå›ºå®š10us,clk_L=650ns  CLK_H=1100ns
	_delay_us(2);					//clké»˜è®¤ä¸ºL
	data1  = ADS_ReadByte();
	data2  = ADS_ReadByte();
	data3  = ADS_ReadByte();
	tuiConvertData = (data1<<16)|(data2<<8)|data3;
	#if 0
	for(i=0;i<24;i++)			//_delay_us(1); åˆ™é«˜ä½clkéƒ½ä¸º2uså·¦å³
	{
		tuiConvertData=tuiConvertData << 1;	//ç›¸å½“äºé€‚å½“çš„å»¶æ—¶
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
//	ADS_WriteByte(D_NOP);//Ç¿ÖÆÀ­¸ßDOUT
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
	//_delay_us(4);//¶ÁĞ´¼Ä´æÆ÷±ØĞëÓĞ´ËÑÓÊ±£¬·ñÔòĞ¾Æ¬ÎŞ·¨Ê¶±ğ
	ADS_WriteByte(0x00);
	//_delay_us(4);//¶ÁĞ´¼Ä´æÆ÷±ØĞëÓĞ´ËÑÓÊ±£¬·ñÔòĞ¾Æ¬ÎŞ·¨Ê¶±ğ
	ADS_WriteByte(Para_ucData);
	//_delay_us(4);//¶ÁĞ´¼Ä´æÆ÷±ØĞëÓĞ´ËÑÓÊ±£¬·ñÔòĞ¾Æ¬ÎŞ·¨Ê¶±ğ
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
//	Delay_AD(1);//¶ÁĞ´¼Ä´æÆ÷±ØĞëÓĞ´ËÑÓÊ±£¬·ñÔòĞ¾Æ¬ÎŞ·¨Ê¶±ğ
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
//	Delay_AD(1);//¶ÁĞ´¼Ä´æÆ÷±ØĞëÓĞ´ËÑÓÊ±£¬·ñÔòĞ¾Æ¬ÎŞ·¨Ê¶±ğ
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
//	Delay_AD(1);//¶ÁĞ´¼Ä´æÆ÷±ØĞëÓĞ´ËÑÓÊ±£¬·ñÔòĞ¾Æ¬ÎŞ·¨Ê¶±ğ
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
		
		//W5500 int Low active PC6->PB0   å¿…é¡»ä¸Šæ‹‰
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
		//SPI_CK_PL_LOW_PH_1EDGE;	  ä¸è¡Œï¼Œç¼ºé«˜ä½
		//SPI_CK_PL_HIGH_PH_2EDGE;	ä¸è¡Œ
		//SPI_CK_PL_HIGH_PH_1EDGE;	ok
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_64;										//æœ€å°SPI_PSC_16 ;	-->SPI_PSC_32		168/16=10M		æœ€å°Tsclk=488ns=2.049180327868852Mhz   
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
	//clkæœ€é«˜é€Ÿåº¦2MHz		0.5us
	//clkä¸­çš„å»¶æ—¶å»æ‰åä¸º<500KHz
	//int i = 0;
	//Ó²¼ş¸´Î»:4¸ötclk,tclk=1/fclk(4.096MHz)=0.244140625us,*4=0.9765625us
	gpio_bit_reset(D_AD_RST_PORT,D_AD_RST);
	_delay_us(4000);//delay 4ms
	gpio_bit_set(D_AD_RST_PORT,D_AD_RST);
	_delay_us(4000);//delay 4ms
	
	//start=1:3¸ötclk
	//gpio_bit_set(D_AD_START_PORT,D_AD_START);		//
	//_delay_us(10);//delay 10us
	
	//CS=0
	ADS_CS(D_HIGH);
	
	_delay_us(10);//delay 10us
		
	//Read_Ads1247_Mode(D_SDATAC);	
	
	ucReadData=ADS_RREG(D_Reg_CFC0,0x01);			//åç§»æ ¡å‡†ç³»æ•°1  é»˜è®¤0	0x04
	ofc = ucReadData;
	//Read_Ads1247_Mode(D_SDATAC);
	ucReadData=ADS_RREG(D_Reg_CFC1,0x01);			//åç§»æ ¡å‡†ç³»æ•°2  é»˜è®¤0	0x05
	ofc = ofc | (ucReadData<<8);
	//Read_Ads1247_Mode(D_SDATAC);
	ucReadData=ADS_RREG(D_Reg_CFC2,0x01);			//åç§»æ ¡å‡†ç³»æ•°3  é»˜è®¤0	0x06
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
	
	ucReadData=ADS_RREG(D_Reg_MUX0,0x01);		//read default 01  ç”µå‹æµ‹é‡  adc-= ain1  adc+=ain0
//	ADS_WREG(D_Reg_MUX0,0x01);//AIN0 as positive inout channel;AIN1 as negative inout channel;0000 0001
//	ucReadData=ADS_RREG(D_Reg_MUX0,0x01);
	
		ucReadData=ADS_RREG(D_Reg_VBias,0x01);		//é»˜è®¤00
//	ADS_WREG(D_Reg_VBias,0x00);//Bias voltage not enabled   æ¨¡æ‹Ÿç”µå‹/2 åŠ åˆ°ain0 ~ ain3
//	ucReadData=ADS_RREG(D_Reg_VBias,0x01);
	
	ucReadData=ADS_RREG(D_Reg_MUX1,0x01);
	ADS_WREG(D_Reg_MUX1,0x38); 						//Onboard reference  0x30åˆ™é‡‡ç”¨å†…éƒ¨2.048vä½œä¸ºå‚è€ƒ ; 0x38åˆ™é‡‡ç”¨å†…éƒ¨2.048vä½œä¸ºå‚è€ƒå¹¶ä¸”è¾“å‡ºåˆ°refp0å’Œrefn0
	ucReadData=ADS_RREG(D_Reg_MUX1,0x01);
		#if 0
	//++++++++++++++++++++++++++++++++++++++++++++++++++  pga=1
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	ADS_WREG(D_Reg_SYS0,0x08);					//PGA=1; ç”µå‹æ”¾å¤§å€æ•° ï¼› data output rate=1000SPS
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	
	ucReadData=ADS_RREG(D_Reg_CFC0,0x01);			//æ»¡é‡ç¨‹æ ¡å‡†ç³»æ•°1  å‡ºå‚æ ¹æ®å¢ç›Šè¿›è¡Œäº†ä¸åŒçš„æ ¡å‡†å‚æ•°
	fsc = ucReadData;	
	ucReadData=ADS_RREG(D_Reg_FSC1,0x01);			//æ»¡é‡ç¨‹æ ¡å‡†ç³»æ•°2  å‡ºå‚æ ¹æ®å¢ç›Šè¿›è¡Œäº†ä¸åŒçš„æ ¡å‡†å‚æ•°
	fsc = fsc | (ucReadData<<8);	
	ucReadData=ADS_RREG(D_Reg_FSC2,0x01);			//æ»¡é‡ç¨‹æ ¡å‡†ç³»æ•°3  å‡ºå‚æ ¹æ®å¢ç›Šè¿›è¡Œäº†ä¸åŒçš„æ ¡å‡†å‚æ•°
	fsc = fsc | (ucReadData<<16);							//0x40147e
	//++++++++++++++++++++++++++++++++++++++++++++++++++
	//++++++++++++++++++++++++++++++++++++++++++++++++++  pga=4
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	ADS_WREG(D_Reg_SYS0,0x28);								//PGA=4; ç”µå‹æ”¾å¤§å€æ•° ï¼› data output rate=1000SPS
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	
	ucReadData=ADS_RREG(D_Reg_CFC0,0x01);			//æ»¡é‡ç¨‹æ ¡å‡†ç³»æ•°1  å‡ºå‚æ ¹æ®å¢ç›Šè¿›è¡Œäº†ä¸åŒçš„æ ¡å‡†å‚æ•°
	fsc = ucReadData;	
	ucReadData=ADS_RREG(D_Reg_FSC1,0x01);			//æ»¡é‡ç¨‹æ ¡å‡†ç³»æ•°2  å‡ºå‚æ ¹æ®å¢ç›Šè¿›è¡Œäº†ä¸åŒçš„æ ¡å‡†å‚æ•°
	fsc = fsc | (ucReadData<<8);	
	ucReadData=ADS_RREG(D_Reg_FSC2,0x01);			//æ»¡é‡ç¨‹æ ¡å‡†ç³»æ•°3  å‡ºå‚æ ¹æ®å¢ç›Šè¿›è¡Œäº†ä¸åŒçš„æ ¡å‡†å‚æ•°
	fsc = fsc | (ucReadData<<16);							//0x40137e
	//++++++++++++++++++++++++++++++++++++++++++++++++++
	#endif
	//++++++++++++++++++++++++++++++++++++++++++++++++++  pga=1
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	ADS_WREG(D_Reg_SYS0,ADS1247_PGA_1|ADS1247_DARATE_5);					//PGA=1; ç”µå‹æ”¾å¤§å€æ•° ï¼› data output rate=1000SPS	//ADS_WREG(D_Reg_SYS0,ADS1247_PGA_1|ADS1247_DARATE_40);					//PGA=1; ç”µå‹æ”¾å¤§å€æ•° ï¼› data output rate=1000SPS
	ucReadData=ADS_RREG(D_Reg_SYS0,0x1);
	_delay_us(1000);//delay 4ms
	ucReadData=ADS_RREG(D_Reg_CFC0,0x01);			//æ»¡é‡ç¨‹æ ¡å‡†ç³»æ•°1  å‡ºå‚æ ¹æ®å¢ç›Šè¿›è¡Œäº†ä¸åŒçš„æ ¡å‡†å‚æ•°
	fsc = ucReadData;	
	ucReadData=ADS_RREG(D_Reg_FSC1,0x01);			//æ»¡é‡ç¨‹æ ¡å‡†ç³»æ•°2  å‡ºå‚æ ¹æ®å¢ç›Šè¿›è¡Œäº†ä¸åŒçš„æ ¡å‡†å‚æ•°
	fsc = fsc | (ucReadData<<8);	
	ucReadData=ADS_RREG(D_Reg_FSC2,0x01);			//æ»¡é‡ç¨‹æ ¡å‡†ç³»æ•°3  å‡ºå‚æ ¹æ®å¢ç›Šè¿›è¡Œäº†ä¸åŒçš„æ ¡å‡†å‚æ•°
	fsc = fsc | (ucReadData<<16);							//0x40137e
	//++++++++++++++++++++++++++++++++++++++++++++++++++
	
	ucReadData=ADS_RREG(D_Reg_IDAC0,0x01);				//0x90  ç‰ˆæœ¬=9
	ucReadData=ADS_RREG(D_Reg_IDAC1,0x01);				//0xff
	//ADS_WREG(D_Reg_IDAC0,0x00);//DOUT/DRDY pin functions only as Data Out:xxxx 0000
	//ucReadData=ADS_RREG(D_Reg_IDAC0,0x01);
	
//	ucReadData=ADS_RREG(D_Reg_MUX1,0x01);
//	ADS_WREG(D_Reg_MUX1,0x38|0x00); 						//Onboard reference  0x30åˆ™é‡‡ç”¨å†…éƒ¨2.048vä½œä¸ºå‚è€ƒ ; 0x38åˆ™é‡‡ç”¨å†…éƒ¨2.048vä½œä¸ºå‚è€ƒå¹¶ä¸”è¾“å‡ºåˆ°refp0å’Œrefn0
//	ucReadData=ADS_RREG(D_Reg_MUX1,0x01);
	
//	AI_Channel_Select(D_AI_CH_0);
	_delay_us(1000);//delay 125us
	
	#if 0
	for(i=0;i<3;i++)
	{	
		ADS_WriteByte(D_RDATA);		//¶Á1´ÎÊı¾İ
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
	
	ADS_WriteByte(D_RDATA);		//¶Á1´ÎÊı¾İ
	_delay_us(100);
	uiADValue=ReadADSConversionData();
	#endif
	//CS=1
//	gpio_bit_set(D_AD_CS_PORT,D_AD_CS);
	
}

//è¯»æ¨¡å¼ D_RDATAC=è¿ç»­è¯»  ;å•æ¬¡è¯»=D_RDATA
void Read_Ads1247_Mode(unsigned char mode)
{
	ADS_CS(D_LOW);					//22us start
	_delay_us(2);
	ADS_WriteByte(mode);		//Â¶Ã1Â´ÎŠÃ½Â¾İ
	_delay_us(2);
	ADS_CS(D_HIGH);					//22us end
}

//å®šä¹‰é‡‡é›†é¢‘ç‡	é»˜è®¤clk=L  mosi=L  miso=h;åœ¨cpuè¿›è¡Œæ•°æ®çš„è¯»å†™çš„æ—¶å€™ï¼Œcsæ‹‰ä½åmisoç«‹å³åŒæ­¥æ‹‰ä½ï¼Œcsæ‹‰é«˜ï¼Œåˆ™misoä¹Ÿæ‹‰é«˜;
#if 0
void Read_AI_Data(void)
{
	ADS_CS(D_LOW);					//22us start
	_delay_us(2);
	ADS_WriteByte(D_RDATA);	//¶Á1´ÎÊı¾İ
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
	
	if(uiADValue>g_Ads1247_Cali.AdC[ch][1])	//æ­£å‹
	{
		k = (f6v-f0v)/(g_Ads1247_Cali.AdC[ch][2]-g_Ads1247_Cali.AdC[ch][1]);		
	}
	else		//è´Ÿå‹
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
	
	#if 1						//ç²¾ç¡®åˆ°mv å–ä¸­é—´16ä½ bit8~bit15
	data = dy*100000;	
	if(data<0)
		data = data*-1;
	data = data%100;		//æœ€å¤š2ä½mv
	Ads1247_Value[ch] = Ads1247_Value[ch]|(data&0xff);
	#endif
}
void Read_AI_Data(unsigned char ch)
{	
	uiADValue=ReadADSConversionData();			//24bit adè½¬æ¢åçš„ç»“æœæ•°æ®
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
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);    //Ö¸¶¨ÖĞ¶ÏÏòÁ¿±íÎªflashÆğÊ¼Î»ÖÃ¡£
	#else	
	NVIC_DeInit();
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0xC000);
	#endif
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);  //È·¶¨ÓÅÏÈ¼¶·Ö×éµÄ·ÖÀà¡£ÇÀÕ¼ÓÅÏÈ¼¶4¸ö£¬´ÓÕ¼ÓÅÏÈ¼¶4¸ö¡£
	//µÚ0×é£ºËùÓĞ4Î»ÓÃÓÚÖ¸¶¨ÏìÓ¦ÓÅÏÈ¼¶
	//µÚ1×é£º×î¸ß1Î»ÓÃÓÚÖ¸¶¨ÇÀÕ¼Ê½ÓÅÏÈ¼¶£¬×îµÍ3Î»ÓÃÓÚÖ¸¶¨ÏìÓ¦ÓÅÏÈ¼¶
	//µÚ2×é£º×î¸ß2Î»ÓÃÓÚÖ¸¶¨ÇÀÕ¼Ê½ÓÅÏÈ¼¶£¬×îµÍ2Î»ÓÃÓÚÖ¸¶¨ÏìÓ¦ÓÅÏÈ¼¶
	//µÚ3×é£º×î¸ß3Î»ÓÃÓÚÖ¸¶¨ÇÀÕ¼Ê½ÓÅÏÈ¼¶£¬×îµÍ1Î»ÓÃÓÚÖ¸¶¨ÏìÓ¦ÓÅÏÈ¼¶
	//µÚ4×é£ºËùÓĞ4Î»ÓÃÓÚÖ¸¶¨ÇÀÕ¼Ê½ÓÅÏÈ¼¶
  /* Configure the NVIC Preemption Priority Bits */  
  /* Configure one bit for preemption priority */
  /* ÓÅÏÈ¼¶×é ËµÃ÷ÁËÇÀÕ¼ÓÅÏÈ¼¶ËùÓÃµÄÎ»Êı£¬ºÍÏìÓ¦ÓÅÏÈ¼¶ËùÓÃµÄÎ»Êı   ÔÚÕâÀïÊÇ0£¬ 4 
  0×é£º  ÇÀÕ¼ÓÅÏÈ¼¶Õ¼0Î»£¬ ÏìÓ¦ÓÅÏÈ¼¶Õ¼4Î»
  1×é£º  ÇÀÕ¼ÓÅÏÈ¼¶Õ¼1Î»£¬ ÏìÓ¦ÓÅÏÈ¼¶Õ¼3Î»
  2×é£º  ÇÀÕ¼ÓÅÏÈ¼¶Õ¼2Î»£¬ ÏìÓ¦ÓÅÏÈ¼¶Õ¼2Î»
  3×é£º  ÇÀÕ¼ÓÅÏÈ¼¶Õ¼3Î»£¬ ÏìÓ¦ÓÅÏÈ¼¶Õ¼1Î»
  4×é£º  ÇÀÕ¼ÓÅÏÈ¼¶Õ¼4Î»£¬ ÏìÓ¦ÓÅÏÈ¼¶Õ¼0Î»  
  */
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM4_IRQn;                        //×î¸ßÓÅÏÈ¼¶¸üĞÂ¿ØÖÆ²ÎÊı¿ª¹Ø»úµÈ¹Ø¼üĞÔµçÆø²Ù×÷¡£
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	                     //PI¿ØÖÆÆ÷ºÍÊı¾İ²É¼¯£¬ÅĞ¶Ï´®¿ÚÍ¨ĞÅµÄÊı¾İÁ¬ĞøĞÔ¡£
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
//		exti_interrupt_flag_clear(EXTI_8);		//EXTI_ClearITPendingBit(EXTI_Line8);	//Ç¥Â³Ã½ÖÂ¶Ï±ê–¾Î»EXTI_Line11-->EXTI_Line14
//	//}
//}




//æ¨¡æ‹Ÿé‡åè®®:  01 03 00 00 00 08 44 0c  é€šé“0~7æ•°æ®é«˜16bit
//æ¨¡æ‹Ÿé‡åè®®:  01 03 00 0a 00 08 64 0e  é€šé“0~7æ•°æ®ä½8bit
//æ¨¡æ‹Ÿé‡åè®®:  01 03 00 c8 00 01 05 f4  æ¨¡å—åœ°å€
//æ¨¡æ‹Ÿé‡åè®®:  01 03 00 c9 00 01 xx xx  æ¨¡å—æ³¢ç‰¹ç‡
//æ¨¡æ‹Ÿé‡åè®®:  01 03 00 d2 00 01 xx xx  æ¨¡å—åç§°  è¿”å› 0028
//æ¨¡æ‹Ÿé‡åè®®:  01 03 00 dc 00 01 xx xx  æ¨¡å—é€šé“çŠ¶æ€   è¿”å› 0x00ff
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

01 05 00 17 FF 00 3C 3E   7é—­åˆ
01 05 00 17 FF 00 3C 3E 

01 05 00 17 00 00 7D CE   7æ–­å¼€
01 05 00 17 00 00 7D CE 

01 01 00 10 00 08 3C 09 01 01 01 9F 11 E0 DO
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 DI
01 01 00 10 00 08 3C 09 01 01 01 9F 11 E0 DO
01 01 00 00 00 08 3D CC 01 01 01 00 51 88 DI
01 01 00 10 00 08 3C 09 01 01 01 9F 11 E0 DO


01 01 00 10 00 08 3C 09 01 01 01 1F 10 40 01 01 00 00 00 08 3D CC 01 01 01 00 51 88 01 01 00 10 00 08 3C 09 01 01 01 1F 10 40 


#endif
