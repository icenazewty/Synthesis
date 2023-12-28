#ifndef	__CFG_FLASH_H
#define	__CFG_FLASH_H

/***************** Common Register *****************/


#define TCP_SERVER		0x00	//TCP服务器模式
#define TCP_CLIENT		0x01	//TCP客户端模式 
#define UDP_MODE		0x02	//UDP(广播)模式 


unsigned char  init_config_net(unsigned char flag);
unsigned char  init_config_ads1247(unsigned char flag);
void  					WriteParametersADS1247(void );
void						WriteParametersToIICAll(void);
unsigned char  init_config(unsigned char flag);
void  					WriteConfigParaFromIICAll();
void 						WriteFlashWifi(void);
void 						ReadFlashWifi(void);
#endif

