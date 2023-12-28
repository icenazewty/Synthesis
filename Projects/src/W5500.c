//#include "GlobalVar.h"
#include "W5500.h"	
#include "gd32e50x.h"
#include "systick.h"
#include "GlobalVar.h"
#include "FreeRTOS.h"
#include "task.h"
#include "limits.h"
#include "stdio.h"
#include "snmplib.h"

//#define DUMMY_BYTE       0xA5

char								w5500_buf[4][D_USART_REC_BUFF_SIZE];		//8ä¸ªé€šé“ï¼Œæ¯ä¸ªé€šé“æœ€å¤š512å­—èŠ‚
unsigned short int	w5500_rd[4];														//8ä¸ªé€šé“çš„è¯»å†™æŒ‡é’ˆ
unsigned short int  w5500_wr[4];														//8ä¸ªé€šé“çš„è¯»å†™æŒ‡é’ˆ

/***************----- ÍøÂç²ÎÊı±äÁ¿¶¨Òå -----***************/
//unsigned char Gateway_IP[4];	//Íø¹ØIPµØÖ· 
//unsigned char Sub_Mask[4];		//×ÓÍøÑÚÂë 
//unsigned char Phy_Addr[6];		//ÎïÀíµØÖ·(MAC) 
//unsigned char IP_Addr[4];			//±¾»úIPµØÖ· 


//unsigned char S0_DIP[4];			//¶Ë¿Ú0Ä¿µÄIPµØÖ· 
//unsigned char S0_DPort[2];		//¶Ë¿Ú0Ä¿µÄ¶Ë¿ÚºÅ(6000) 

//unsigned char S1_Port[2];			//¶Ë¿Ú1µÄ¶Ë¿ÚºÅ(502)

unsigned char S2_Port[2];			//¶Ë¿Ú2µÄ¶Ë¿ÚºÅ(8888) 
unsigned char UDP_DIPR[4];		//UDP(¹ã²¥)Ä£Ê½,Ä¿µÄÖ÷»úIPµØÖ·
unsigned char UDP_DPORT[2];		//UDP(¹ã²¥)Ä£Ê½,Ä¿µÄÖ÷»ú¶Ë¿ÚºÅ

unsigned char 	UDP_DIPR_A[4][4];					//UDP(Ú£Ò¥)Ä£Ê½,Ä¿Ö„×·ÜºIPÖ˜Ö·
unsigned char 	UDP_DPORT_A[4][2];				//UDP(Ú£Ò¥)Ä£Ê½,Ä¿Ö„×·Üº×‹à šÛ¿

/***************----- ¶Ë¿ÚµÄÔËĞĞÄ£Ê½ -----***************/
//unsigned char S0_Mode =3;		//¶Ë¿Ú0µÄÔËĞĞÄ£Ê½,0:TCP·şÎñÆ÷Ä£Ê½,1:TCP¿Í»§¶ËÄ£Ê½,2:UDP(¹ã²¥)Ä£Ê½
//unsigned char S1_Mode =3;		//¶Ë¿Ú1µÄÔËĞĞÄ£Ê½,0:TCP·şÎñÆ÷Ä£Ê½,1:TCP¿Í»§¶ËÄ£Ê½,2:UDP(¹ã²¥)Ä£Ê½
//unsigned char S2_Mode =3;		//¶Ë¿Ú1µÄÔËĞĞÄ£Ê½,0:TCP·şÎñÆ÷Ä£Ê½,1:TCP¿Í»§¶ËÄ£Ê½,2:UDP(¹ã²¥)Ä£Ê½
//#define TCP_SERVER	0x00		//TCP·şÎñÆ÷Ä£Ê½
//#define TCP_CLIENT	0x01		//TCP¿Í»§¶ËÄ£Ê½ 
//#define UDP_MODE	0x02			//UDP(¹ã²¥)Ä£Ê½ 

/***************----- ¶Ë¿ÚµÄÔËĞĞ×´Ì¬ -----***************/
unsigned char S_State[8] ;	//¶Ë¿Ú0×´Ì¬¼ÇÂ¼,1:¶Ë¿ÚÍê³É³õÊ¼»¯,2¶Ë¿ÚÍê³ÉÁ¬½Ó(¿ÉÒÔÕı³£´«ÊäÊı¾İ) 

#define S_INIT		0x01			//¶Ë¿ÚÍê³É³õÊ¼»¯ 
#define S_CONN		0x02			//¶Ë¿ÚÍê³ÉÁ¬½Ó,¿ÉÒÔÕı³£´«ÊäÊı¾İ 

/***************----- ¶Ë¿ÚÊÕ·¢Êı¾İµÄ×´Ì¬ -----***************/
unsigned char S_Data[8];		//¶Ë¿Ú0½ÓÊÕºÍ·¢ËÍÊı¾İµÄ×´Ì¬,1:¶Ë¿Ú½ÓÊÕµ½Êı¾İ,2:¶Ë¿Ú·¢ËÍÊı¾İÍê³É 
//unsigned char S1_Data;		//¶Ë¿Ú1½ÓÊÕºÍ·¢ËÍÊı¾İµÄ×´Ì¬,1:¶Ë¿Ú½ÓÊÕµ½Êı¾İ,2:¶Ë¿Ú·¢ËÍÊı¾İÍê³É
//unsigned char S2_Data;		//¶Ë¿Ú1½ÓÊÕºÍ·¢ËÍÊı¾İµÄ×´Ì¬,1:¶Ë¿Ú½ÓÊÕµ½Êı¾İ,2:¶Ë¿Ú·¢ËÍÊı¾İÍê³É
unsigned char Data_Status;  //0ÎªÎ´·¢ËÍÍê³É×´Ì¬£¬1Îª·¢ËÍÍê³É×´Ì¬
#define S_RECEIVE	 0x01			//¶Ë¿Ú½ÓÊÕµ½Ò»¸öÊı¾İ°ü 
#define S_TRANSMITOK 0x02		//¶Ë¿Ú·¢ËÍÒ»¸öÊı¾İ°üÍê³É 

/***************----- ¶Ë¿ÚÊı¾İ»º³åÇø -----***************/

unsigned char Rx_Buffer[1588];									//2048
//unsigned char Tx_Buffer[2048];								//¶Ë¿Ú·¢ËÍÊı¾İ»º³åÇø 

unsigned char W5500_Interrupt;									//W5500ÖĞ¶Ï±êÖ¾(0:ÎŞÖĞ¶Ï,1:ÓĞÖĞ¶Ï)
extern TaskHandle_t 	EtherNet_Task_Handle;			/* w5500ä»»åŠ¡å¥æŸ„ */

//unsigned short Upload_Time=60;

//extern t_SaveNetIPCfg gs_SaveNetIPCfg;

void w5500_config(void)
{
		spi_parameter_struct spi_init_struct;

    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOA);
		rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_SPI1);
		rcu_periph_clock_enable(RCU_AF);

    /* SPI1_SCK(PB15), SPI1_MISO(PB13) and SPI1_MOSI(PB14) GPIO pin configuration */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15 | GPIO_PIN_13 | GPIO_PIN_14);
    //gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    /* SPI0_CS(PA4) GPIO pin configuration */
	#if RJ451
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);		//cs
		
		//W5500 RST Low active   PB1
		gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);		//reset
	#else
		gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);		//cs
		
		//W5500 RST Low active   PB1
		gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_4);		//re
	#endif
		W5500_RST_LOW();
		
		//W5500 int Low active PC6->PB0   å¿…é¡»ä¸Šæ‹‰
		#if RJ451
		gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_0);		//gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
		#else
		gpio_init(GPIOC, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_0);		//gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
		#endif
    /* chip select invalid*/
    W5500_CS_HIGH();
		W5500_RST_HIGH();
	
    /* SPI0 parameter config */
    spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.device_mode          = SPI_MASTER;
    spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;	//SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_8;								//SPI_PSC_8 ;			//84/8=10M
    spi_init_struct.endian               = SPI_ENDIAN_MSB;;
    spi_init(SPI1, &spi_init_struct);
		
		//spi_nss_output_disable(SPI1);
		
    /* set crc polynomial */
    //spi_crc_polynomial_set(SPI1,7);
    /* enable SPI0 */
    spi_enable(SPI1);
		
		/* enable and set EXTI interrupt priority PC6 */
		nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);		// NVIC_PRIGROUP_PRE0_SUB4  0 0  åˆ™æœ€é«˜çº§ä¸­æ–­ï¼›   NVIC_PRIGROUP_PRE4_SUB0  15  0 åˆ™ä¸ºæœ€ä½ä¸­æ–­
    nvic_irq_enable(EXTI0_IRQn, 1U, 0U);
    /* connect key EXTI line to key GPIO pin */
		#if RJ451
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_0);			//GPB0
		#else
		gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOC, GPIO_PIN_SOURCE_0);			//GPC0	
		#endif
    /* configure key EXTI line */
    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_0);		
}

/*!
    \brief      send a byte through the SPI interface and return the byte received from the SPI bus
    \param[in]  byte: byte to send
    \param[out] none
    \retval     the value of the received byte
*/
uint8_t w5500_send_byte(uint8_t byte)
{
    /* loop while data register in not empty */
    while (RESET == spi_i2s_flag_get(SPI1,SPI_FLAG_TBE));

    /* send byte through the SPI0 peripheral */
    spi_i2s_data_transmit(SPI1,byte);

    /* wait to receive a byte */
    while(RESET == spi_i2s_flag_get(SPI1,SPI_FLAG_RBNE));

    /* return the byte read from the SPI bus */
    return(spi_i2s_data_receive(SPI1));
}

/*!
    \brief      read a byte from the SPI flash
    \param[in]  none
    \param[out] none
    \retval     byte read from the SPI flash
*/
uint8_t w5500_read_byte(void)	//w5500_read_byte
{
    return(w5500_send_byte(0x00));
}


/*!
    \brief      send a half word through the SPI interface and return the half word received from the SPI bus
    \param[in]  half_word: half word to send
    \param[out] none
    \retval     the value of the received byte
*/
void w5500_send_halfword(uint16_t half_word)		//w5500_send_halfword
{
   w5500_send_byte(half_word/256);	//Ğ´Ê½Â¾İ¸ßÂ»
	 w5500_send_byte(half_word);			//Ğ´Ê½Â¾İµÍÂ»
}

