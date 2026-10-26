#ifndef __SubFun_H
#define __SubFun_H

#include <hk32f0xxa_gpio.h>
#include "hk32f0xxa_conf.h"
#include "main.h"

typedef struct  
{
	int K_O2;	//纯氧气系数
	int B_O2;
	int Cn_O2;	//氧气浓度
} CalibrationCoefficient;

typedef struct  
{
	char SendOrder;                //发送顺序
	unsigned char cENAutoSendFlag; //自动发送标志位
	int nWarnVale[3];		       //报警数据
	CalibrationCoefficient coef;   //标定系数
} FlashData;

typedef struct  
{
	long  lFf;			  //
	long  lFb;			  //
	long  CountLong;	//进行计算的值	
	int   nTemp;								//温度 单位℃
	int   Result ;							//计算结果
	float Coefficient[2];				//计算系数
	int AmbientResult;         //环境压力
  int   PressResult;          //背压修正
  int   ReResult ;            //最终结果
} MyStructTrait5P;

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


#endif /* __SubFun_H */
