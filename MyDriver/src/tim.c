#include <stdio.h>
#include "hk32f0xxa_tim.h"
#include "hk32f0xxa_conf.h"
#include "tim.h"

#ifdef SYSCLK_HSE8M_PLL_FREQ_96MHz
	#define AUTO_RELOAD_VALUE 420 * 96 / 72 - 1
#else
	#define AUTO_RELOAD_VALUE 420 - 1
#endif

void TIM16Delay_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16,ENABLE);  ///使能TIM16时钟
	
    TIM_TimeBaseInitStructure.TIM_Period = AUTO_RELOAD_VALUE; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=0;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM16,&TIM_TimeBaseInitStructure);//初始化TIM16
	//TIM_Cmd(TIM16, ENABLE);
}
