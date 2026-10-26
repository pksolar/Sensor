#include "gpio.h"


// 配置MCO引脚，输出系统时钟SYSCLK到PA8
void MCO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // 1. 使能GPIOA时钟（MCO引脚通常为PA8）
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE); //GPIOA时钟初始化
    
    // 2. 配置PA8为复用推挽输出（MCO专用引脚）
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;  // 复用
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; //推挽输出
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 3. 配置MCO输出时钟源（选择要测量的时钟）
    RCC_MCOConfig(RCC_MCOSource_SYSCLK, RCC_MCOPrescaler_1);  // 输出系统时钟,不分频
}

void GpioConfig(void)
{
	 GPIO_InitTypeDef  GPIO_InitStruct;//定义GPIO结构体
  
   //初始化A5,A6,A7
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE); //GPIOA时钟初始化
	 RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE); //GPIOB时钟初始化
   GPIO_InitStruct.GPIO_Pin=GPIO_Pin_13 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_15 ; 
	
   GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz; //设置GPIO的管脚最大频率
   GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;    //设置GPIO的管脚为输出
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; //推挽输出	
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOA,&GPIO_InitStruct);          //GPIO A设置初始化 
	
	/****************2018.5.24  5:00*****************/
	 GPIO_InitStruct.GPIO_Pin=GPIO_Pin_1  ;//OUT  recieve  PA8-->PB1
   GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;    
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOB,&GPIO_InitStruct); 
	/***********************************************/
	
	 GPIO_InitStruct.GPIO_Pin=GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_0; //2018.5.24  5:00 PB0  OPEN POWER
	
   GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz; //设置GPIO的管脚最大频率
   GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;    //设置GPIO的管脚为输出
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; //推挽输出	
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOB,&GPIO_InitStruct);          //GPIO B设置初始化 
	 
	 
	 
	 
    GPIO_InitStruct.GPIO_Pin=GPIO_Pin_0; 
   GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AN;    //设置GPIO的管脚为输出
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOA,&GPIO_InitStruct);          //GPIO B设置初始化 
	
}
