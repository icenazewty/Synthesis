//#define D_IWDG_EN						开启内部看门狗则必须启动timer2定时器	 Timer2_Init_Config  
//#define D_50HZ

#define		BOOT_NO				0				//如果从启动代码启动，则该为设置为0，如果直接启动则为1
#define		RS485_NO_AUTO	0				//如果rs485不自动切换则为1,自动切换为0


//----------- ADS1247 ---------------------------------------------------------------------------------------
//#define D_AD_START                      GPIO_PIN_8    //PB8
//#define D_AD_START_PORT                 GPIOB

#define D_AD_CS                         GPIO_PIN_7    //PB7
#define D_AD_CS_PORT                    GPIOB

#define D_AD_RST                        GPIO_PIN_9    //PB9
#define D_AD_RST_PORT                   GPIOB

#define D_AD_DRDY												GPIO_PIN_8		//PB8 int
#define D_AD_DRDY_PORT									GPIOB

#define D_AD_DOUT												GPIO_PIN_6		//PA6		stm接收
#define D_AD_DOUT_PORT									GPIOA

#define D_AD_DIN	          						GPIO_PIN_7    //PA7  stm发送
#define D_AD_DIN_PORT                   GPIOA

#define D_AD_SCK                        GPIO_PIN_5    //PA5
#define D_AD_SCK_PORT                   GPIOA

#define D_AI_SELECT_A0                  GPIO_PIN_3    //PC3
#define D_AI_SELECT_A0_PORT        			GPIOC 
#define D_AI_SELECT_A1									GPIO_PIN_2    //PC2
#define D_AI_SELECT_A1_PORT  						GPIOC
#define D_AI_SELECT_A2  								GPIO_PIN_1 		//PC1
#define D_AI_SELECT_A2_PORT             GPIOC
//-----------------------------------------------------------------------------------------------------------


//clock
#define D_S_PER_MIN			(60)	// 60
#define D_S_PER_15MIN 	(15*(D_S_PER_MIN))	// 900
#define D_S_PER_HR			(60*(D_S_PER_MIN))	// 3600
#define D_S_PER_DAY			(24*(D_S_PER_HR))		// 86400
#define D_S_PER_YR			(365*(D_S_PER_DAY))
#define D_S_PER_LEAPYR	(366*(D_S_PER_DAY))
//--------------------------------------------------------------------------------
#define bit(x) (1<<x)

