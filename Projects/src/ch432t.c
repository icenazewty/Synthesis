#include "ch432t.h"
#include "GlobalVar.h"
#include "string.h"
#include "systick.h"
#include "gd32e50x_spi.h"
#include "ads1247.h"
void ch432t_CS(unsigned char Para_ucState)
{
	if(1==Para_ucState)
	{
		gpio_bit_set(GPIOC,GPIO_PIN_15);
	}
	else
	{
		gpio_bit_reset(GPIOC,GPIO_PIN_15);
	}
}

void delay_us2(int _us)
{
	volatile unsigned int _dcnt;
	_dcnt=(_us*60);
	while(_dcnt-->0)
	{
		continue;
	}
}

unsigned char g_dat;
void WriteCH432Data(unsigned char addr,unsigned char byte)
{	
	ch432t_CS(0);	
	while(spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET) 	;
	
  /* Send byte through the SPI1 peripheral */
  spi_i2s_data_transmit(SPI2, ( addr<<2 ) | 0x02 );	

  /* Wait to receive a byte */
	while(spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET) 	;

  /* Return the byte read from the SPI bus */
  spi_i2s_data_transmit(SPI2, byte );		
	delay_us2(2);									//必须增加TBE，TBE的标志表示发送了第1位则可以进行写入，并不表示写完了。如果调整了sck的频率则需要适当修改延时。否则cs会提前结束  最高频率 13MHz
	ch432t_CS(1);
	//delay_us2(1);
}

void WriteCH432Block( unsigned char mAddr, unsigned char mLen, unsigned char* mBuf )    /* 向指定起始地址写入数据块 */
{
	int i = 0; 
	ch432t_CS(0);
	while(spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET) ;
  /* Send byte through the SPI1 peripheral */
  spi_i2s_data_transmit(SPI2, ( mAddr<<2 ) | 0x02 );	

  /* Wait to receive a byte */
  while(spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET) ;
	
  /* Return the byte read from the SPI bus */
	for(i=0;i<mLen;i++)
	{
		spi_i2s_data_transmit(SPI2, *mBuf++);	
		while(spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET) ;		
	}	
	delay_us2(2);			//必须增加TBE，TBE的标志表示发送了第1位则可以进行写入，并不表示写完了。如果调整了sck的频率则需要适当修改延时。否则cs会提前结束  最高频率 13MHz
	ch432t_CS(1);
	//delay_us2(1);
}

unsigned char ReadCH432Data(unsigned char byte)
{
	unsigned char  dat = 0;
	ch432t_CS(0);	
	while(spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET) 	;

  /* Send byte through the SPI1 peripheral */
  spi_i2s_data_transmit(SPI2, ( byte<<2 ) &0xFD )	;	

  /* Wait to receive a byte */
  while(spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE) == RESET) ;
	dat = spi_i2s_data_receive(SPI2);	
	
	while(spi_i2s_flag_get(SPI2, SPI_FLAG_TBE) == RESET) 	;	
  /* Return the byte read from the SPI bus */
	spi_i2s_data_transmit(SPI2,0xff);
	while(spi_i2s_flag_get(SPI2, SPI_FLAG_RBNE) == RESET) ;		//当开始发送最后一字节，适当延时读数据则时序正确，如果RB则可能存在是上次读的数据。	
	dat = spi_i2s_data_receive(SPI2);	
	delay_us2(1);																							//必须增加TBE，TBE的标志表示发送了第1位则可以进行写入，并不表示写完了。如果调整了sck的频率则需要适当修改延时。否则cs会提前结束  最高频率 13MHz
  dat = spi_i2s_data_receive(SPI2);	
	ch432t_CS(1);
	//delay_us2(1);
	return dat;
}


#define CH432_BPS   9600   				/* 定义CH432串口0通讯波特率 */
#define CH432_BPS1  9600   				/* 定义CH432串口1通讯波特率 */
#define Fpclk    		1843200   		/* 定义内部时钟频率  外部22.1184MHz CK2X=0，内部1/12分频  最大波特率 115200 */


