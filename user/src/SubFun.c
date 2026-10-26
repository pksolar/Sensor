#include "SubFun.h"
#include "cap.h"
#include "flash.h"
#include <hk32f0xxa_usart.h>
#include <hk32f0xxa_tim.h>
#include "Linearization.h"
#include "ResultCount.h"

static unsigned char  tempchar[4];

static int nT1BuffLeft[WAVE_NUM];     //进行滤波      add20131119
static int nT1BuffRight[WAVE_NUM];     //进行滤波      add20131119


/******************************************************************** 
*
* 名称:FnTandRHToH2O
* 说明:
* 功能: 计算当前温度下，当前相对湿度的H2O含量
* 调用: 
* 输入: (nTemp /100)=实际温度，
    (nInRH /100)=实际相对湿度，
       （fCnH2O/100) = 实际含量 %   一位有效小数
* 返回值: 
***********************************************************************/ 
//float O2_k=0;
//float O2_b=0;
//float Flow_k=0;
//float Flow_b=0;
static float FnTandRHToH2O(int nInTemp,int nInRH)
{
 float fCnH2O; //H2O含量
 float fCnH2OMax;
 float fTemp; 

 fTemp = nInTemp; 
 fTemp /= 100;
 
 fCnH2OMax = 0.0000008734* fTemp* fTemp* fTemp 
      - 0.0000013617* fTemp* fTemp
      + 0.0004784740* fTemp
      + 0.0068091716;
 fCnH2OMax = fCnH2OMax*1000;
 fCnH2O = nInRH * fCnH2OMax /1000;
 
 if(debugflag)
 {
  printf("T=%.2f,RH=%d,H2OMax=%.4f, CnH2O=%.4f\n",fTemp,nInRH,fCnH2OMax,fCnH2O);
 }
 
 return(fCnH2O);
}

/******************************************************************** 
*
* 名称:FnTandRHToAIRO2
* 说明:
* 功能: 计算当前温度下，当前相对湿度的H2O含量
* 调用: 
* 输入: (nTemp /100) =实际温度，如实际温度为25度，则调用时，需传入2500
				(nInRH /1000) =实际相对湿度(小数表示），如实际相对湿度为45%，则调用时，需传入450
       （fCnH2O/1000) = 实际水含量(小数表示），如实际水含量为10.7%，则计算出的fCnH2O为107
       （fCnAIRO2FromH2O) = 实际校正量，可以直接加到校正前的氧气浓度上
* 返回值: 
***********************************************************************/ 
float FnTandRHToAIRO2(int nInTemp,int nInRH)
{
 float fCnH2O;     //当前温度下的H2O含量，
// float fCnR32FromAir; //当前
 float fCnAIRO2FromH2O;
// float fVelocityH2O;
 float fK_H2OToAIRO2; //修正系数，0.00%，0.00%
 float fB_H2OToAIRO2; //修正系数，0.00%，0.00%  
// fCnH2O =  FnTandRHToH2O(nInTemp,nInRH)*1000.0;//当前温度下的H2O含量
// fVelocityH2O =  FnCnToVelocity(_AIR_,_H2O_,nInTemp,fCnH2O);    //
// fCnR32FromH2O = FnVelocityToCn(_AIR_,_R32_,nInTemp,fVelocityH2O);
// return(fCnR32FromH2O); 
 fK_H2OToAIRO2 = 1.3837;
	//fK_H2OToAIRO2 = 0.9507;
		//fK_H2OToAIRO2 = 1.1487;//重新拟合的系数
		//fK_H2OToAIRO2 = -0.2260919324;//石总给的原始版本
//   fB_H2OToAIRO2 = 0.0014835524 ;
	 fB_H2OToAIRO2 = -14.279 ;//重新拟合的系数
		 //fB_H2OToAIRO2 = -6.1389 ;//重新拟合的系数
   //fB_H2OToAIRO2 = 0 ;//石总给的原始版本
 
   fCnH2O =  FnTandRHToH2O(nInTemp,nInRH) ;//当前温度下的H2O含量 0.0% 
   fCnAIRO2FromH2O = fCnH2O * fK_H2OToAIRO2 + fB_H2OToAIRO2;
  if(debugflag)
  {
   printf("K=%.8f,B=%.3f,H2O=%.4f, δAIRO2=%.4f\n",fK_H2OToAIRO2,fB_H2OToAIRO2,fCnH2O,fCnAIRO2FromH2O);
  }
	if(fCnAIRO2FromH2O<0)
	{
		fCnAIRO2FromH2O = 0;
	}
	//fCnAIRO2FromH2O = 0;
 return(fCnAIRO2FromH2O); //0.1%
}

