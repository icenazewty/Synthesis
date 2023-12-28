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

char								w5500_buf[4][D_USART_REC_BUFF_SIZE];		//8个通道，每个通道最多512字节
unsigned short int	w5500_rd[4];														//8个通道的读写指针
unsigned short int  w5500_wr[4];														//8个通道的读写指针

/***************----- ��������������� -----***************/
//unsigned char Gateway_IP[4];	//����IP��ַ 
//unsigned char Sub_Mask[4];		//�������� 
//unsigned char Phy_Addr[6];		//�����ַ(MAC) 
//unsigned char IP_Addr[4];			//����IP��ַ 


//unsigned char S0_DIP[4];			//�˿�0Ŀ��IP��ַ 
//unsigned char S0_DPort[2];		//�˿�0Ŀ�Ķ˿ں�(6000) 

//unsigned char S1_Port[2];			//�˿�1�Ķ˿ں�(502)

unsigned char S2_Port[2];			//�˿�2�Ķ˿ں�(8888) 
unsigned char UDP_DIPR[4];		//UDP(�㲥)ģʽ,Ŀ������IP��ַ
unsigned char UDP_DPORT[2];		//UDP(�㲥)ģʽ,Ŀ�������˿ں�

unsigned char 	UDP_DIPR_A[4][4];					//UDP(ڣҥ)ģʽ,Ŀք׷ܺIPַ֘
unsigned char 	UDP_DPORT_A[4][2];				//UDP(ڣҥ)ģʽ,Ŀք׷ܺ׋ࠚۿ

/***************----- �˿ڵ�����ģʽ -----***************/
//unsigned char S0_Mode =3;		//�˿�0������ģʽ,0:TCP������ģʽ,1:TCP�ͻ���ģʽ,2:UDP(�㲥)ģʽ
//unsigned char S1_Mode =3;		//�˿�1������ģʽ,0:TCP������ģʽ,1:TCP�ͻ���ģʽ,2:UDP(�㲥)ģʽ
//unsigned char S2_Mode =3;		//�˿�1������ģʽ,0:TCP������ģʽ,1:TCP�ͻ���ģʽ,2:UDP(�㲥)ģʽ
//#define TCP_SERVER	0x00		//TCP������ģʽ
//#define TCP_CLIENT	0x01		//TCP�ͻ���ģʽ 
//#define UDP_MODE	0x02			//UDP(�㲥)ģʽ 

/***************----- �˿ڵ�����״̬ -----***************/
unsigned char S_State[8] ;	//�˿�0״̬��¼,1:�˿���ɳ�ʼ��,2�˿��������(����������������) 

#define S_INIT		0x01			//�˿���ɳ�ʼ�� 
#define S_CONN		0x02			//�˿��������,���������������� 

/***************----- �˿��շ����ݵ�״̬ -----***************/
unsigned char S_Data[8];		//�˿�0���պͷ������ݵ�״̬,1:�˿ڽ��յ�����,2:�˿ڷ���������� 
//unsigned char S1_Data;		//�˿�1���պͷ������ݵ�״̬,1:�˿ڽ��յ�����,2:�˿ڷ����������
//unsigned char S2_Data;		//�˿�1���պͷ������ݵ�״̬,1:�˿ڽ��յ�����,2:�˿ڷ����������
unsigned char Data_Status;  //0Ϊδ�������״̬��1Ϊ�������״̬
#define S_RECEIVE	 0x01			//�˿ڽ��յ�һ�����ݰ� 
#define S_TRANSMITOK 0x02		//�˿ڷ���һ�����ݰ���� 

/***************----- �˿����ݻ����� -----***************/

unsigned char Rx_Buffer[1588];									//2048
//unsigned char Tx_Buffer[2048];								//�˿ڷ������ݻ����� 

unsigned char W5500_Interrupt;									//W5500�жϱ�־(0:���ж�,1:���ж�)
extern TaskHandle_t 	EtherNet_Task_Handle;			/* w5500任务句柄 */

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
		
		//W5500 int Low active PC6->PB0   必须上拉
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
		nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);		// NVIC_PRIGROUP_PRE0_SUB4  0 0  则最高级中断；   NVIC_PRIGROUP_PRE4_SUB0  15  0 则为最低中断
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
   w5500_send_byte(half_word/256);	//дʽ¾ݸߎ»
	 w5500_send_byte(half_word);			//дʽ¾ݵ͎»
}

