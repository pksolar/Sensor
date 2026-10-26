#include "adc.h"
#include "IIC_LQTP.h"


void Adc_Init_NTC(void)
{ 	
    ADC_InitTypeDef ADC_InitStructure; 
    GPIO_InitTypeDef GPIO_InitStructure;

    // 补全：使能 ADC 时钟 + GPIOB 时钟（关键！）
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC ,ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); // 新增：开GPIOB时钟

    ADC_ClockModeConfig(ADC, ADC_ClockMode_SynClkDiv2);
                    
    // 配置 ADC 引脚（GPIOB Pin0，通道8）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;	    // 模拟输入（正确）
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  // 新增：无上下拉（模拟输入必须）
    GPIO_Init(GPIOB, &GPIO_InitStructure);	 


    // ADC 复位 + 初始化
    ADC_DeInit(ADC);
    ADC_StructInit(&ADC_InitStructure); 
    ADC_InitStructure.ADC_Resolution  = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ContinuousConvMode  = DISABLE; // 单次转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // 软件触发
    ADC_InitStructure.ADC_DataAlign  = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ScanDirection  = ADC_ScanDirection_Upward; 
    ADC_Init(ADC, &ADC_InitStructure);
	// 首次配置通道 + 启动转换
    ADC_ChannelConfig(ADC, ADC_Channel_8, ADC_SampleTime_71_5Cycles); 
	ADC_GetCalibrationFactor(ADC);
    // 补全：ADC 完整校准流程
    ADC_Cmd(ADC, ENABLE);  // 使能ADC
    // 等待 ADC 就绪
    while(!ADC_GetFlagStatus(ADC, ADC_FLAG_ADEN)); // 软件请求
	while(!ADC_GetFlagStatus(ADC, ADC_FLAG_ADRDY)); // 硬件真正就绪
	 
    ADC_StartOfConversion(ADC);
}

unsigned int Get_Adc()   
{	
    unsigned int result = 0;
    unsigned int temp = 0;
    unsigned char i = 0;
    unsigned char n = 0;
    unsigned int timeout = 0; // 新增：超时保护，避免死循环
    static int old_temp = 0, new_temp = 0;

    for(i=0;i<30;i++)
    {
        ADC_StartOfConversion(ADC);
        
        // 新增：超时保护（防止死循环）
        timeout = 0;
        while(!ADC_GetFlagStatus(ADC, ADC_FLAG_EOC))
        {
            timeout++;
            if(timeout > 100000) // 超时阈值，根据主频调整
            {
                printf("ADC recv timeout, ADC_ISR=0x%08X\r\n", ADC->ISR);
                break; // 退出超时循环
            }
        }
        Delay_Us(10);
        temp = ADC_GetConversionValue(ADC);
        old_temp = new_temp;
        new_temp = temp;
        if(((old_temp - new_temp) < 200) && ((old_temp - new_temp) > -200))
        {
            result += temp;
            n++;
        }
        // printf("result:%d\r\n", result);
        
        // 清除EOC标志位，避免下次误判
        ADC_ClearFlag(ADC, ADC_FLAG_EOC);
        Delay_Us(50);
    }
    result = result/n;
    return result;	
}