float FnPressureCalibration(float Pressure)
{
	float pressure=0;     //当前温度下的H2O含量，
	float presCalibration = 0;
	pressure = Pressure;
	presCalibration = 0.000000242*pressure*pressure*pressure-0.00055644*pressure*pressure+0.469487548*pressure-152.73390324;
	return(presCalibration); //0.1%
}

//******************************************************************** 
//*
//* 名称: FnRecover
//* 说明: 
//* 功能: 读取标定数据
//* 调用:	
//* 输入: 
//* 返回值: 
//*********************************************************************
void FnRecover(FlashData *flash_data)
{
    unsigned char i;
	flash_data->SendOrder = ReadEeprom(130);
	
   for(i=0;i<3;i++)
  	{
  		tempchar[0]  = ReadEeprom(131+i*2);
   		tempchar[1]  = ReadEeprom(132+i*2);
		flash_data->nWarnVale[i] = tempchar[1]*256+tempchar[0];
   	} 
//报警值设置   		
   	if((flash_data->nWarnVale[0] > 999)||(flash_data->nWarnVale[0] < 0))
   	{
	   	flash_data->nWarnVale[0] = 700;		//报警值0
		}	
   	if((flash_data->nWarnVale[1] > 999)||(flash_data->nWarnVale[1] < 0))
   	{
	   	flash_data->nWarnVale[1] = 400;		//报警值1
		}
		
   	if((flash_data->nWarnVale[2] > 999)||(flash_data->nWarnVale[2] < 0))
   	{
	   	flash_data->nWarnVale[2] = 500;
		}
		
	 flash_data->cENAutoSendFlag = ReadEeprom(139);	// ENAutoSendFlag 
		

	tempchar[0]  = ReadEeprom(140+0*2);
	tempchar[1]  = ReadEeprom(141+0*2);
    flash_data->coef.K_O2 = tempchar[1]*256+tempchar[0];

	tempchar[0]  = ReadEeprom(142+0*2);
	tempchar[1]  = ReadEeprom(143+0*2);
    flash_data->coef.B_O2 = tempchar[1]*256+tempchar[0];
		
	if((flash_data->coef.K_O2 > 2000)||(flash_data->coef.K_O2 < 0))	//默认系数
	{
		flash_data->coef.K_O2 =  1142;			//1.142
		flash_data->coef.B_O2 = -3417;			//34.17   0.1% 是实际氧气浓度*10倍
	}
}

//******************************************************************** 
//*
//* 名称: 
//* 说明: 
//* 功能: 
//* 调用:	
//* 输入: tFlag
//* 返回值: 
//*********************************************************************
void fnRUNLED(void)
{
static unsigned char cRunFlag;
	if(cRunFlag==0)
	{
		cRunFlag = 1;		
		GPIO_SetBits(GPIOB, GPIO_Pin_5);      //RNNLED
	}
	else
	{
		cRunFlag = 0;
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);      //RNNLED
	}
}

static void DelayTime(unsigned int tDelayTime)
{
	if(tDelayTime==0) return;	
	
	while(--tDelayTime);
}

static void TimerDelay12_5us(void)
{
	//TIM_SelectOnePulseMode(TIM16, TIM_OPMode_Single);
 /* 清除SR中的UIF标志 */
 //TIM_ClearFlag(TIM16, TIM_IT_Update);
	TIM16->SR = (uint16_t)~TIM_IT_Update;
 //TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
 //TIM_SetCounter(TIM16,0);
	TIM16->CNT = 0;
 /* 打开定时器6 */
 TIM16->CR1 |= TIM_CR1_CEN;
 /* 检测定时时间是否到来 */
 //while(TIM_GetFlagStatus(TIM16, TIM_IT_Update)==RESET);
	while((TIM16->SR&TIM_IT_Update)==0);  
 /* 软件清除更新标志 */
 //TIM_ClearFlag(TIM16, TIM_IT_Update);
	TIM16->SR = (uint16_t)~TIM_IT_Update;
	TIM16->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
 //TIM_Cmd(TIM16, DISABLE);
}

