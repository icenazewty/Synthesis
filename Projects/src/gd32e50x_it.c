/*!
    \file    gd32e50x_it.c
    \brief   main interrupt service routines

    \version 2020-09-04, V1.0.0, demo for GD32E50x
    \version 2021-03-31, V1.1.0, demo for GD32E50x
*/

/*
    Copyright (c) 2021, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "gd32e50x_it.h"
#include "usbd_lld_int.h"
#include "systick.h"
#include "GlobalVar.h"
#include "gd32e503v_eval.h"
#include <stdio.h>
#include "gd32e503v_eval.h"
#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t 	EtherNet_Task_Handle;			/* w5500任务句柄 */
extern char 					g_tranflag;
static unsigned short systickTemp=0; 
extern can_receive_message_struct g_receive_message;
extern FlagStatus can0_receive_flag;
extern uint8_t W5500_Interrupt;
/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void SVC_Handler(void)
//{
//}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void PendSV_Handler(void)
//{
//}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/

//void SysTick_Handler(void)
//{
//		systickCount++;
//		systickTemp++;
//		if(systickTemp>999)
//		{
//			systickTemp = 0;
//			g_sysTick++;
//		}
//    delay_decrement();
//}

//CAN0_RX1_IRQn
void CAN0_RX1_IRQHandler (void)
{    
		
}

/*!
    \brief      this function handles USBD interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
unsigned int tj = 0;
void USBD_LP_CAN0_RX0_IRQHandler (void)
{  		
		if(g_can_bus)
		{
			if(RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFL0))		
			{
			//can_message_receive(CAN0, CAN_FIFO1, &g_receive_message);
				can_message_receive(CAN0, CAN_FIFO0, &g_receive_message);
				can0_receive_flag = SET; 
			}			
		}
		else
		{
			//if(RESET != can_interrupt_flag_get(CAN0, CAN_INT_FLAG_RFL1))		
			//printf("1");
			tj++;
			usbd_isr();			
		}
}

#ifdef USBD_LOWPWR_MODE_ENABLE

/*!
    \brief      this function handles USBD wakeup interrupt request.
    \param[in]  none
    \param[out] none
    \retval     none
*/
void USBD_WKUP_IRQHandler (void)
{
    exti_interrupt_flag_clear(EXTI_18);
}

#endif /* USBD_LOWPWR_MODE_ENABLE */