/*******************************************************************************
* º¯ÊıÃû  : Write_W5500_1Byte
* ÃèÊö    : Í¨¹ıSPI1ÏòÖ¸¶¨µØÖ·¼Ä´æÆ÷Ğ´1¸ö×Ö½ÚÊı¾İ
* ÊäÈë    : reg:16Î»¼Ä´æÆ÷µØÖ·,dat:´ıĞ´ÈëµÄÊı¾İ
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
void Write_W5500_1Byte(unsigned short reg, unsigned char dat)
{
	W5500_CS_LOW();														//ÖÃW5500µÄSCSÎªµÍµçÆ½
	w5500_send_halfword(reg);									//Í¨¹ıSPI1Ğ´16Î»¼Ä´æÆ÷µØÖ·
	w5500_send_byte(FDM1|RWB_WRITE|COMMON_R);	//Í¨¹ıSPI1Ğ´¿ØÖÆ×Ö½Ú,1¸ö×Ö½ÚÊı¾İ³¤¶È,Ğ´Êı¾İ,Ñ¡ÔñÍ¨ÓÃ¼Ä´æÆ÷
	w5500_send_byte(dat);											//Ğ´1¸ö×Ö½ÚÊı¾İ
	W5500_CS_HIGH(); 													//ÖÃW5500µÄSCSÎª¸ßµçÆ½
}

/*******************************************************************************
* º¯ÊıÃû  : Write_W5500_2Byte
* ÃèÊö    : Í¨¹ıSPI1ÏòÖ¸¶¨µØÖ·¼Ä´æÆ÷Ğ´2¸ö×Ö½ÚÊı¾İ
* ÊäÈë    : reg:16Î»¼Ä´æÆ÷µØÖ·,dat:16Î»´ıĞ´ÈëµÄÊı¾İ(2¸ö×Ö½Ú)
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
void Write_W5500_2Byte(unsigned short reg, unsigned short dat)
{
	W5500_CS_LOW();														//ÖÃW5500µÄSCSÎªµÍµçÆ½		
	w5500_send_halfword(reg);									//Í¨¹ıSPI1Ğ´16Î»¼Ä´æÆ÷µØÖ·
	w5500_send_byte(FDM2|RWB_WRITE|COMMON_R);	//Í¨¹ıSPI1Ğ´¿ØÖÆ×Ö½Ú,2¸ö×Ö½ÚÊı¾İ³¤¶È,Ğ´Êı¾İ,Ñ¡ÔñÍ¨ÓÃ¼Ä´æÆ÷
	w5500_send_halfword(dat);									//Ğ´16Î»Êı¾İ
	W5500_CS_HIGH(); 													//ÖÃW5500µÄSCSÎª¸ßµçÆ½
}

/*******************************************************************************
* º¯ÊıÃû  : Write_W5500_nByte
* ÃèÊö    : Í¨¹ıSPI1ÏòÖ¸¶¨µØÖ·¼Ä´æÆ÷Ğ´n¸ö×Ö½ÚÊı¾İ
* ÊäÈë    : reg:16Î»¼Ä´æÆ÷µØÖ·,*dat_ptr:´ıĞ´ÈëÊı¾İ»º³åÇøÖ¸Õë,size:´ıĞ´ÈëµÄÊı¾İ³¤¶È
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
void Write_W5500_nByte(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short i;
	W5500_CS_LOW();															//ÖÃW5500µÄSCSÎªµÍµçÆ½			
	w5500_send_halfword(reg);										//Í¨¹ıSPI1Ğ´16Î»¼Ä´æÆ÷µØÖ·
	w5500_send_byte(VDM|RWB_WRITE|COMMON_R);		//Í¨¹ıSPI1Ğ´¿ØÖÆ×Ö½Ú,N¸ö×Ö½ÚÊı¾İ³¤¶È,Ğ´Êı¾İ,Ñ¡ÔñÍ¨ÓÃ¼Ä´æÆ÷
	for(i=0;i<size;i++)															//Ñ­»·½«»º³åÇøµÄsize¸ö×Ö½ÚÊı¾İĞ´ÈëW5500
	{
		w5500_send_byte(*dat_ptr++);							//Ğ´Ò»¸ö×Ö½ÚÊı¾İ
	}
	W5500_CS_HIGH(); 														//ÖÃW5500µÄSCSÎª¸ßµçÆ½
}

/*******************************************************************************
* º¯ÊıÃû  : Write_W5500_SOCK_1Byte
* ÃèÊö    : Í¨¹ıSPI1ÏòÖ¸¶¨¶Ë¿Ú¼Ä´æÆ÷Ğ´1¸ö×Ö½ÚÊı¾İ
* ÊäÈë    : s:¶Ë¿ÚºÅ,reg:16Î»¼Ä´æÆ÷µØÖ·,dat:´ıĞ´ÈëµÄÊı¾İ
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
	W5500_CS_LOW();																	//ÖÃW5500µÄSCSÎªµÍµçÆ½			
	w5500_send_halfword(reg);												//Í¨¹ıSPI1Ğ´16Î»¼Ä´æÆ÷µØÖ·
	w5500_send_byte(FDM1|RWB_WRITE|(s*0x20+0x08));	//Í¨¹ıSPI1Ğ´¿ØÖÆ×Ö½Ú,1¸ö×Ö½ÚÊı¾İ³¤¶È,Ğ´Êı¾İ,Ñ¡Ôñ¶Ë¿ÚsµÄ¼Ä´æÆ÷
	w5500_send_byte(dat);														//Ğ´1¸ö×Ö½ÚÊı¾İ
	W5500_CS_HIGH(); 																//ÖÃW5500µÄSCSÎª¸ßµçÆ½
}

/*******************************************************************************
* º¯ÊıÃû  : Write_W5500_SOCK_2Byte
* ÃèÊö    : Í¨¹ıSPI1ÏòÖ¸¶¨¶Ë¿Ú¼Ä´æÆ÷Ğ´2¸ö×Ö½ÚÊı¾İ
* ÊäÈë    : s:¶Ë¿ÚºÅ,reg:16Î»¼Ä´æÆ÷µØÖ·,dat:16Î»´ıĞ´ÈëµÄÊı¾İ(2¸ö×Ö½Ú)
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
	W5500_CS_LOW();																	//ÖÃW5500µÄSCSÎªµÍµçÆ½			
	w5500_send_halfword(reg);												//Í¨¹ıSPI1Ğ´16Î»¼Ä´æÆ÷µØÖ·
	w5500_send_byte(FDM2|RWB_WRITE|(s*0x20+0x08));	//Í¨¹ıSPI1Ğ´¿ØÖÆ×Ö½Ú,2¸ö×Ö½ÚÊı¾İ³¤¶È,Ğ´Êı¾İ,Ñ¡Ôñ¶Ë¿ÚsµÄ¼Ä´æÆ÷
	w5500_send_halfword(dat);												//Ğ´16Î»Êı¾İ
	W5500_CS_HIGH(); 																//ÖÃW5500µÄSCSÎª¸ßµçÆ½
}

/*******************************************************************************
* º¯ÊıÃû  : Write_W5500_SOCK_4Byte
* ÃèÊö    : Í¨¹ıSPI1ÏòÖ¸¶¨¶Ë¿Ú¼Ä´æÆ÷Ğ´4¸ö×Ö½ÚÊı¾İ
* ÊäÈë    : s:¶Ë¿ÚºÅ,reg:16Î»¼Ä´æÆ÷µØÖ·,*dat_ptr:´ıĞ´ÈëµÄ4¸ö×Ö½Ú»º³åÇøÖ¸Õë
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
	W5500_CS_LOW();																	//ÖÃW5500µÄSCSÎªµÍµçÆ½			
	w5500_send_halfword(reg);												//Í¨¹ıSPI1Ğ´16Î»¼Ä´æÆ÷µØÖ·
	w5500_send_byte(FDM4|RWB_WRITE|(s*0x20+0x08));	//Í¨¹ıSPI1Ğ´¿ØÖÆ×Ö½Ú,4¸ö×Ö½ÚÊı¾İ³¤¶È,Ğ´Êı¾İ,Ñ¡Ôñ¶Ë¿ÚsµÄ¼Ä´æÆ÷
	w5500_send_byte(*dat_ptr++);										//Ğ´µÚ1¸ö×Ö½ÚÊı¾İ
	w5500_send_byte(*dat_ptr++);										//Ğ´µÚ2¸ö×Ö½ÚÊı¾İ
	w5500_send_byte(*dat_ptr++);										//Ğ´µÚ3¸ö×Ö½ÚÊı¾İ
	w5500_send_byte(*dat_ptr++);										//Ğ´µÚ4¸ö×Ö½ÚÊı¾İ
	W5500_CS_HIGH(); 																//ÖÃW5500µÄSCSÎª¸ßµçÆ½
}

/*******************************************************************************
* º¯ÊıÃû  : Read_W5500_1Byte
* ÃèÊö    : ¶ÁW5500Ö¸¶¨µØÖ·¼Ä´æÆ÷µÄ1¸ö×Ö½ÚÊı¾İ
* ÊäÈë    : reg:16Î»¼Ä´æÆ÷µØÖ·
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ¶ÁÈ¡µ½¼Ä´æÆ÷µÄ1¸ö×Ö½ÚÊı¾İ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
unsigned char Read_W5500_1Byte(unsigned short reg)
{
	unsigned char i;
	W5500_CS_LOW();							
	w5500_send_halfword(reg);
	i=w5500_send_byte(FDM1|RWB_READ|COMMON_R);
	//i=w5500_read_byte();
	//w5500_send_byte(0x00);
	i=w5500_read_byte();
	W5500_CS_HIGH();
	return i;							
}

/*******************************************************************************
* º¯ÊıÃû  : Read_W5500_SOCK_1Byte
* ÃèÊö    : ¶ÁW5500Ö¸¶¨¶Ë¿Ú¼Ä´æÆ÷µÄ1¸ö×Ö½ÚÊı¾İ
* ÊäÈë    : s:¶Ë¿ÚºÅ,reg:16Î»¼Ä´æÆ÷µØÖ·
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ¶ÁÈ¡µ½¼Ä´æÆ÷µÄ1¸ö×Ö½ÚÊı¾İ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
unsigned char Read_W5500_SOCK_1Byte(SOCKET s, unsigned short reg)
{
	unsigned char i;
	W5500_CS_LOW();			
	w5500_send_halfword(reg);
	i = w5500_send_byte(FDM1|RWB_READ|(s*0x20+0x08));
	//i=w5500_read_byte();
	//w5500_send_byte(0x00);
	i=w5500_read_byte();
	W5500_CS_HIGH();
	return i;
}

/*******************************************************************************
* º¯ÊıÃû  : Read_W5500_SOCK_2Byte
* ÃèÊö    : ¶ÁW5500Ö¸¶¨¶Ë¿Ú¼Ä´æÆ÷µÄ2¸ö×Ö½ÚÊı¾İ
* ÊäÈë    : s:¶Ë¿ÚºÅ,reg:16Î»¼Ä´æÆ÷µØÖ·
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ¶ÁÈ¡µ½¼Ä´æÆ÷µÄ2¸ö×Ö½ÚÊı¾İ(16Î»)
* ËµÃ÷    : ÎŞ
*******************************************************************************/
unsigned short Read_W5500_SOCK_2Byte(SOCKET s, unsigned short reg)
{
	unsigned short i;
	W5500_CS_LOW();			
	w5500_send_halfword(reg);
	i=w5500_send_byte(FDM2|RWB_READ|(s*0x20+0x08));
	//i=w5500_read_byte();
	//w5500_send_byte(0x00);
	i=w5500_read_byte();
	//w5500_send_byte(0x00);
	i*=256;
	i+=w5500_read_byte();
	W5500_CS_HIGH();
	return i;
}