#define addr    0x0000    /* 根据硬件地址修改 */
/**************************************************************************

        定义CH432串口0的寄存器地址

**************************************************************************/
#define CH432_RBR_PORT    ( addr + REG_RBR_ADDR )     /* 假定CH432接收缓冲寄存器0的I/O地址 */
#define CH432_THR_PORT    ( addr + REG_THR_ADDR )     /* 假定CH432发送保持寄存器0的I/O地址 */
#define CH432_IER_PORT    ( addr + REG_IER_ADDR )     /* 假定CH432中断使能寄存器0的I/O地址*/
#define CH432_IIR_PORT    ( addr + REG_IIR_ADDR )     /* 假定CH432中断识别寄存器0的I/O地址 */
#define CH432_FCR_PORT    ( addr + REG_FCR_ADDR )     /* 假定CH432FIFO控制寄存器0的I/O地址 */
#define CH432_LCR_PORT    ( addr + REG_LCR_ADDR )     /* 假定CH432线路控制寄存器0的I/O地址 */
#define CH432_MCR_PORT    ( addr + REG_MCR_ADDR )     /* 假定CH432MODEM控制寄存器0的I/O地址 */
#define CH432_LSR_PORT    ( addr + REG_LSR_ADDR )     /* 假定CH432线路状态寄存器0的I/O地址 */
#define CH432_MSR_PORT    ( addr + REG_MSR_ADDR )     /* 假定CH432MODEM状态寄存器0的I/O地址 */
#define CH432_SCR_PORT    ( addr + REG_SCR_ADDR )     /* 假定CH432用户可定义寄存器0的I/O地址 */
#define CH432_DLL_PORT    ( addr + REG_DLL_ADDR )     /* 假定CH432波特率除数锁存器0低8位I/O地址 */
#define CH432_DLM_PORT    ( addr + REG_DLM_ADDR )     /* 假定CH432波特率除数锁存器0高8位I/O地址 */
/**************************************************************************
        定义CH432串口1的寄存器地址

**************************************************************************/
#define CH432_RBR1_PORT   ( addr + REG_RBR1_ADDR )    /* 假定CH432接收缓冲寄存器1的I/O地址 */
#define CH432_THR1_PORT   ( addr + REG_THR1_ADDR )    /* 假定CH432发送保持寄存器1的I/O地址 */
#define CH432_IER1_PORT   ( addr + REG_IER1_ADDR )    /* 假定CH432中断使能寄存器1的I/O地址 */
#define CH432_IIR1_PORT   ( addr + REG_IIR1_ADDR )    /* 假定CH432中断识别寄存器1的I/O地址 */
#define CH432_FCR1_PORT   ( addr + REG_FCR1_ADDR )    /* 假定CH432FIFO控制寄存器1的I/O地址 */
#define CH432_LCR1_PORT   ( addr + REG_LCR1_ADDR )    /* 假定CH432线路控制寄存器1的I/O地址 */
#define CH432_MCR1_PORT   ( addr + REG_MCR1_ADDR )    /* 假定CH432MODEM控制寄存器1的I/O地址 */
#define CH432_LSR1_PORT   ( addr + REG_LSR1_ADDR )    /* 假定CH432线路状态寄存器1的I/O地址 */
#define CH432_MSR1_PORT   ( addr + REG_MSR1_ADDR )    /* 假定CH432MODEM状态寄存器1的I/O地址 */
#define CH432_SCR1_PORT   ( addr + REG_SCR1_ADDR )    /* 假定CH432用户可定义寄存器1的I/O地址 */
#define CH432_DLL1_PORT   ( addr + REG_DLL1_ADDR )    /* 假定CH432波特率除数锁存器1低8位I/O地址 */   
#define CH432_DLM1_PORT   ( addr + REG_DLM1_ADDR )    /* 假定CH432波特率除数锁存器1高8位I/O地址 */



//unsigned char 	SEND_STRING[ ] = { "444444555566666777788889999000009999995555555hello" };
//unsigned char 	SEND_STRING1[ ] = { "33333333wwwwww" };
//unsigned char  	buf[ 100 ];    			/* 缓冲区保存数据 */
//unsigned char  	rcvbuf[ 64 ];   		/* 缓冲区保存数据 */

