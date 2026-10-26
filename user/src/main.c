/*******************************************************************************
* @copyright: Shenzhen Hangshun Chip Technology R&D Co., Ltd
* @filename:  main.c
* @brief:     Main program body
* @author:    AE Team
* @version:   V1.0.0/2023-10-08
*             1.Initial version
* @log:
*******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "iwdg.h"
#include "flash.h"
#include "usart.h"
#include "tim.h"
#include "GXHT40.h"
#include "adc.h"
#include "pwm.h"
#include "cap.h"
#include "SubFun.h"
#include "ResultCount.h"
#include "Linearization.h"
#include "ntc.h"
#include "WarnMod.h"
#include "RXExecute.h"
#include "LQTP.h"
#include "IIC_LQTP.h"
#include "stdlib.h"
#include "math.h"

/** @addtogroup HK32F0xxA_StdPeriph_Examples
  * @{
  */

/** @addtogroup _template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

unsigned char cAutoSendFlag = 1; //临时开


static unsigned short Cnt10ms=0;
static unsigned char Flag10ms=0;
static unsigned short Cnt20ms=0;
static unsigned char Flag20ms=0;
static unsigned short Cnt100ms=0;
static unsigned char Flag100ms=0;
static unsigned short Cnt500ms=0;
static unsigned char Flag500ms=0;
static unsigned short Cnt1000ms=0;
static unsigned char Flag1000ms=0;

static unsigned char StartCaiyangFlag;

unsigned char StateFlag;

static unsigned char firstcount1 = 0;
static unsigned char firstcount2 = 0;
static unsigned char firstcount3 = 0;

#define FLY_BUFFER_NUM   50
#define FLY_DIFF_NUM     20

#define FLOW_BUFFER_NUM   60
#define FLOW_DIFF_NUM     20

#define O2_BUFFER_NUM   60
#define O2_DIFF_NUM     20

int  nBufferFf[FLY_BUFFER_NUM] = {0};
int  nBufferFb[FLY_BUFFER_NUM] = {0};
int  nBuffREflow[FLOW_BUFFER_NUM] = {0};
int  nBuffREO2[O2_BUFFER_NUM] = {0};


static long lFlowRefCountFf;		//流量测试值
static long lFlowRefCountFb;

char cFpoint; 		//10次计数


/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	unsigned char cReadTCount;	//读温度计数
	unsigned char setBaudrateFlag = 0;
	unsigned short eprom_data;
	SensorValue sensor_value = {0};
	FlashData flash_data = {0};
	float nH2o = 0.0f;
	float nPress = 0.0f;
	unsigned char OutputFlag = 0;
	static unsigned char press_count = 0;
	static float press_in_buff[20] = {0};
	static float press_out_buff[20] = {0};
	float press_in_temp = 0.0f;
	float press_out_temp = 0.0f;

	SystemInit(); 
	//SystemCoreClockUpdate(); //ST的时钟频率是读出来的，航顺的是定义的，航顺去掉了锁相环时钟源导致对不上
	SysTick_Config(SystemCoreClock / 1000);  //时间片1ms
	GpioConfig();
	setBaudrateFlag  = ReadEeprom(255);

	if(setBaudrateFlag==2)
	{
		UsartInit(38400);
	}
	else if(setBaudrateFlag==1)
	{
		UsartInit(115200);
	}
	else
	{
		UsartInit(9600);
	}

	TIM16Delay_Init();

	// GXHT40_Init();//20260106修改处 温湿度芯片替换

	Adc_Init_NTC();
	PwmInit();  //和USART接收冲突
	CapInit();
	FnRecover(&flash_data);

	LQTP_GPIO_Init(); //iic各引脚初始化

	// FnSaveO2();

	FnLinearization1();
	FnLinearization(1);

	sensor_value.temperature_NTC = test_tab();//NTC读取温度

	AotuSetSendOrder(&flash_data.SendOrder, sensor_value.temperature_NTC);//自动判断顺序
	IWDG_Init(4, 800); // 3200ms

	cAutoSendFlag = 1;		//add  去掉5秒等待
	cAlarmRS232Count = 5;

	while (1)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
	{
	IWDG_Feed();

	if(Flag10ms == 1)
	{
		Flag10ms=0;	
		
		if(StartCaiyangFlag == 0)
		{ 
			StartCaiyangFlag = 1;
			StartCaiyangLeft(flash_data.SendOrder);					
		}
		else if(StartCaiyangFlag == 1)
		{
			StartCaiyangFlag = 0;
			StartCaiyangRight(flash_data.SendOrder);
			USFScreen(sensor_value, nPress);
		}

		for (int cBufferPoint = 0; cBufferPoint < FLY_BUFFER_NUM - 1; cBufferPoint++)
		{
			nBufferFf[cBufferPoint] = nBufferFf[cBufferPoint + 1];
			nBufferFb[cBufferPoint] = nBufferFb[cBufferPoint + 1];
		}
		nBufferFf[FLY_BUFFER_NUM - 1] = nT1CountLeft;
		nBufferFb[FLY_BUFFER_NUM - 1] = nT1CountRight;
	}

	if(Flag20ms == 1)
	{			
		Flag20ms = 0;
		//==========排序平均值===============
		if(firstcount1 < FLY_BUFFER_NUM)
		{
			firstcount1++;
			lFlowRefCountFf  = nT1CountLeft * 10;	
			lFlowRefCountFb  = nT1CountRight * 10;
		}
		else
		{
			lFlowRefCountFf =  FilterIntDataTask(nBufferFf, FLY_BUFFER_NUM, FLY_DIFF_NUM);
			lFlowRefCountFb =  FilterIntDataTask(nBufferFb, FLY_BUFFER_NUM, FLY_DIFF_NUM);
		}
		nOrderFf = lFlowRefCountFf / 10;
		nOrderFb = lFlowRefCountFb / 10;

		ResultCount5P_O2(lFlowRefCountFf, lFlowRefCountFb, sensor_value);
		ResultCount5P_Flow(lFlowRefCountFf, lFlowRefCountFb, sensor_value);
		
		if(debugflag)
		{	
			printf( "%ld,%ld; ", lFlowRefCountFf, lFlowRefCountFb);
		}
	}

	if(Flag100ms==1)//100ms任务
		{
			Flag100ms=0;		
				//**************增加滑动*****************
			for (int cBufferPoint = 0; cBufferPoint < FLOW_BUFFER_NUM - 1; cBufferPoint++)
			{
				nBuffREflow[cBufferPoint] = nBuffREflow[cBufferPoint + 1];
			}
			nBuffREflow[FLOW_BUFFER_NUM - 1] = mysGasTrait5P[1].PressResult;
			//==========排序平均值===============
			if(firstcount2 < FLOW_BUFFER_NUM)
			{
				firstcount2++;
				mysGasTrait5P[1].ReResult  = mysGasTrait5P[1].PressResult; //流量	
			}
			else
			{
				if(abs(nBuffREflow[0] - nBuffREflow[FLOW_BUFFER_NUM-1]) > 10)  //跳变大于1L
				{
					mysGasTrait5P[1].ReResult = mysGasTrait5P[1].PressResult;
				}else{
					mysGasTrait5P[1].ReResult =  FilterIntDataTask(nBuffREflow, FLOW_BUFFER_NUM, FLOW_DIFF_NUM) / 20;
				}
			}
	//				mysGasTrait5P[0].Result -= nFnO2CorrectionT(nTempGas,mysGasTrait5P[0].Result);
				//mysGasTrait5P[0].Result = mysGasTrait5P[0].Result+(int)(nH2o);	
	//				Cn_O2 = K_O2/1000.0*mysGasTrait5P[0].Result +B_O2/100;		//同时显示纯氧气浓度PSAResult
	//        mysGasTrait5P[0].PSAResult = (mysGasTrait5P[0].Result+34.2)/1.142;		//同时显示PSA浓度
			if(debugflag)
			{
				printf("TO:%d,",mysGasTrait5P[0].Result);
			}	
			if(debugflag)
			{
				printf("Cn_O2:%d,%d,%d\n",flash_data.coef.Cn_O2,flash_data.coef.K_O2,flash_data.coef.B_O2);
			}			
			/*~~~~~~~~~~~~状态位报警~~~~~~~~~~~~*/	
			StateFlag &= OverFlag;          //其他状态位复位			
			if(mysGasTrait5P[0].Result <820)			
			{
					StateFlag |= AlarmFlagO2;
			} 
			if(mysGasTrait5P[0].Result <500)			
			{
					StateFlag |= AlarmFlagO2Low;
			} 
			//低流量报警
			if(mysGasTrait5P[1].Result <7)			
			{
				StateFlag |= AlarmFlagFlow;
			}
			cAlarmRS232Count++;
			if(cAlarmRS232Count >= 25 )	
			{               
				cAlarmRS232Count = 25;	
				StateFlag |= AlarmFlagRS232;
			}  
			if(cAlarmRS232Count > 5)	//5S没有收到数据就会进行自动发送
			{
				cAutoSendFlag = 1;
			}
			else
			{
				cAutoSendFlag = 0;
			}
			/*~~~~~~~~~~~~报警指示~~~~~~~~~~~~*/			
			fnPinWarnLEDs(mysGasTrait5P[0].Result, flash_data.nWarnVale);
			/*~~~~~~~~~~~~模拟输出~~~~~~~~~~~~*/			
			PWMDAOutput(0,(mysGasTrait5P[0].ReResult/**/   ));	//O2输出 *****************************************************修改处
			if(mysGasTrait5P[1].Result >= 0)
			{PWMDAOutput(1, (mysGasTrait5P[1].ReResult));}	//流量输出
			else
			{PWMDAOutput(1,-(mysGasTrait5P[1].ReResult));}	//流量输出				
			
		}

	if(Flag500ms==1)//500ms任务
		{
			Flag500ms=0;
			mysGasTrait5P[0].PressResult = mysGasTrait5P[0].Result + sensor_value.h2o_press_factor;
			for (int cBufferPoint = 0; cBufferPoint < O2_BUFFER_NUM - 1; cBufferPoint++)
			{
				nBuffREO2[cBufferPoint] = nBuffREO2[cBufferPoint + 1];
			}
			nBuffREO2[O2_BUFFER_NUM - 1] = mysGasTrait5P[0].PressResult;
			//==========排序平均值===============
			if(firstcount3 < O2_BUFFER_NUM)
			{
				firstcount3++;	
				mysGasTrait5P[0].ReResult  = mysGasTrait5P[0].PressResult; //浓度
			}
			else
			{
				if(abs(nBuffREO2[0] - nBuffREO2[O2_BUFFER_NUM-1]) > 150)  //跳变大于15%
				{
					mysGasTrait5P[0].ReResult = mysGasTrait5P[0].PressResult;
				}else{
					mysGasTrait5P[0].ReResult =  FilterIntDataTask(nBuffREO2, O2_BUFFER_NUM, O2_DIFF_NUM) / 20;
				}	
			}
			fnRUNLED();
			Get_OutputFlag(&OutputFlag);
			if(setBaudrateFlag!=2)
			{
				if(	cAutoSendFlag ==1)
				{
					if((debugflag == 0)&&(OutputFlag == 0))
					{
						if(flash_data.cENAutoSendFlag)	//自动发送发布
						{
							AutoSendData(sensor_value);
						}
					}
				}
			}		
		}

	if(Flag1000ms==1)//1000ms任务
		{
			Flag1000ms=0;
			sensor_value.temperature_NTC = test_tab();//NTC读取温度	
			if(cReadTCount==0)
			{
				IIC_GPIO_SET_Init(0);
				LQTP_Get_Pressure(&press_in_temp, 0);
				cReadTCount = 1;
			}
			else
			{
				IIC_GPIO_SET_Init(1);
				LQTP_Get_Pressure(&press_out_temp, 1);
				cReadTCount = 0;
			}
			for (int pressPoint = 0; pressPoint < 19; pressPoint++) 
			{
				press_in_buff[pressPoint] = press_in_buff[pressPoint + 1];
				press_out_buff[pressPoint] = press_out_buff[pressPoint + 1];
			}
			press_in_buff[19] = press_in_temp;
			press_out_buff[19] = press_out_temp;

			if(press_count < 20)
			{
				sensor_value.press_in = press_in_temp;
				sensor_value.press_out = press_out_temp;
				press_count++;
			}
			else
			{
				if(fabsf(press_in_buff[19] - press_in_buff[0]) > 50) //跳变大于5kPa
				{
					sensor_value.press_in = press_in_temp;
				}else{
					sensor_value.press_in = FilterFloatDataTask(press_in_buff, 20, 8);
				}

				if(fabsf(press_out_buff[19] - press_out_buff[0]) > 50) //跳变大于5kPa
				{
					sensor_value.press_out = press_out_temp;
				}else{
					sensor_value.press_out = FilterFloatDataTask(press_out_buff, 20, 8);
				}
			}

			// GXHT40_ReadT_RH(&sensor_value.temperature, &sensor_value.nRH);
			// nH2o = FnTandRHToAIRO2(sensor_value.temperature, sensor_value.nRH);	
			nH2o = 0;	
			nPress = FnPressureCalibration(sensor_value.press_out);
			sensor_value.h2o_press_factor = (int)(nH2o) + (int)(nPress);
			// printf("p_in:%f, p_out:%f\r\n",sensor_value.press_in, sensor_value.press_out);
		}

	if(rx232Flag==1)
		{
			cAlarmRS232Count= 0;  
			RXExecute(sensor_value, &flash_data);	
						
			if(CaliFlag ==1 )
			{
				CaliFlag = 0;
				
				if(debugflag)
				{	
					putchar('\n');	//输出编号	
					printf("Num: ");
					for(int i=0;i<5;i++)
					{
						eprom_data  = (unsigned short)ReadEeprom(120+i*2+0);
						eprom_data  = eprom_data*256;
						eprom_data += (unsigned short)ReadEeprom(120+i*2+1);						
						printf("%u-",eprom_data);																  
					}						
					putchar('\n');	//输出编号
			
					for(int i=0;i<8;i++) putchar(version[i]);	//输出软件版本?
					putchar('\n');
				}					
				
					FnLinearization1();				
	//                FnLinearization(0);
					FnLinearization(1);	
				if(debugflag)
				{
					printf("Flash %d %d\n",FlashCheckData[0],FlashCheckData[1]);
				}
				debugflag = 0;
			}		

			rx232Flag=0;
			rxCount = 0;		
		}

	__WFI();   //进入低功耗

	}
	}

	void SysTick_Handler(void)//1ms中断
	{
	// 		MeasureCheck();//1ms任务
		if(Flag10ms==0)	
		{
			Cnt10ms++;
			if(Cnt10ms>=10)//10ms
			{
				Cnt10ms=0;
				Flag10ms=1;
			}
		}	
		if(Flag20ms==0)
		{
			Cnt20ms++;
			if(Cnt20ms>=100)//20ms
			{
				Cnt20ms=0;
				Flag20ms=1;
			}
		}	
		if(Flag100ms==0)
		{
			Cnt100ms++;
			if(Cnt100ms>=100)//20ms
			{
				Cnt100ms=0;
				Flag100ms=1;
			}
		}
		if(Flag500ms==0)
		{
			Cnt500ms++;
			if(Cnt500ms>=500)
			{
				Cnt500ms=0;
				Flag500ms=1;
			}
		}
		if(Flag1000ms==0)
		{
			Cnt1000ms++;
			if(Cnt1000ms>=1000)
			{
				Cnt1000ms=0;
				Flag1000ms=1;
			}
		}
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}

#endif

/**
  * @}
  */

/**
  * @}
  */