/*******************************************************************************
* º¯ÊıÃû  : Read_SOCK_Data_Buffer
* ÃèÊö    : ´ÓW5500½ÓÊÕÊı¾İ»º³åÇøÖĞ¶ÁÈ¡Êı¾İ
* ÊäÈë    : s:¶Ë¿ÚºÅ,*dat_ptr:Êı¾İ±£´æ»º³åÇøÖ¸Õë
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ¶ÁÈ¡µ½µÄÊı¾İ³¤¶È,rx_size¸ö×Ö½Ú
* ËµÃ÷    : ÎŞ
*******************************************************************************/
unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
	unsigned short rx_size;
	unsigned short offset, offset1;
	unsigned short i;
	unsigned char j;

	rx_size=Read_W5500_SOCK_2Byte(s,Sn_RX_RSR);
	if(rx_size==0) return 0;										//æ²¡æ¥æ”¶åˆ°æ•°æ®åˆ™è¿”å›
	if(rx_size>1460) rx_size=1460;

	offset=Read_W5500_SOCK_2Byte(s,Sn_RX_RD);
	offset1=offset; 
	offset&=(S_RX_SIZE-1);						//è®¡ç®—å®é™…çš„ç‰©ç†åœ°å€

	W5500_CS_LOW();//ÖÃW5500µÄSCSÎªµÍµçÆ½

	w5500_send_halfword(offset);//Ğ´16Î»µØÖ·   
	j = w5500_send_byte(VDM|RWB_READ|(s*0x20+0x18));		//å†™æ§åˆ¶å­—èŠ‚,Nä¸ªå­—èŠ‚æ•°æ®é•¿åº¦,è¯»æ•°æ®,é€‰æ‹©ç«¯å£sçš„å¯„å­˜å™¨
	//j=w5500_read_byte();

	if((offset+rx_size)<S_RX_SIZE)		//å¦‚æœæœ€å¤§åœ°å€æœªè¶…è¿‡W5500æ¥æ”¶ç¼“å†²åŒºå¯„å­˜å™¨çš„æœ€å¤§åœ°å€
	{
		for(i=0;i<rx_size;i++)//Ñ­»·¶ÁÈ¡rx_size¸ö×Ö½ÚÊı¾İ
		{
			//w5500_send_byte(0x00);//·¢ËÍÒ»¸öÑÆÊı¾İ
			j=w5500_read_byte();//¶ÁÈ¡1¸ö×Ö½ÚÊı¾İ
			*dat_ptr=j;//½«¶ÁÈ¡µ½µÄÊı¾İ±£´æµ½Êı¾İ±£´æ»º³åÇø
			dat_ptr++;//Êı¾İ±£´æ»º³åÇøÖ¸ÕëµØÖ·×ÔÔö1
		}
	}
	else//Èç¹û×î´óµØÖ·³¬¹ıW5500½ÓÊÕ»º³åÇø¼Ä´æÆ÷µÄ×î´óµØÖ·
	{
		offset=S_RX_SIZE-offset;
		for(i=0;i<offset;i++)//Ñ­»·¶ÁÈ¡³öÇ°offset¸ö×Ö½ÚÊı¾İ
		{
			//w5500_send_byte(0x00);//·¢ËÍÒ»¸öÑÆÊı¾İ
			j = w5500_read_byte();//¶ÁÈ¡1¸ö×Ö½ÚÊı¾İ
			*dat_ptr=j;//½«¶ÁÈ¡µ½µÄÊı¾İ±£´æµ½Êı¾İ±£´æ»º³åÇø
			dat_ptr++;//Êı¾İ±£´æ»º³åÇøÖ¸ÕëµØÖ·×ÔÔö1
		}
		W5500_CS_HIGH(); //ÖÃW5500µÄSCSÎª¸ßµçÆ½

		W5500_CS_LOW();//ÖÃW5500µÄSCSÎªµÍµçÆ½

		w5500_send_halfword(0x00);//Ğ´16Î»µØÖ·
		j=w5500_send_byte(VDM|RWB_READ|(s*0x20+0x18));//Ğ´¿ØÖÆ×Ö½Ú,N¸ö×Ö½ÚÊı¾İ³¤¶È,¶ÁÊı¾İ,Ñ¡Ôñ¶Ë¿ÚsµÄ¼Ä´æÆ÷
		//j=w5500_read_byte();

		for(;i<rx_size;i++)//Ñ­»·¶ÁÈ¡ºórx_size-offset¸ö×Ö½ÚÊı¾İ
		{
			//w5500_send_byte(0x00);//·¢ËÍÒ»¸öÑÆÊı¾İ
			j=w5500_read_byte();//¶ÁÈ¡1¸ö×Ö½ÚÊı¾İ
			*dat_ptr=j;//½«¶ÁÈ¡µ½µÄÊı¾İ±£´æµ½Êı¾İ±£´æ»º³åÇø
			dat_ptr++;//Êı¾İ±£´æ»º³åÇøÖ¸ÕëµØÖ·×ÔÔö1
		}
	}
	W5500_CS_HIGH(); //ÖÃW5500µÄSCSÎª¸ßµçÆ½

	offset1+=rx_size;//¸üĞÂÊµ¼ÊÎïÀíµØÖ·,¼´ÏÂ´Î¶ÁÈ¡½ÓÊÕµ½µÄÊı¾İµÄÆğÊ¼µØÖ·
	Write_W5500_SOCK_2Byte(s, Sn_RX_RD, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);//·¢ËÍÆô¶¯½ÓÊÕÃüÁî
	return rx_size;//·µ»Ø½ÓÊÕµ½Êı¾İµÄ³¤¶È
}

/*******************************************************************************
* º¯ÊıÃû  : Write_SOCK_Data_Buffer
* ÃèÊö    : ½«Êı¾İĞ´ÈëW5500µÄÊı¾İ·¢ËÍ»º³åÇø
* ÊäÈë    : s:¶Ë¿ÚºÅ,*dat_ptr:Êı¾İ±£´æ»º³åÇøÖ¸Õë,size:´ıĞ´ÈëÊı¾İµÄ³¤¶È
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short offset,offset1;
	unsigned short i;

	//Èç¹ûÊÇUDPÄ£Ê½,¿ÉÒÔÔÚ´ËÉèÖÃÄ¿µÄÖ÷»úµÄIPºÍ¶Ë¿ÚºÅ
//	unsigned char ucRead;
	//ucRead = Read_W5500_SOCK_1Byte(s,Sn_SR);
	Read_W5500_SOCK_1Byte(s,Sn_SR);
	//if(ucRead&0x0f) != SOCK_UDP)//Èç¹ûSocket´ò¿ªÊ§°Ü
	if(s==2)		//udpçš„æ—¶å€™é‡‡ç”?		8888
	{		
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, UDP_DIPR);													//ÉèÖÃÄ¿µÄÖ÷»úIP  		
		//Write_W5500_SOCK_4Byte(s, Sn_DIPR, gs_SaveNetIPCfg.ucDestIP);						//É¨ÖƒÄ¿ÂµÄ–Ã·Â»ÃºIP  		
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT[0]*256+UDP_DPORT[1]);	//ÉèÖÃÄ¿µÄÖ÷»ú¶Ë¿ÚºÅ				
		//Write_W5500_SOCK_2Byte(s, Sn_DPORTR, gs_SaveNetIPCfg.ucDestIP);					//É¨ÖƒÄ¿ÂµÄ–Ã·Â»ÃºÂ¶Ë¿ÚºÅ‰			
	}
	else if(s>3)		
	{
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, UDP_DIPR_A[s-4]);														//É¨ÖƒÄ¿ÂµÄ–Ã·Â»ÃºIP  		
		//Write_W5500_SOCK_4Byte(s, Sn_DIPR, gs_SaveNetIPCfg.ucDestIP);									//É¨ÖƒÄ¿ÂµÄ–Ã·Â»ÃºIP  		
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT_A[s-4][0]*256+UDP_DPORT_A[s-4][1]);	//É¨ÖƒÄ¿ÂµÄ–Ã·Â»ÃºÂ¶Ë¿ÚºÅ‰			
	}
	//else if(s==0)			//æ£€æµ‹ä¸Šä¼ ipå’Œportæ˜¯å¦å‘ç”Ÿå˜åŒ–,å¦‚æœå‘ç”Ÿå˜åŒ–ï¼Œåˆ™ä¿®æ”¹ä¸Šä¼ portå’Œip
	{

	}
	offset=Read_W5500_SOCK_2Byte(s,Sn_TX_WR);
	offset1=offset;
	offset&=(S_TX_SIZE-1);//è®¡ç®—å®é™…çš„ç‰©ç†åœ°å€

	W5500_CS_LOW();//ÖÃW5500µÄSCSÎªµÍµçÆ½

	w5500_send_halfword(offset);//Ğ´16Î»µØÖ·
	w5500_send_byte(VDM|RWB_WRITE|(s*0x20+0x10));//Ğ´¿ØÖÆ×Ö½Ú,N¸ö×Ö½ÚÊı¾İ³¤¶È,Ğ´Êı¾İ,Ñ¡Ôñ¶Ë¿ÚsµÄ¼Ä´æÆ÷

	if((offset+size)<S_TX_SIZE)		//å¦‚æœæœ€å¤§åœ°å€æœªè¶…è¿‡W5500å‘é€ç¼“å†²åŒºå¯„å­˜å™¨çš„æœ€å¤§åœ°å€
	{
		for(i=0;i<size;i++)//Ñ­»·Ğ´Èësize¸ö×Ö½ÚÊı¾İ
		{
			w5500_send_byte(*dat_ptr++);//Ğ´ÈëÒ»¸ö×Ö½ÚµÄÊı¾İ		
		}
	}
	else//Èç¹û×î´óµØÖ·³¬¹ıW5500·¢ËÍ»º³åÇø¼Ä´æÆ÷µÄ×î´óµØÖ·
	{
		offset=S_TX_SIZE-offset;
		for(i=0;i<offset;i++)//Ñ­»·Ğ´ÈëÇ°offset¸ö×Ö½ÚÊı¾İ
		{
			w5500_send_byte(*dat_ptr++);//Ğ´ÈëÒ»¸ö×Ö½ÚµÄÊı¾İ
		}
		W5500_CS_HIGH(); //ÖÃW5500µÄSCSÎª¸ßµçÆ½

		W5500_CS_LOW();//ÖÃW5500µÄSCSÎªµÍµçÆ½

		w5500_send_halfword(0x00);//Ğ´16Î»µØÖ·
		w5500_send_byte(VDM|RWB_WRITE|(s*0x20+0x10));//Ğ´¿ØÖÆ×Ö½Ú,N¸ö×Ö½ÚÊı¾İ³¤¶È,Ğ´Êı¾İ,Ñ¡Ôñ¶Ë¿ÚsµÄ¼Ä´æÆ÷

		for(;i<size;i++)//Ñ­»·Ğ´Èësize-offset¸ö×Ö½ÚÊı¾İ
		{
			w5500_send_byte(*dat_ptr++);//Ğ´ÈëÒ»¸ö×Ö½ÚµÄÊı¾İ
		}
	}
	W5500_CS_HIGH(); //ÖÃW5500µÄSCSÎª¸ßµçÆ½

	offset1+=size;//¸üĞÂÊµ¼ÊÎïÀíµØÖ·,¼´ÏÂ´ÎĞ´´ı·¢ËÍÊı¾İµ½·¢ËÍÊı¾İ»º³åÇøµÄÆğÊ¼µØÖ·
	Write_W5500_SOCK_2Byte(s, Sn_TX_WR, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);//·¢ËÍÆô¶¯·¢ËÍÃüÁî				
}

/*******************************************************************************
* º¯ÊıÃû  : W5500_Hardware_Reset
* ÃèÊö    : Ó²¼ş¸´Î»W5500
* ÊäÈë    : ÎŞ
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : W5500µÄ¸´Î»Òı½Å±£³ÖµÍµçÆ½ÖÁÉÙ500usÒÔÉÏ,²ÅÄÜÖØÎ§W5500
*******************************************************************************/
void W5500_Hardware_Reset(void)
{
	W5500_RST_LOW();			//¸´Î»Òı½ÅÀ­µÍ	
	delay_1ms(5);			
	W5500_RST_HIGH();
	delay_1ms(50);	
	
//	while((Read_W5500_1Byte(PHYCFGR)&LINK)==0);//µÈ´ıÒÔÌ«ÍøÁ¬½ÓÍê³É	
}

