#include "gd32e50x_rcu.h"
#include "gd32e50x_rtc.h"
#include "time.h"
struct tm * localtime2 (  const  time_t *ptime2 );
time_t   mktime2 (        struct tm *tb        );

void RTC_Test(void){
	struct tm sm;
	struct tm sm2,sm3[5];
	time_t t,t1;
	//return;
	sm.tm_year = 2023-1900;//��ƫ��1900����
	sm.tm_mon = 1;
	sm.tm_mday = 1;//0???
	sm.tm_hour = 1;
	sm.tm_min = 1;
	sm.tm_sec = 1;
	t = mktime2(&sm);
	sm2 = *localtime2((const time_t *)&t);//yday:31 wday:3 

	sm.tm_year = 2023-1900;//��ƫ��1900����
	sm.tm_mon = 1;
	sm.tm_mday = 1;//0???
	sm.tm_hour = 1;
	sm.tm_min = 1;
	sm.tm_sec = 1;
	t = mktime2(&sm);
	sm2 = *localtime2((const time_t *)&t);//yday 31  wday 3  xxx-1-1
	t -= 30*24*3600;
	sm2 = *localtime2((const time_t *)&t);//yday 1 wday 1 xxx-0-2
	t -= 1*24*3600;
	sm2 = *localtime2((const time_t *)&t);//yday 0 wday 0 xxx-0-1
	t -= 1*24*3600;
	sm2 = *localtime2((const time_t *)&t);//yday 364 wday 6  xxx-11-31
	t -= 1*24*3600;
	sm2 = *localtime2((const time_t *)&t);//yday 363 wd5 xx-11-30
	t -= 1*24*3600;
	sm2 = *localtime2((const time_t *)&t);//yday 362 wd4 xx-11-29
	t -= 1*24*3600;
	sm2 = *localtime2((const time_t *)&t);//yday 361 wd3 xx-11-28

	sm.tm_year = 2023-1900;//��ƫ��1900����
	sm.tm_mon = 0;
	sm.tm_mday = 0;//0???
	sm.tm_hour = 1;
	sm.tm_min = 1;
	sm.tm_sec = 1;
	t = mktime2(&sm);
	sm2 = *localtime2((const time_t *)&t);//yd364  11-31
	sm.tm_year = 2023-1900;//��ƫ��1900����
	sm.tm_mon = 0;
	sm.tm_mday = 0;//0???
	sm.tm_hour = 0;
	sm.tm_min = 0;
	sm.tm_sec = 0;
	t = mktime2(&sm);
	sm2 = *localtime2((const time_t *)&t);
	t -= 24*3600;
	sm2 = *localtime2((const time_t *)&t);

	sm2 = *localtime2((const time_t *)&t);

	sm.tm_year = 2023-1900;//��ƫ��1900����
	sm.tm_mon = 7;
	sm.tm_mday = 25;//0???
	sm.tm_hour = 0;
	sm.tm_min = 0;
	sm.tm_sec = 0;
	t = mktime2(&sm);
	rtc_counter_set(t);delay_1ms(5);
	t=rtc_counter_get();
	sm3[0] = *localtime2((const time_t *)&t);
	delay_1ms(10000);
	t1=rtc_counter_get();
	t=t1;
	sm3[1] = *localtime2((const time_t *)&t);
	delay_1ms(10000);
	t1=rtc_counter_get();
	t=t1;
	sm3[2] = *localtime2((const time_t *)&t);
	delay_1ms(10000);
	t1=rtc_counter_get();
	t=t1;
	sm3[3] = *localtime2((const time_t *)&t);

}
void RTC_Configuration(void){
//---------- С�� -----------------------------------------------------------------
//	unsigned char temp=0;
//	BKP_DeInit();	//��λ��������
//	RCC_LSEConfig(RCC_LSE_ON);	//�����ⲿ���پ���(LSE),ʹ��������پ���
//	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
//  {
//		temp++;
//		DlymS(10);
//  }
//	if(temp>=250)return 1;//��ʼ��ʱ��ʧ��,����������	
//	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��    
//	RCC_RTCCLKCmd(ENABLE);	//ʹ��RTCʱ��  
//	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
//	RTC_WaitForSynchro();		//�ȴ�RTC�Ĵ���ͬ��  
//	RTC_ITConfig(RTC_IT_SEC, ENABLE);		//ʹ��RTC���ж�
//	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
//	RTC_SetPrescaler(32767); //����RTCԤ��Ƶ��ֵ
//	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
//	return 0; //ok
	/* Enable PWR and BKP clocks */
		
	//ʹ��PMU��BKPIʱ��
  rcu_periph_clock_enable(RCU_BKPI);
  rcu_periph_clock_enable(RCU_PMU);
  
  /* allow access to BKP domain */
	pmu_backup_write_enable();//ʹ�ܶԱ������мĴ�����д���ʣ�����PMU_CTL�Ĵ�����BKPWENλ

	/* reset backup domain */
	bkp_deinit();//�������������λ������RCU_BDCL�Ĵ���BKPRSTλ
	
	#if 0
	/* enable LXTAL */
	rcu_osci_on(RCU_LXTAL);//ʹ���ⲿʱ��LXTAL
	/* wait till LXTAL is ready */
	rcu_osci_stab_wait(RCU_LXTAL);
	
	/* select RCU_LXTAL as RTC clock source */
	rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);//ѡ���ⲿʱ��
	#else
	/* enable LXTAL */
	//rcu_osci_on(RCU_LXTAL);//ʹĜ͢²¿ʱ֓LXTAL
	/* wait till LXTAL is ready */
	//rcu_osci_stab_wait(RCU_HXTAL);
	
	/* select RCU_LXTAL as RTC clock source */
	rcu_rtc_clock_config(RCU_RTCSRC_HXTAL_DIV_128);//ѡԱ͢²¿ʱ֓
	#endif
	/* enable RTC Clock */
	rcu_periph_clock_enable(RCU_RTC);//ʹ��RTCʱ��

	/* wait for RTC registers synchronization */
	rtc_register_sync_wait();

	/* wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();

	/* enable the RTC second interrupt*/