void InitCH432( void )    /* 初始化CH432 */
{
    unsigned short int div;
    unsigned char  DLL, DLM;
/**************************************************************************
          设置CH432串口0的寄存器

**************************************************************************/
		WriteCH432Data( CH432_SCR_PORT, 0x39 );   
		DLL = ReadCH432Data(CH432_SCR_PORT);
	
		WriteCH432Data( CH432_LCR_PORT, BIT_LCR_WORDSZ1 | BIT_LCR_WORDSZ0 );   	/* 字长8位，1位停止位、无校验 DLAB=0*/
		DLL = ReadCH432Data(CH432_LCR_PORT);
	
		WriteCH432Data( CH432_IER_PORT, BIT_IER_RESET  );   									 	/* 串口0 reset */
		DLL = ReadCH432Data(CH432_IER_PORT);
		
	if(gs_SaveNetIPCfg.baud2[1]>115200)															//22.1184MHz / 12 = 1.8432MHz  最大115.2Kbps  否则设备死机
			div = ( Fpclk >> 4 ) / 115200;												
		else
			div = ( Fpclk >> 4 ) / gs_SaveNetIPCfg.baud2[1];						//CH432_BPS-->gs_SaveNetIPCfg.baud2[1]  com7
    DLM = div >> 8;
    DLL = div & 0xff;
    WriteCH432Data( CH432_LCR_PORT, BIT_LCR_DLAB );    																				/* 设置DLAB为1 */
		WriteCH432Data( CH432_DLL_PORT, DLL );   					 																				/* 设置波特率 */
    WriteCH432Data( CH432_DLM_PORT, DLM );
		DLM = ReadCH432Data(CH432_DLM_PORT);		
		DLL = ReadCH432Data(CH432_DLL_PORT);				//0x80 为何不对
		DLL = ReadCH432Data(CH432_LCR_PORT);
		
		WriteCH432Data( CH432_LCR_PORT, BIT_LCR_WORDSZ1 | BIT_LCR_WORDSZ0 );   	/* 字长8位，1位停止位、无校验 DLAB=0*/
		DLL = ReadCH432Data(CH432_LCR_PORT);
		
    WriteCH432Data( CH432_FCR_PORT, BIT_FCR_RECVTG0 | BIT_FCR_RECVTG1 | BIT_FCR_FIFOEN );    	/* 设置FIFO模式， 触发点为14   接收数据个数   | BIT_FCR_TFIFORST | BIT_FCR_RFIFORST  */
    DLL = ReadCH432Data(CH432_FCR_PORT);
		
		DLL = ReadCH432Data(CH432_MCR_PORT);
		
    WriteCH432Data( CH432_IER_PORT, BIT_IER_IERECV );    									/* 使能中断  BIT_IER_IETHRE  9600字节  16ms  */  
		DLL = ReadCH432Data(CH432_IER_PORT);
		
    WriteCH432Data( CH432_MCR_PORT, BIT_MCR_OUT2  );  										/* 允许中断输出,DTR,RTS为1 */
		DLL = ReadCH432Data(CH432_MCR_PORT);

/**************************************************************************
          设置CH432串口1的寄存器

**************************************************************************/
		WriteCH432Data( CH432_SCR1_PORT, 0x39 );   
		DLL = ReadCH432Data(CH432_SCR1_PORT);
		
		WriteCH432Data( CH432_LCR1_PORT, BIT_LCR_WORDSZ1 | BIT_LCR_WORDSZ0 );   	/* 字长8位，1位停止位、无校验 DLAB=0*/
		DLL = ReadCH432Data(CH432_LCR1_PORT);
	
		WriteCH432Data( CH432_IER1_PORT, BIT_IER_RESET  );   									 		/* 串口0 reset */
		DLL = ReadCH432Data(CH432_IER1_PORT);
		
		if(gs_SaveNetIPCfg.baud2[2]>115200)																//22.1184MHz / 12 = 1.8432MHz  最大115.2Kbps
			div = ( Fpclk >> 4 ) / 115200;																	
		else
			div = ( Fpclk >> 4 ) / gs_SaveNetIPCfg.baud2[2];								//CH432_BPS1-->gs_SaveNetIPCfg.baud2[2]  com8
		
    
    DLM = div >> 8;
    DLL = div & 0xff;
		
    WriteCH432Data( CH432_LCR1_PORT, BIT_LCR_DLAB );    																				/* 设置DLAB为1 */
    WriteCH432Data( CH432_DLL1_PORT, DLL );   					 																				/* 设置波特率 */
    WriteCH432Data( CH432_DLM1_PORT, DLM );
		DLM = ReadCH432Data(CH432_DLM1_PORT);		
		DLL = ReadCH432Data(CH432_DLL1_PORT);
		DLL = ReadCH432Data(CH432_LCR1_PORT);
		
		WriteCH432Data( CH432_LCR1_PORT, BIT_LCR_WORDSZ1 | BIT_LCR_WORDSZ0 );  	/* 字长8位，1位停止位、无校验 DLAB=0*/
		DLL = ReadCH432Data(CH432_LCR1_PORT);
		
		WriteCH432Data( CH432_FCR1_PORT,   BIT_FCR_RECVTG0 | BIT_FCR_RECVTG1 | BIT_FCR_FIFOEN );    											/* 设置FIFO模式，触发点为14 | BIT_FCR_TFIFORST | BIT_FCR_RFIFORST*/
		DLL = ReadCH432Data(CH432_FCR1_PORT);
		
		DLL = ReadCH432Data(CH432_IER1_PORT);
		
		WriteCH432Data( CH432_IER1_PORT,  BIT_IER_IERECV );    									/* 使能中断 BIT_IER_IETHRE*/
		DLL = ReadCH432Data(CH432_IER1_PORT);   
        
    WriteCH432Data( CH432_MCR1_PORT, BIT_MCR_OUT2);   											/* 允许中断输出，DTR,RTS为1 */
		DLL = ReadCH432Data(CH432_IER1_PORT);
}