/*******************************************************************************
* º¯ÊıÃû  : W5500_Init
* ÃèÊö    : ³õÊ¼»¯W5500¼Ä´æÆ÷º¯Êı
* ÊäÈë    : ÎŞ
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : ÔÚÊ¹ÓÃW5500Ö®Ç°£¬ÏÈ¶ÔW5500³õÊ¼»¯
*******************************************************************************/
void W5500_Init(void)
{
	uint8_t i=0;

	Write_W5500_1Byte(MR, RST);//Èí¼ş¸´Î»W5500,ÖÃ1ÓĞĞ§,¸´Î»ºó×Ô¶¯Çå0
	//Delay(10);//ÑÓÊ±10ms,×Ô¼º¶¨Òå¸Ãº¯Êı
	delay_1ms(10);
	//ÉèÖÃÍø¹Ø(Gateway)µÄIPµØÖ·,Gateway_IPÎª4×Ö½Úunsigned charÊı×é,×Ô¼º¶¨Òå 
	//Ê¹ÓÃÍø¹Ø¿ÉÒÔÊ¹Í¨ĞÅÍ»ÆÆ×ÓÍøµÄ¾ÖÏŞ£¬Í¨¹ıÍø¹Ø¿ÉÒÔ·ÃÎÊµ½ÆäËü×ÓÍø»ò½øÈëInternet
	Write_W5500_nByte(GAR, gs_SaveNetIPCfg.ucGateWay, 4);
			
	//ÉèÖÃ×ÓÍøÑÚÂë(MASK)Öµ,SUB_MASKÎª4×Ö½Úunsigned charÊı×é,×Ô¼º¶¨Òå
	//×ÓÍøÑÚÂëÓÃÓÚ×ÓÍøÔËËã
	Write_W5500_nByte(SUBR,gs_SaveNetIPCfg.ucSubMASK,4);		
	
	//ÉèÖÃÎïÀíµØÖ·,PHY_ADDRÎª6×Ö½Úunsigned charÊı×é,×Ô¼º¶¨Òå,ÓÃÓÚÎ¨Ò»±êÊ¶ÍøÂçÉè±¸µÄÎïÀíµØÖ·Öµ
	//¸ÃµØÖ·ÖµĞèÒªµ½IEEEÉêÇë£¬°´ÕÕOUIµÄ¹æ¶¨£¬Ç°3¸ö×Ö½ÚÎª³§ÉÌ´úÂë£¬ºóÈı¸ö×Ö½ÚÎª²úÆ·ĞòºÅ
	//Èç¹û×Ô¼º¶¨ÒåÎïÀíµØÖ·£¬×¢ÒâµÚÒ»¸ö×Ö½Ú±ØĞëÎªÅ¼Êı
	Write_W5500_nByte(SHAR,gs_SaveNetIPCfg.ucMAC,6);		

	//ÉèÖÃ±¾»úµÄIPµØÖ·,IP_ADDRÎª4×Ö½Úunsigned charÊı×é,×Ô¼º¶¨Òå
	//×¢Òâ£¬Íø¹ØIP±ØĞëÓë±¾»úIPÊôÓÚÍ¬Ò»¸ö×ÓÍø£¬·ñÔò±¾»ú½«ÎŞ·¨ÕÒµ½Íø¹Ø
	Write_W5500_nByte(SIPR,gs_SaveNetIPCfg.ucSelfIP,4);		
	
	//ÉèÖÃ·¢ËÍ»º³åÇøºÍ½ÓÊÕ»º³åÇøµÄ´óĞ¡£¬²Î¿¼W5500Êı¾İÊÖ²á
	for(i=0;i<8;i++)
	{
		Write_W5500_SOCK_1Byte(i,Sn_RXBUF_SIZE, 0x02);//Socket Rx memory size=2k
		Write_W5500_SOCK_1Byte(i,Sn_TXBUF_SIZE, 0x02);//Socket Tx mempry size=2k
	}

	//ÉèÖÃÖØÊÔÊ±¼ä£¬Ä¬ÈÏÎª2000(200ms) 
	//Ã¿Ò»µ¥Î»ÊıÖµÎª100Î¢Ãë,³õÊ¼»¯Ê±ÖµÉèÎª2000(0x07D0),µÈÓÚ200ºÁÃë
	Write_W5500_2Byte(RTR_W5500, 0x0BB8);			//Write_W5500_2Byte(RTR, 0x07d0);			300ms

	//ÉèÖÃÖØÊÔ´ÎÊı£¬Ä¬ÈÏÎª8´Î 
	//Èç¹ûÖØ·¢µÄ´ÎÊı³¬¹ıÉè¶¨Öµ,Ôò²úÉú³¬Ê±ÖĞ¶Ï(Ïà¹ØµÄ¶Ë¿ÚÖĞ¶Ï¼Ä´æÆ÷ÖĞµÄSn_IR ³¬Ê±Î»(TIMEOUT)ÖÃ¡°1¡±)
	Write_W5500_1Byte(RCR_W5500,8);

	//Æô¶¯ÖĞ¶Ï£¬²Î¿¼W5500Êı¾İÊÖ²áÈ·¶¨×Ô¼ºĞèÒªµÄÖĞ¶ÏÀàĞÍ
	//IMR_CONFLICTÊÇIPµØÖ·³åÍ»Òì³£ÖĞ¶Ï,IMR_UNREACHÊÇUDPÍ¨ĞÅÊ±£¬µØÖ·ÎŞ·¨µ½´ïµÄÒì³£ÖĞ¶Ï
	//ÆäËüÊÇSocketÊÂ¼şÖĞ¶Ï£¬¸ù¾İĞèÒªÌí¼Ó
	Write_W5500_1Byte(IMR_W5500, IM_IR6);				//ç›®çš„åœ°å€ä¸èƒ½åˆ°è¾¾å’Œipå†²çªä¸­æ–­å¼€å? IM_IR7=ipå†²çªå»æ‰ï¼Œå¦åˆ™ä¸€ç›´äº§ç”Ÿä¸­æ–?
#if RJ45_TCPSERVER_S1
	Write_W5500_1Byte(SIMR,S0_IMR | S1_IMR | S2_IMR | S4_IMR | S5_IMR | S6_IMR | S7_IMR);
	#else	
	Write_W5500_1Byte(SIMR,S0_IMR | S2_IMR | S4_IMR | S5_IMR | S6_IMR | S7_IMR);
	#endif
	
	Write_W5500_SOCK_1Byte(0, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
	
	#if RJ45_TCPSERVER_S1
	Write_W5500_SOCK_1Byte(1, Sn_IMR, IMR_SENDOK | IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
	#endif
	
	Write_W5500_SOCK_1Byte(2, Sn_IMR,   IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);		//IMR_SENDOK
	
	Write_W5500_SOCK_1Byte(4, Sn_IMR,   IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
	Write_W5500_SOCK_1Byte(5, Sn_IMR,   IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
	Write_W5500_SOCK_1Byte(6, Sn_IMR,   IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
	Write_W5500_SOCK_1Byte(7, Sn_IMR,   IMR_TIMEOUT | IMR_RECV | IMR_DISCON | IMR_CON);
}

/*******************************************************************************
* º¯ÊıÃû  : Detect_Gateway
* ÃèÊö    : ¼ì²éÍø¹Ø·şÎñÆ÷
* ÊäÈë    : ÎŞ
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ³É¹¦·µ»ØTRUE(0xFF),Ê§°Ü·µ»ØFALSE(0x00)
* ËµÃ÷    : ÎŞ
*******************************************************************************/
unsigned char Detect_Gateway(void)
{
	unsigned char ip_adde[4];
	ip_adde[0]=gs_SaveNetIPCfg.ucSelfIP[0]+1;
	ip_adde[1]=gs_SaveNetIPCfg.ucSelfIP[1]+1;
	ip_adde[2]=gs_SaveNetIPCfg.ucSelfIP[2]+1;
	ip_adde[3]=gs_SaveNetIPCfg.ucSelfIP[3]+1;

	//¼ì²éÍø¹Ø¼°»ñÈ¡Íø¹ØµÄÎïÀíµØÖ·
	Write_W5500_SOCK_4Byte(0,Sn_DIPR,ip_adde);//ÏòÄ¿µÄµØÖ·¼Ä´æÆ÷Ğ´ÈëÓë±¾»úIP²»Í¬µÄIPÖµ
	Write_W5500_SOCK_1Byte(0,Sn_MR,MR_TCP);//ÉèÖÃsocketÎªTCPÄ£Ê½
	Write_W5500_SOCK_1Byte(0,Sn_CR,OPEN);//´ò¿ªSocket	
	//Delay(5);//ÑÓÊ±5ms 	
	delay_1ms(5);
	if(Read_W5500_SOCK_1Byte(0,Sn_SR) != SOCK_INIT)//Èç¹ûsocket´ò¿ªÊ§°Ü
	{
		Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);//´ò¿ª²»³É¹¦,¹Ø±ÕSocket
		return FALSE;//·µ»ØFALSE(0x00)
	}

	Write_W5500_SOCK_1Byte(0,Sn_CR,CONNECT);//ÉèÖÃSocketÎªConnectÄ£Ê½						

	do
	{
		uint8_t j=0;	
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);//¶ÁÈ¡Socket0ÖĞ¶Ï±êÖ¾¼Ä´æÆ÷
		if(j!=0)
		Write_W5500_SOCK_1Byte(0,Sn_IR,j);	
		delay_1ms(5);
		if((j&IR_TIMEOUT) == IR_TIMEOUT)
		{
			return FALSE;	
		}
		else if(Read_W5500_SOCK_1Byte(0,Sn_DHAR) != 0xff)
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);//¹Ø±ÕSocket
			return TRUE;							
		}
		wdt();
	}while(1);
}

/*******************************************************************************
* º¯ÊıÃû  : Socket_Init
* ÃèÊö    : Ö¸¶¨Socket(0~7)³õÊ¼»¯
* ÊäÈë    : s:´ı³õÊ¼»¯µÄ¶Ë¿Ú
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
void Socket_Init(SOCKET s)
{
	char tmp[8];
	unsigned short port = 0;
	Write_W5500_SOCK_2Byte(s, Sn_MSSR, 1460);							//Socket n çš„æœ€å¤§ä¼ è¾“å•å…ƒ MTU
	//ÉèÖÃÖ¸¶¨¶Ë¿Ú
	if(s>3)
	{
				//S2_Port[0]=0x22;								//0x22b7=8887
				//S2_Port[1]=0xb8;
				//S2_Mode = UDP_MODE;					//Â¼Ó”Ø¶Ë¿Ú²ÂµÄ¹Â¤×·Ä£Ê½,UDPÄ£Ê½
				port = (gs_SaveNetIPCfg.ucUdpSourcePort[0]<<8|gs_SaveNetIPCfg.ucUdpSourcePort[1])+s-3;
				Write_W5500_SOCK_2Byte(s, 	Sn_PORT, 		port);		//ç›‘å¬ç«¯å£8888		
				
				port = (gs_SaveNetIPCfg.ucUdpDestPort[0]<<8|gs_SaveNetIPCfg.ucUdpDestPort[1])+s-3;
				Write_W5500_SOCK_2Byte(s, 	Sn_DPORTR, 	port);				//ç›®æ ‡ç«¯å£8887
				Write_W5500_SOCK_4Byte(s, 	Sn_DIPR, 		gs_SaveNetIPCfg.ucUdpDestIP);				//ç›®æ ‡IP		
	}
	else
	{
		switch(s)
		{
			case 0:										//tcp client
				//ÉèÖÃ¶Ë¿Ú0µÄ¶Ë¿ÚºÅ
				Write_W5500_SOCK_2Byte(0, 	Sn_PORT, 		gs_SaveNetIPCfg.ucSourcePort[0]*256+gs_SaveNetIPCfg.ucSourcePort[1]);
				//ÉèÖÃ¶Ë¿Ú0Ä¿µÄ(Ô¶³Ì)¶Ë¿ÚºÅ
				port = atoi(g_configRead.remotePort);
				Write_W5500_SOCK_2Byte(0, 	Sn_DPORTR, 	port);	//Write_W5500_SOCK_2Byte(0, 	Sn_DPORTR, 	gs_SaveNetIPCfg.ucDestPort[0]*256+gs_SaveNetIPCfg.ucDestPort[1]);
				//ÉèÖÃ¶Ë¿Ú0Ä¿µÄ(Ô¶³Ì)IPµØÖ·
			
				if(strIP2ip(g_configRead.remoteIP,g_configRead.IPLen,(unsigned char*)tmp))
				{	
					gs_SaveNetIPCfg.ucDestIP[0] = tmp[0];
					gs_SaveNetIPCfg.ucDestIP[1] = tmp[1];
					gs_SaveNetIPCfg.ucDestIP[2] = tmp[2];
					gs_SaveNetIPCfg.ucDestIP[3] = tmp[3];
				}		
				Write_W5500_SOCK_4Byte(0, 	Sn_DIPR, 		gs_SaveNetIPCfg.ucDestIP);						
				break;

			case 1:									//tcp server
				Write_W5500_SOCK_2Byte(1, Sn_PORT, (gs_SaveNetIPCfg.ucMonitorPort[0]<<8|gs_SaveNetIPCfg.ucMonitorPort[1]));		
				break;

			case 2:									//udp	8888  udpå‘é€ç«¯å?
				S2_Port[0]=0x22;			//0x22b7=8887
				S2_Port[1]=0xb8;
				//S2_Mode = UDP_MODE;		//¼ÓÔØ¶Ë¿Ú2µÄ¹¤×÷Ä£Ê½,UDPÄ£Ê½
				Write_W5500_SOCK_2Byte(2, 	Sn_PORT, 		gs_SaveNetIPCfg.ucUdpSourcePort[0]<<8|gs_SaveNetIPCfg.ucUdpSourcePort[1]);		//ç›‘å¬ç«¯å£8888		
				Write_W5500_SOCK_2Byte(2, 	Sn_DPORTR, 	gs_SaveNetIPCfg.ucUdpDestPort[0]<<8|gs_SaveNetIPCfg.ucUdpDestPort[1]);				//ç›®æ ‡ç«¯å£8887
				Write_W5500_SOCK_4Byte(2, 	Sn_DIPR, 		gs_SaveNetIPCfg.ucUdpDestIP);				//ç›®æ ‡IP		
				break;

			case 3:
				//S2_Mode = UDP_MODE;		//Â¼Ó”Ø¶Ë¿Ú²ÂµÄ¹Â¤×·Ä£Ê½,UDPÄ£Ê½
				Write_W5500_SOCK_2Byte(3, 	Sn_PORT, 		161);													// snmp æœ¬åœ°ç›‘å¬ç«¯å£ 161
				Write_W5500_SOCK_2Byte(3, 	Sn_DPORTR, 	161);													// ç›®æ ‡ç«¯å£161  è¢«åŠ¨æ¥æ”¶æ•°æ®ï¼Œå®é™…ç›®æ ‡portä¸ºæ”¶åˆ°çš„portç«¯å£ 
				Write_W5500_SOCK_4Byte(3, 	Sn_DIPR, 		gs_SaveNetIPCfg.ucUdpDestIP);	// ç›®æ ‡IP		    è¢«åŠ¨æ¥æ”¶æ•°æ®ï¼Œå®é™…ç›®æ ‡ipä¸ºæ”¶åˆ°çš„ipåœ°å€ 
				break;	

			default:
				break;
		}
	}
}

/*******************************************************************************
* º¯ÊıÃû  : Socket_Connect
* ÃèÊö    : ÉèÖÃÖ¸¶¨Socket(0~7)Îª¿Í»§¶ËÓëÔ¶³Ì·şÎñÆ÷Á¬½Ó
* ÊäÈë    : s:´ıÉè¶¨µÄ¶Ë¿Ú
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ³É¹¦·µ»ØTRUE(0xFF),Ê§°Ü·µ»ØFALSE(0x00)
* ËµÃ÷    : µ±±¾»úSocket¹¤×÷ÔÚ¿Í»§¶ËÄ£Ê½Ê±,ÒıÓÃ¸Ã³ÌĞò,ÓëÔ¶³Ì·şÎñÆ÷½¨Á¢Á¬½Ó
*			Èç¹ûÆô¶¯Á¬½Óºó³öÏÖ³¬Ê±ÖĞ¶Ï£¬ÔòÓë·şÎñÆ÷Á¬½ÓÊ§°Ü,ĞèÒªÖØĞÂµ÷ÓÃ¸Ã³ÌĞòÁ¬½Ó
*			¸Ã³ÌĞòÃ¿µ÷ÓÃÒ»´Î,¾ÍÓë·şÎñÆ÷²úÉúÒ»´ÎÁ¬½Ó
*******************************************************************************/
unsigned char Socket_Connect(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);//ÉèÖÃsocketÎªTCPÄ£Ê½
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);  //´ò¿ªSocket
	//Delay(5);//ÑÓÊ±5ms
	delay_1ms(5);
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)//Èç¹ûsocket´ò¿ªÊ§°Ü
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//´ò¿ª²»³É¹¦,¹Ø±ÕSocket
		return FALSE;//·µ»ØFALSE(0x00)
	}
	Write_W5500_SOCK_1Byte(s,Sn_CR,CONNECT);//ÉèÖÃSocketÎªConnectÄ£Ê½
	return TRUE;//·µ»ØTRUE,ÉèÖÃ³É¹¦
}

