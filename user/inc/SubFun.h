#ifndef USER_SUBFUN_H
#define USER_SUBFUN_H

#include <hk32f0xxa_gpio.h>
#include "hk32f0xxa_conf.h"
#include "app_types.h"

extern MyStructTrait5P  mysGasTrait5P[2];



extern int nT1CountLeft;
extern int nT1CountRight;	//T1计数

extern volatile unsigned char cIntiFlag;

#define PIN_LM393_OFF   	GPIO_SetBits(GPIOB, GPIO_Pin_0)
#define PIN_LM393_ON	 	GPIO_ResetBits(GPIOB, GPIO_Pin_0)

//BZ0
#define TX1_L()    GPIOA->BRR  = 0x0002      //SIN0    0x0040-->0x0002      
#define TX1_H()    GPIOA->BSRR = 0x0002      //PB6-->PA1

//BZ1
#define TX2_L()    GPIOA->BRR  =0x0004      //SIN1   0x4000-->0x0004
#define TX2_H()    GPIOA->BSRR =0x0004      //PA14-->PA2


//BZ2
#define TX3_L()    GPIOA->BRR  = 0x0100     //SIN3   0x2000-->0x0100
#define TX3_H()    GPIOA->BSRR = 0x0100     //PA13-->PA8

//BZ3
#define TX4_L()    GPIOA->BRR =0x0080      //SIN2   0x1000-->0x0080
#define TX4_H()    GPIOA->BSRR=0x0080      //PA12-->PA7

//GPIOA->ODR
//GPIOA->IDR


#define 	backward()  GPIOA->BRR  = 0x2000               //逆流  0x0800-->0x2000
#define   frontward()	GPIOA->BSRR = 0x2000               //顺流   PA11-->PA13

#define FRONTWARD    0
#define BACKWARD     1

#ifdef SYSCLK_HSE8M_PLL_FREQ_96MHz
	#define _Fx_OFFSET_Negative_   400			//72M  80*3.6  
	#define _Fx_OFFSET_Positive_   1600      //72M  400*3.6  通氧气
#else
	#define _Fx_OFFSET_Negative_   300			//72M  80*3.6  
	#define _Fx_OFFSET_Positive_   1200      //72M  400*3.6  通氧气
#endif

void FnRecover(FlashData *flash_data);
void fnRUNLED(void);
void StartCaiyangLeft(char tSendFlag);
void StartCaiyangRight(char tSendFlag);
int USFCountAirTemp(int InTempGas);
void USFScreen(SensorValue sensor_value, float nPress);
void AotuSetSendOrder(char *INcSendOrder, int InTempGas);
float FnTandRHToAIRO2(int nInTemp,int nInRH);
float FnPressureCalibration(float Pressure);
void Get_OutputFlag(unsigned char *flag);
void Set_OutputFlag(unsigned char flag);


#endif /* USER_SUBFUN_H */