void UART0_SendByte( unsigned char dat )    /* CH432串口0发送一字节子程序 */
{
    while( ( ReadCH432Data( CH432_LSR_PORT ) & BIT_LSR_THRE ) == 0 );    /* 等待数据发送完毕 */
    WriteCH432Data( CH432_THR_PORT, dat );
}

unsigned char UART0_RcvByte()    /* CH432串口0接收一字节子程序 */
{
    unsigned char Rcvdat;
    if( !( ReadCH432Data( CH432_LSR_PORT ) & ( BIT_LSR_BREAKINT | BIT_LSR_FRAMEERR | BIT_LSR_PARERR | BIT_LSR_OVERR ) ) )    /*  b1-b4无错误 */
    {
        while( ( ReadCH432Data( CH432_LSR_PORT ) & BIT_LSR_DATARDY ) == 0 )	;    /* 等待数据准备好 */
        Rcvdat = ReadCH432Data( CH432_RBR_PORT );    /* 从接收缓冲寄存器读出数据 */
        return( Rcvdat );
    }
    else 
		{
			return ReadCH432Data( CH432_RBR_PORT );    /* 有错误清除 */
		}
}

void UART0_SendStr( unsigned char *str )    /* CH432串口0发送一字符串子程序 */
{
    while( 1 )
    {
        if( *str == '\0' ) break;
        UART0_SendByte( *str++ );
    }
}

void UART1_SendByte( unsigned char dat )    /* CH432串口1发送一字节子程序 */
{
    while( ( ReadCH432Data( CH432_LSR1_PORT ) & BIT_LSR_THRE ) == 0 );    /* 等待数据发送完毕 */
    WriteCH432Data( CH432_THR1_PORT, dat );
}

unsigned char UART1_RcvByte()    /* CH432串口1接收一字节子程序 */
{
    unsigned char Rcvdat;
    if( !( ReadCH432Data( CH432_LSR1_PORT ) & (BIT_LSR_BREAKINT | BIT_LSR_FRAMEERR | BIT_LSR_PARERR | BIT_LSR_OVERR ) ) )    /* b1-b4无错误 */
    {
        while( ( ReadCH432Data( CH432_LSR1_PORT ) & BIT_LSR_DATARDY ) == 0 );    /* 等待数据准备好 */
        Rcvdat = ReadCH432Data( CH432_RBR1_PORT );    /* 从接收缓冲寄存器读出数据 */
        return( Rcvdat );
    }
    else 
		{
			return ReadCH432Data( CH432_RBR1_PORT );  				  /* b1-b4有错误清除 */
		}
}

void  UART1_SendStr( unsigned char *str )    /* CH432串口1发送一字符串子程序 */
{
    while(1)
    {
        if( *str == '\0' ) break;
        UART1_SendByte( *str++ );
    }
}

