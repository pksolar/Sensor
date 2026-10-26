/********************************************************************** 
*  Copyright (C), WuHan Cubic Optoelectronics
*  FileName:  .H
*  Author:shipingjing      Version :2.0     Date: 2006/06/09
*  Email:shipingjing@yahoo.com.cn	QQ:164658701
*  Description: 

*  Others:  

*  Function List:  
	1.
	2.

*  History:       
    1. Date:20060808
       Author:shipingjing
       Modification:建立
    2. 
***********************************************************************/



#ifndef _LED_MOD_H_
#define _LED_MOD_H_
	#ifndef uchar
		#define uchar unsigned char
	#endif
	#ifndef uint
		#define uint  unsigned int
	#endif
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	~                    PUBLIC VARIABLE 
	~           (公共的全局变量,可以供其它模块使用的(extern)变量)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	~                    FUNCTION PROTOTYPES
	~            (外部函数,可以供其它模块使用的(extern)函数)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//	void fnPinWarn0(uchar tFlag);
//	void fnPinWarn1(uchar tFlag);
//	void fnPinWarn2(uchar tFlag);
		
/******************************************************************** 
*
* 名称: 
* 说明: 
* 功能: 
* 调用:	
* 输入:
		IN_O2 氧气测试浓度
		INnWarnValeSet  三个设置的报警参数

* 返回值: 
********************************************************************/
#define TRUE 	1
#define FALSE 	0

//端口的影子内存
/*
PA15   REDLED
PB3    GREENLED
PB4    YELLOWLED
*/
	
#define PIN_LED_Y_ON  	 	GPIO_ResetBits(GPIOB, GPIO_Pin_4)
#define PIN_LED_Y_OFF	    GPIO_SetBits(GPIOB, GPIO_Pin_4)     

#define PIN_LED_G_ON  	 	GPIO_ResetBits(GPIOB, GPIO_Pin_3)
#define PIN_LED_G_OFF	    GPIO_SetBits(GPIOB, GPIO_Pin_3)          

#define PIN_LED_R_ON  	 	GPIO_ResetBits(GPIOA, GPIO_Pin_15)
#define PIN_LED_R_OFF	    GPIO_SetBits(GPIOA, GPIO_Pin_15)


void fnPinWarnLEDs(int IN_O2 ,int INnWarnValeSet[3]);
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	~                    PRIVATE VARIABLE 
	~                (私有变量,仅在MAIN中定义一次)
	~    (本模块中的一些参数的设置,在模块内部都使用的外部变量extern)	
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/	
	#ifdef	_MAIN_	//进行原型定义

	#endif	
#endif
