#include "can_bus.h"
#include "GlobalVar.h"
//#include "stm32f10x_conf.h"
//#include "hardware.h"
#include "string.h"
#include "systick.h"
#include <stdio.h>
#include "gd32e503v_eval.h"

can_trasnmit_message_struct g_transmit_message;
can_receive_message_struct 	g_receive_message;
FlagStatus 									can0_receive_flag;
int g_can = 0;	
/*!
    \brief      initialize CAN function
    \param[in]  none
    \param[out] none
    \retval     none
*/
void can_config()
{
	  can_parameter_struct can_parameter;
     
    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    /* initialize CAN register */
    can_deinit(CAN0);
    
    /* initialize CAN parameters */
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = DISABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = DISABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
	 //	can_parameter.working_mode = CAN_LOOPBACK_MODE;
	
    /* configure CAN0 baud rate 1Mbps */
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;
    can_parameter.time_segment_1 = CAN_BT_BS1_5TQ;
    can_parameter.time_segment_2 = CAN_BT_BS2_4TQ;
		can_parameter.prescaler = 42;		//9			180M/45=2M   168M/42=4M  4/10=400K  正确: 时钟168/2=84M   84/42=2M  2M/10=200K 标准帧 帧id为0001
    /* initialize CAN */
    can_init(CAN0, &can_parameter);
    
    /* initialize filter */ 
    can1_filter_start_bank(14);
		//can_filter_mask_mode_init(CAN0,DEV_CAN0_ID, DEV_CAN0_MASK, CAN_EXTENDED_FIFO0, 0);
		//can_filter_mask_mode_init(CAN0,DEV_CAN0_ID, DEV_CAN0_MASK, CAN_STANDARD_FIFO1, 0);			
		can_filter_mask_mode_init(CAN0,DEV_CAN0_ID, DEV_CAN0_MASK, CAN_STANDARD_FIFO0, 0);			
    
		nvic_priority_group_set(NVIC_PRIGROUP_PRE0_SUB4);		// NVIC_PRIGROUP_PRE0_SUB4  0 0  则最高级中断；   NVIC_PRIGROUP_PRE4_SUB0  15  0 则为最低中断
		//抢占优先级0位 ，响应优先级4位   数字越小级别越高  NVIC_PRIGROUP_PRE0_SUB4:0  [0,15] ; NVIC_PRIGROUP_PRE1_SUB3: [0,1]  [0,7]; ...  NVIC_PRIGROUP_PRE4_SUB0: [0,15]  0
		
    /* configure CAN0 NVIC */
		//nvic_irq_enable(CAN0_RX1_IRQn, 0, 7);
		nvic_irq_enable(USBD_LP_CAN0_RX0_IRQn, 0, 7);
		
    /* enable can receive FIFO0 not empty interrupt */		
		
    //can_interrupt_enable(CAN0, CAN_INT_RFNE1);				//接收0  中断	CAN_INT_RFNE0
		can_interrupt_enable(CAN0, CAN_INT_RFNE0);				//接收0  中断	CAN_INT_RFNE0
		
		/* initialize transmit message */
		can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &g_transmit_message);
		g_transmit_message.tx_sfid = 0x00;
		g_transmit_message.tx_efid = 0x00;
		g_transmit_message.tx_ft = CAN_FT_DATA;
		g_transmit_message.tx_ff = CAN_FF_STANDARD;	//CAN_FF_EXTENDED;
		g_transmit_message.tx_dlen = 8;
		/* initialize receive message */
		can_struct_para_init(CAN_RX_MESSAGE_STRUCT, &g_receive_message);		
}

void can_send(uint32_t can_periph,uint16_t sfid,uint8_t *str,uint8_t len)
{
			g_transmit_message.tx_sfid = sfid;
			memcpy(g_transmit_message.tx_data,str,len);
			can_message_transmit(can_periph, &g_transmit_message); 		/* transmit message */
      //printf("\r\n can0 transmit data:%08x", *(uint32_t *)g_transmit_message.tx_data);        
}


void communication_check(void)
{
    /* CAN0 receive data correctly, the received data is printed */
    if(SET == can0_receive_flag)
		{
        can0_receive_flag = RESET;
        printf("\r\n can0 receive data:%08x %08x", *(uint32_t *)g_receive_message.rx_data,*(uint32_t *)(g_receive_message.rx_data+4));
        
    }
}

void uart_can_test(unsigned char ch)
{	
	//com  usb固定通道 USB
	unsigned char buf[512];
	int		len = 0;	
	if(g_can==0)
	{
		//can_gpio_config();
		g_can = 1;
	}
	while(Com.Usart[RS485].usRec_WR != Com.Usart[RS485].usRec_RD)
	{
			buf[len++] = Com.Usart[RS485].RX_Buf[Com.Usart[RS485].usRec_RD];
			Com.Usart[RS485].usRec_RD = (Com.Usart[RS485].usRec_RD+1)% D_USART_REC_BUFF_SIZE;	
	}		
	if(len>0)
	{
		 can_send(CAN0,DEV_CAN0_ID,buf,len);					
	}			
}
