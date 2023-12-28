//=============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//=============================================================================
// Project   :  SHT3x Sample Code (V1.1)
// File      :  i2c_hal.c (V1.1)
// Author    :  RFU
// Date      :  6-Mai-2015
// Controller:  STM32F100RB
// IDE       :  �Vision V5.12.0.0
// Compiler  :  Armcc
// Brief     :  I2C hardware abstraction layer
//=============================================================================

//-- Includes -----------------------------------------------------------------
#include "i2c_hal.h"
#include "prj.h"
#include "gd32e503v_eval.h"
//#include "GlobalVar.h"
//#include "systick.h"


//-- Defines ------------------------------------------------------------------
// I2C IO-Pins                        /* -- adapt the defines for your uC -- */

#if 0
#define SDA_LOW()  (GPIOB->BSRR = 0x40000000) // set SDA to low
#define SDA_OPEN() (GPIOB->BSRR = 0x00004000) // set SDA to open-drain
#define SDA_READ   (GPIOB->IDR  & 0x4000)     // read SDA

// SCL on port B, bit 13  ->15            /* -- adapt the defines for your uC -- */
#define SCL_LOW()  (GPIOB->BSRR = 0x80000000) // set SCL to low
#define SCL_OPEN() (GPIOB->BSRR = 0x00008000) // set SCL to open-drain
#define SCL_READ   (GPIOB->IDR  & 0x8000)     // read SCL

#endif

//-- Static function prototypes -----------------------------------------------
static etError I2c_WaitWhileClockStreching(unsigned char timeout);

void DelayMicroSeconds(unsigned int nbrOfUs)   /* -- adapt this delay for your uC -- */
{ 
  unsigned int i=0,j=0;														//72MHz  测试说明  估计32个1us 
  for(i = 0; i < nbrOfUs; i++)			//8MHz   8个nop则频率 1MHz 即 1us   必须延时1us。否则后面的延时会变动。
  {  
		for(j=0;j<180;j++)			//72MHz为8MHz的9倍，本处采用5倍测试
		{
			;
		}	
  }
}

void SDA_LOW(void)
{
		//assert_param(IS_GPIO_ALL_PERIPH(Sht3x[g_sht3x_ch].GPIOx_Dat));
		//assert_param(IS_GPIO_PIN(Sht3x[g_sht3x_ch].GPIO_Pin_Dat));  
		gpio_bit_reset(GPIOA, GPIO_PIN_13);		//Sht3x[g_sht3x_ch].GPIOx_Dat->BSRR = Sht3x[g_sht3x_ch].GPIO_Pin_Dat<<16;
}

void SDA_OPEN(void)
{
		gpio_bit_set(GPIOA, GPIO_PIN_13);		//GPIO_SetBits(Sht3x[g_sht3x_ch].GPIOx_Dat, Sht3x[g_sht3x_ch].GPIO_Pin_Dat);
}

int SDA_READ(void)
{
		return gpio_input_bit_get(GPIOA,GPIO_PIN_13);	 //return GPIO_ReadInputDataBit(Sht3x[g_sht3x_ch].GPIOx_Dat, Sht3x[g_sht3x_ch].GPIO_Pin_Dat);	
}

void SCL_LOW(void)
{
//	assert_param(IS_GPIO_ALL_PERIPH(Sht3x[g_sht3x_ch].GPIOx_Clk));
//  assert_param(IS_GPIO_PIN(Sht3x[g_sht3x_ch].GPIO_Pin_Clk));  
  	gpio_bit_reset(GPIOA, GPIO_PIN_14);		//Sht3x[g_sht3x_ch].GPIOx_Clk->BSRR = Sht3x[g_sht3x_ch].GPIO_Pin_Clk<<16;
	//GPIO_ResetBits(Sht3x[i].GPIOx_Clk, Sht3x[i].GPIO_Pin_Clk);		
}

void SCL_OPEN(void)
{
		gpio_bit_set(GPIOA, GPIO_PIN_14);		//GPIO_SetBits(Sht3x[g_sht3x_ch].GPIOx_Clk, Sht3x[g_sht3x_ch].GPIO_Pin_Clk);			 
}

int SCL_READ(void)
{
		return gpio_input_bit_get(GPIOA,GPIO_PIN_14);		//GPIO_ReadInputDataBit(Sht3x[g_sht3x_ch].GPIOx_Clk, Sht3x[g_sht3x_ch].GPIO_Pin_Clk);	
}

void sht3x_Para_Init(void)
{

}

void sht3x_Init()
{		
		gpio_pin_remap_config(GPIO_SWJ_DISABLE_REMAP,ENABLE);							//PA13 = swdio   PA14 = swclk 都禁止为 jtag和sw功能 
		gpio_init(GPIOA, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ,GPIO_PIN_13|GPIO_PIN_14);		
			
		//gpio_bit_reset(GPIOA, GPIO_PIN_14);			
		gpio_bit_set(GPIOA, GPIO_PIN_13);		
		gpio_bit_set(GPIOA, GPIO_PIN_14);		
}