/*******************************************************************************
* º¯ÊıÃû  : Socket_Listen
* ÃèÊö    : ÉèÖÃÖ¸¶¨Socket(0~7)×÷Îª·şÎñÆ÷µÈ´ıÔ¶³ÌÖ÷»úµÄÁ¬½Ó
* ÊäÈë    : s:´ıÉè¶¨µÄ¶Ë¿Ú
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ³É¹¦·µ»ØTRUE(0xFF),Ê§°Ü·µ»ØFALSE(0x00)
* ËµÃ÷    : µ±±¾»úSocket¹¤×÷ÔÚ·şÎñÆ÷Ä£Ê½Ê±,ÒıÓÃ¸Ã³ÌĞò,µÈµÈÔ¶³ÌÖ÷»úµÄÁ¬½Ó
*			¸Ã³ÌĞòÖ»µ÷ÓÃÒ»´Î,¾ÍÊ¹W5500ÉèÖÃÎª·şÎñÆ÷Ä£Ê½
*******************************************************************************/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);//ÉèÖÃsocketÎªTCPÄ£Ê½ 
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//´ò¿ªSocket	
	//Delay(5);//ÑÓÊ±5ms
	delay_1ms(5);
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)//Èç¹ûsocket´ò¿ªÊ§°Ü
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//´ò¿ª²»³É¹¦,¹Ø±ÕSocket
		return FALSE;//·µ»ØFALSE(0x00)
	}	
	Write_W5500_SOCK_1Byte(s,Sn_CR,LISTEN);//ÉèÖÃSocketÎªÕìÌıÄ£Ê½	
	delay_1ms(5);//Delay(5);//ÑÓÊ±5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_LISTEN)//Èç¹ûsocketÉèÖÃÊ§°Ü
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//ÉèÖÃ²»³É¹¦,¹Ø±ÕSocket
		return FALSE;//·µ»ØFALSE(0x00)
	}
	return TRUE;

	//ÖÁ´ËÍê³ÉÁËSocketµÄ´ò¿ªºÍÉèÖÃÕìÌı¹¤×÷,ÖÁÓÚÔ¶³Ì¿Í»§¶ËÊÇ·ñÓëËü½¨Á¢Á¬½Ó,ÔòĞèÒªµÈ´ıSocketÖĞ¶Ï£¬
	//ÒÔÅĞ¶ÏSocketµÄÁ¬½ÓÊÇ·ñ³É¹¦¡£²Î¿¼W5500Êı¾İÊÖ²áµÄSocketÖĞ¶Ï×´Ì¬
	//ÔÚ·şÎñÆ÷ÕìÌıÄ£Ê½²»ĞèÒªÉèÖÃÄ¿µÄIPºÍÄ¿µÄ¶Ë¿ÚºÅ
}

/*******************************************************************************
* º¯ÊıÃû  : Socket_UDP
* ÃèÊö    : ÉèÖÃÖ¸¶¨Socket(0~7)ÎªUDPÄ£Ê½
* ÊäÈë    : s:´ıÉè¶¨µÄ¶Ë¿Ú
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ³É¹¦·µ»ØTRUE(0xFF),Ê§°Ü·µ»ØFALSE(0x00)
* ËµÃ÷    : Èç¹ûSocket¹¤×÷ÔÚUDPÄ£Ê½,ÒıÓÃ¸Ã³ÌĞò,ÔÚUDPÄ£Ê½ÏÂ,SocketÍ¨ĞÅ²»ĞèÒª½¨Á¢Á¬½Ó
*			¸Ã³ÌĞòÖ»µ÷ÓÃÒ»´Î£¬¾ÍÊ¹W5500ÉèÖÃÎªUDPÄ£Ê½
*******************************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_UDP);//ÉèÖÃSocketÎªUDPÄ£Ê½*/
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//´ò¿ªSocket*/
	delay_1ms(5);	//Delay(5);//ÑÓÊ±5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_UDP)//Èç¹ûSocket´ò¿ªÊ§°Ü
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//´ò¿ª²»³É¹¦,¹Ø±ÕSocket
		return FALSE;//·µ»ØFALSE(0x00)
	}
	else
		return TRUE;

	//ÖÁ´ËÍê³ÉÁËSocketµÄ´ò¿ªºÍUDPÄ£Ê½ÉèÖÃ,ÔÚÕâÖÖÄ£Ê½ÏÂËü²»ĞèÒªÓëÔ¶³ÌÖ÷»ú½¨Á¢Á¬½Ó
	//ÒòÎªSocket²»ĞèÒª½¨Á¢Á¬½Ó,ËùÒÔÔÚ·¢ËÍÊı¾İÇ°¶¼¿ÉÒÔÉèÖÃÄ¿µÄÖ÷»úIPºÍÄ¿µÄSocketµÄ¶Ë¿ÚºÅ
	//Èç¹ûÄ¿µÄÖ÷»úIPºÍÄ¿µÄSocketµÄ¶Ë¿ÚºÅÊÇ¹Ì¶¨µÄ,ÔÚÔËĞĞ¹ı³ÌÖĞÃ»ÓĞ¸Ä±ä,ÄÇÃ´Ò²¿ÉÒÔÔÚÕâÀïÉèÖÃ
}