/*******************************************************************************
* ������  : Write_W5500_1Byte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���д1���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,dat:��д�������
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_1Byte(unsigned short reg, unsigned char dat)
{
	W5500_CS_LOW();														//��W5500��SCSΪ�͵�ƽ
	w5500_send_halfword(reg);									//ͨ��SPI1д16λ�Ĵ�����ַ
	w5500_send_byte(FDM1|RWB_WRITE|COMMON_R);	//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���
	w5500_send_byte(dat);											//д1���ֽ�����
	W5500_CS_HIGH(); 													//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_2Byte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���д2���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,dat:16λ��д�������(2���ֽ�)
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_2Byte(unsigned short reg, unsigned short dat)
{
	W5500_CS_LOW();														//��W5500��SCSΪ�͵�ƽ		
	w5500_send_halfword(reg);									//ͨ��SPI1д16λ�Ĵ�����ַ
	w5500_send_byte(FDM2|RWB_WRITE|COMMON_R);	//ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���
	w5500_send_halfword(dat);									//д16λ����
	W5500_CS_HIGH(); 													//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_nByte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���дn���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,*dat_ptr:��д�����ݻ�����ָ��,size:��д������ݳ���
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_nByte(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short i;
	W5500_CS_LOW();															//��W5500��SCSΪ�͵�ƽ			
	w5500_send_halfword(reg);										//ͨ��SPI1д16λ�Ĵ�����ַ
	w5500_send_byte(VDM|RWB_WRITE|COMMON_R);		//ͨ��SPI1д�����ֽ�,N���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���
	for(i=0;i<size;i++)															//ѭ������������size���ֽ�����д��W5500
	{
		w5500_send_byte(*dat_ptr++);							//дһ���ֽ�����
	}
	W5500_CS_HIGH(); 														//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_1Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д1���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,dat:��д�������
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
	W5500_CS_LOW();																	//��W5500��SCSΪ�͵�ƽ			
	w5500_send_halfword(reg);												//ͨ��SPI1д16λ�Ĵ�����ַ
	w5500_send_byte(FDM1|RWB_WRITE|(s*0x20+0x08));	//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���
	w5500_send_byte(dat);														//д1���ֽ�����
	W5500_CS_HIGH(); 																//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_2Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д2���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,dat:16λ��д�������(2���ֽ�)
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
	W5500_CS_LOW();																	//��W5500��SCSΪ�͵�ƽ			
	w5500_send_halfword(reg);												//ͨ��SPI1д16λ�Ĵ�����ַ
	w5500_send_byte(FDM2|RWB_WRITE|(s*0x20+0x08));	//ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���
	w5500_send_halfword(dat);												//д16λ����
	W5500_CS_HIGH(); 																//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_4Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д4���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,*dat_ptr:��д���4���ֽڻ�����ָ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
	W5500_CS_LOW();																	//��W5500��SCSΪ�͵�ƽ			
	w5500_send_halfword(reg);												//ͨ��SPI1д16λ�Ĵ�����ַ
	w5500_send_byte(FDM4|RWB_WRITE|(s*0x20+0x08));	//ͨ��SPI1д�����ֽ�,4���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���
	w5500_send_byte(*dat_ptr++);										//д��1���ֽ�����
	w5500_send_byte(*dat_ptr++);										//д��2���ֽ�����
	w5500_send_byte(*dat_ptr++);										//д��3���ֽ�����
	w5500_send_byte(*dat_ptr++);										//д��4���ֽ�����
	W5500_CS_HIGH(); 																//��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Read_W5500_1Byte
* ����    : ��W5500ָ����ַ�Ĵ�����1���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����1���ֽ�����
* ˵��    : ��
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
* ������  : Read_W5500_SOCK_1Byte
* ����    : ��W5500ָ���˿ڼĴ�����1���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����1���ֽ�����
* ˵��    : ��
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
* ������  : Read_W5500_SOCK_2Byte
* ����    : ��W5500ָ���˿ڼĴ�����2���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����2���ֽ�����(16λ)
* ˵��    : ��
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
* ������  : Read_SOCK_Data_Buffer
* ����    : ��W5500�������ݻ������ж�ȡ����
* ����    : s:�˿ں�,*dat_ptr:���ݱ��滺����ָ��
* ���    : ��
* ����ֵ  : ��ȡ�������ݳ���,rx_size���ֽ�
* ˵��    : ��
*******************************************************************************/
unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
	unsigned short rx_size;
	unsigned short offset, offset1;
	unsigned short i;
	unsigned char j;

	rx_size=Read_W5500_SOCK_2Byte(s,Sn_RX_RSR);
	if(rx_size==0) return 0;										//没接收到数据则返回
	if(rx_size>1460) rx_size=1460;

	offset=Read_W5500_SOCK_2Byte(s,Sn_RX_RD);
	offset1=offset; 
	offset&=(S_RX_SIZE-1);						//计算实际的物理地址

	W5500_CS_LOW();//��W5500��SCSΪ�͵�ƽ

	w5500_send_halfword(offset);//д16λ��ַ   
	j = w5500_send_byte(VDM|RWB_READ|(s*0x20+0x18));		//写控制字节,N个字节数据长度,读数据,选择端口s的寄存器
	//j=w5500_read_byte();

	if((offset+rx_size)<S_RX_SIZE)		//如果最大地址未超过W5500接收缓冲区寄存器的最大地址
	{
		for(i=0;i<rx_size;i++)//ѭ����ȡrx_size���ֽ�����
		{
			//w5500_send_byte(0x00);//����һ��������
			j=w5500_read_byte();//��ȡ1���ֽ�����
			*dat_ptr=j;//����ȡ�������ݱ��浽���ݱ��滺����
			dat_ptr++;//���ݱ��滺����ָ���ַ����1
		}
	}
	else//�������ַ����W5500���ջ������Ĵ���������ַ
	{
		offset=S_RX_SIZE-offset;
		for(i=0;i<offset;i++)//ѭ����ȡ��ǰoffset���ֽ�����
		{
			//w5500_send_byte(0x00);//����һ��������
			j = w5500_read_byte();//��ȡ1���ֽ�����
			*dat_ptr=j;//����ȡ�������ݱ��浽���ݱ��滺����
			dat_ptr++;//���ݱ��滺����ָ���ַ����1
		}
		W5500_CS_HIGH(); //��W5500��SCSΪ�ߵ�ƽ

		W5500_CS_LOW();//��W5500��SCSΪ�͵�ƽ

		w5500_send_halfword(0x00);//д16λ��ַ
		j=w5500_send_byte(VDM|RWB_READ|(s*0x20+0x18));//д�����ֽ�,N���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���
		//j=w5500_read_byte();

		for(;i<rx_size;i++)//ѭ����ȡ��rx_size-offset���ֽ�����
		{
			//w5500_send_byte(0x00);//����һ��������
			j=w5500_read_byte();//��ȡ1���ֽ�����
			*dat_ptr=j;//����ȡ�������ݱ��浽���ݱ��滺����
			dat_ptr++;//���ݱ��滺����ָ���ַ����1
		}
	}
	W5500_CS_HIGH(); //��W5500��SCSΪ�ߵ�ƽ

	offset1+=rx_size;//����ʵ�������ַ,���´ζ�ȡ���յ������ݵ���ʼ��ַ
	Write_W5500_SOCK_2Byte(s, Sn_RX_RD, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);//����������������
	return rx_size;//���ؽ��յ����ݵĳ���
}