//-----------------------------------------------------------------------------
void I2c_Init()                      /* -- adapt the init for your uC -- */
{
	sht3x_Init();	
} 

//-----------------------------------------------------------------------------
void I2c_StartCondition(void)
{
  SDA_OPEN();
  DelayMicroSeconds(8);			
  SCL_OPEN();
  DelayMicroSeconds(8);			
  SDA_LOW();
  DelayMicroSeconds(8);  		// hold time start condition (t_HD;STA)
  SCL_LOW();
  DelayMicroSeconds(8);
}

//-----------------------------------------------------------------------------
void I2c_StopCondition(void)
{
  SCL_LOW();
  DelayMicroSeconds(8);			//1->10
  SDA_LOW();
  DelayMicroSeconds(8);			//1->10
  SCL_OPEN();
  DelayMicroSeconds(8);  	// set-up time stop condition (t_SU;STO)
  SDA_OPEN();
  DelayMicroSeconds(8);
}

//-----------------------------------------------------------------------------
etError I2c_WriteByte(unsigned char txByte)
{
	unsigned short int i = 0;
  etError error = NO_ERROR;
  unsigned char     mask;
  for(mask = 0x80; mask > 0; mask >>= 1)// shift bit for masking (8 times)
  {
    if((mask & txByte) == 0) SDA_LOW(); // masking txByte, write bit to SDA-Line
    else                     SDA_OPEN();
    DelayMicroSeconds(10);               // data set-up time (t_SU;DAT)1->5  足够的时间进行数据保持。
    SCL_OPEN();                         // generate clock pulse on SCL
    DelayMicroSeconds(10);               // SCL high time (t_HIGH)		5->10
    SCL_LOW();
    DelayMicroSeconds(1);               // data hold time(t_HD;DAT)
  }
  SDA_OPEN();                           // release SDA-line
  SCL_OPEN();                           // clk #9 for ack
	DelayMicroSeconds(5);                 // data set-up time (t_SU;DAT)
	for(i=0;i<20;i++)						//大概100=600us   20=150us
	{
		if(SDA_READ())
		{		
			error = ACK_ERROR;       // check ack from i2c slave
		}
		else
		{
			error = NO_ERROR;
			break;
		}
		DelayMicroSeconds(1);                 // data set-up time (t_SU;DAT)
	}
  
  SCL_LOW();
  DelayMicroSeconds(10);                // wait to see byte package on scope
  return error;                         // return error code
}

//-----------------------------------------------------------------------------
etError I2c_ReadByte(unsigned char *rxByte, etI2cAck ack, unsigned char timeout)
{
  etError error = NO_ERROR;
  unsigned char mask;
  *rxByte = 0x00;
  SDA_OPEN();                            					// release SDA-line
  for(mask = 0x80; mask > 0; mask >>= 1) 					// shift bit for masking (8 times)
  { 
    SCL_OPEN();                         		 			// start clock on SCL-line
    DelayMicroSeconds(4);               					 // clock set-up time (t_SU;CLK)
    error = I2c_WaitWhileClockStreching(timeout);	// wait while clock streching
    DelayMicroSeconds(5);                					// SCL high time (t_HIGH)
    if(SDA_READ()) *rxByte |= mask;        					// read bit
    SCL_LOW();
    DelayMicroSeconds(3);                // data hold time(t_HD;DAT)
  }
  if(ack == ACK) SDA_LOW();              // send acknowledge if necessary
  else           SDA_OPEN();
  DelayMicroSeconds(10);                  // data set-up time (t_SU;DAT)
  SCL_OPEN();                            // clk #9 for ack
  DelayMicroSeconds(5);                  // SCL high time (t_HIGH)
  SCL_LOW();
  SDA_OPEN();                            // release SDA-line
  DelayMicroSeconds(10);                 // wait to see byte package on scope  
  return error;                          // return with no error
}

//-----------------------------------------------------------------------------
etError I2c_GeneralCallReset(void)
{
  etError error;
  
  I2c_StartCondition();
                        error = I2c_WriteByte(0x00);
  if(error == NO_ERROR) error = I2c_WriteByte(0x06);
  
  return error;
}

  unsigned char  abc = 0;
//-----------------------------------------------------------------------------
static etError I2c_WaitWhileClockStreching(unsigned char timeout_)			//68=3ms
{
  etError error = NO_ERROR;

  while(SCL_READ() == 0)
  {
    if(timeout_-- == 0) 
		{
			abc = 0;
			return TIMEOUT_ERROR;
		}
    DelayMicroSeconds(21);												//20=100us
  }
  abc = timeout_;
  return error;
}
