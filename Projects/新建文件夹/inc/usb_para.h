/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : spi_flash.h
* Author             : MCD Application Team
* Date First Issued  : 02/05/2007
* Description        : Header for spi_flash.c file.
********************************************************************************
* History:
* 05/21/2007: V0.3
* 04/02/2007: V0.2
* 02/05/2007: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_PARA_H
#define __USB_PARA_H

/* Includes ------------------------------------------------------------------*/
void comm_process_usb(void);
void putStr(char* str,unsigned int len);
int strIntTostrFloat(char* dest, char* src, int srcLen);
void strtomac(unsigned char *ip, char*src,int len);
void rtc_second_cal_active_date(void);
void PCF_setsystime(char* time,int len);
#endif /* __USB_PARA_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
