/*
 * FreeRTOS V202012.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/******************************************************************************
	See http://www.freertos.org/a00110.html for an explanation of the
	definitions contained in this file.
******************************************************************************/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 * http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

extern uint32_t SystemCoreClock;

/* Cortex M33 port configuration. */
#define configENABLE_MPU										0		//1-->0	wjj   2022.7.13
#define configENABLE_FPU										1
#define configENABLE_TRUSTZONE							0		//1-->0  wjj  2022.7.13
#define configSUPPORT_DYNAMIC_ALLOCATION		1		//支持动态申请内存,系统就会自动为该任务从系统总堆栈中获取内存

/* Constants related to the behaviour or the scheduler. */
/* 某些运行FreeRTOS的硬件有两种方法选择下一个要执行的任务：
 * 通用方法和特定于硬件的方法（以下简称“特殊方法”）。
 * 
 * 通用方法：
 *      1.configUSE_PORT_OPTIMISED_TASK_SELECTION 为 0 或者硬件不支持这种特殊方法。
 *      2.可以用于所有FreeRTOS支持的硬件
 *      3.完全用C实现，效率略低于特殊方法。
 *      4.不强制要求限制最大可用优先级数目
 * 特殊方法：
 *      1.必须将configUSE_PORT_OPTIMISED_TASK_SELECTION设置为1。
 *      2.依赖一个或多个特定架构的汇编指令（一般是类似计算前导零[CLZ]指令）。
 *      3.比通用方法更高效
 *      4.一般强制限定最大可用优先级数目为32
 * 一般是硬件计算前导零指令，如果所使用的，MCU没有这些硬件指令的话此宏应该设置为0！
 */ 
#define configUSE_PORT_OPTIMISED_TASK_SELECTION			0
/* 置1：RTOS使用抢占式调度器；置0：RTOS使用协作式调度器（时间片）
 * 
 * 注：在多任务管理机制上，操作系统可以分为抢占式和协作式两种。
 * 协作式操作系统是任务主动释放CPU后，切换到下一个任务。
 * 任务切换的时机完全取决于正在运行的任务。
 */
#define configUSE_PREEMPTION								1				//1=抢占式内核;0=使用协程内核
#define configUSE_TIME_SLICING							1				//1使能时间片调度(默认式使能的);就绪态的优先级相同的任务就会使用时间片轮转调度器获取运行时间
#define configMAX_PRIORITIES							( 32 )		//可使用的最大优先级;数值越大优先级越高
#define configIDLE_SHOULD_YIELD							1				//空闲任务放弃CPU使用权给其他同优先级的用户任务
#define configUSE_16_BIT_TICKS							0 			/* Only for 8 and 16-bit hardware. */

/* Constants that describe the hardware and memory usage. */
#define configCPU_CLOCK_HZ								SystemCoreClock
#define configMINIMAL_STACK_SIZE					( ( uint16_t ) 128 )					//空闲任务使用的堆栈大小
#define configMINIMAL_SECURE_STACK_SIZE		( 2048 )											//1024-->2048
#define configMAX_TASK_NAME_LEN						( 16 )												//任务名字字符串长度
#define configTOTAL_HEAP_SIZE							( ( size_t ) ( 36 * 1024 ) )	//系统所有总的堆大小 , 动态内存管理时,在创建任务、队列、信号量时就会自动使用heap_x.c中的内存申请函数来申请内存

/* Constants that build features in or out. */
/* 置1：使能低功耗tickless模式；置0：保持系统节拍（tick）中断一直运行
 * 假设开启低功耗的话可能会导致下载出现问题，因为程序在睡眠中,可用以下办法解决
 * 
 * 下载方法：
 *      1.将开发版正常连接好
 *      2.按住复位按键，点击下载瞬间松开复位按键
 *     
 *      1.通过跳线帽将 BOOT 0 接高电平(3.3V)
 *      2.重新上电，下载
 *    
 * 			1.使用FlyMcu擦除一下芯片，然后进行下载
 *			STMISP -> 清除芯片(z)
 */
 
