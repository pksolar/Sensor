#pragma import(__use_no_semihosting_swi)  //关闭半主机模式，必须
#include <stdio.h>
#include <stdarg.h>
#include <hk32f0xxa_usart.h>
#include <hk32f0xxa_gpio.h>
#include <hk32f0xxa_misc.h>
#include "usart.h"

unsigned char UsartCmd = 0;
unsigned char rxBuffer[RXBUFSSIZE];
unsigned char rxCount = 0;
unsigned char rx232Flag = 0;

void UsartGpioConfig(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1 , ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);//GPIOA时钟初始化	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_1);		//PA9  USART1 TX
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_1);		//PA10 USART1 RX	
	
	//配置串口PA9 PA10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
}

void UsartConfig(int baudrate)
{
	USART_InitTypeDef USART_InitStructure;
	
  USART_InitStructure.USART_BaudRate =baudrate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure); 
	
	
	USART_ITConfig(USART1, USART_IT_PE, ENABLE);//开启PE错误接收中断
	USART_ITConfig(USART1, USART_IT_ERR, ENABLE);//CR2开启ERR中断
	
	USART_ClearFlag(USART1,USART_FLAG_TC);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//??????
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	USART_Cmd(USART1, ENABLE);
	USART_ClearFlag(USART1,USART_FLAG_TC);   //??TC_FLAG
	//USART1->SR;//?????????TC ?????? ???????	
	
}

void UsartNvicConfig(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;//??????
	
	//USART1?????????
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//???????1??
	NVIC_InitStructure.NVIC_IRQChannelPriority=0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;        //????
	NVIC_Init(&NVIC_InitStructure);                        //???	
}

void UsartInit(int baudrate)
{
	UsartGpioConfig();
	UsartConfig(baudrate);
	UsartNvicConfig();
}

void Usart1SendByte(unsigned char ByteData)
{
	USART_SendData(USART1,ByteData);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) != SET);//wait untill transmitt complete	
}

int fputc(int ch, FILE *_File)
{
	Usart1SendByte(ch);
	return (ch);
}

void USART1_IRQHandler(void)
{ 	
	unsigned char ushTemp;
	if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
	{//同  @arg USART_IT_ORE_ER : OverRun Error interrupt if the EIE bit is set  
		ushTemp = USART_ReceiveData(USART1); //取出来扔掉
		ushTemp = ushTemp;
		USART_ClearFlag(USART1, USART_FLAG_ORE);
	}

	if(USART_GetFlagStatus(USART1, USART_FLAG_NE) != RESET)
	{//同  @arg USART_IT_NE     : Noise Error interrupt
		USART_ClearFlag(USART1, USART_FLAG_NE);
	}


	if(USART_GetFlagStatus(USART1, USART_FLAG_FE) != RESET)
	{//同   @arg USART_IT_FE     : Framing Error interrupt
		USART_ClearFlag(USART1, USART_FLAG_FE);
	}

	if(USART_GetFlagStatus(USART1, USART_FLAG_PE) != RESET)
	{//同  @arg USART_IT_PE     : Parity Error interrupt
		USART_ClearFlag(USART1, USART_FLAG_PE);
	}	
	
	
   if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)    
   {
		UsartCmd=USART_ReceiveData(USART1);

		if(rx232Flag==0)
		{
			// printf("recv data\r\n");
			rxBuffer[rxCount]=UsartCmd;
			// printf("rxcount:%d, data:%x\r\n", rxCount, UsartCmd);
			
			if((rxBuffer[0]==0x11)) 	//"id"
			{
				if((rxCount>=3)&&(rxBuffer[1]==(rxCount-2)))
				{
					rx232Flag=1;
					rxCount=0;
					// USART_ClearITPendingBit(USART1,USART_IT_RXNE);        //清除串口接收中断
					return;
				} 
				rxCount++;
				if(rxCount>=RXBUFSSIZE)//????
				{
					rxCount=0;
				}
			}
			else
			{
				rxCount=0;
			}
		}
		
	}	 
 }
