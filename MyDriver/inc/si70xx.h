#ifndef __SI7XX_H_
#define __SI7XX_H_

#include "hk32f030xxA.h"

#define ACK 0x00
#define NACK 0x01


//移植更换IO口只需要修改以下即可。如果切换了不同主频的MCU需要修改相应的延时时间
///////////////////////////////////////////////////////////////////////
#define GPIO_DATA       GPIOA                 //数据端引脚所在组     
#define GPIO_DATA_PIN   GPIO_Pin_3            //数据端引脚号      
#define RCC_GPIO_DATA   RCC_AHBPeriph_GPIOA   //数据端引脚时钟

#define GPIO_SCK       GPIOA                 
#define GPIO_SCK_PIN   GPIO_Pin_2           
#define RCC_GPIO_SCK   RCC_AHBPeriph_GPIOA 


///////////////////////////////////////////////////////////////////////

#define SI7XX_SCK_WriteHigh()		GPIO_SetBits( GPIO_SCK, GPIO_SCK_PIN )
#define SI7XX_SCK_WriteLow()		GPIO_ResetBits( GPIO_SCK, GPIO_SCK_PIN )
#define SI7XX_SDA_WriteHigh()		GPIO_SetBits( GPIO_DATA, GPIO_DATA_PIN )
#define SI7XX_SDA_WriteLow()		GPIO_ResetBits( GPIO_DATA,GPIO_DATA_PIN )


#define SI7XX_SDA_ReadPin()		GPIO_ReadInputDataBit(GPIO_DATA,GPIO_DATA_PIN)
#define SI7XX_SCK_ReadPin()		GPIO_ReadInputDataBit(GPIO_SCK,GPIO_SCK_PIN)
/* I2C slave address of Si70xx */
#define SI70XX_READ_ADDR                     0x81
#define SI70XX_WRITE_ADDR                    0x80
#define READ_T  0x00        //读温度
#define READ_RH 0x01        //读湿度

/* Commands */
#define CMD_MEASURE_HUMIDITY_HOLD       0xE5
#define CMD_MEASURE_TEMPERATURE_HOLD    0xE3

#define CMD_RESET                       0xFE


extern uint8_t Sensor_ReadTemperature(uint8_t SI7XX_Measure_Type,int *TH);
extern void Si70xx_GPIO_Init(void);


#endif
