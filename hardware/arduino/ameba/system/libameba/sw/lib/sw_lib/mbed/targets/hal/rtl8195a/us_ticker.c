/* mbed Microcontroller Library
 *******************************************************************************
 * Copyright (c) 2014, Realtek Semiconductor Corp.
 * All rights reserved.
 *
 * This module is a confidential and proprietary property of RealTek and
 * possession or use of this module requires written permission of RealTek.
 *******************************************************************************
 */
#include "objects.h"
#include <stddef.h>

#include "cmsis_os.h"
#include "rt_time.h"

#include "PeripheralNames.h"
#include "section_config.h"

#include "us_ticker_api.h"


#define OS_CLOCK       (200000000UL/6*5)
#define OS_TICK 	   1000
#define OS_TRV          ((uint32_t)(((double)OS_CLOCK*(double)OS_TICK)/1E6)-1)
#define NVIC_ST_CTRL    (*((volatile U32 *)0xE000E010))
#define NVIC_ST_RELOAD  (*((volatile U32 *)0xE000E014))
#define NVIC_ST_CURRENT (*((volatile U32 *)0xE000E018))


#define SYS_TIM_ID      1   // the G-Timer ID for System
#define OS_TIM_ID       2   // the G-Timer ID for Application
#define MBED_TIM_ID      3   // the G-Timer ID for Application

static int us_ticker_inited = 0;
static TIMER_ADAPTER TimerAdapter;


extern HAL_TIMER_OP HalTimerOp;

IMAGE2_TEXT_SECTION
VOID _us_ticker_irq_handler(IN  VOID *Data)
{
    us_ticker_irq_handler();
}

IMAGE2_TEXT_SECTION
void us_ticker_init(void) 
{
    if (us_ticker_inited) return;
    us_ticker_inited = 1;

    // Initial a G-Timer
    TimerAdapter.IrqDis = 1;    // Disable Irq
    TimerAdapter.IrqHandle.IrqFun = (IRQ_FUN) _us_ticker_irq_handler;
    TimerAdapter.IrqHandle.IrqNum = TIMER2_7_IRQ;
    TimerAdapter.IrqHandle.Priority = 0x20;
    TimerAdapter.IrqHandle.Data = (u32)NULL;
    TimerAdapter.TimerId = MBED_TIM_ID;
    TimerAdapter.TimerIrqPriority = 0;
    TimerAdapter.TimerLoadValueUs = 1;
    TimerAdapter.TimerMode = FREE_RUN_MODE; // Countdown Free Run

    HalTimerOp.HalTimerInit((VOID*) &TimerAdapter);

}

IMAGE2_TEXT_SECTION
uint32_t us_ticker_read() 
{
    uint32_t tick_cnt;
    uint32_t us_tick, ms;
	static uint32_t last_us_tick=0;
	//
	// all the timeline shifts delta_time, so doesn't need to calculate
	//const static uint32_t delta_time = (20 * 1000000) / OS_CLOCK; // suppose additinal 20 cycles overhead


	//NeoJou
	#if 1
	ms = rt_time_get();
	us_tick = (uint32_t)(ms*1000);
	
	tick_cnt = OS_TRV - NVIC_ST_CURRENT;
	us_tick += (uint32_t)((tick_cnt*1000)/(OS_TRV+1) );
		
	if ( (last_us_tick > us_tick) && (last_us_tick < 0xFFFFFC00) ) {
		us_tick += 1000;
	}
	last_us_tick = us_tick;
	return us_tick;
	
	#else
    //1 Our G-timer  resolution is ~31 us (1/32K), and is a countdown timer
    if (!us_ticker_inited) 
        us_ticker_init();
	
    tick_cnt =  HalTimerOp.HalTimerReadCount(SYS_TIM_ID);
    us_tick = (0xffffffff - tick_cnt)*TIMER_TICK_US;
    // TODO: handle overflow
    return (us_tick);
	#endif
}

IMAGE2_TEXT_SECTION
void us_ticker_set_interrupt(timestamp_t timestamp) 
{
    uint32_t cur_time_us;
    uint32_t time_def;

    cur_time_us = us_ticker_read();
    if ((uint32_t)timestamp >= cur_time_us) {
        time_def = (uint32_t)timestamp - cur_time_us;
    }
    else {
//        time_def = 0xffffffff - cur_time_us + (uint32_t)timestamp;
	   time_def = TIMER_TICK_US;
    }    

    if (time_def < TIMER_TICK_US) {
        time_def = TIMER_TICK_US;       // at least 1 tick
    }

    TimerAdapter.IrqDis = 0;    // Enable Irq
    TimerAdapter.TimerLoadValueUs = time_def;
    TimerAdapter.TimerMode = USER_DEFINED; // Countdown Free Run
    
	InterruptUnRegister(&TimerAdapter.IrqHandle);
    HalTimerOp.HalTimerInit((VOID*) &TimerAdapter);
}

IMAGE2_TEXT_SECTION
void us_ticker_disable_interrupt(void) 
{
    HalTimerOp.HalTimerDis((u32)TimerAdapter.TimerId);
}

IMAGE2_TEXT_SECTION
void us_ticker_clear_interrupt(void) 
{
    HalTimerOp.HalTimerIrqClear((u32)TimerAdapter.TimerId);
}