/*******************************************************************************
* ������  : Write_SOCK_Data_Buffer
* ����    : ������д��W5500�����ݷ��ͻ�����
* ����    : s:�˿ں�,*dat_ptr:���ݱ��滺����ָ��,size:��д�����ݵĳ���
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short offset,offset1;
	unsigned short i;

	//�����UDPģʽ,�����ڴ�����Ŀ��������IP�Ͷ˿ں�
//	unsigned char ucRead;
	//ucRead = Read_W5500_SOCK_1Byte(s,Sn_SR);
	Read_W5500_SOCK_1Byte(s,Sn_SR);
	//if(ucRead&0x0f) != SOCK_UDP)//���Socket��ʧ��
	if(s==2)		//udp的时候采�?		8888
	{		
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, UDP_DIPR);													//����Ŀ������IP  		
		//Write_W5500_SOCK_4Byte(s, Sn_DIPR, gs_SaveNetIPCfg.ucDestIP);						//ɨփĿµĖ÷»úIP  		
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT[0]*256+UDP_DPORT[1]);	//����Ŀ�������˿ں�				
		//Write_W5500_SOCK_2Byte(s, Sn_DPORTR, gs_SaveNetIPCfg.ucDestIP);					//ɨփĿµĖ÷»ú¶˿ںŉ			
	}
	else if(s>3)		
	{
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, UDP_DIPR_A[s-4]);														//ɨփĿµĖ÷»úIP  		
		//Write_W5500_SOCK_4Byte(s, Sn_DIPR, gs_SaveNetIPCfg.ucDestIP);									//ɨփĿµĖ÷»úIP  		
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT_A[s-4][0]*256+UDP_DPORT_A[s-4][1]);	//ɨփĿµĖ÷»ú¶˿ںŉ			
	}
	//else if(s==0)			//检测上传ip和port是否发生变化,如果发生变化，则修改上传port和ip
	{

	}
	offset=Read_W5500_SOCK_2Byte(s,Sn_TX_WR);
	offset1=offset;
	offset&=(S_TX_SIZE-1);//计算实际的物理地址

	W5500_CS_LOW();//��W5500��SCSΪ�͵�ƽ

	w5500_send_halfword(offset);//д16λ��ַ
	w5500_send_byte(VDM|RWB_WRITE|(s*0x20+0x10));//д�����ֽ�,N���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���

	if((offset+size)<S_TX_SIZE)		//如果最大地址未超过W5500发送缓冲区寄存器的最大地址
	{
		for(i=0;i<size;i++)//ѭ��д��size���ֽ�����
		{
			w5500_send_byte(*dat_ptr++);//д��һ���ֽڵ�����		
		}
	}
	else//�������ַ����W5500���ͻ������Ĵ���������ַ
	{
		offset=S_TX_SIZE-offset;
		for(i=0;i<offset;i++)//ѭ��д��ǰoffset���ֽ�����
		{
			w5500_send_byte(*dat_ptr++);//д��һ���ֽڵ�����
		}
		W5500_CS_HIGH(); //��W5500��SCSΪ�ߵ�ƽ

		W5500_CS_LOW();//��W5500��SCSΪ�͵�ƽ

		w5500_send_halfword(0x00);//д16λ��ַ
		w5500_send_byte(VDM|RWB_WRITE|(s*0x20+0x10));//д�����ֽ�,N���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���

		for(;i<size;i++)//ѭ��д��size-offset���ֽ�����
		{
			w5500_send_byte(*dat_ptr++);//д��һ���ֽڵ�����
		}
	}
	W5500_CS_HIGH(); //��W5500��SCSΪ�ߵ�ƽ

	offset1+=size;//����ʵ�������ַ,���´�д���������ݵ��������ݻ���������ʼ��ַ
	Write_W5500_SOCK_2Byte(s, Sn_TX_WR, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);//����������������				
}

/*******************************************************************************
* ������  : W5500_Hardware_Reset
* ����    : Ӳ����λW5500
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : W5500�ĸ�λ���ű��ֵ͵�ƽ����500us����,������ΧW5500
*******************************************************************************/
void W5500_Hardware_Reset(void)
{
	W5500_RST_LOW();			//��λ��������	
	delay_1ms(5);			
	W5500_RST_HIGH();
	delay_1ms(50);	
	
//	while((Read_W5500_1Byte(PHYCFGR)&LINK)==0);//�ȴ���̫���������	
}