/*******************************************************************************
* º¯ÊıÃû  : W5500_Interrupt_Process
* ÃèÊö    : W5500ÖĞ¶Ï´¦Àí³ÌĞò¿ò¼Ü
* ÊäÈë    : ÎŞ
* Êä³ö    : ÎŞ
* ·µ»ØÖµ  : ÎŞ
* ËµÃ÷    : ÎŞ
*******************************************************************************/
void W5500_Interrupt_Process(void)
{
	unsigned char i,j,k;
	IntDispose:
	W5500_Interrupt=0;								//ÇåÁãÖĞ¶Ï±êÖ¾
	i = Read_W5500_1Byte(IR);					//¶ÁÈ¡ÖĞ¶Ï±êÖ¾¼Ä´æÆ÷
	Write_W5500_1Byte(IR, (i&0xf0));	//»ØĞ´Çå³ıÖĞ¶Ï±êÖ¾	
	if((i & CONFLICT) == CONFLICT)		//IPµØÖ·³åÍ»Òì³£´¦Àí
	{
		 //×Ô¼ºÌí¼Ó´úÂë
	}
	wdt();
	if((i & UNREACH) == UNREACH)			//UDPÄ£Ê½ÏÂµØÖ·ÎŞ·¨µ½´ïÒì³£´¦Àí
	{
			//×Ô¼ºÌí¼Ó´úÂë
	}

	i=Read_W5500_1Byte(SIR);					//¶ÁÈ¡¶Ë¿ÚÖĞ¶Ï±êÖ¾¼Ä´æÆ÷	
	if((i & S0_INT) == S0_INT)				//Socket0ÊÂ¼ş´¦Àí 
	{
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);		//¶ÁÈ¡Socket0ÖĞ¶Ï±êÖ¾¼Ä´æÆ÷
		Write_W5500_SOCK_1Byte(0,Sn_IR,j);
		if(j&IR_CON)												//ÔÚTCPÄ£Ê½ÏÂ,Socket0³É¹¦Á¬½Ó 
		{
			S_State[0]|=S_CONN;									//ÍøÂçÁ¬½Ó×´Ì¬0x02,¶Ë¿ÚÍê³ÉÁ¬½Ó£¬¿ÉÒÔÕı³£´«ÊäÊı¾İ
		}
		if(j&IR_DISCON)				//ÔÚTCPÄ£Ê½ÏÂSocket¶Ï¿ªÁ¬½Ó´¦Àí
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);//¹Ø±Õ¶Ë¿Ú,µÈ´ıÖØĞÂ´ò¿ªÁ¬½Ó 
			Socket_Init(0);			//Ö¸¶¨Socket(0~7)³õÊ¼»¯,³õÊ¼»¯¶Ë¿Ú0
			S_State[0]=0;					//ÍøÂçÁ¬½Ó×´Ì¬0x00,¶Ë¿ÚÁ¬½ÓÊ§°Ü
		}
		if(j&IR_SEND_OK)			//Socket0Êı¾İ·¢ËÍÍê³É,¿ÉÒÔÔÙ´ÎÆô¶¯S_tx_process()º¯Êı·¢ËÍÊı¾İ 
		{
			Data_Status=1;
			S_Data[0]|=S_TRANSMITOK;//¶Ë¿Ú·¢ËÍÒ»¸öÊı¾İ°üÍê³É 
		}
		if(j&IR_RECV)						//Socket½ÓÊÕµ½Êı¾İ,¿ÉÒÔÆô¶¯S_rx_process()º¯Êı 
		{
			S_Data[0]|=S_RECEIVE;		//¶Ë¿Ú½ÓÊÕµ½Ò»¸öÊı¾İ°ü
		}
		if(j&IR_TIMEOUT)													//SocketÁ¬½Ó»òÊı¾İ´«Êä³¬Ê±´¦Àí 
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);	// ¹Ø±Õ¶Ë¿Ú,µÈ´ıÖØĞÂ´ò¿ªÁ¬½Ó 
			S_State[0]=0;															//ÍøÂçÁ¬½Ó×´Ì¬0x00,¶Ë¿ÚÁ¬½ÓÊ§°Ü
		}
	}
	
	#if RJ45_TCPSERVER_S1	
	if((i & S1_INT) == S1_INT)//Socket1ÊÂ¼ş´¦Àí 
	{
		j=Read_W5500_SOCK_1Byte(1,Sn_IR);//¶ÁÈ¡Socket0ÖĞ¶Ï±êÖ¾¼Ä´æÆ÷
		Write_W5500_SOCK_1Byte(1,Sn_IR,j);
		if(j&IR_CON)//ÔÚTCPÄ£Ê½ÏÂ,Socket0³É¹¦Á¬½Ó 
		{
			S_State[1]|=S_CONN;//ÍøÂçÁ¬½Ó×´Ì¬0x02,¶Ë¿ÚÍê³ÉÁ¬½Ó£¬¿ÉÒÔÕı³£´«ÊäÊı¾İ
		}
		if(j&IR_DISCON)//ÔÚTCPÄ£Ê½ÏÂSocket¶Ï¿ªÁ¬½Ó´¦Àí
		{
			Write_W5500_SOCK_1Byte(1,Sn_CR,CLOSE);//¹Ø±Õ¶Ë¿Ú,µÈ´ıÖØĞÂ´ò¿ªÁ¬½Ó 
			Socket_Init(1);		//Ö¸¶¨Socket(0~7)³õÊ¼»¯,³õÊ¼»¯¶Ë¿Ú0
			S_State[1]=0;//ÍøÂçÁ¬½Ó×´Ì¬0x00,¶Ë¿ÚÁ¬½ÓÊ§°Ü
		}
		if(j&IR_SEND_OK)//Socket0Êı¾İ·¢ËÍÍê³É,¿ÉÒÔÔÙ´ÎÆô¶¯S_tx_process()º¯Êı·¢ËÍÊı¾İ 
		{
			S_Data[1]|=S_TRANSMITOK;//¶Ë¿Ú·¢ËÍÒ»¸öÊı¾İ°üÍê³É 
		}
		if(j&IR_RECV)//Socket½ÓÊÕµ½Êı¾İ,¿ÉÒÔÆô¶¯S_rx_process()º¯Êı 
		{
			S_Data[1]|=S_RECEIVE;//¶Ë¿Ú½ÓÊÕµ½Ò»¸öÊı¾İ°ü
		}
		if(j&IR_TIMEOUT)//SocketÁ¬½Ó»òÊı¾İ´«Êä³¬Ê±´¦Àí 
		{
			Write_W5500_SOCK_1Byte(1,Sn_CR,CLOSE);// ¹Ø±Õ¶Ë¿Ú,µÈ´ıÖØĞÂ´ò¿ªÁ¬½Ó 
			S_State[1]=0;//ÍøÂçÁ¬½Ó×´Ì¬0x00,¶Ë¿ÚÁ¬½ÓÊ§°Ü
		}
	}
#endif
	
	if((i & S2_INT) == S2_INT)//Socket0ÊÂ¼ş´¦Àí 
	{
		j=Read_W5500_SOCK_1Byte(2,Sn_IR);//¶ÁÈ¡Socket0ÖĞ¶Ï±êÖ¾¼Ä´æÆ÷
		Write_W5500_SOCK_1Byte(2,Sn_IR,j);
		if(j&IR_CON)//ÔÚTCPÄ£Ê½ÏÂ,Socket0³É¹¦Á¬½Ó 
		{
			S_State[2]|=S_CONN;//ÍøÂçÁ¬½Ó×´Ì¬0x02,¶Ë¿ÚÍê³ÉÁ¬½Ó£¬¿ÉÒÔÕı³£´«ÊäÊı¾İ
		}
		if(j&IR_DISCON)//ÔÚTCPÄ£Ê½ÏÂSocket¶Ï¿ªÁ¬½Ó´¦Àí
		{
			Write_W5500_SOCK_1Byte(2,Sn_CR,CLOSE);//¹Ø±Õ¶Ë¿Ú,µÈ´ıÖØĞÂ´ò¿ªÁ¬½Ó 
			Socket_Init(2);		//Ö¸¶¨Socket(0~7)³õÊ¼»¯,³õÊ¼»¯¶Ë¿Ú0
			S_State[2]=0;//ÍøÂçÁ¬½Ó×´Ì¬0x00,¶Ë¿ÚÁ¬½ÓÊ§°Ü
		}
		if(j&IR_SEND_OK)						//Socket0Êı¾İ·¢ËÍÍê³É,¿ÉÒÔÔÙ´ÎÆô¶¯S_tx_process()º¯Êı·¢ËÍÊı¾İ 
		{
			S_Data[2]|=S_TRANSMITOK;	//¶Ë¿Ú·¢ËÍÒ»¸öÊı¾İ°üÍê³É 
		}
		if(j&IR_RECV)//Socket½ÓÊÕµ½Êı¾İ,¿ÉÒÔÆô¶¯S_rx_process()º¯Êı 
		{
			S_Data[2]|=S_RECEIVE;//¶Ë¿Ú½ÓÊÕµ½Ò»¸öÊı¾İ°ü
		}
		if(j&IR_TIMEOUT)//SocketÁ¬½Ó»òÊı¾İ´«Êä³¬Ê±´¦Àí 
		{
			Write_W5500_SOCK_1Byte(2,Sn_CR,CLOSE);// ¹Ø±Õ¶Ë¿Ú,µÈ´ıÖØĞÂ´ò¿ªÁ¬½Ó 
			S_State[2]=0;//ÍøÂçÁ¬½Ó×´Ì¬0x00,¶Ë¿ÚÁ¬½ÓÊ§°Ü
		}
	}
	
	if((i & S3_INT) == S3_INT)//Socket0Ê‚Â¼Ã¾Â´Â¦Ã€ğ“Š	
	{
		j=Read_W5500_SOCK_1Byte(3,Sn_IR);//Â¶ÃÈ¡Socket0ÖÂ¶Ï±ê–¾Â¼Ä´æ†·
		Write_W5500_SOCK_1Byte(3,Sn_IR,j);
		if(j&IR_CON)//ÔšTCPÄ£Ê½Ï‚,Socket0Â³É¹Â¦ÃÂ¬Â½Ó 
		{
			S_State[3]|=S_CONN;//Í¸Â§ÃÂ¬Â½Ó—Â´Ì¬0x02,Â¶Ë¿Úê³‰ÃÂ¬Â½Ó£Â¬Â¿É’Ô•Ã½Â³Â£Â´Â«Ê¤Ê½Â¾İ
		}
		if(j&IR_DISCON)//ÔšTCPÄ£Ê½Ï‚SocketÂ¶Ï¿ÂªÃÂ¬Â½Ó´Â¦Ã€íŠ		
		{
			Write_W5500_SOCK_1Byte(3,Sn_CR,CLOSE);//Â¹Ø±Õ¶Ë¿Ú¬ÂµÈ´Ã½Ö˜Ğ‚Â´ò¿ªÂ¬Â½Ó 
			Socket_Init(3);		//Ö¸Â¶Â¨Socket(0~7)Â³ÃµÊ¼Â»Â¯,Â³ÃµÊ¼Â»Â¯Â¶Ë¿Ú°
			S_State[3]=0;//Í¸Â§ÃÂ¬Â½Ó—Â´Ì¬0x00,Â¶Ë¿ÚÂ¬Â½ÓŠÂ§Â°Ü
		}
		if(j&IR_SEND_OK)						//Socket0Ê½Â¾İ·Â¢ËÍªÂ³É¬Â¿É’Ô”Ù´Î†ğ¶¯“_tx_process()ÂºÂ¯Ê½Â·Â¢ËÊ½Â¾İ 
		{
			S_Data[3]|=S_TRANSMITOK;	//Â¶Ë¿Ú·Â¢ËÒ»Â¸Ã¶Ê½Â¾İ°Ã¼ÍªÂ³É 
		}
		if(j&IR_RECV)//SocketÂ½ÓŠÕµÂ½Ê½Â¾İ¬Â¿É’Ô†ğ¶¯“_rx_process()ÂºÂ¯Ê½ 
		{
			S_Data[3]|=S_RECEIVE;//Â¶Ë¿Ú½ÓŠÕµÂ½Ò»Â¸Ã¶Ê½Â¾İ°Ã¼
		}
		if(j&IR_TIMEOUT)//SocketÃÂ¬Â½Ó»òŠ½¾İ´Â«Ê¤Â³Â¬Ê±Â´Â¦Ã€ğ“Š		
		{
			Write_W5500_SOCK_1Byte(3,Sn_CR,CLOSE);// Â¹Ø±Õ¶Ë¿Ú¬ÂµÈ´Ã½Ö˜Ğ‚Â´ò¿ªÂ¬Â½Ó 
			S_State[3]=0;//Í¸Â§ÃÂ¬Â½Ó—Â´Ì¬0x00,Â¶Ë¿ÚÂ¬Â½ÓŠÂ§Â°Ü
		}
	}
	
	for(k=4;k<8;k++)
	{
			if(i & (1<<k))												//Socket0Ê‚Â¼Ã¾Â´Â¦Ã€ğ“Š	
			{
				j=Read_W5500_SOCK_1Byte(k,Sn_IR);		//Â¶ÃÈ¡Socket0ÖÂ¶Ï±ê–¾Â¼Ä´æ†·
				Write_W5500_SOCK_1Byte(k,Sn_IR,j);
				if(j&IR_CON)												//ÔšTCPÄ£Ê½Ï‚,Socket0Â³É¹Â¦ÃÂ¬Â½Ó 
				{
					S_State[k]|=S_CONN;								//Í¸Â§ÃÂ¬Â½Ó—Â´Ì¬0x02,Â¶Ë¿Úê³‰ÃÂ¬Â½Ó£Â¬Â¿É’Ô•Ã½Â³Â£Â´Â«Ê¤Ê½Â¾İ
				}
				if(j&IR_DISCON)											//ÔšTCPÄ£Ê½Ï‚SocketÂ¶Ï¿ÂªÃÂ¬Â½Ó´Â¦Ã€íŠ		
				{
					Write_W5500_SOCK_1Byte(k,Sn_CR,CLOSE);	//Â¹Ø±Õ¶Ë¿Ú¬ÂµÈ´Ã½Ö˜Ğ‚Â´ò¿ªÂ¬Â½Ó 
					Socket_Init(k);													//Ö¸Â¶Â¨Socket(0~7)Â³ÃµÊ¼Â»Â¯,Â³ÃµÊ¼Â»Â¯Â¶Ë¿Ú°
					S_State[k]=0;														//Í¸Â§ÃÂ¬Â½Ó—Â´Ì¬0x00,Â¶Ë¿ÚÂ¬Â½ÓŠÂ§Â°Ü
				}
				if(j&IR_SEND_OK)													//Socket0Ê½Â¾İ·Â¢ËÍªÂ³É¬Â¿É’Ô”Ù´Î†ğ¶¯“_tx_process()ÂºÂ¯Ê½Â·Â¢ËÊ½Â¾İ 
				{
					S_Data[k]|=S_TRANSMITOK;								//Â¶Ë¿Ú·Â¢ËÒ»Â¸Ã¶Ê½Â¾İ°Ã¼ÍªÂ³É 
				}
				if(j&IR_RECV)															//SocketÂ½ÓŠÕµÂ½Ê½Â¾İ¬Â¿É’Ô†ğ¶¯“_rx_process()ÂºÂ¯Ê½ 
				{
					S_Data[k]|=S_RECEIVE;										//Â¶Ë¿Ú½ÓŠÕµÂ½Ò»Â¸Ã¶Ê½Â¾İ°Ã¼
				}
				if(j&IR_TIMEOUT)													//SocketÃÂ¬Â½Ó»òŠ½¾İ´Â«Ê¤Â³Â¬Ê±Â´Â¦Ã€ğ“Š		
				{
					Write_W5500_SOCK_1Byte(k,Sn_CR,CLOSE);	// Â¹Ø±Õ¶Ë¿Ú¬ÂµÈ´Ã½Ö˜Ğ‚Â´ò¿ªÂ¬Â½Ó 
					S_State[k]=0;														//Í¸Â§ÃÂ¬Â½Ó—Â´Ì¬0x00,Â¶Ë¿ÚÂ¬Â½ÓŠÂ§Â°Ü
				}
			}
	}
	
	if(Read_W5500_1Byte(SIR) != 0) 
		goto IntDispose;
}

