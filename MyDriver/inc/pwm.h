#ifndef __PWM_H_
#define __PWM_H_

extern void PwmInit(void);
//extern void SetPWM2Duty(unsigned short Duty);
/******************************************************************** 
*
* 名称:	PWMDAOutput
* 说明:	
* 功能: 更新脉冲宽度
* 调用:	
* 输入: 
* 返回值: 
***********************************************************************/	
extern void PWMDAOutput(unsigned char tDACH,int tData);
#endif