/*******************************************************************************
* ������  : W5500_Init
* ����    : ��ʼ��W5500�Ĵ�������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��ʹ��W5500֮ǰ���ȶ�W5500��ʼ��
*******************************************************************************/
void W5500_Init(void)
{
	uint8_t i=0;

	Write_W5500_1Byte(MR, RST);//�����λW5500,��1��Ч,��λ���Զ���0
	//Delay(10);//��ʱ10ms,�Լ�����ú���
	delay_1ms(10);
	//��������(Gateway)��IP��ַ,Gateway_IPΪ4�ֽ�unsigned char����,�Լ����� 
	//ʹ�����ؿ���ʹͨ��ͻ�������ľ��ޣ�ͨ�����ؿ��Է��ʵ��������������Internet
	Write_W5500_nByte(GAR, gs_SaveNetIPCfg.ucGateWay, 4);
			
	//������������(MASK)ֵ,SUB_MASKΪ4�ֽ�unsigned char����,�Լ�����
	//��������������������
	Write_W5500_nByte(SUBR,gs_SaveNetIPCfg.ucSubMASK,4);		
	
	//���������ַ,PHY_ADDRΪ6�ֽ�unsigned char����,�Լ�����,����Ψһ��ʶ�����豸�������ֵַ
	//�õ�ֵַ��Ҫ��IEEE���룬����OUI�Ĺ涨��ǰ3���ֽ�Ϊ���̴��룬�������ֽ�Ϊ��Ʒ���
	//����Լ����������ַ��ע���һ���ֽڱ���Ϊż��
	Write_W5500_nByte(SHAR,gs_SaveNetIPCfg.ucMAC,6);		

	//���ñ�����IP��ַ,IP_ADDRΪ4�ֽ�unsigned char����,�Լ�����
	//ע�⣬����IP�����뱾��IP����ͬһ�����������򱾻����޷��ҵ�����
	Write_W5500_nByte(SIPR,gs_SaveNetIPCfg.ucSelfIP,4);		
	
	//���÷��ͻ������ͽ��ջ������Ĵ�С���ο�W5500�����ֲ�
	for(i=0;i<8;i++)
	{
		Write_W5500_SOCK_1Byte(i,Sn_RXBUF_SIZE, 0x02);//Socket Rx memory size=2k
		Write_W5500_SOCK_1Byte(i,Sn_TXBUF_SIZE, 0x02);//Socket Tx mempry size=2k
	}

	//��������ʱ�䣬Ĭ��Ϊ2000(200ms) 
	//ÿһ��λ��ֵΪ100΢��,��ʼ��ʱֵ��Ϊ2000(0x07D0),����200����
	Write_W5500_2Byte(RTR_W5500, 0x0BB8);			//Write_W5500_2Byte(RTR, 0x07d0);			300ms

	//�������Դ�����Ĭ��Ϊ8�� 
	//����ط��Ĵ��������趨ֵ,�������ʱ�ж�(��صĶ˿��жϼĴ����е�Sn_IR ��ʱλ(TIMEOUT)�á�1��)
	Write_W5500_1Byte(RCR_W5500,8);

	//�����жϣ��ο�W5500�����ֲ�ȷ���Լ���Ҫ���ж�����
	//IMR_CONFLICT��IP��ַ��ͻ�쳣�ж�,IMR_UNREACH��UDPͨ��ʱ����ַ�޷�������쳣�ж�
	//������Socket�¼��жϣ�������Ҫ���
	Write_W5500_1Byte(IMR_W5500, IM_IR6);				//目的地址不能到达和ip冲突中断开�? IM_IR7=ip冲突去掉，否则一直产生中�?
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
* ������  : Detect_Gateway
* ����    : ������ط�����
* ����    : ��
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ��
*******************************************************************************/
unsigned char Detect_Gateway(void)
{
	unsigned char ip_adde[4];
	ip_adde[0]=gs_SaveNetIPCfg.ucSelfIP[0]+1;
	ip_adde[1]=gs_SaveNetIPCfg.ucSelfIP[1]+1;
	ip_adde[2]=gs_SaveNetIPCfg.ucSelfIP[2]+1;
	ip_adde[3]=gs_SaveNetIPCfg.ucSelfIP[3]+1;

	//������ؼ���ȡ���ص������ַ
	Write_W5500_SOCK_4Byte(0,Sn_DIPR,ip_adde);//��Ŀ�ĵ�ַ�Ĵ���д���뱾��IP��ͬ��IPֵ
	Write_W5500_SOCK_1Byte(0,Sn_MR,MR_TCP);//����socketΪTCPģʽ
	Write_W5500_SOCK_1Byte(0,Sn_CR,OPEN);//��Socket	
	//Delay(5);//��ʱ5ms 	
	delay_1ms(5);
	if(Read_W5500_SOCK_1Byte(0,Sn_SR) != SOCK_INIT)//���socket��ʧ��
	{
		Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);//�򿪲��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}

	Write_W5500_SOCK_1Byte(0,Sn_CR,CONNECT);//����SocketΪConnectģʽ						

	do
	{
		uint8_t j=0;	
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);//��ȡSocket0�жϱ�־�Ĵ���
		if(j!=0)
		Write_W5500_SOCK_1Byte(0,Sn_IR,j);	
		delay_1ms(5);
		if((j&IR_TIMEOUT) == IR_TIMEOUT)
		{
			return FALSE;	
		}
		else if(Read_W5500_SOCK_1Byte(0,Sn_DHAR) != 0xff)
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);//�ر�Socket
			return TRUE;							
		}
		wdt();
	}while(1);
}