static void  modechange(unsigned char mode)
{
	GPIO_InitTypeDef  GPIO_InitStruct;//定义GPIO结构体
	if(mode==FRONTWARD) // bz1 send bz2 recieve
	{
		GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2 ;   //PA14-->PA2
		GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz; //设置GPIO的管脚最大频率
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;    //设置GPIO的管脚为输出
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; //推挽输出	
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA,&GPIO_InitStruct);  
		GPIO_InitStruct.GPIO_Pin=GPIO_Pin_1;  //PB6-->PA1  
		GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz; //设置GPIO的管脚最大频率
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;    //设置GPIO的管脚为输出
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; //推挽输出	
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA,&GPIO_InitStruct); 

		GPIO_InitStruct.GPIO_Pin= GPIO_Pin_8 ;  //PA13-->PA8
		GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz; //设置GPIO的管脚最大频率
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;    
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; //推挽输出	
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_Init(GPIOA,&GPIO_InitStruct);

		GPIO_ResetBits(GPIOA, GPIO_Pin_8);

		GPIO_InitStruct.GPIO_Pin=GPIO_Pin_7  ; //PA12-->PA7

		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;    
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA,&GPIO_InitStruct); 
	}
	if(mode==BACKWARD)//bz2 send bz1 recieve
	{
		GPIO_InitStruct.GPIO_Pin= GPIO_Pin_2;// t1 
		GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz; //设置GPIO的管脚最大频率
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;    
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; //推挽输出	
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA,&GPIO_InitStruct); 
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);	 // t1 gnd
		GPIO_InitStruct.GPIO_Pin=GPIO_Pin_1 ;// t0 recieve
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;    //设置GPIO的管脚为输入
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA,&GPIO_InitStruct);  

		GPIO_InitStruct.GPIO_Pin=GPIO_Pin_7 | GPIO_Pin_8;//
		GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz; //设置GPIO的管脚最大频率
		GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;    //设置GPIO的管脚为输出
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; //推挽输出	
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(GPIOA,&GPIO_InitStruct); 
	}
}

static void ReadTimeLeft(void)
{
	  unsigned char i;
    volatile unsigned int tnCapCnt[CAP_TIMES];	
//		if(OutputFlag)
//    {  
//      printf("L;"); 
//			for(i=0;i<CAP_TIMES;i++)
//			{
//				 printf("%d,",CapCnt[i]); 
//			}
//      printf(";"); 
//		}
		for(i=0;i<CAP_TIMES;i++)
		{
			tnCapCnt[i] = CapCnt[i];
		}		
		if((tnCapCnt[1]- tnCapCnt[0])>900)	//第一比较信号有问题
		{
			for(i=0;i<CAP_TIMES-1;i++)
			{
					tnCapCnt[i] = tnCapCnt[i+1];
			}	
		}
		if((tnCapCnt[1]- tnCapCnt[0])>900)	//第二比较信号有问题
		{
			for(i=0;i<CAP_TIMES-1;i++)
			{
					tnCapCnt[i] = tnCapCnt[i+1];
			}	
		}	
	  for(i=0;i<WAVE_NUM;i++)
		{
				 nT1BuffLeft[i]= (tnCapCnt[2*i] + tnCapCnt[2*i+1] )/2;
		}
}

static void ReadTimeRight(void)
{
	  unsigned char i;
    volatile unsigned int tnCapCnt[CAP_TIMES];	
//		if(OutputFlag)
//		{  
//			printf("R;"); 
//			for(i=0;i<CAP_TIMES;i++)
//			{
//				 printf("%d,",CapCnt[i]); 
//			}
//			printf(";"); 
//		}
		for(i=0;i<CAP_TIMES;i++)
		{
			tnCapCnt[i] = CapCnt[i];
		}		
		if((tnCapCnt[1]- tnCapCnt[0])>900)	//第一比较信号有问题
		{
			for(i=0;i<CAP_TIMES-1;i++)
			{
					tnCapCnt[i] = tnCapCnt[i+1];
			}	
		}
		if((tnCapCnt[1]- tnCapCnt[0])>900)	//第二比较信号有问题
		{
			for(i=0;i<CAP_TIMES-1;i++)
			{
					tnCapCnt[i] = tnCapCnt[i+1];
			}	
		}		
	  for(i=0;i<WAVE_NUM;i++)
		{
				 nT1BuffRight[i]=(tnCapCnt[2*i] + tnCapCnt[2*i+1] )/2;
		}		
}