/*!
    \brief      this function handles USART RBNE interrupt request and TBE interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/

uint8_t 	g_ch[6];
void USART0_IRQHandler(void)			//COM0  rs485
{		
		#define COM_PORT0	0
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE))
		{			
				g_ch[COM_PORT0] = (uint8_t)usart_data_receive(USART0);				
				if(com1_sended)
				{						
					Com.Usart[COM_PORT0].RX_Buf[Com.Usart[COM_PORT0].usRec_WR] = g_ch[COM_PORT0];				
					Com.Usart[COM_PORT0].usRec_WR=(Com.Usart[COM_PORT0].usRec_WR+1)% D_USART_REC_BUFF_SIZE;	
					Com.Usart[COM_PORT0].usTick = systickCount;									//最后接收的时间
					if(g_ch[COM_PORT0]=='\n')
					{			
						b_wr = 0;
						b_rd = 0;
					}		
					boot_buf[b_wr] = g_ch[COM_PORT0];
					b_wr = (b_wr+1)%128;		
				}
				usart_interrupt_flag_clear(USART0,USART_INT_FLAG_RBNE);
    }   
		
		if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE_ORERR))	
		{				
				g_ch[COM_PORT0] = (uint8_t)usart_data_receive(USART0);
				usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE_ORERR);									
    } 	
}

void USART1_IRQHandler(void)		//COM1  RS485_2
{
		#define COM_PORT1	1
	  if(RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE))
		{			
//				g_ch[COM_PORT1] = (uint8_t)usart_data_receive(USART1);	     			
//				if(com2_sended)
				{					
					//usart_data_transmit(USART2, (uint8_t)g_ch[COM_PORT1]);			
					Com.Usart[COM_PORT1].RX_Buf[Com.Usart[COM_PORT1].usRec_WR] = (uint8_t)usart_data_receive(USART1);	//g_ch[COM_PORT1];				
					Com.Usart[COM_PORT1].usRec_WR=(Com.Usart[COM_PORT1].usRec_WR+1)% D_USART_REC_BUFF_SIZE;	
					Com.Usart[COM_PORT1].usTick = systickCount;																						//最后接收的时间
				}
				usart_interrupt_flag_clear(USART1,USART_INT_FLAG_RBNE);
    }
		
		if(RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE_ORERR))	
		{		
				g_ch[COM_PORT1] = (uint8_t)usart_data_receive(USART1);	     						
				usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE_ORERR);									
    } 		
}

int	 ready_sta[6];
void USART2_IRQHandler(void)		//COM2  WIFI
{
		#define COM_PORT2	2
	  if(RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE))
		{
				g_ch[COM_PORT2] = (uint8_t)usart_data_receive(USART2);					
				//ready
				//unsigned char 		wifi_reset_sta;			//wifi重启后的状态  0=表示重启开始  1=表示进入已开机但非透传模式    2=表示进入了透传模式   3=没有接收到任何数据超时
				if(0==wifi_reset_sta)
				{
					//usart_data_transmit(USART1, (uint8_t)g_ch[COM_PORT2]);				//调试打印				
					wifi_rev_cnt++;				
					if(g_ch[COM_PORT2]=='>')		
					{
						ready_sta[0] = wifi_rev_cnt;
					}	
					else if(g_ch[COM_PORT2]=='r')
					{
						ready_sta[1] = wifi_rev_cnt;
					}
					else if(g_ch[COM_PORT2]=='e')
					{
						ready_sta[2] = wifi_rev_cnt;
					}
					else if(g_ch[COM_PORT2]=='a')
					{
						ready_sta[3] = wifi_rev_cnt;
					}
					else if(g_ch[COM_PORT2]=='d')
					{
						ready_sta[4] = wifi_rev_cnt;
					}
					else if(g_ch[COM_PORT2]=='y')		
					{
						ready_sta[5] = wifi_rev_cnt;
						if((ready_sta[1]+1)==ready_sta[2] && (ready_sta[1]+2)==ready_sta[3] && (ready_sta[1]+3)==ready_sta[4] && (ready_sta[1]+4)==ready_sta[5])
						{							
							if((ready_sta[0]+3) == ready_sta[1] || (ready_sta[0]+2) == ready_sta[1])		//处于透传模式
							{
									wifi_reset_sta = 2;
									usart_data_transmit(USART0, '2');	
							}
							else																																				//非透传模式
							{
									wifi_reset_sta = 1;
									usart_data_transmit(USART0, '1');	
							}
						}
					}						
				}		
				else
				{
					//usart_data_transmit(UART3, (uint8_t)g_ch[COM_PORT2]);				/调试打印
					Com.Usart[COM_PORT2].RX_Buf[Com.Usart[COM_PORT2].usRec_WR] = g_ch[COM_PORT2];
					Com.Usart[COM_PORT2].usTick = systickCount;									//最后接收的时间
					Com.Usart[COM_PORT2].usRec_WR=(Com.Usart[COM_PORT2].usRec_WR+1)% D_USART_REC_BUFF_SIZE;	
				}
				usart_interrupt_flag_clear(USART2,USART_INT_FLAG_RBNE);
    }   
		
		if(RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE_ORERR))	
		{			
				g_ch[COM_PORT2] = (uint8_t)usart_data_receive(USART2);								
				usart_interrupt_flag_clear(USART2, USART_INT_FLAG_RBNE_ORERR);									
    } 
}

void UART3_IRQHandler(void)		//COM3   RS485_4
{
		#define COM_PORT3	3
			
	  if(RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE))
		{			
			//	g_ch[COM_PORT3] = (uint8_t)usart_data_receive(UART3);	 							
			//	if(com4_sended)      
				{			
					Com.Usart[COM_PORT3].RX_Buf[Com.Usart[COM_PORT3].usRec_WR] = (uint8_t)usart_data_receive(UART3);	 	//g_ch[COM_PORT3];														
					Com.Usart[COM_PORT3].usRec_WR=(Com.Usart[COM_PORT3].usRec_WR+1)% D_USART_REC_BUFF_SIZE;	
					Com.Usart[COM_PORT3].usTick = systickCount;																													//最后接收的时间	
				}
				usart_interrupt_flag_clear(UART3,USART_INT_FLAG_RBNE);
    } 		
		
		if(RESET != usart_interrupt_flag_get(UART3, USART_INT_FLAG_RBNE_ORERR))	
		{		
				g_ch[COM_PORT3] = (uint8_t)usart_data_receive(UART3);	 						
				usart_interrupt_flag_clear(UART3, USART_INT_FLAG_RBNE_ORERR);									
				//usart_data_transmit(USART0, (uint8_t)(g_ch[COM_PORT2]+2));				//调试打印			
    }
}

void UART4_IRQHandler(void)		//COM4  Meter
{
		#define COM_PORT4	4
	  if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE))
		{			
				g_ch[COM_PORT4] = (uint8_t)usart_data_receive(UART4);										
				if(com5_sended)
				{					
					Com.Usart[COM_PORT4].usTick = systickCount;									//最后接收的时间
					Com.Usart[COM_PORT4].RX_Buf[Com.Usart[COM_PORT4].usRec_WR] = g_ch[COM_PORT4];				
					Com.Usart[COM_PORT4].usRec_WR=(Com.Usart[COM_PORT4].usRec_WR+1)% D_USART_REC_BUFF_SIZE;					
				}
				usart_interrupt_flag_clear(UART4,USART_INT_FLAG_RBNE);
    }  
		
		if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE_ORERR))	
		{			
				g_ch[COM_PORT4] = (uint8_t)usart_data_receive(UART4);							
				usart_interrupt_flag_clear(UART4, USART_INT_FLAG_RBNE_ORERR);									
    }   
}

void USART5_IRQHandler(void)	//COM5  4G
{
		#define COM_PORT5	5			//ORERR 溢出错误  需要向USART_INTC 寄存器的 OREC 位写 1 清 0  或 USART_RDATA读
		//printf("usart_ctl0=0x%08x   usart_ctl1=0x%08x  usart_ctl2=0x%08x\r\n",REG32(0x40017000),REG32(0x40017004),REG32(0x40017008));																				//0x0060(96K sram) 0100(256K Flash)
		//printf("USART_STA=0x%08x    USART_INTC=0x%08x  USART_RFCS=0x%08x\r\n",REG32(0x4001701c),REG32(0x40017020),REG32(0x400170D0));
		//printf("cpu id=0x%08x%08x%08x \r\n",REG32(0x1ffff7f0),REG32(0x1ffff7ec),REG32(0x1ffff7e8));		//从左到右依次为   从高位到低位 0x413848541337343155315a18	
	  if(RESET != usart5_interrupt_flag_get(USART5, USART5_INT_FLAG_RBNE))
		{
				g_ch[COM_PORT5] = (uint8_t)usart_data_receive(USART5);	       
				Com.Usart[COM_PORT5].RX_Buf[Com.Usart[COM_PORT5].usRec_WR] = g_ch[COM_PORT5];
				Com.Usart[COM_PORT5].usTick = systickCount;												//最后接收的时间
				Com.Usart[COM_PORT5].usRec_WR=(Com.Usart[COM_PORT5].usRec_WR+1)% D_USART_REC_BUFF_SIZE;	
			
				if(m_gprsinfo.g_bGPRSConnected && g_tranflag)											//gprs假设处于连接状态
				{
					RSGPRS_buff[RSGPRS_rec_WR] = g_ch[COM_PORT5];
					RSGPRS_rec_WR = (RSGPRS_rec_WR+1)%RSGPRS_REC_BUFF_SIZE;		
				}	
				usart5_interrupt_flag_clear(USART5, USART5_INT_FLAG_RBNE);				
    }   
		
		if(RESET != usart5_interrupt_flag_get(USART5, USART5_INT_FLAG_RBNE_ORERR))
		{		
				g_ch[COM_PORT5] = (uint8_t)usart_data_receive(USART5);	       			
				usart5_interrupt_flag_clear(USART5, USART5_INT_FLAG_RBNE_ORERR);									
    }   
}



/*!
    \brief      this function handles external lines 5 to 9 interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI0_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(EXTI_0))		//PA0 	busy0
		{   				
				W5500_Interrupt = 1;
//				if(NULL!=EtherNet_Task_Handle)
//				{
//						BaseType_t xHigherPriorityTaskWoken = pdFALSE;        
//						xTaskNotifyFromISR((TaskHandle_t)EtherNet_Task_Handle,				//接收任务通知的任务句柄
//													(uint32_t		)0x80000000,												//要触发的事件
//													(eNotifyAction)eSetBits,												//设置任务通知值中的位
//													&xHigherPriorityTaskWoken);											//
//						portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//				}
				exti_interrupt_flag_clear(EXTI_0);
    }
}

/*!
    \brief      this function handles external lines 5 to 9 interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI1_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(EXTI_1))					//PA1 	busy1;	PB1		INT0 
		{   
				if(RESET==gpio_input_bit_get(GPIOA, GPIO_PIN_1))	//PA1 busy1	trigger
				{
				
				}
				
				if(RESET==gpio_input_bit_get(GPIOB, GPIO_PIN_1))	//PB1 INT0  trigger
				{
				
				}			
        exti_interrupt_flag_clear(EXTI_1);
    }
}

/*!
    \brief      this function handles external lines 5 to 9 interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EXTI4_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(EXTI_4))		//PC4		INT1	
		{   				
        exti_interrupt_flag_clear(EXTI_4);
    }
}

//问题分析，先确认COM1收到的数据是否完整  向COM2 进行转发
//向COM4转发过程中出了问题  发送过程可能有问题，逆变器可以收到数据并返回，但另外两个设备不能收到数据
