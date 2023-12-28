#ifndef __ads1247_H
#define	__ads1247_H

#include "gd32e50x.h"
//---------- SPI命令 -----------------------------------------------------------------------------------------
#define D_WAKEUP				0x00//Exit sleep mode:唤醒
#define D_SLEEP					0x02//Enter sleep mode:休眠
#define D_SYNC					0x04//Synchronize the A/D conversion:同步AD转换
#define D_RESET					0x06//Reset to power-up values:复位
#define D_NOP						0xFF//No operation:空指令
#define D_RDATA					0x12//Read data once:读一次数据
#define D_RDATAC				0x14//Read data continously:连续读数据
#define D_SDATAC				0x16//Stop reading data continuously:停止连续读数据
#define D_RREG					0x20//Read from register rrrr, 0010 rrrr(2xh):读寄存器
#define D_WREG					0x40//Write to register rrrr, 0100 rrrr(4xh):写寄存器

//system control register 0 (0ffset=03h)[reset==00h]  reversed[7]=0  PGA[6:4]  data output rate DR[3:0]
#define ADS1247_PGA_1			0x00 
#define ADS1247_PGA_2			0x10 
#define ADS1247_PGA_4			0x20 
#define ADS1247_PGA_8			0x30 
#define ADS1247_PGA_16		0x40 
#define ADS1247_PGA_32		0x50 
#define ADS1247_PGA_64		0x60 
#define ADS1247_PGA_128		0x70 

#define ADS1247_DARATE_5		0x00 
#define ADS1247_DARATE_10		0x01 
#define ADS1247_DARATE_20		0x02 
#define ADS1247_DARATE_40		0x03 
#define ADS1247_DARATE_80		0x04 
#define ADS1247_DARATE_160	0x05 
#define ADS1247_DARATE_320	0x06 
#define ADS1247_DARATE_640	0x07 
#define ADS1247_DARATE_1K		0x08 
#define ADS1247_DARATE_2K		0x09 

//---------- 寄存器地址 -------------------------------------------------------------------------------------
#define D_Reg_MUX0			0x00//Multiplexer Control Register 0
#define D_Reg_VBias			0x01//Bias Voltage Register
#define D_Reg_MUX1			0x02//Multiplexer Control Register 1
#define D_Reg_SYS0			0x03//System Control Register 0
#define D_Reg_CFC0			0x04
#define D_Reg_CFC1			0x05
#define D_Reg_CFC2			0x06
#define D_Reg_FSC0			0x07
#define D_Reg_FSC1			0x08
#define D_Reg_FSC2			0x09
#define D_Reg_IDAC0			0x0A//IDAC Control Register 0
#define D_Reg_IDAC1			0x0B
#define D_Reg_GPIOCFG		0x0C
#define D_Reg_GPIODIR		0x0D
#define D_Reg_GPIODAT		0x0E

#define D_AD_REF				2.047998
#define D_AD_LSB        D_AD_REF/262143.0

#define D_HIGH					1
#define D_LOW						0

#define D_AI_CH_0				0
#define D_AI_CH_1				1
#define D_AI_CH_2				2
#define D_AI_CH_3				3
#define D_AI_CH_4				4
#define D_AI_CH_5				5
#define D_AI_CH_6				6
#define D_AI_CH_7				7

void ADS1247_IO_Init(void);	
void ADS1247_Init(void);
void Read_Ads1247_Mode(unsigned char mode);
void Read_AI_Data(unsigned char ch);
	
#endif