void StartCaiyangLeft(char tSendFlag)	
{
//	  unsigned int i;
		modechange(FRONTWARD);
		DelayTime(6000);
		frontward();
		DelayTime(20000);
		SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;     //关滴答定时器中断
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//禁用USART中断
		TimerCountStart();
		__disable_irq();	//捕获定时器初始化
	  if(tSendFlag != 0 )	
		{
			TX1_L();
			TX2_L();				
			TX1_H();
			TX2_L();			
		  TimerDelay12_5us();  //1
	    TX1_L();
		  TX2_H();
		  //Delay12_5us();	//1
			TimerDelay12_5us();
	    TX1_H();
		  TX2_L();
		  TimerDelay12_5us();  //2
	    TX1_L();
		  TX2_H();
		  TimerDelay12_5us();  //2
	    TX1_H();
		  TX2_L();
		  TimerDelay12_5us();  //3
	    TX1_L();
		  TX2_H();			 
			TimerDelay12_5us();  //3			
			TX1_H();
			TX2_L();
		  TimerDelay12_5us();  //1
	    TX1_L();
		  TX2_H();
		  TimerDelay12_5us();	//1
	    TX1_H();
		  TX2_L();
		  TimerDelay12_5us();  //2
	    TX1_L();
		  TX2_H();
		  TimerDelay12_5us();  //2
	    TX1_H();
		  TX2_L();
		  TimerDelay12_5us();  //3
	    TX1_L();
		  TX2_H();			 
			TimerDelay12_5us();  //3		

	    TX1_L();
		  TX2_L();	
			TX3_L();
//			TX4_L();				
		}			
	  else
		{
			TX1_L();
			TX2_L();
			
			TX2_H();
			TX1_L();
			TimerDelay12_5us();  //1
			TX2_L();
			TX1_H();
			TimerDelay12_5us();	//1
			TX2_H();
			TX1_L();
			TimerDelay12_5us();	//2
			TX2_L();
			TX1_H();
			TimerDelay12_5us();  //2
			TX2_H();
			TX1_L();
			TimerDelay12_5us();  //3
			TX2_L();
			TX1_H();
			TimerDelay12_5us();  //3
			TX2_H();
			TX1_L();
			TimerDelay12_5us();  //4
			TX2_L();
			TX1_H();
			TimerDelay12_5us();  //4
			TX2_H();
			TX1_L();
			TimerDelay12_5us();  //5
			TX2_L();
			TX1_H();
			TimerDelay12_5us();  //5
			TX2_H();
			TX1_L();
			TimerDelay12_5us();  //6
			TX2_L();
			TX1_H();
			TimerDelay12_5us();  //6				

	    TX1_L();
		  TX2_L();	 
			TX3_L();
//			TX4_L();	//一端接地
		}	
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//??????
		__enable_irq();
		TIM14->CR1 |= TIM_CR1_CEN;	                   //开捕获定时器时钟
		TIM_ClearFlag(TIM14, TIM_FLAG_Update|TIM_FLAG_CC1);		//清除溢出中断标志 和捕获中断标志
		TIM_ITConfig(TIM14,TIM_IT_Update|TIM_IT_CC1,ENABLE);	//允许更新中断和捕获中断		
	  while(!cIntiFlag);
		TX1_L();
		TX2_L();	 
	  TX3_L();
    TX4_L();	//一端接地
		TIM_ClearFlag(TIM14, TIM_FLAG_Update|TIM_FLAG_CC1);		//清除溢出中断标志 和捕获中断标志
		TIM_ITConfig(TIM14,TIM_IT_Update|TIM_IT_CC1,DISABLE);	//允许更新中断和捕获中断		
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;	   //开滴答定时器中断		
	  ReadTimeLeft();
		
}