#define configUSE_MUTEXES									0			//使用互斥信号量
#define configUSE_TICKLESS_IDLE						0			//1-->0  使用tickless低功耗模式,此时在执行空闲任务的时候系统时钟节拍中断就会停止，当然停止的这段时间也必须要补上。
#define configUSE_APPLICATION_TASK_TAG		0
#define configUSE_NEWLIB_REENTRANT				0
#define configUSE_CO_ROUTINES							0			//启用协程，启用协程以后必须添加文件croutine.c
#define configUSE_COUNTING_SEMAPHORES			0			//1-->0为1时使用计数信号量
#define configUSE_RECURSIVE_MUTEXES				0			//1-->1使用递归互斥信号量    
#define configUSE_QUEUE_SETS							0			//启用队列集合
#define configUSE_TASK_NOTIFICATIONS			1			//开启任务通知功能，默认开启,每个任务将会多消耗8byte。
#define configUSE_TRACE_FACILITY					0			 //0-->1启用可视化跟踪调试
	
/* Constants that define which hook (callback) functions should be used. */
/* 置1：使用空闲钩子（Idle Hook类似于回调函数）；置0：忽略空闲钩子
 * 
 * 空闲任务钩子是一个函数，这个函数由用户来实现，
 * FreeRTOS规定了函数的名字和参数：void vApplicationIdleHook(void )，
 * 这个函数在每个空闲任务周期都会被调用
 * 对于已经删除的RTOS任务，空闲任务可以释放分配给它们的堆栈内存。
 * 因此必须保证空闲任务可以被CPU执行
 * 使用空闲钩子函数设置CPU进入省电模式是很常见的
 * 不可以调用会引起空闲任务阻塞的API函数
 */
#define configUSE_IDLE_HOOK								0
/* 置1：使用时间片钩子（Tick Hook）；置0：忽略时间片钩子
 * 
 * 
 * 时间片钩子是一个函数，这个函数由用户来实现，
 * FreeRTOS规定了函数的名字和参数：void vApplicationTickHook(void )
 * 时间片中断可以周期性的调用
 * 函数必须非常短小，不能大量使用堆栈，
 * 不能调用以”FromISR" 或 "FROM_ISR”结尾的API函数
 */
 /*xTaskIncrementTick函数是在xPortSysTickHandler中断函数中被调用的。因此，vApplicationTickHook()函数执行的时间必须很短才行*/
#define configUSE_TICK_HOOK								0
//使用内存申请失败钩子函数
#define configUSE_MALLOC_FAILED_HOOK			0

/* Constants provided for debugging and optimisation assistance. */
/*
 * 大于0时启用堆栈溢出检测功能，如果使用此功能 
 * 用户必须提供一个栈溢出钩子函数，如果使用的话
 * 此值可以为1或者2，因为有两种栈溢出检测方法 */
#define configCHECK_FOR_STACK_OVERFLOW					0				//2-->0  wjj 	2022.7.13  configCHECK_FOR_STACK_OVERFLOW
#define configASSERT( x )								if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }
#define configQUEUE_REGISTRY_SIZE								10			//0->10 		/* 设置可以注册的信号量和消息队列个数,用以记录队列和信号量的最大数量 */

/* Software timer definitions. */
#define configUSE_TIMERS										0				 													//启用软件定时器
#define configTIMER_TASK_PRIORITY						( configMAX_PRIORITIES-1 )				//软件定时器优先级  3->  (configMAX_PRIORITIES-1)        
#define configTIMER_QUEUE_LENGTH						5																	//软件定时器队列长度,通过命令队列向软件定时器任务发送消息
#define configTIMER_TASK_STACK_DEPTH				( configMINIMAL_STACK_SIZE  )			//软件定时器任务堆栈大小