void  CH432Seril0Send( unsigned char *Data, unsigned char Num )    /* 禁用FIFO,CH432串口0发送多字节子程序 */
{
    if(Num>0)
		{
			do
			{
        while( ( ReadCH432Data( CH432_LSR_PORT ) & BIT_LSR_THRE ) == 0 );    /* 等待数据发送完毕 */
        WriteCH432Data( CH432_THR_PORT, *Data++ );
			}
			while( --Num );
		}
}

void  CH432Seril1Send( unsigned char *Data, unsigned char Num )    /* 禁用FIFO,CH432串口1发送多字节子程序 */
{
		if(Num>0)
		{
			do
			{
        while( ( ReadCH432Data( CH432_LSR1_PORT ) & BIT_LSR_THRE ) == 0 );    /* 等待数据发送完毕 */
        WriteCH432Data( CH432_THR1_PORT, *Data++ );
			}
			while( --Num );
		}
}

unsigned char  CH432SerilRcv( unsigned char Chan )    /* 禁用FIFO,CH432串口0接收多字节子程序 */
{
		unsigned char com_port = Chan + 7;
    unsigned char RcvNum = ReadCH432Data( CH432_LSR_PORT + Chan*8);
    if( !(RcvNum  & ( BIT_LSR_BREAKINT | BIT_LSR_FRAMEERR | BIT_LSR_PARERR | BIT_LSR_OVERR ) ) )    /* b1-b4无错误 */
    {
				RcvNum = 0;
        while( ( ReadCH432Data( CH432_LSR_PORT + Chan*8 ) & BIT_LSR_DATARDY ) == 0 );    															/* 等待数据准备好 */
        do
        {						
						Com.Usart[com_port].RX_Buf[Com.Usart[com_port].usRec_WR] = ReadCH432Data( CH432_RBR_PORT + Chan*8  );    	/* 从接收缓冲寄存器读出数据 */			
						Com.Usart[com_port].usRec_WR=(Com.Usart[com_port].usRec_WR+1)% D_USART_REC_BUFF_SIZE;						           
            RcvNum++;																																																	//调试用
        }
        while( ( ReadCH432Data( CH432_LSR_PORT + Chan*8  ) & BIT_LSR_DATARDY ) == 0x01 );
    }
    else 
		{			
			RcvNum = ReadCH432Data( CH432_RBR_PORT + Chan*8);
			RcvNum = 0;
		}
    return( RcvNum );
}

//Chan为通道,0=表示1通道，1=表示2通道
void  CH432UARTSend(unsigned char Chan, unsigned char *Data, unsigned short Num )    /* 启用FIFO,一次最多16字节，CH432串口0发送多字节子程序 */
{
    while( 1 )
    {
        while( ( ReadCH432Data( CH432_LSR_PORT + Chan*8) & BIT_LSR_TEMT ) == 0 );    /* 等待数据发送完毕，THR,TSR全空 */
        if( Num <= 16 )
        {
            WriteCH432Block( CH432_THR_PORT + Chan*8, Num, Data );
            break;
        }
        else
        {
            WriteCH432Block( CH432_THR_PORT + Chan*8, 16, Data );
            Num -= 16;
            Data += 16;
        }
    }
}