void StartCaiyangRight(char tSendFlag)	
{
//	unsigned int i;
		modechange(BACKWARD);
		DelayTime(6000);	 
		backward();
		DelayTime(20000);
		SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;     //关滴答定时器中断
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);//??????
		TimerCountStart();                              //捕获定时器初始化     
		__disable_irq();
		if(tSendFlag != 0 )	
		{
			TX3_L();
			TX4_L();
			
			TX4_H();
			TX3_L();
			TimerDelay12_5us(); //1
			TX4_L();
			TX3_H();
			TimerDelay12_5us(); //1
			TX4_H();
			TX3_L();
			TimerDelay12_5us(); //2
			TX4_L();
			TX3_H();
			TimerDelay12_5us(); //2
			TX4_H();
			TX3_L();
			TimerDelay12_5us(); //3
			TX4_L();
			TX3_H();
			TimerDelay12_5us(); //3
			TX4_H();
			TX3_L();
			TimerDelay12_5us(); //1
			TX4_L();
			TX3_H();
			TimerDelay12_5us(); //1
			TX4_H();
			TX3_L();
			TimerDelay12_5us(); //2
			TX4_L();
			TX3_H();
			TimerDelay12_5us(); //2
			TX4_H();
			TX3_L();
			TimerDelay12_5us(); //3
			TX4_L();
			TX3_H();
			TimerDelay12_5us(); //3			
	    TX1_L();
		  //TX2_L();			//一端接地
			TX3_L();
			TX4_L();	
		}	
	  else
		{
			TX3_L();
			TX4_L();
			
			TX3_H();
			TX4_L();			
		  TimerDelay12_5us();  //1
	    TX3_L();
		  TX4_H();
		  TimerDelay12_5us();	//1
	    TX3_H();
		  TX4_L();
		  TimerDelay12_5us();  //2
	    TX3_L();
		  TX4_H();
		  TimerDelay12_5us();  //2
	    TX3_H();
		  TX4_L();
		  TimerDelay12_5us();  //3
	    TX3_L();
		  TX4_H();			 
			TimerDelay12_5us();  //3
			TX3_H();
			TX4_L();			
		  TimerDelay12_5us();  //1
	    TX3_L();
		  TX4_H();
		  TimerDelay12_5us();	//1
	    TX3_H();
		  TX4_L();
		  TimerDelay12_5us();  //2
	    TX3_L();
		  TX4_H();
		  TimerDelay12_5us();  //2
	    TX3_H();
		  TX4_L();
		  TimerDelay12_5us();  //3
	    TX3_L();
		  TX4_H();			 
			TimerDelay12_5us();  //3
			
	    TX1_L();
		  //TX2_L();	
			TX3_L();
			TX4_L();			
		}	
			USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//??????
		__enable_irq();
		TIM14->CR1 |= TIM_CR1_CEN;	                   //开捕获定时器时钟
		TIM_ClearFlag(TIM14, TIM_FLAG_Update|TIM_FLAG_CC1);		//清除溢出中断标志 和捕获中断标志
		TIM_ITConfig(TIM14,TIM_IT_Update|TIM_IT_CC1,ENABLE);	//允许更新中断和捕获中断
	  while(!cIntiFlag);
			TX1_L();
		  TX2_L();	
			TX3_L();
			TX4_L();	
		TIM_ClearFlag(TIM14, TIM_FLAG_Update|TIM_FLAG_CC1);		//清除溢出中断标志 和捕获中断标志
		TIM_ITConfig(TIM14,TIM_IT_Update|TIM_IT_CC1,DISABLE);	//允许更新中断和捕获中断
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;	   //开滴答定时器中断
		ReadTimeRight();
}

int USFCountAirTemp(int InTempGas)
{
	#ifdef SYSCLK_HSE8M_PLL_FREQ_96MHz
		return (int)((-0.2686)*InTempGas + 33685);
	#else
		return (int)((-0.2015)*InTempGas + 25264);  //-1200
	#endif
}

int nT1CountLeft;
int nT1CountRight;	//T1计数

static unsigned char  OutputFlag = 0;

void Get_OutputFlag(unsigned char *flag)
{
	*flag = OutputFlag;
}

void Set_OutputFlag(unsigned char flag)
{
	OutputFlag = flag;
}

MyStructTrait5P  mysGasTrait5P[2]; 

