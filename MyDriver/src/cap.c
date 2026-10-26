#include "cap.h"
#include <stdio.h>
#include <hk32f0xxa_tim.h>
#include <hk32f0xxa_gpio.h>

//#define CAP_TIMES  10U  //捕获次数


static unsigned int OverFlowCnt=0;
static unsigned char CapState=0;
//static unsigned char cIntiFlag=0;     
volatile unsigned char cIntiFlag=0;                 //去掉static修饰，在subfun.h中有引用

//unsigned char cIntiCount;	//中断次数
//unsigned char cIntiFlag;	//

volatile unsigned int CapCnt[CAP_TIMES]={0};
//unsigned int CapCnt_Debug[CAP_TIMES]={0};

void CapGpioConfig(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;			
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE); 
	//PA4 输入捕获
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 ;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource4,GPIO_AF_4);		//PA4 输入捕获  GPIO_AF_4 ：TIM14
}

void CapTimerInit(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM14_ICInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	   
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14,ENABLE);    //使能TIM14时钟
     
	  TIM_DeInit(TIM14);
    //初始化定时器14
    TIM_TimeBaseStructure.TIM_Period = 0XFFFF; //设定计数器自动重装值
    TIM_TimeBaseStructure.TIM_Prescaler = 0; //预分频器 
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
    TIM_TimeBaseInit(TIM14,&TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
            
    // 初始化TIM14输入捕获参数 
    TIM14_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01     选择输入端 IC1映射到TI1上   
//    TIM14_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising; //上升沿捕获
//	   TIM14_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling; //下降沿捕获
	  TIM14_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_BothEdge  ; //上升下降沿捕获     
	
	
    TIM14_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
    TIM14_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1; //配置输入分频,不分频
    TIM14_ICInitStructure.TIM_ICFilter = 0; //IC1F=0000 配置输入滤波器 不滤波
    TIM_ICInit(TIM14,&TIM14_ICInitStructure); 

//		中断分组初始化		
		NVIC_InitStructure.NVIC_IRQChannel=TIM14_IRQn;  
		NVIC_InitStructure.NVIC_IRQChannelPriority=0;  
		NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;  
		NVIC_Init(&NVIC_InitStructure); 	
    
		TIM_ClearFlag(TIM14, TIM_FLAG_Update|TIM_FLAG_CC1);//清除溢出中断标志 和捕获中断标志
    TIM_ITConfig(TIM14,TIM_IT_Update|TIM_IT_CC1,DISABLE);//允许更新中断和捕获中断
		  
		TIM_SetCounter(TIM14,0);   // 清零CNT
		//TIM_Cmd(TIM14,ENABLE); //使能定时器14
		TIM_Cmd(TIM14,DISABLE); //定时器关闭
			
}

void   CapInit(void)
{
	CapGpioConfig();
	CapTimerInit();
}

void TimerCountStart(void)//定时器计数开始
{
	unsigned char i;
	
		OverFlowCnt=0;
		CapState=0;	 
		cIntiFlag = 0;
	
		for(i=0;i<CAP_TIMES;i++)
		{
			CapCnt[i]=0;	
			//CapCnt_Debug[i]=0;
		}

	
		TIM_SetCounter(TIM14,0);   // 清零CNT
//	  printf("CNT=%d \r\n",TIM_GetCounter(TIM14));	
		TIM_ClearFlag(TIM14, TIM_FLAG_Update|TIM_FLAG_CC1);		//清除溢出中断标志 和捕获中断标志
		TIM_ITConfig(TIM14,TIM_IT_Update|TIM_IT_CC1,DISABLE);	//允许更新中断和捕获中断
		TIM_Cmd(TIM14,ENABLE); //使能定时器14		

}

void TIM14_IRQHandler(void)
{
		if (TIM_GetITStatus(TIM14, TIM_IT_Update) != RESET)	//溢出中断	 
		{	
			TIM_ITConfig(TIM14,TIM_IT_Update|TIM_IT_CC1,DISABLE);		//禁止更新中断和捕获中断
			TIM_Cmd(TIM14,DISABLE); 																//定时器关闭
			
			TIM_SetCounter(TIM14,0);   // 清零CNT
			OverFlowCnt++;
			
			cIntiFlag = 1; 		//停止数据采集 
		}
		if(TIM_GetITStatus(TIM14, TIM_IT_CC1) != RESET)  //中断
		{	
    		CapCnt[CapState]=TIM_GetCapture1(TIM14)+(OverFlowCnt<<16);	
			  //CapCnt_Debug[CapState]=TIM_GetCapture1(TIM14);	
				if(CapCnt[CapState]>MIN_WAVE_VALUE)
				{
					CapState++;
					if(CapState>=CAP_TIMES)
					{					
						TIM_ITConfig(TIM14,TIM_IT_Update|TIM_IT_CC1,DISABLE);		//允许更新中断和捕获中断
						TIM_Cmd(TIM14,DISABLE); 																//定时器关闭
						//CapState=0;
						cIntiFlag = 1; 		//停止数据采集
						cIntiFlag = cIntiFlag;
					}
				}
		}			     	    					   	 
    TIM_ClearITPendingBit(TIM14, TIM_IT_CC1|TIM_IT_Update); 			//清除中断标志位 
}

void CapTime(void)//捕获时间计算
{
	unsigned char i;
//	if(CapState<CAP_TIMES) 		return;
//	else 											CapState=0;
	
//	printf("CapCnt=");

	for(i=0;i<CAP_TIMES;i++)
	{
		printf("%d, ",CapCnt[i]);	
	}
	printf(";");	
  for(i=0;i<CAP_TIMES;i++)
	{
		//printf("%d, ",CapCnt_Debug[i]);	
	}
		printf("\n");	
}