/*******************************************************************************
* ������  : Socket_Init
* ����    : ָ��Socket(0~7)��ʼ��
* ����    : s:����ʼ���Ķ˿�
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Socket_Init(SOCKET s)
{
	char tmp[8];
	unsigned short port = 0;
	Write_W5500_SOCK_2Byte(s, Sn_MSSR, 1460);							//Socket n 的最大传输单元 MTU
	//����ָ���˿�
	if(s>3)
	{
				//S2_Port[0]=0x22;								//0x22b7=8887
				//S2_Port[1]=0xb8;
				//S2_Mode = UDP_MODE;					//¼Ӕض˿ڲµĹ¤׷ģʽ,UDPģʽ
				port = (gs_SaveNetIPCfg.ucUdpSourcePort[0]<<8|gs_SaveNetIPCfg.ucUdpSourcePort[1])+s-3;
				Write_W5500_SOCK_2Byte(s, 	Sn_PORT, 		port);		//监听端口8888		
				
				port = (gs_SaveNetIPCfg.ucUdpDestPort[0]<<8|gs_SaveNetIPCfg.ucUdpDestPort[1])+s-3;
				Write_W5500_SOCK_2Byte(s, 	Sn_DPORTR, 	port);				//目标端口8887
				Write_W5500_SOCK_4Byte(s, 	Sn_DIPR, 		gs_SaveNetIPCfg.ucUdpDestIP);				//目标IP		
	}
	else
	{
		switch(s)
		{
			case 0:										//tcp client
				//���ö˿�0�Ķ˿ں�
				Write_W5500_SOCK_2Byte(0, 	Sn_PORT, 		gs_SaveNetIPCfg.ucSourcePort[0]*256+gs_SaveNetIPCfg.ucSourcePort[1]);
				//���ö˿�0Ŀ��(Զ��)�˿ں�
				port = atoi(g_configRead.remotePort);
				Write_W5500_SOCK_2Byte(0, 	Sn_DPORTR, 	port);	//Write_W5500_SOCK_2Byte(0, 	Sn_DPORTR, 	gs_SaveNetIPCfg.ucDestPort[0]*256+gs_SaveNetIPCfg.ucDestPort[1]);
				//���ö˿�0Ŀ��(Զ��)IP��ַ
			
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

			case 2:									//udp	8888  udp发送端�?
				S2_Port[0]=0x22;			//0x22b7=8887
				S2_Port[1]=0xb8;
				//S2_Mode = UDP_MODE;		//���ض˿�2�Ĺ���ģʽ,UDPģʽ
				Write_W5500_SOCK_2Byte(2, 	Sn_PORT, 		gs_SaveNetIPCfg.ucUdpSourcePort[0]<<8|gs_SaveNetIPCfg.ucUdpSourcePort[1]);		//监听端口8888		
				Write_W5500_SOCK_2Byte(2, 	Sn_DPORTR, 	gs_SaveNetIPCfg.ucUdpDestPort[0]<<8|gs_SaveNetIPCfg.ucUdpDestPort[1]);				//目标端口8887
				Write_W5500_SOCK_4Byte(2, 	Sn_DIPR, 		gs_SaveNetIPCfg.ucUdpDestIP);				//目标IP		
				break;

			case 3:
				//S2_Mode = UDP_MODE;		//¼Ӕض˿ڲµĹ¤׷ģʽ,UDPģʽ
				Write_W5500_SOCK_2Byte(3, 	Sn_PORT, 		161);													// snmp 本地监听端口 161
				Write_W5500_SOCK_2Byte(3, 	Sn_DPORTR, 	161);													// 目标端口161  被动接收数据，实际目标port为收到的port端口 
				Write_W5500_SOCK_4Byte(3, 	Sn_DIPR, 		gs_SaveNetIPCfg.ucUdpDestIP);	// 目标IP		    被动接收数据，实际目标ip为收到的ip地址 
				break;	

			default:
				break;
		}
	}
}

/*******************************************************************************
* ������  : Socket_Connect
* ����    : ����ָ��Socket(0~7)Ϊ�ͻ�����Զ�̷���������
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ������Socket�����ڿͻ���ģʽʱ,���øó���,��Զ�̷�������������
*			����������Ӻ���ֳ�ʱ�жϣ��������������ʧ��,��Ҫ���µ��øó�������
*			�ó���ÿ����һ��,�������������һ������
*******************************************************************************/
unsigned char Socket_Connect(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);//����socketΪTCPģʽ
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);  //��Socket
	//Delay(5);//��ʱ5ms
	delay_1ms(5);
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)//���socket��ʧ��
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//�򿪲��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}
	Write_W5500_SOCK_1Byte(s,Sn_CR,CONNECT);//����SocketΪConnectģʽ
	return TRUE;//����TRUE,���óɹ�
}

/*******************************************************************************
* ������  : Socket_Listen
* ����    : ����ָ��Socket(0~7)��Ϊ�������ȴ�Զ������������
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ������Socket�����ڷ�����ģʽʱ,���øó���,�ȵ�Զ������������
*			�ó���ֻ����һ��,��ʹW5500����Ϊ������ģʽ
*******************************************************************************/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);//����socketΪTCPģʽ 
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//��Socket	
	//Delay(5);//��ʱ5ms
	delay_1ms(5);
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)//���socket��ʧ��
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//�򿪲��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}	
	Write_W5500_SOCK_1Byte(s,Sn_CR,LISTEN);//����SocketΪ����ģʽ	
	delay_1ms(5);//Delay(5);//��ʱ5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_LISTEN)//���socket����ʧ��
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//���ò��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}
	return TRUE;

	//���������Socket�Ĵ򿪺�������������,����Զ�̿ͻ����Ƿ�������������,����Ҫ�ȴ�Socket�жϣ�
	//���ж�Socket�������Ƿ�ɹ����ο�W5500�����ֲ��Socket�ж�״̬
	//�ڷ���������ģʽ����Ҫ����Ŀ��IP��Ŀ�Ķ˿ں�
}

/*******************************************************************************
* ������  : Socket_UDP
* ����    : ����ָ��Socket(0~7)ΪUDPģʽ
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ���Socket������UDPģʽ,���øó���,��UDPģʽ��,Socketͨ�Ų���Ҫ��������
*			�ó���ֻ����һ�Σ���ʹW5500����ΪUDPģʽ
*******************************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_UDP);//����SocketΪUDPģʽ*/
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//��Socket*/
	delay_1ms(5);	//Delay(5);//��ʱ5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_UDP)//���Socket��ʧ��
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//�򿪲��ɹ�,�ر�Socket
		return FALSE;//����FALSE(0x00)
	}
	else
		return TRUE;

	//���������Socket�Ĵ򿪺�UDPģʽ����,������ģʽ��������Ҫ��Զ��������������
	//��ΪSocket����Ҫ��������,�����ڷ�������ǰ����������Ŀ������IP��Ŀ��Socket�Ķ˿ں�
	//���Ŀ������IP��Ŀ��Socket�Ķ˿ں��ǹ̶���,�����й�����û�иı�,��ôҲ��������������
}