void USFScreen(SensorValue sensor_value, float nPress)
{
 //20161031 2.605T
	unsigned int   USFCountAir ;   
	int tempint;
	unsigned char i,j,k;
	
	  USFCountAir = USFCountAirTemp(sensor_value.temperature_NTC) + nFxdFair;			//CHG20161101 7500b 
    if(OutputFlag)
    {  

        printf("\nO:%d,%d,%d; F:%d,%d,%d; ",mysGasTrait5P[0].Result, mysGasTrait5P[0].PressResult, mysGasTrait5P[0].ReResult, mysGasTrait5P[1].Result, mysGasTrait5P[1].AmbientResult, mysGasTrait5P[1].ReResult);
		printf("P_In:%.1f,%.3f; P_Out:%.1f,%.3f; ",sensor_value.press_in, 0.0f, sensor_value.press_out, 0.0f);
		printf("T:%d; H:%d; ", sensor_value.temperature_NTC, sensor_value.nRH);
        printf("%d,%d; ",nFxdFair,USFCountAir); 
        printf("%d,%d; ",nT1CountLeft,nT1CountRight);
			
        for(i=0;i<WAVE_NUM;i++)
        {        		
           	printf("%d,",nT1BuffLeft[i]);
        }    
		    printf("; " ); 
        for(i=0;i<WAVE_NUM;i++)
        {        		
           	printf("%d,",nT1BuffRight[i]);
        }
				printf("Range;%d,%d;",USFCountAir - _Fx_OFFSET_Negative_,USFCountAir + _Fx_OFFSET_Positive_); 

    }
		
		
//		nT1CountLeft  = nT1BuffLeft[2];		//否则不更新数据		
//	  nT1CountRight = nT1BuffRight[2];	//否则不更新数据		
 //=================队列对齐===========================  
    for(i=0;i<WAVE_NUM;i++)
    {  
        for(j=0;j<WAVE_NUM;j++)
        {
            tempint = nT1BuffLeft[j]- nT1BuffRight[i];
					
            if((tempint< (260*72.0/20.0))    //根据流量量程定
             &&(tempint>(-(220*72.0/20.0))))      //队列对齐
            {
                for(k=0;k<WAVE_NUM;k++)
                {
										if(k+j >= WAVE_NUM) break;		//限制
										if(k+i >= WAVE_NUM) break;
									
                    tempint = nT1BuffLeft[k+j]+ nT1BuffRight[k+i];
                    tempint = tempint/2;    //去掉流量影响

                    if((tempint > USFCountAir - _Fx_OFFSET_Negative_ )
                     &&(tempint < USFCountAir + _Fx_OFFSET_Positive_))
                    {
                        nT1CountLeft  = nT1BuffLeft [k+j];
                        nT1CountRight = nT1BuffRight[k+i];
												//调试输出
												if(OutputFlag)
												{  
														printf("R;%d,%d;",nT1CountLeft,nT1CountRight); 
												}
                        return;
                    }
                }
            }
        }
    } 
}

void AotuSetSendOrder(char *INcSendOrder, int InTempGas)
{
	int i;
	int	tnBuffer_0[5];
	int	tnBuffer_1[5];	
	int	tnOrder_0;
	int	tnOrder_1;	
	unsigned int   USFCountAir ; 

	if((*INcSendOrder==0)||(*INcSendOrder==1))	//进行过顺序判断
		return;
	
	//触发时序影响结果，一般两次测试数据中 较大的是需要的波形。第一个波为反向
	for(i=0;i<5;i++)
	{
		StartCaiyangLeft(0);

//		tnBuffer_0[i] = CapCnt[2];
			tnBuffer_0[i]= (CapCnt[(2)*2] + CapCnt[(2)*2+1] )/2;		//ADD20190103	
		
//		printf("t0:%d,%d,%d,%d,%d,\n",CapCnt[0],CapCnt[1],CapCnt[2],CapCnt[3],CapCnt[4]);
	}		
	tnOrder_0 =  nOrderBuffer_5(tnBuffer_0);
	
	for(i=0;i<5;i++)
	{
		StartCaiyangLeft(1);
//		tnBuffer_1[i] = CapCnt[2];	
		
			tnBuffer_1[i]= (CapCnt[(2)*2] + CapCnt[(2)*2+1] )/2;		//ADD20190103	
		
//		printf("t1:%d,%d,%d,%d,%d,\n",CapCnt[0],CapCnt[1],CapCnt[2],CapCnt[3],CapCnt[4]);
		
	}		
	tnOrder_1 =  nOrderBuffer_5(tnBuffer_1);	
	
		
	//nTempGas=ReadTempTask();
////////	Sensor_ReadTemperature(READ_T,&nTempGas);
////////	nTempGas = nTempGas+46;
////////	if(nTempGas>10000)
////////	{
////////		nTempGas=3000;
////////	}
//  USFCountAir = USFCountAirTemp(nTempGas) + nFxdFair;			//ADD 20170818 7500b 	
	  USFCountAir = USFCountAirTemp(InTempGas) ;			//ADD 20170818 7500b 	
	
	tnOrder_0 = tnOrder_0 - USFCountAir;
	tnOrder_1 = tnOrder_1 - USFCountAir;
	
	if(tnOrder_0 < 0) tnOrder_0 = -tnOrder_0;	//ABS()
	if(tnOrder_1 < 0) tnOrder_1 = -tnOrder_1;	//ABS()
	
	*INcSendOrder = 1;
	if(tnOrder_0 <	tnOrder_1) *INcSendOrder = 0; // 绝对值小的是需要结果
	
//=======存储========
	ReadFlashAll();
	WriteEeprom(130, *INcSendOrder);	
	WriteFlashAll();		
}
