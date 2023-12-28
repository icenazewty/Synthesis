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
	sm.tm_year = 2023-1900;//是偏移1900的年
	sm.tm_mon = 1;
	sm.tm_mday = 1;//0???
	sm.tm_hour = 1;
	sm.tm_min = 1;
	sm.tm_sec = 1;
	t = mktime2(&sm);
	sm2 = *localtime2((const time_t *)&t);//yday:31 wday:3 

	sm.tm_year = 2023-1900;//是偏移1900的年
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

	sm.tm_year = 2023-1900;//是偏移1900的年
	sm.tm_mon = 0;
	sm.tm_mday = 0;//0???
	sm.tm_hour = 1;
	sm.tm_min = 1;
	sm.tm_sec = 1;
	t = mktime2(&sm);
	sm2 = *localtime2((const time_t *)&t);//yd364  11-31
	sm.tm_year = 2023-1900;//是偏移1900的年
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

	sm.tm_year = 2023-1900;//是偏移1900的年
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
//---------- 小李 -----------------------------------------------------------------
//	unsigned char temp=0;
//	BKP_DeInit();	//复位备份区域
//	RCC_LSEConfig(RCC_LSE_ON);	//设置外部低速晶振(LSE),使用外设低速晶振
//	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
//  {
//		temp++;
//		DlymS(10);
//  }
//	if(temp>=250)return 1;//初始化时钟失败,晶振有问题	
//	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
//	RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟  
//	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
//	RTC_WaitForSynchro();		//等待RTC寄存器同步  
//	RTC_ITConfig(RTC_IT_SEC, ENABLE);		//使能RTC秒中断
//	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
//	RTC_SetPrescaler(32767); //设置RTC预分频的值
//	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
//	return 0; //ok
	/* Enable PWR and BKP clocks */
		
	//使能PMU、BKPI时钟
  rcu_periph_clock_enable(RCU_BKPI);
  rcu_periph_clock_enable(RCU_PMU);
  
  /* allow access to BKP domain */
	pmu_backup_write_enable();//使能对备份域中寄存器的写访问；设置PMU_CTL寄存器的BKPWEN位

	/* reset backup domain */
	bkp_deinit();//软件触发备份域复位：设置RCU_BDCL寄存器BKPRST位
	
	#if 0
	/* enable LXTAL */
	rcu_osci_on(RCU_LXTAL);//使能外部时钟LXTAL
	/* wait till LXTAL is ready */
	rcu_osci_stab_wait(RCU_LXTAL);
	
	/* select RCU_LXTAL as RTC clock source */
	rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);//选择外部时钟
	#else
	/* enable LXTAL */
	//rcu_osci_on(RCU_LXTAL);//使臏廷虏驴时謸LXTAL
	/* wait till LXTAL is ready */
	//rcu_osci_stab_wait(RCU_HXTAL);
	
	/* select RCU_LXTAL as RTC clock source */
	rcu_rtc_clock_config(RCU_RTCSRC_HXTAL_DIV_128);//选员廷虏驴时謸
	#endif
	/* enable RTC Clock */
	rcu_periph_clock_enable(RCU_RTC);//使能RTC时钟

	/* wait for RTC registers synchronization */
	rtc_register_sync_wait();

	/* wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();

	/* enable the RTC second interrupt*/
//	rtc_interrupt_enable(RTC_INT_SECOND);//使能秒中断
//	rtc_interrupt_enable(RTC_INT_ALARM);//使能闹钟中断
	/* wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();

	/* set RTC prescaler: set RTC period to 1s */
	rtc_prescaler_set(62500);				//LSE鍊间负32767    濡傛灉涓虹墖澶栦富鏅舵尟鍒欎负  8000000 /128=62500

	/* wait until last write operation on RTC registers has finished */
	rtc_lwoff_wait();
}

//RTC初始化，第一次上电和断电后再上电分别处理。	
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