//	rtc_interrupt_enable(RTC_INT_SECOND);//ʹ�����ж�
//	rtc_interrupt_enable(RTC_INT_ALARM);//ʹ�������ж�
	/* wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();

	/* set RTC prescaler: set RTC period to 1s */
	rtc_prescaler_set(62500);				//LSE值为32767    如果为片外主晶振则为  8000000 /128=62500

	/* wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();
}

//RTC��ʼ������һ���ϵ�Ͷϵ�����ϵ�ֱ���	
void	RTC_Init(void)
{	
  /* Backup data register value is not correct or not yet programmed (when the first time the program is executed) */
  if(bkp_read_data(BKP_DATA_0) != 0xA5A5)
	{
    RTC_Configuration();	/* RTC Configuration */
		rtc_counter_set(0);
		rtc_lwoff_wait();
    bkp_write_data(BKP_DATA_0, 0xA5A5);
  }
	else
	{
		/* check if the power on reset flag is set */
    if(rcu_flag_get(RCU_FLAG_PORRST) != RESET)/* Check if the Power On Reset flag is set */
    {
//			printf("\r\n\n Power On Reset occurred....");
    }
    else if(rcu_flag_get(RCU_FLAG_SWRST) != RESET)/* Check if the Pin Reset flag is set */
    {
//			printf("\r\n\n External Reset occurred....");
    }
		
		/* allow access to BKP domain */
		rcu_periph_clock_enable(RCU_PMU);
		pmu_backup_write_enable();
		
		//printf("\r\n No need to configure RTC....");
    rtc_register_sync_wait();/* Wait for RTC registers synchronization */
		rtc_lwoff_wait();
//		rtc_counter_get();
//    rtc_interrupt_enable(RTC_INT_SECOND);/* Enable the RTC Second */  
//		rtc_interrupt_enable(RTC_INT_ALARM);
    rtc_lwoff_wait();    /* Wait until last write operation on RTC registers has finished */
  }
  rcu_all_reset_flag_clear();/* Clear reset flags */
} 			  