void CH432_Int_Process(void)
{
			unsigned char InterruptStatus;
			unsigned char RcvNum = 0;
			//unsigned char ModemStatus;
			InterruptStatus = ReadCH432Data( CH432_IIR_PORT ) & ( ~ CH432_IIR_FIFOS_ENABLED );
			if( 0x01 != InterruptStatus )    /* 没有中断转到串口1 */
			{
        switch( InterruptStatus )
        {              		
							case INT_RCV_SUCCESS:    						/* 串口接收可用数据中断 */
										RcvNum = CH432SerilRcv(0);
										if(RcvNum>0)
											Com.Usart[7].usTick = systickCount;																											//最后接收的时间
                    //RcvNum = CH432Seril0Rcv( buf );
                    //UART0_SendByte( RcvNum );
                    //CH432Seril0Send( buf, RcvNum );
                    break;            
							case INT_RCV_OVERTIME:    					/* 接收数据超时中断 */
										RcvNum = CH432SerilRcv(0);
										if(RcvNum>0)
											Com.Usart[7].usTick = systickCount;																											//最后接收的时间
                    //RcvNum = CH432Seril0Rcv( buf );
                    //UART0_SendByte( RcvNum );
                    //CH432Seril0Send( buf, RcvNum );
                    break;
							case INT_THR_EMPTY:    							/* 发送保持寄存器空中断 */
                    //UART0_SendStr( SEND_STRING );
                    break;
							case INT_RCV_LINES:    							/* 接收线路状态中断 */
                    ReadCH432Data( CH432_LSR_PORT);
                    break;
							case INT_MODEM_CHANGE:							/* MODEM输入变化中断 */
										break;
							case INT_NOINT:    									/* 没有中断 */
										break;
							default:    												/* 不可能发生的中断 */
                    break;
					}
				}
			
				InterruptStatus = ReadCH432Data( CH432_IIR1_PORT ) & ( ~ CH432_IIR_FIFOS_ENABLED );    /* 读串口1的中断状态 */
        if( 0x01 != InterruptStatus ) 			
        {					
            switch( InterruptStatus )
            {							
                case INT_RCV_SUCCESS:    						/* 串口接收可用数据中断 */
										RcvNum = CH432SerilRcv(1);
										if(RcvNum>0)
											Com.Usart[8].usTick = systickCount;																											//最后接收的时间
                    //RcvNum = CH432Seril1Rcv( buf );
                    //CH432Seril1Send( buf, RcvNum );
                    break;                
                case INT_RCV_OVERTIME:   		 				/* 接收数据超时中断 */
										RcvNum = CH432SerilRcv(1);
										if(RcvNum>0)
											Com.Usart[8].usTick = systickCount;																											//最后接收的时间
                    //RcvNum = CH432Seril1Rcv( buf );
                    //CH432Seril1Send( buf, RcvNum );
                    break;
								case INT_THR_EMPTY:    							/* 发送保持寄存器空中断 THR空中断*/
                    //UART1_SendStr( SEND_STRING1);
                    break;
								case INT_RCV_LINES:    							/* 接收线路状态中断 */
                    ReadCH432Data( CH432_LSR1_PORT);
                    break;
								case INT_MODEM_CHANGE:							/* MODEM输入变化中断 */
									break;
                case INT_NOINT:    									/* 没有中断 */
                    break;
                default:    												/* 不可能发生的中断 */
                    break;
            }
        }				
}
//unsigned char uu8[16];
void ch432t_IO_Init(void)	
{
		int i = 0;
		spi_parameter_struct spi_init_struct;
   
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_SPI2);
		rcu_periph_clock_enable(RCU_AF);

		gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);	
	
		gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
    /* SPI1_SCK(PB15), SPI1_MISO(PB13) and SPI1_MOSI(PB14) GPIO pin configuration */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5); 
		gpio_pin_remap_config(GPIO_SPI2_REMAP,DISABLE);									//SPI2_REMAP = 0  则pb3 4 5作为spi2
	
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
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;		//SPI_CK_PL_LOW_PH_2EDGE;			
		//SPI_CK_PL_LOW_PH_2EDGE	  不行  默认sck低电平  mosi数据在sck高电平时刻发送
		//SPI_CK_PL_LOW_PH_1EDGE;	  00模式
		//SPI_CK_PL_HIGH_PH_2EDGE;	11模式 不行  默认sck高电平  
		//SPI_CK_PL_HIGH_PH_1EDGE;	不行  默认sck高电平  PL=pulse   PH=phase
    spi_init_struct.nss                  = SPI_NSS_SOFT;
    spi_init_struct.prescale             = SPI_PSC_32;								//最小SPI_PSC_16 ;			168/16=10M  100ns    5M
    spi_init_struct.endian               = SPI_ENDIAN_MSB;
    spi_init(SPI2, &spi_init_struct);
		
		//spi_nss_output_disable(SPI1);
		
    /* set crc polynomial */
    //spi_crc_polynomial_set(SPI1,7);
    /* enable SPI0 */
    spi_enable(SPI2);		
		
}
//数据发送说明
//如果不处于中断状态则进行查询，发送第1包数据，发送完成后，进入发送中断，继续发送。直到全部发送完成。