/*******************************************************************************
* ÂºÂ¯Ê½Ã»  : W5500_Initialization
* Ã¨Ê¶    : W5500Â³ÃµÊ¼Â»ÃµÅ¤Öƒ
* Ê¤È«    : Î
* Ê¤Â³Ã¶    : Î
* Â·ÂµÂ»Ø–Âµ  : Î
* ËµÃ·    : Î
*******************************************************************************/
void W5500_Initialization(void)
{
	W5500_Init();			//Â³ÃµÊ¼Â»Â¯W5500Â¼Ä´æ†·ÂºÂ¯Ê½	
	Detect_Gateway();	//Â¼ì²©Í¸Â¹Ø·Ã¾Î±Æ· 
	Socket_Init(0);		//Ö¸Â¶Â¨Socket(0~7)Â³ÃµÊ¼Â»Â¯,Â³ÃµÊ¼Â»Â¯Â¶Ë¿Ú°
#if RJ45_TCPSERVER_S1		
	Socket_Init(1);		//Ö¸Â¶Â¨Socket(0~7)Â³ÃµÊ¼Â»Â¯,Â³ÃµÊ¼Â»Â¯Â¶Ë¿Ú±
#endif	
	Socket_Init(2);		//Ö¸Â¶Â¨Socket(0~7)Â³ÃµÊ¼Â»Â¯,Â³ÃµÊ¼Â»Â¯Â¶Ë¿Ú²
	Socket_Init(3);		//snmp udpé€šé“
	Socket_Init(4);
	Socket_Init(5);
	Socket_Init(6);
	Socket_Init(7);
}

void W5500_Socket_Set(void)
{
#if	RJ45_TCPCLIENT_S0
	if(S_State[0]==0 || S_State[0]==1)						//tcp client
	{
		if(Socket_Connect(0)==TRUE)
				S_State[0]=S_INIT;
		else
				S_State[0]=0;		
	}
#endif
	
#if RJ45_TCPSERVER_S1		
	if(S_State[1]==0)							//tcp server
	{
			if(Socket_Listen(1)==TRUE)
				S_State[1] = S_INIT;
			else
				S_State[1] = 0;				
	}
#endif
	
	if(S_State[2]==0)							//udp
	{
		if(Socket_UDP(2)==TRUE)
				S_State[2]=S_INIT|S_CONN;
		else
				S_State[2]=0;		
	}	
	
	if(S_State[3]==0)							//udp
	{
		if(Socket_UDP(3)==TRUE)
				S_State[3]=S_INIT|S_CONN;
		else
				S_State[3]=0;		
	}	
	
	if(S_State[4]==0)							//udp
	{
		if(Socket_UDP(4)==TRUE)
				S_State[4]=S_INIT|S_CONN;
		else
				S_State[4]=0;		
	}	
	
	if(S_State[5]==0)							//udp
	{
		if(Socket_UDP(5)==TRUE)
				S_State[5]=S_INIT|S_CONN;
		else
				S_State[5]=0;		
	}	
	
	if(S_State[6]==0)							//udp
	{
		if(Socket_UDP(6)==TRUE)
				S_State[6]=S_INIT|S_CONN;
		else
				S_State[6]=0;		
	}	
	
	if(S_State[7]==0)							//udp
	{
		if(Socket_UDP(7)==TRUE)
				S_State[7]=S_INIT|S_CONN;
		else
				S_State[7]=0;		
	}	
}

void Process_Socket_Data(SOCKET s)
{
	unsigned short size;
	//unsigned short nCRC=0;
	//unsigned short tmpID=0;
	//unsigned short addrStart=0;//Â²é•’Æ°Ê¼ÂµØ–Â·
	int i=0;
	//unsigned char ucTxBuffer[300];		
	wdt();
	size=Read_SOCK_Data_Buffer(s, Rx_Buffer);	
	if(s==0)
	{
//		for(i=0;i<size;i++)
//		{
//				g_rxbuf_tcp[g_wr_tcp]=Rx_Buffer[i];
//				g_wr_tcp=(g_wr_tcp+1)%MAXRecvBuf;							
//		}
		if(size>MAXRecvBuf)
			size = MAXRecvBuf;
		Mqtt_Rece_Proces(Rx_Buffer,size);
			
		//memcpy(ucTxBuffer,Rx_Buffer,size);									//æµ‹è¯•ç”¨
		//Write_SOCK_Data_Buffer(0, ucTxBuffer, size);
		//Write_SOCK_Data_Buffer(0, Rx_Buffer, size);		
  }	
#if RJ45_TCPSERVER_S1			
	else if(s==1)
	{
		if(size >= 300)
		{
			size = 300;
		}		
		//memcpy(ucTxBuffer,Rx_Buffer,size);
		//Write_SOCK_Data_Buffer(1, Rx_Buffer, size);	//Write_SOCK_Data_Buffer(1, ucTxBuffer, size);
	}
#endif	
	else if(s==2)
	{
		UDP_DIPR[0]  = Rx_Buffer[0];
		UDP_DIPR[1]  = Rx_Buffer[1];
		UDP_DIPR[2]  = Rx_Buffer[2];
		UDP_DIPR[3]  = Rx_Buffer[3];

		UDP_DPORT[0] = Rx_Buffer[4];
		UDP_DPORT[1] = Rx_Buffer[5];
		
		//Â¶ÃÊ½Â¾İ·Åˆë»ºÂ³å‡¸ÖÂ¡Â£
		g_wr = 0;
		for(i=0;i<size-8 && i < MAXRecvBuf;i++)
		{
				g_rxbuf[g_wr]=Rx_Buffer[8+i];
				g_wr=(g_wr+1)%MAXRecvBuf;
		}		
		//memcpy(Tx_Buffer, Rx_Buffer+8, size-8);			
		//Write_SOCK_Data_Buffer(s, Tx_Buffer, size-8);
		//Write_SOCK_Data_Buffer(s, Rx_Buffer+8, size-8);		
	}	
	
	else if(s>3)
	{
		UDP_DIPR_A[s-4][0]  = Rx_Buffer[0];
		UDP_DIPR_A[s-4][1]  = Rx_Buffer[1];
		UDP_DIPR_A[s-4][2]  = Rx_Buffer[2];
		UDP_DIPR_A[s-4][3]  = Rx_Buffer[3];

		UDP_DPORT_A[s-4][0] = Rx_Buffer[4];
		UDP_DPORT_A[s-4][1] = Rx_Buffer[5];
		
		//Â¶ÃÊ½Â¾İ·Åˆë»ºÂ³å‡¸ÖÂ¡Â£
		#if 1
		if(size>8)
		{
			for(i=0;i<size-8;i++)
			{
				g_rxbuf_UDP[s-4][g_wr_UDP[s-4]]=Rx_Buffer[8+i];
				g_wr_UDP[s-4]=(g_wr_UDP[s-4]+1)%MAXRecvBuf;
			}		
			g_UDP_usTick[s-4] = systickCount;									//æœ€åæ¥æ”¶çš„æ—¶é—´
		}
		#else
		//memcpy(Tx_Buffer, Rx_Buffer+8, size-8);			
		//Write_SOCK_Data_Buffer(s, Tx_Buffer, size-8);
		//Write_SOCK_Data_Buffer(s, Rx_Buffer+8, size-8);		
//		if(4==s)
//			Com_Send(RS485_2, Rx_Buffer+8, size-8);   //RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7
//		else if(5==s)
//			Com_Send(CH432T_1, Rx_Buffer+8, size-8);   //RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7
//		else if(6==s)
//			Com_Send(RS485_4, Rx_Buffer+8, size-8);   //RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7
//		else
//			Com_Send(CH432T_2, Rx_Buffer+8, size-8);   //RS485_2	1=4	CH432T_1 7=5		RS485_4	3=6		CH432T_2 8 =7
		#endif
		
//		RS485 		= 0,		//rs485 master
//    RS485_2 	= 1,  	//rs485
//		WIFI_BLE 	= 2,		//wifi & ble
//		RS485_4 	= 3,  	//rs485
//	  METER 		= 4,		//meter
//		G4				= 5,		//4g
//		USB				= 6,		//USB
//	  CH432T_1  = 7,		//ch432T com1
//	  CH432T_2  = 8		//ch432T com2  A	
	}
}

