#ifndef __CAP_H_
#define __CAP_H_

#include "hk32f0xxa_conf.h"

extern void CapInit(void);
extern void CapTime(void);					//捕获时间计算
extern void TimerCountStart(void);	//定时器计数开始

#ifdef SYSCLK_HSE8M_PLL_FREQ_96MHz
    #define MIN_WAVE_VALUE 24000
#else
    #define MIN_WAVE_VALUE 19000 //最低抓波值 原值：19000
#endif 

#define WAVE_NUM 10  //抓波数量
#define CAP_TIMES  (2 * WAVE_NUM + 4)  //捕获次数
extern volatile unsigned int CapCnt[CAP_TIMES];

#endif
