#include "pwm.h"
#include <stdio.h>
#include <hk32f0xxa_tim.h>
#include <hk32f0xxa_gpio.h>

static unsigned short TimerPeriod;

/*
PA6    O2PWM
PA3    FLOPWM
*/

void PwmGpioConfig(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;			
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE); 
	
	//PA7
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6|GPIO_Pin_3 ;    //PA7-->PA3      PA3对应TIM2通道4
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_1);		//PA6 PWM   AF1-->AF2 复用冲突
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_2);		//PA7 PWM	 PA7-->PA3	
}

void TIM2_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;
	
	//unsigned short Channel1Pulse,Channel2Pulse,Channel3Pulse,Channel4Pulse;
	unsigned short Channel2Pulse;
 /* Compute the value to be set in ARR regiter to generate signal frequency at 1 Khz */
  TimerPeriod = (SystemCoreClock / 1000 ) - 1;
  /* Compute CCR1 value to generate a duty cycle at 50% for channel 1 */
 // Channel1Pulse = (uint16_t) (((uint32_t) 5 * (TimerPeriod - 1)) / 10);
  /* Compute CCR2 value to generate a duty cycle at 50.0%  for channel 2 */
  Channel2Pulse = (uint16_t) (((uint32_t) 400 * (TimerPeriod - 1)) / 1000);
  /* Compute CCR3 value to generate a duty cycle at 25%  for channel 3 */
  //Channel3Pulse = (uint16_t) (((uint32_t) 25 * (TimerPeriod - 1)) / 100);
  /* Compute CCR4 value to generate a duty cycle at 12.5%  for channel 4 */
 // Channel4Pulse = (uint16_t) (((uint32_t) 125 * (TimerPeriod- 1)) / 1000);

  /* TIM1 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
  
  /* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  /* Channel 1, 2, 3 and 4 Configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  //TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
  //TIM_OC1Init(TIM2, &TIM_OCInitStructure);

  TIM_OCInitStructure.TIM_Pulse = Channel2Pulse;
  // TIM_OC2Init(TIM2, &TIM_OCInitStructure);            //PA7   
  // TIM_OC1Init(TIM2, &TIM_OCInitStructure);            //PA6
  //TIM_OCInitStructure.TIM_Pulse = Channel3Pulse;
  //TIM_OC3Init(TIM2, &TIM_OCInitStructure);

  //TIM_OCInitStructure.TIM_Pulse = Channel4Pulse;
  TIM_OC4Init(TIM2, &TIM_OCInitStructure);     //PA3

  /* TIM1 counter enable */
  TIM_Cmd(TIM2, ENABLE);

  /* TIM1 Main Output Enable */
  TIM_CtrlPWMOutputs(TIM2, ENABLE);	
		
}

void TIM3_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;
	
	//unsigned short Channel1Pulse,Channel2Pulse,Channel3Pulse,Channel4Pulse;
	unsigned short Channel2Pulse;
 /* Compute the value to be set in ARR regiter to generate signal frequency at 1 Khz */
  TimerPeriod = (SystemCoreClock / 1000 ) - 1;
  /* Compute CCR1 value to generate a duty cycle at 50% for channel 1 */
 // Channel1Pulse = (uint16_t) (((uint32_t) 5 * (TimerPeriod - 1)) / 10);
  /* Compute CCR2 value to generate a duty cycle at 50.0%  for channel 2 */
  Channel2Pulse = (uint16_t) (((uint32_t) 400 * (TimerPeriod - 1)) / 1000);
  /* Compute CCR3 value to generate a duty cycle at 25%  for channel 3 */
  //Channel3Pulse = (uint16_t) (((uint32_t) 25 * (TimerPeriod - 1)) / 100);
  /* Compute CCR4 value to generate a duty cycle at 12.5%  for channel 4 */
 // Channel4Pulse = (uint16_t) (((uint32_t) 125 * (TimerPeriod- 1)) / 1000);

  /* TIM1 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
  
  /* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* Channel 1, 2, 3 and 4 Configuration in PWM mode */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  //TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
  //TIM_OC1Init(TIM3, &TIM_OCInitStructure);

  TIM_OCInitStructure.TIM_Pulse = Channel2Pulse;
  // TIM_OC2Init(TIM3, &TIM_OCInitStructure);            //PA7   
  TIM_OC1Init(TIM3, &TIM_OCInitStructure);            //PA6
  //TIM_OCInitStructure.TIM_Pulse = Channel3Pulse;
  //TIM_OC3Init(TIM3, &TIM_OCInitStructure);

  //TIM_OCInitStructure.TIM_Pulse = Channel4Pulse;
  //TIM_OC4Init(TIM3, &TIM_OCInitStructure);

  /* TIM1 counter enable */
  TIM_Cmd(TIM3, ENABLE);

  /* TIM1 Main Output Enable */
  TIM_CtrlPWMOutputs(TIM3, ENABLE);	
		
}

void PwmInit(void)
{
	PwmGpioConfig();
  TIM2_PWM_Init();
	TIM3_PWM_Init();	
}

//设置FLOPWM
//Duty为占空比 放大了10倍 范围0-1000
void SetPWM2Duty(unsigned short Duty)
{
	unsigned short TempDuty;
	TempDuty = (uint16_t) (((uint32_t) Duty * (TimerPeriod - 1)) / 1000);
	TIM_SetCompare4(TIM2,TempDuty);	
}

//设置O2PWM
//Duty为占空比 放大了10倍 范围0-1000
void SetPWM1Duty(unsigned short Duty)
{
	unsigned short TempDuty;
	TempDuty = (uint16_t) (((uint32_t) Duty * (TimerPeriod - 1)) / 1000);
	TIM_SetCompare1(TIM3,TempDuty);	
}

/******************************************************************** 
*
* 名称:	PWMDAOutput
* 说明:	
* 功能: 更新脉冲宽度
* 调用:	
* 输入: tDACH == 0     O2PWM
        tDACH == 1     FLOPWM
   
* 返回值: 
***********************************************************************/	
void PWMDAOutput(unsigned char tDACH,int tData)
{
	float tempfloat;
	unsigned int SpanData;   //3.3v-1000   2.5v - 758
	
	SpanData = 764;
	
	if(tData>1000)   tData = 1000;
	else if(tData<0) tData = 0;	
	
	tempfloat = tData;
	
	tData = (int) (tempfloat* SpanData /1000.0 +0.5);
	
//	printf("AD:%d",tData);   //debug
	if(tDACH == 0)
	  SetPWM1Duty(tData);   //O2PWM 
	else
		SetPWM2Duty(tData);   //FLOPWM	
	
}
