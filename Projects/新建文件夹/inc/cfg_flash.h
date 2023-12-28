#ifndef	__CFG_FLASH_H
#define	__CFG_FLASH_H

/***************** Common Register *****************/


#define TCP_SERVER		0x00	//TCP������ģʽ
#define TCP_CLIENT		0x01	//TCP�ͻ���ģʽ 
#define UDP_MODE		0x02	//UDP(�㲥)ģʽ 


unsigned char  init_config_net(unsigned char flag);
unsigned char  init_config_ads1247(unsigned char flag);
void  					WriteParametersADS1247(void );
void						WriteParametersToIICAll(void);
unsigned char  init_config(unsigned char flag);
void  					WriteConfigParaFromIICAll();
void 						WriteFlashWifi(void);
void 						ReadFlashWifi(void);
#endif

