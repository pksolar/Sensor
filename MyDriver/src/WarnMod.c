#include "WarnMod.h"
#include <hk32f0xxa_gpio.h>

//				fnPinWarn2(TRUE );	//黄
//				fnPinWarn1(FALSE);	//绿
//				fnPinWarn0(FALSE);	//红

static void fnPinWarn0(uchar tFlag)
{
	if(tFlag == 0)	//低电平报警
	{
		PIN_LED_R_OFF ;
	}
	else
	{
		PIN_LED_R_ON;
	}	
}

static void fnPinWarn1(uchar tFlag)
{
	if(tFlag == 0)	//低电平报警
	{
		PIN_LED_G_OFF ;
	}
	else
	{
		PIN_LED_G_ON;
	}	
}

static void fnPinWarn2(uchar tFlag)
{
	if(tFlag == 0)	//低电平报警
	{
		PIN_LED_Y_OFF ;
	}
	else
	{
		PIN_LED_Y_ON;
	}	
}

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
void fnPinWarnLEDs(int IN_O2 ,int INnWarnValeSet[3])
{
	
static uchar cWarnFlag =0;			//报警模式
static uchar cWarnLEDFlag =0;		//报警模式
	
				if(IN_O2 >INnWarnValeSet[0] )	 //	>80
				{		
					cWarnFlag =0;
					cWarnLEDFlag = 0;	//绿色
				}	
				else if(IN_O2 <INnWarnValeSet[1] )	// < 40
				{
					cWarnFlag = 2;
					cWarnLEDFlag = 2;	//红色
				}
				else if((IN_O2 >(INnWarnValeSet[0]-100) )		//70
							&&(cWarnFlag == 0))
				{
					cWarnLEDFlag = 0;	//绿色
				}		
				else if((IN_O2 < (INnWarnValeSet[1]+100) )		//50
							&&(cWarnFlag == 2))
				{
					cWarnLEDFlag = 2;	//红色
				}
				else
				{
					cWarnFlag = 1;
					cWarnLEDFlag = 1;	//黄色		
				} 
					
				if(cWarnLEDFlag == 0)			//O2	>80				绿色
				{
					fnPinWarn2(FALSE);	//黄
					fnPinWarn1(TRUE);	  //绿
					fnPinWarn0(FALSE);	//红
				}	
				else if(cWarnLEDFlag == 1)		//O2	>50 - 80		黄色
				{
					fnPinWarn2(TRUE );	//黄
					fnPinWarn1(FALSE);	//绿
					fnPinWarn0(FALSE);	//红					
				}	
				else							//O2	<50				红色
				{
					fnPinWarn2(FALSE);	//黄
					fnPinWarn1(FALSE);	//绿
					fnPinWarn0(TRUE );	//红	
				}	
}
/*
CS（RA1）对应红灯，
CLK（RA2）对应黄灯，
SDT（RA3）对应绿灯，
*/
