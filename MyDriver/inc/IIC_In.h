#ifndef _SHT20_H_
#define _SHT20_H_

#include <stdint.h>

////#define I2C_SCL_PIN      		      GPIO_Pin_2         
////#define I2C_SCL_GPIO_PORT   	  	GPIOA           
////#define I2C_SCL_GPIO_RCC      		RCC_AHBPeriph_GPIOA
//// 
////#define I2C_SDA_PIN      		      GPIO_Pin_3               
////#define I2C_SDA_GPIO_PORT   	    GPIOA           
////#define I2C_SDA_GPIO_RCC      		RCC_AHBPeriph_GPIOA  


#define SCL_H         	 GPIO_SetBits(  I2C_SCL_GPIO_PORT , I2C_SCL_PIN)   /*GPIOB->BSRR = GPIO_Pin_6*/
#define SCL_L            GPIO_ResetBits(I2C_SCL_GPIO_PORT , I2C_SCL_PIN)   /*GPIOB->BRR  = GPIO_Pin_6 */
   
#define SDA_H         	 GPIO_SetBits(I2C_SDA_GPIO_PORT ,   I2C_SDA_PIN)   /*GPIOB->BSRR = GPIO_Pin_7*/
#define SDA_L         	 GPIO_ResetBits(I2C_SDA_GPIO_PORT , I2C_SDA_PIN)   /*GPIOB->BRR  = GPIO_Pin_7*/

//#define SCL_read       	 GPIO_ReadInputDataBit(I2C_SCL_GPIO_PORT , I2C_SCL_PIN)/* GPIOB->IDR  & GPIO_Pin_6   */
#define SHT_READ_SDA     GPIO_ReadInputDataBit(I2C_SDA_GPIO_PORT , I2C_SDA_PIN)/* GPIOB->IDR  & GPIO_Pin_7	  */


#define TIME  8U

#define SHT_IIC_SDA_H   SDA_H
#define SHT_IIC_SDA_L   SDA_L

#define SHT_IIC_SCL_H  SCL_H
#define SHT_IIC_SCL_L  SCL_L

extern float pressure_hPa;
extern float temperature_degC;

extern char ADDRESS_WRITE;
extern char ADDRESS_READ;
void SHT20_GPIO_SET_Init(int flag);
void SHT20_GPIO_Init(void);
void test_iic(void);

void User_lps22hb_Init(void);
void User_lps22hb_Task(void);


#endif