/*******************************************************************************
* ������  : W5500_Interrupt_Process
* ����    : W5500�жϴ��������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void W5500_Interrupt_Process(void)
{
	unsigned char i,j,k;
	IntDispose:
	W5500_Interrupt=0;								//�����жϱ�־
	i = Read_W5500_1Byte(IR);					//��ȡ�жϱ�־�Ĵ���
	Write_W5500_1Byte(IR, (i&0xf0));	//��д����жϱ�־	
	if((i & CONFLICT) == CONFLICT)		//IP��ַ��ͻ�쳣����
	{
		 //�Լ���Ӵ���
	}
	wdt();
	if((i & UNREACH) == UNREACH)			//UDPģʽ�µ�ַ�޷������쳣����
	{
			//�Լ���Ӵ���
	}

	i=Read_W5500_1Byte(SIR);					//��ȡ�˿��жϱ�־�Ĵ���	
	if((i & S0_INT) == S0_INT)				//Socket0�¼����� 
	{
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);		//��ȡSocket0�жϱ�־�Ĵ���
		Write_W5500_SOCK_1Byte(0,Sn_IR,j);
		if(j&IR_CON)												//��TCPģʽ��,Socket0�ɹ����� 
		{
			S_State[0]|=S_CONN;									//��������״̬0x02,�˿�������ӣ�����������������
		}
		if(j&IR_DISCON)				//��TCPģʽ��Socket�Ͽ����Ӵ���
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);//�رն˿�,�ȴ����´����� 
			Socket_Init(0);			//ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
			S_State[0]=0;					//��������״̬0x00,�˿�����ʧ��
		}
		if(j&IR_SEND_OK)			//Socket0���ݷ������,�����ٴ�����S_tx_process()������������ 
		{
			Data_Status=1;
			S_Data[0]|=S_TRANSMITOK;//�˿ڷ���һ�����ݰ���� 
		}
		if(j&IR_RECV)						//Socket���յ�����,��������S_rx_process()���� 
		{
			S_Data[0]|=S_RECEIVE;		//�˿ڽ��յ�һ�����ݰ�
		}
		if(j&IR_TIMEOUT)													//Socket���ӻ����ݴ��䳬ʱ���� 
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);	// �رն˿�,�ȴ����´����� 
			S_State[0]=0;															//��������״̬0x00,�˿�����ʧ��
		}
	}
	
	#if RJ45_TCPSERVER_S1	
	if((i & S1_INT) == S1_INT)//Socket1�¼����� 
	{
		j=Read_W5500_SOCK_1Byte(1,Sn_IR);//��ȡSocket0�жϱ�־�Ĵ���
		Write_W5500_SOCK_1Byte(1,Sn_IR,j);
		if(j&IR_CON)//��TCPģʽ��,Socket0�ɹ����� 
		{
			S_State[1]|=S_CONN;//��������״̬0x02,�˿�������ӣ�����������������
		}
		if(j&IR_DISCON)//��TCPģʽ��Socket�Ͽ����Ӵ���
		{
			Write_W5500_SOCK_1Byte(1,Sn_CR,CLOSE);//�رն˿�,�ȴ����´����� 
			Socket_Init(1);		//ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
			S_State[1]=0;//��������״̬0x00,�˿�����ʧ��
		}
		if(j&IR_SEND_OK)//Socket0���ݷ������,�����ٴ�����S_tx_process()������������ 
		{
			S_Data[1]|=S_TRANSMITOK;//�˿ڷ���һ�����ݰ���� 
		}
		if(j&IR_RECV)//Socket���յ�����,��������S_rx_process()���� 
		{
			S_Data[1]|=S_RECEIVE;//�˿ڽ��յ�һ�����ݰ�
		}
		if(j&IR_TIMEOUT)//Socket���ӻ����ݴ��䳬ʱ���� 
		{
			Write_W5500_SOCK_1Byte(1,Sn_CR,CLOSE);// �رն˿�,�ȴ����´����� 
			S_State[1]=0;//��������״̬0x00,�˿�����ʧ��
		}
	}
#endif
	
	if((i & S2_INT) == S2_INT)//Socket0�¼����� 
	{
		j=Read_W5500_SOCK_1Byte(2,Sn_IR);//��ȡSocket0�жϱ�־�Ĵ���
		Write_W5500_SOCK_1Byte(2,Sn_IR,j);
		if(j&IR_CON)//��TCPģʽ��,Socket0�ɹ����� 
		{
			S_State[2]|=S_CONN;//��������״̬0x02,�˿�������ӣ�����������������
		}
		if(j&IR_DISCON)//��TCPģʽ��Socket�Ͽ����Ӵ���
		{
			Write_W5500_SOCK_1Byte(2,Sn_CR,CLOSE);//�رն˿�,�ȴ����´����� 
			Socket_Init(2);		//ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
			S_State[2]=0;//��������״̬0x00,�˿�����ʧ��
		}
		if(j&IR_SEND_OK)						//Socket0���ݷ������,�����ٴ�����S_tx_process()������������ 
		{
			S_Data[2]|=S_TRANSMITOK;	//�˿ڷ���һ�����ݰ���� 
		}
		if(j&IR_RECV)//Socket���յ�����,��������S_rx_process()���� 
		{
			S_Data[2]|=S_RECEIVE;//�˿ڽ��յ�һ�����ݰ�
		}
		if(j&IR_TIMEOUT)//Socket���ӻ����ݴ��䳬ʱ���� 
		{
			Write_W5500_SOCK_1Byte(2,Sn_CR,CLOSE);// �رն˿�,�ȴ����´����� 
			S_State[2]=0;//��������״̬0x00,�˿�����ʧ��
		}
	}
	
	if((i & S3_INT) == S3_INT)//Socket0ʂ¼þ´¦À𓐊	
	{
		j=Read_W5500_SOCK_1Byte(3,Sn_IR);//¶ÁȡSocket0֐¶ϱꖾ¼Ĵ憷
		Write_W5500_SOCK_1Byte(3,Sn_IR,j);
		if(j&IR_CON)//ԚTCPģʽς,Socket0³ɹ¦Á¬½Ӡ
		{
			S_State[3]|=S_CONN;//͸§Á¬½ӗ´̬0x02,¶˿ڍ곉Á¬½ӣ¬¿ɒԕý³£´«ʤʽ¾ݍ
		}
		if(j&IR_DISCON)//ԚTCPģʽςSocket¶ϿªÁ¬½Ӵ¦À퍊		
		{
			Write_W5500_SOCK_1Byte(3,Sn_CR,CLOSE);//¹رն˿ڬµȴý֘Ђ´򿪁¬½Ӡ
			Socket_Init(3);		//ָ¶¨Socket(0~7)³õʼ»¯,³õʼ»¯¶˿ڰ
			S_State[3]=0;//͸§Á¬½ӗ´̬0x00,¶˿ځ¬½ӊ§°܍
		}
		if(j&IR_SEND_OK)						//Socket0ʽ¾ݷ¢ˍͪ³ɬ¿ɒԔٴΆ𶯓_tx_process()º¯ʽ·¢ˍʽ¾ݠ
		{
			S_Data[3]|=S_TRANSMITOK;	//¶˿ڷ¢ˍһ¸öʽ¾ݰüͪ³ɠ
		}
		if(j&IR_RECV)//Socket½ӊյ½ʽ¾ݬ¿ɒԆ𶯓_rx_process()º¯ʽ 
		{
			S_Data[3]|=S_RECEIVE;//¶˿ڽӊյ½һ¸öʽ¾ݰü
		}
		if(j&IR_TIMEOUT)//SocketÁ¬½ӻ򊽾ݴ«ʤ³¬ʱ´¦À𓐊		
		{
			Write_W5500_SOCK_1Byte(3,Sn_CR,CLOSE);// ¹رն˿ڬµȴý֘Ђ´򿪁¬½Ӡ
			S_State[3]=0;//͸§Á¬½ӗ´̬0x00,¶˿ځ¬½ӊ§°܍
		}
	}
	
	for(k=4;k<8;k++)
	{
			if(i & (1<<k))												//Socket0ʂ¼þ´¦À𓐊	
			{
				j=Read_W5500_SOCK_1Byte(k,Sn_IR);		//¶ÁȡSocket0֐¶ϱꖾ¼Ĵ憷
				Write_W5500_SOCK_1Byte(k,Sn_IR,j);
				if(j&IR_CON)												//ԚTCPģʽς,Socket0³ɹ¦Á¬½Ӡ
				{
					S_State[k]|=S_CONN;								//͸§Á¬½ӗ´̬0x02,¶˿ڍ곉Á¬½ӣ¬¿ɒԕý³£´«ʤʽ¾ݍ
				}
				if(j&IR_DISCON)											//ԚTCPģʽςSocket¶ϿªÁ¬½Ӵ¦À퍊		
				{
					Write_W5500_SOCK_1Byte(k,Sn_CR,CLOSE);	//¹رն˿ڬµȴý֘Ђ´򿪁¬½Ӡ
					Socket_Init(k);													//ָ¶¨Socket(0~7)³õʼ»¯,³õʼ»¯¶˿ڰ
					S_State[k]=0;														//͸§Á¬½ӗ´̬0x00,¶˿ځ¬½ӊ§°܍
				}
				if(j&IR_SEND_OK)													//Socket0ʽ¾ݷ¢ˍͪ³ɬ¿ɒԔٴΆ𶯓_tx_process()º¯ʽ·¢ˍʽ¾ݠ
				{
					S_Data[k]|=S_TRANSMITOK;								//¶˿ڷ¢ˍһ¸öʽ¾ݰüͪ³ɠ
				}
				if(j&IR_RECV)															//Socket½ӊյ½ʽ¾ݬ¿ɒԆ𶯓_rx_process()º¯ʽ 
				{
					S_Data[k]|=S_RECEIVE;										//¶˿ڽӊյ½һ¸öʽ¾ݰü
				}
				if(j&IR_TIMEOUT)													//SocketÁ¬½ӻ򊽾ݴ«ʤ³¬ʱ´¦À𓐊		
				{
					Write_W5500_SOCK_1Byte(k,Sn_CR,CLOSE);	// ¹رն˿ڬµȴý֘Ђ´򿪁¬½Ӡ
					S_State[k]=0;														//͸§Á¬½ӗ´̬0x00,¶˿ځ¬½ӊ§°܍
				}
			}
	}
	
	if(Read_W5500_1Byte(SIR) != 0) 
		goto IntDispose;
}

/*******************************************************************************
* º¯ʽû  : W5500_Initialization
* èʶ    : W5500³õʼ»õŤփ
* ʤȫ    : Ξ
* ʤ³ö    : Ξ
* ·µ»ؖµ  : Ξ
* ˵÷    : Ξ
*******************************************************************************/
void W5500_Initialization(void)
{
	W5500_Init();			//³õʼ»¯W5500¼Ĵ憷º¯ʽ	
	Detect_Gateway();	//¼첩͸¹طþαƷ 
	Socket_Init(0);		//ָ¶¨Socket(0~7)³õʼ»¯,³õʼ»¯¶˿ڰ
#if RJ45_TCPSERVER_S1		
	Socket_Init(1);		//ָ¶¨Socket(0~7)³õʼ»¯,³õʼ»¯¶˿ڱ
#endif	
	Socket_Init(2);		//ָ¶¨Socket(0~7)³õʼ»¯,³õʼ»¯¶˿ڲ
	Socket_Init(3);		//snmp udp通道
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
	//unsigned short addrStart=0;//²镒ưʼµؖ·
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
			
		//memcpy(ucTxBuffer,Rx_Buffer,size);									//测试用
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
		
		//¶Áʽ¾ݷň뻺³凸֐¡£
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
		
		//¶Áʽ¾ݷň뻺³凸֐¡£
		#if 1
		if(size>8)
		{
			for(i=0;i<size-8;i++)
			{
				g_rxbuf_UDP[s-4][g_wr_UDP[s-4]]=Rx_Buffer[8+i];
				g_wr_UDP[s-4]=(g_wr_UDP[s-4]+1)%MAXRecvBuf;
			}		
			g_UDP_usTick[s-4] = systickCount;									//最后接收的时间
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
  * @ 函数名  ： LED_Task
  * @ 功能说明： LED_Task任务主体
  * @ 参数    ：   
  * @ 返回值  ： 无
  ********************************************************************/
void EtherNet_Task(void* parameter)
{	
	 uint32_t r_event = 0;  									/* 定义一个事件接收变量 */
	 uint32_t last_event = 0;									/* 定义一个保存事件的变量 */
	 BaseType_t xReturn = pdTRUE;							/* 定义一个创建信息返回值，默认为pdPASS */
	
   while (1)
   {				
			if(g_configRead.b_rj45_work&0x01)		
			{	
				W5500_Socket_Set();									//W5500׋ࠚԵʼۯƤ׃	
				xReturn = xTaskNotifyWait(0x0,			//进入函数的时候不清除任务bit
                              ULONG_MAX,	  //退出函数的时候清除所有的bitR
                              &r_event,		  //保存任务通知值                    
                              10000);				//阻塞时间
				if( pdTRUE == xReturn )
				{ 
						last_event |= r_event;      						/* 如果接收完成并且正确 */
						//定义bit
						//bit0 				中断发生,查询状态，如果有数据则读出并写入对应缓冲中，根据配置采用事件通知方式告知其他线程。						
					  //bit1~bit8 	循环缓冲中有数据待发送,需要读数据并通过w5500发送出去。不关心数据是谁写入的。可能多个设备都向其中写入
					  //问题:	通道没有连接成功或没有初始化则不能向该通道中写入数据,环形缓冲最后可能满而丢失。					
						//bit31				w5500基本参数需要重新初始化， 
						//最终实现任意通道之间进行数据的透明传输。
					
						if(last_event&0x80000000)											//中断
						{
								last_event &= ~(1<<31);
								W5500_Interrupt_Process();								//W5500א׏ԦmԌѲ࠲ݜ					
								//if(((S0_Data & S_RECEIVE) == S_RECEIVE) || ((S1_Data & S_RECEIVE) == S_RECEIVE) || ((S2_Data & S_RECEIVE) == S_RECEIVE))	//ɧڻSocket0ޓ˕ս˽ߝ
								if((S_Data[0] & S_RECEIVE) == S_RECEIVE)
								{				
										S_Data[0]&=~S_RECEIVE;
										Process_Socket_Data(0);		
								}							
								#if RJ45_TCPSERVER_S1			
								if((S_Data[1] & S_RECEIVE) == S_RECEIVE)
								{				
									S_Data[1]&=~S_RECEIVE;
									Process_Socket_Data(1);											//W5500ޓ˕Ңע̍ޓ˕սք˽ߝ
								}
								#endif			
								if((S_Data[2] & S_RECEIVE) == S_RECEIVE)
								{				
									S_Data[2]&=~S_RECEIVE;
									Process_Socket_Data(2);			
								}		
								if((S_Data[3] & S_RECEIVE) == S_RECEIVE)			//收到数据
								{				
										S_Data[3]&=~S_RECEIVE;
										SnmpXDaemon();														//Process_Socket_Data(3);			
								}			
						}		
						if(last_event&0x100)				//通道参数发生变化具体那个通道不知道 通道1参数变化
						{
							last_event &= ~(1<<8);
						}
						if(last_event&0x200)				//通道参数发生变化具体那个通道不知道 通道2参数变化
						{
							last_event &= ~(1<<9);
						}
						
						if(last_event&0x01)					//通道1待发数据
						{
								Chan_Send(0);
								last_event &= ~(1<<0);
						}
						if(last_event&0x02)					//通道2待发数据
						{
								Chan_Send(1);
								last_event &= ~(1<<1);
						}
						if(last_event&0x04)					//通道3待发数据
						{
								Chan_Send(2);
								last_event &= ~(1<<2);
						}
				}	
				else		//超时，进行1次状态和中断等检查，防止中断漏掉
				{
						//printf("rj45 等待超时10s！\n");
				}
			}		
			else
			{					
					if(NULL!=EtherNet_Task_Handle)
						vTaskSuspend(EtherNet_Task_Handle);
					else
						vTaskDelay(10000); 									//10秒1次
			}
   }//while(1)
}

//在bsp_init中对初始化w5500公共部分,例如io口初始化等。重启等。
//初始化w5500相关内容,例如spi,中断等。

//1. 等待中断，如果规定时间内没有中断，则超时，手动检测状态。例如10秒。
//2. 接收到数据则转发到对应缓冲区，同时进行通知对应线程,目前测试com1主口
//3. 等待接收数据，有数据则发送到rj45.rj45数据来自其他口，例如com1的转发。
//4. 修改自己ip等信息（发送通知后,不需要立即切换，不紧急），目的让本任务重新设置自己的相关信息
//5. 在任务恢复的时候，需要检查自身参数是否已经变化过。
//6. 每个通讯口都有自己的 发送等待buf,转发的时候向其中写入数据。
//7. 每个通讯口都有自己的接收buf,接收完成后，将数据转发，


//整个过程独立运行。

//数据发送过程,main(rs485等)-->rj45发送数据,通过缓冲接收数据,接收到数据，进行数据的发送

//com(3 485,meter,4g,wifi) usb can数据通道之间 任意转换 合计 8个 通道
//rj45转换
//lora转换

//通讯
//显示
//存储
//采集
//控制