/* Set the following definitions to 1 to include the API function, or zero
 * to exclude the API function.  NOTE:  Setting an INCLUDE_ parameter to 0 is
 * only necessary if the linker does not automatically remove functions that are
 * not referenced anyway. */
#define INCLUDE_vTaskPrioritySet						1
#define INCLUDE_uxTaskPriorityGet						1
#define INCLUDE_vTaskDelete									1
#define INCLUDE_vTaskCleanUpResources				1
#define INCLUDE_vTaskSuspend								1
#define INCLUDE_vTaskDelayUntil							1
#define INCLUDE_vTaskDelay									1
#define INCLUDE_uxTaskGetStackHighWaterMark	0
#define INCLUDE_xTaskGetIdleTaskHandle			0
#define INCLUDE_eTaskGetState								1
#define INCLUDE_xTaskResumeFromISR					0			//no
#define INCLUDE_xTaskGetCurrentTaskHandle		1			//不能修改为0
#define INCLUDE_xTaskGetSchedulerState			1			//0-->1  wjj  2022.7.13
#define INCLUDE_xSemaphoreGetMutexHolder		0			
#define INCLUDE_xTimerPendFunctionCall			0			//1-->0

/* This demo makes use of one or more example stats formatting functions.  These
 * format the raw data provided by the uxTaskGetSystemState() function in to
 * human readable ASCII form.  See the notes in the implementation of vTaskList()
 * within FreeRTOS/Source/tasks.c for limitations. */
 
 /* 与宏configUSE_TRACE_FACILITY同时为1时会编译下面3个函数
 * prvWriteNameToBuffer()
 * vTaskList(),
 * vTaskGetRunTimeStats()
*/
#define configUSE_STATS_FORMATTING_FUNCTIONS			1

/* Dimensions a buffer that can be used by the FreeRTOS+CLI command interpreter.
 * See the FreeRTOS+CLI documentation for more information:
 * http://www.FreeRTOS.org/FreeRTOS-Plus/FreeRTOS_Plus_CLI/ */
#define configCOMMAND_INT_MAX_OUTPUT_SIZE				2048

/* Interrupt priority configuration follows...................... */

/* Use the system definition, if there is one. */
#ifdef __NVIC_PRIO_BITS
	#define configPRIO_BITS								__NVIC_PRIO_BITS
#else
	#define configPRIO_BITS								3	 /* 8 priority levels. */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
 * function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY			0x07			//中断最低优先级

/* The highest interrupt priority that can be used by any interrupt service
 * routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT
 * CALL INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A
 * HIGHER PRIORITY THAN THIS! (higher priorities are lower numeric values). */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	5			//系统可管理的最高中断优先级

/* Interrupt priorities used by the kernel port layer itself.  These are generic
 * to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY					( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )				//设置PendSV和SysTick优先级

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 * See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
//低于configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY优先级的可以安全调用FreeRTOS 的 API 函数
#define configMAX_SYSCALL_INTERRUPT_PRIORITY			( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << ( 8 - configPRIO_BITS ) )	

/* The #ifdef guards against the file being included from IAR assembly files. */
#ifndef __IASMARM__

	/* Constants related to the generation of run time stats. */
	#define configGENERATE_RUN_TIME_STATS				0											//启用运行时间统计功能,=1还需实现portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()和portGET_RUN_TIME_COUNTER_VALUE()。
	#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
	#define portGET_RUN_TIME_COUNTER_VALUE()			0
	#define configTICK_RATE_HZ							( ( TickType_t ) 1000 )			//1ms  //RTOS系统节拍中断的频率。即一秒中断的次数，每次中断RTOS都会进行任务调度  单位 Hz

#endif /* __IASMARM__ */

/* Enable static allocation. */
#define configSUPPORT_STATIC_ALLOCATION					0				//支持静态内存

#endif /* FREERTOS_CONFIG_H */