void ch432t_Init(void)
{
		ch432t_IO_Init();
//	for(i=0;i<16;i++)	
//		{
//				//uu8[i] = ReadCH432Data(i);
//		}
		InitCH432();
		//Com.Usart[7].usDelay_Rev = 8;	
		//Com.Usart[8].usDelay_Rev = 8;	
//		while(1)											//读测试
//		{
//			
//			i = gpio_input_bit_get(D_ANDROID_POWER_PORT,D_ANDROID_POWER_CTRL); 		//低电平打开电源,默认上拉
//			if(0==i)
//			{
//					CH432_Int_Process();
//			}
//			delay_us2(10000);
//			delay_us2(10000);
//			UART0_SendByte( 0xAA );
//			UART1_SendByte( 0xBB );
//			delay_us2(10000);
//			delay_us2(10000);
//			CH432UARTSend( 0,SEND_STRING, 50 );
//			CH432UARTSend(1, SEND_STRING, 50 );
//		}
}

/*!
    \brief      this function handles external lines 5 to 9 interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/

//unsigned int  g_ads1247_tick[128];
//int					  g_ads1247_cnt  = 0;	

int					  Ads1247_Ch = 0;

int						g_calib_mode = 0;
int						g_calib_ch = 0;

extern int	 	g_calc_data[10];
extern int	 	g_calc_cnt;
extern int    uiADValue;
unsigned char g_ads1247_flag = 0;
void EXTI5_9_IRQHandler(void)
{		
		if(RESET != exti_interrupt_flag_get(EXTI_8))			//ads1247  pb8
		{   
				Irq_Ads1247_Ready = 1;
				//g_ads1247_tick[g_ads1247_cnt] = systickCount;
				//g_ads1247_cnt = (g_ads1247_cnt+1)%128;				
        exti_interrupt_flag_clear(EXTI_8);
				if(g_calib_mode)
				{
						if(g_calc_cnt<0)
						{
							AI_Channel_Select(g_calib_ch);			//通道切换，并且扔掉1个通道的数据
							g_calc_cnt++;
						}
						else if(g_calc_cnt<10)
						{							
							//Read_Ads1247_Mode(D_RDATA);					//启动单次读									
							Read_AI_Data(g_calib_ch);						//读上次的数据 Ads1247_Ch_Pre		
							g_calc_data[g_calc_cnt]=uiADValue;							
							g_calc_cnt++;
							if(g_calc_cnt>9)
							{
									AI_Channel_Select(Ads1247_Ch);			//AI_Channel_Select(Ads1247_Ch);
									g_calib_mode = 0;									
							}
						}		
						else
						{
								g_calc_cnt = 10;
								AI_Channel_Select(Ads1247_Ch);			//AI_Channel_Select(Ads1247_Ch);
								g_calib_mode = 0;
						}
				}
				else
				{
					if(0==g_ads1247_flag)
					{
						AI_Channel_Select(Ads1247_Ch);			//AI_Channel_Select(Ads1247_Ch);
						//Read_Ads1247_Mode(D_RDATA);					//启动单次读									
						//ReadADSConversionData();
						g_ads1247_flag++;
					}
					else if(1==g_ads1247_flag)
					{
						//Read_Ads1247_Mode(D_RDATA);					//启动单次读									
						//ReadADSConversionData();
						g_ads1247_flag++;
					}
					else
					{
						g_ads1247_flag = 0;
						//Read_Ads1247_Mode(D_RDATA);					//启动单次读									
						Read_AI_Data(Ads1247_Ch);						//读上次的数据 Ads1247_Ch_Pre			
//						if(1==Ads1247_Ch)
//						{
//							ADT(1);
//						}
						Ads1247_Ch = (Ads1247_Ch+1)%8;			//切换下一个通道					
						AI_Channel_Select(Ads1247_Ch);			//AI_Channel_Select(Ads1247_Ch);
					}					
				}
				//Read_AI_Data(Ads1247_Ch_Pre);			//读上次的数据 Ads1247_Ch_Pre
				//wdt();
				//Ads1247_Cnt++;										//合计采集的次数
				//Read_Ads1247_Mode(D_RDATA);				//启动单次读							
    }
		
		if(RESET != exti_interrupt_flag_get(EXTI_9))			//ch432T pe9
		{   
				//Irq_Ads1247_Ready = 1;											//读数据				
				CH432_Int_Process();
        exti_interrupt_flag_clear(EXTI_9);
    }
}