void Chan_Send(unsigned char ch)
{			
		if(w5500_rd[ch] < w5500_wr[ch])
		{
				Write_SOCK_Data_Buffer(ch,(unsigned char*)(w5500_buf[ch]+w5500_rd[ch]),w5500_wr[ch]-w5500_rd[ch]);						
		}
		else
		{
				Write_SOCK_Data_Buffer(ch,(unsigned char*)(w5500_buf[ch]+w5500_rd[ch]),D_USART_REC_BUFF_SIZE-w5500_rd[ch]);						
				Write_SOCK_Data_Buffer(ch,(unsigned char*)w5500_buf[ch],w5500_wr[ch]);										
		}		
		w5500_rd[ch] = w5500_wr[ch];		
}
	
/**********************************************************************
  * @ å‡½æ•°å  ï¼š LED_Task
  * @ åŠŸèƒ½è¯´æ˜ï¼š LED_Taskä»»åŠ¡ä¸»ä½“
  * @ å‚æ•°    ï¼š   
  * @ è¿”å›å€¼  ï¼š æ— 
  ********************************************************************/
void EtherNet_Task(void* parameter)
{	
	 uint32_t r_event = 0;  									/* å®šä¹‰ä¸€ä¸ªäº‹ä»¶æ¥æ”¶å˜é‡ */
	 uint32_t last_event = 0;									/* å®šä¹‰ä¸€ä¸ªä¿å­˜äº‹ä»¶çš„å˜é‡ */
	 BaseType_t xReturn = pdTRUE;							/* å®šä¹‰ä¸€ä¸ªåˆ›å»ºä¿¡æ¯è¿”å›å€¼ï¼Œé»˜è®¤ä¸ºpdPASS */
	
   while (1)
   {				
			if(g_configRead.b_rj45_work&0x01)		
			{	
				W5500_Socket_Set();									//W5500×‹à šÔµÊ¼Û¯Æ¤×ƒ	
				xReturn = xTaskNotifyWait(0x0,			//è¿›å…¥å‡½æ•°çš„æ—¶å€™ä¸æ¸…é™¤ä»»åŠ¡bit
                              ULONG_MAX,	  //é€€å‡ºå‡½æ•°çš„æ—¶å€™æ¸…é™¤æ‰€æœ‰çš„bitR
                              &r_event,		  //ä¿å­˜ä»»åŠ¡é€šçŸ¥å€¼                    
                              10000);				//é˜»å¡æ—¶é—´
				if( pdTRUE == xReturn )
				{ 
						last_event |= r_event;      						/* å¦‚æœæ¥æ”¶å®Œæˆå¹¶ä¸”æ­£ç¡® */
						//å®šä¹‰bit
						//bit0 				ä¸­æ–­å‘ç”Ÿ,æŸ¥è¯¢çŠ¶æ€ï¼Œå¦‚æœæœ‰æ•°æ®åˆ™è¯»å‡ºå¹¶å†™å…¥å¯¹åº”ç¼“å†²ä¸­ï¼Œæ ¹æ®é…ç½®é‡‡ç”¨äº‹ä»¶é€šçŸ¥æ–¹å¼å‘ŠçŸ¥å…¶ä»–çº¿ç¨‹ã€‚						
					  //bit1~bit8 	å¾ªç¯ç¼“å†²ä¸­æœ‰æ•°æ®å¾…å‘é€,éœ€è¦è¯»æ•°æ®å¹¶é€šè¿‡w5500å‘é€å‡ºå»ã€‚ä¸å…³å¿ƒæ•°æ®æ˜¯è°å†™å…¥çš„ã€‚å¯èƒ½å¤šä¸ªè®¾å¤‡éƒ½å‘å…¶ä¸­å†™å…¥
					  //é—®é¢˜:	é€šé“æ²¡æœ‰è¿æ¥æˆåŠŸæˆ–æ²¡æœ‰åˆå§‹åŒ–åˆ™ä¸èƒ½å‘è¯¥é€šé“ä¸­å†™å…¥æ•°æ®,ç¯å½¢ç¼“å†²æœ€åå¯èƒ½æ»¡è€Œä¸¢å¤±ã€‚					
						//bit31				w5500åŸºæœ¬å‚æ•°éœ€è¦é‡æ–°åˆå§‹åŒ–ï¼Œ 
						//æœ€ç»ˆå®ç°ä»»æ„é€šé“ä¹‹é—´è¿›è¡Œæ•°æ®çš„é€æ˜ä¼ è¾“ã€‚
					
						if(last_event&0x80000000)											//ä¸­æ–­
						{
								last_event &= ~(1<<31);
								W5500_Interrupt_Process();								//W5500××Ô¦mÔŒÑ²à ²İœ					
								//if(((S0_Data & S_RECEIVE) == S_RECEIVE) || ((S1_Data & S_RECEIVE) == S_RECEIVE) || ((S2_Data & S_RECEIVE) == S_RECEIVE))	//É§Ú»Socket0Ş“Ë•Õ½Ë½ß
								if((S_Data[0] & S_RECEIVE) == S_RECEIVE)
								{				
										S_Data[0]&=~S_RECEIVE;
										Process_Socket_Data(0);		
								}							
								#if RJ45_TCPSERVER_S1			
								if((S_Data[1] & S_RECEIVE) == S_RECEIVE)
								{				
									S_Data[1]&=~S_RECEIVE;
									Process_Socket_Data(1);											//W5500Ş“Ë•Ò¢×¢ÌŞ“Ë•Õ½Ö„Ë½ß
								}
								#endif			
								if((S_Data[2] & S_RECEIVE) == S_RECEIVE)
								{				
									S_Data[2]&=~S_RECEIVE;
									Process_Socket_Data(2);			
								}		
								if((S_Data[3] & S_RECEIVE) == S_RECEIVE)			//æ”¶åˆ°æ•°æ®
								{				
										S_Data[3]&=~S_RECEIVE;
										SnmpXDaemon();														//Process_Socket_Data(3);			
								}			
						}		
						if(last_event&0x100)				//é€šé“å‚æ•°å‘ç”Ÿå˜åŒ–å…·ä½“é‚£ä¸ªé€šé“ä¸çŸ¥é“ é€šé“1å‚æ•°å˜åŒ–
						{
							last_event &= ~(1<<8);
						}
						if(last_event&0x200)				//é€šé“å‚æ•°å‘ç”Ÿå˜åŒ–å…·ä½“é‚£ä¸ªé€šé“ä¸çŸ¥é“ é€šé“2å‚æ•°å˜åŒ–
						{
							last_event &= ~(1<<9);
						}
						
						if(last_event&0x01)					//é€šé“1å¾…å‘æ•°æ®
						{
								Chan_Send(0);
								last_event &= ~(1<<0);
						}
						if(last_event&0x02)					//é€šé“2å¾…å‘æ•°æ®
						{
								Chan_Send(1);
								last_event &= ~(1<<1);
						}
						if(last_event&0x04)					//é€šé“3å¾…å‘æ•°æ®
						{
								Chan_Send(2);
								last_event &= ~(1<<2);
						}
				}	
				else		//è¶…æ—¶ï¼Œè¿›è¡Œ1æ¬¡çŠ¶æ€å’Œä¸­æ–­ç­‰æ£€æŸ¥ï¼Œé˜²æ­¢ä¸­æ–­æ¼æ‰
				{
						//printf("rj45 ç­‰å¾…è¶…æ—¶10sï¼\n");
				}
			}		
			else
			{					
					if(NULL!=EtherNet_Task_Handle)
						vTaskSuspend(EtherNet_Task_Handle);
					else
						vTaskDelay(10000); 									//10ç§’1æ¬¡
			}
   }//while(1)
}

//åœ¨bsp_initä¸­å¯¹åˆå§‹åŒ–w5500å…¬å…±éƒ¨åˆ†,ä¾‹å¦‚ioå£åˆå§‹åŒ–ç­‰ã€‚é‡å¯ç­‰ã€‚
//åˆå§‹åŒ–w5500ç›¸å…³å†…å®¹,ä¾‹å¦‚spi,ä¸­æ–­ç­‰ã€‚

//1. ç­‰å¾…ä¸­æ–­ï¼Œå¦‚æœè§„å®šæ—¶é—´å†…æ²¡æœ‰ä¸­æ–­ï¼Œåˆ™è¶…æ—¶ï¼Œæ‰‹åŠ¨æ£€æµ‹çŠ¶æ€ã€‚ä¾‹å¦‚10ç§’ã€‚
//2. æ¥æ”¶åˆ°æ•°æ®åˆ™è½¬å‘åˆ°å¯¹åº”ç¼“å†²åŒºï¼ŒåŒæ—¶è¿›è¡Œé€šçŸ¥å¯¹åº”çº¿ç¨‹,ç›®å‰æµ‹è¯•com1ä¸»å£
//3. ç­‰å¾…æ¥æ”¶æ•°æ®ï¼Œæœ‰æ•°æ®åˆ™å‘é€åˆ°rj45.rj45æ•°æ®æ¥è‡ªå…¶ä»–å£ï¼Œä¾‹å¦‚com1çš„è½¬å‘ã€‚
//4. ä¿®æ”¹è‡ªå·±ipç­‰ä¿¡æ¯ï¼ˆå‘é€é€šçŸ¥å,ä¸éœ€è¦ç«‹å³åˆ‡æ¢ï¼Œä¸ç´§æ€¥ï¼‰ï¼Œç›®çš„è®©æœ¬ä»»åŠ¡é‡æ–°è®¾ç½®è‡ªå·±çš„ç›¸å…³ä¿¡æ¯
//5. åœ¨ä»»åŠ¡æ¢å¤çš„æ—¶å€™ï¼Œéœ€è¦æ£€æŸ¥è‡ªèº«å‚æ•°æ˜¯å¦å·²ç»å˜åŒ–è¿‡ã€‚
//6. æ¯ä¸ªé€šè®¯å£éƒ½æœ‰è‡ªå·±çš„ å‘é€ç­‰å¾…buf,è½¬å‘çš„æ—¶å€™å‘å…¶ä¸­å†™å…¥æ•°æ®ã€‚
//7. æ¯ä¸ªé€šè®¯å£éƒ½æœ‰è‡ªå·±çš„æ¥æ”¶buf,æ¥æ”¶å®Œæˆåï¼Œå°†æ•°æ®è½¬å‘ï¼Œ


//æ•´ä¸ªè¿‡ç¨‹ç‹¬ç«‹è¿è¡Œã€‚

//æ•°æ®å‘é€è¿‡ç¨‹,main(rs485ç­‰)-->rj45å‘é€æ•°æ®,é€šè¿‡ç¼“å†²æ¥æ”¶æ•°æ®,æ¥æ”¶åˆ°æ•°æ®ï¼Œè¿›è¡Œæ•°æ®çš„å‘é€

//com(3 485,meter,4g,wifi) usb canæ•°æ®é€šé“ä¹‹é—´ ä»»æ„è½¬æ¢ åˆè®¡ 8ä¸ª é€šé“
//rj45è½¬æ¢
//loraè½¬æ¢

//é€šè®¯
//æ˜¾ç¤º
//å­˜å‚¨
//é‡‡é›†
//æ§åˆ¶