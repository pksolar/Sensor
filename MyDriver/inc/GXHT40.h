#ifndef __GXHT40_H
#define __GXHT40_H

#include "IIC_GXHT40.h"


/* 芯片及寄存器地址 */
#define GXHT40_Device_Adderss      0x44    //芯片地址


/* 配置指令 */
#define GXHT40_Rst                 0x94		//软复位
#define GXHT40_High_Dup_Measure    0xFD		//高重复率测量
#define GXHT40_Mid_Dup_Measure     0xF6		//中重复率测量
#define GXHT40_Low_Dup_Measure     0xE0		//低重复率测量

#define GXHT40_HIGH 0
#define GXHT40_MID 1
#define GXHT40_LOW 2

void GXHT40_Init(void);
void GXHT40_ReadT_RH(int *pTBuffer,int *pRHBuffer);



#endif /* __GXHT4_H */
