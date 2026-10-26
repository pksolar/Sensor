#include "si70xx.h"

void DelayMs(unsigned int j)
{
	unsigned int i = 0;
	
	while(j--)
	{
		//for(i=0;i<1150;i++);
		for(i=0;i<150;i++);
	}
}

void Si70xx_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd(RCC_GPIO_DATA,ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_DATA_PIN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIO_DATA, &GPIO_InitStructure);

	RCC_AHBPeriphClockCmd(RCC_GPIO_SCK,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_SCK_PIN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIO_SCK, &GPIO_InitStructure);
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_ResetBits( GPIOA, GPIO_Pin_3 );
	
}

/*----------------------------------
函数名：Si70xx_DATA_OUT
描述：Si70xx  DATA(PC15)设置为输出
----------------------------------*/
void SI7XX_SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_GPIO_DATA,ENABLE);
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_DATA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIO_DATA, &GPIO_InitStructure);
}

/*----------------------------------
函数名：Si70xx_DATA_IN
描述：Si70xx  DATA(PC15)设置为输入
----------------------------------*/
void SI7XX_SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_GPIO_DATA,ENABLE);
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_DATA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIO_DATA, &GPIO_InitStructure);
}

/****************************************************************************
函数介绍：发送启始位
 输入参数：
 输出参数：
返回值 ：
****************************************************************************/
void SI7XX_I2c_StartCondition(void)
{
	SI7XX_SDA_OUT();
	SI7XX_SDA_WriteHigh();
	SI7XX_SCK_WriteHigh();
	DelayMs(3);
	SI7XX_SDA_WriteLow();
	DelayMs(3);
	SI7XX_SCK_WriteLow();
	DelayMs(3);
}
/****************************************************************************
函数介绍：发送停止位
输入参数：
输出参数：
返回值 ：
****************************************************************************/
void SI7XX_I2c_StopCondition(void)
{
	SI7XX_SDA_OUT();
	SI7XX_SDA_WriteLow();
	SI7XX_SCK_WriteHigh();
	DelayMs(3); // set-up time stop condition (t_SU;STO)
	SI7XX_SDA_WriteHigh();
	DelayMs(3);
//	SI7XX_SCK_WriteLow();
}

/****************************************************************************
函数介绍：写一个字节
输入参数：txByte
输出参数：
返回值 ：0 写成功 1写失败
***************************************************************************/
uint8_t SI7XX_I2c_WriteByte(uint8_t txByte)
{
	uint8_t mask,error=0,i;
	SI7XX_SDA_OUT();
	SI7XX_SDA_WriteHigh();
	for(i=1;i<20;i++); 
	for(mask=0x80; mask>0; mask>>=1)//shift bit for masking (8 times)
	{ 
		if ((mask & txByte) == 0) 
			SI7XX_SDA_WriteLow();				//masking txByte, write bit to SDA-Line
		else 
			SI7XX_SDA_WriteHigh();
		DelayMs(1); 		//data hold time(t_HD;DAT)
		SI7XX_SCK_WriteHigh(); 					//generate clock pulse on SCL
		DelayMs(1); 		//data set-up time (t_SU;DAT)
		SI7XX_SCK_WriteLow();
		DelayMs(1); 		//SCL high time (t_HIGH)
	}
	SI7XX_SDA_IN(); 						//release SDA-line
	SI7XX_SCK_WriteHigh(); 						//clk #9 for ack
	DelayMs(5); 			//data set-up time (t_SU;DAT)
	if(SI7XX_SDA_ReadPin()) 
		error=1; 			//check ack from i2c slave
	SI7XX_SCK_WriteLow();
	DelayMs(5); 			//wait time to see byte package on scope
	return error; 					//return error code
}
/****************************************************************************
函数介绍：读一个字节
输入参数：ACK:0要应答  ACK:1 无需应答
输出参数：
返回值 ：读取的值
***************************************************************************/
uint8_t SI7XX_I2c_ReadByte(uint8_t nACK)
{
	uint8_t mask,rxByte=0,i;
	
	SI7XX_SDA_IN(); 						//release SDA-line
	for(i=1;i<20;i++); 
	for(mask=0x80; mask>0; mask>>=1)//shift bit for masking (8 times)
	{ 
		SI7XX_SCK_WriteHigh(); 					//start clock on SCL-line
		DelayMs(4); 		//data set-up time (t_SU;DAT)
		if(SI7XX_SDA_ReadPin()) 
			rxByte=(rxByte | mask); //read bit
		SI7XX_SCK_WriteLow();
		DelayMs(1); 		//data hold time(t_HD;DAT)
	}
	SI7XX_SDA_OUT(); 	
	if(nACK == 0)
		SI7XX_SDA_WriteLow(); 
	else
		SI7XX_SDA_WriteHigh();
	DelayMs(1); 			//data set-up time (t_SU;DAT)
	SI7XX_SCK_WriteHigh(); 						//clk #9 for ack
	DelayMs(5); 			//SCL high time (t_HIGH)
	SI7XX_SCK_WriteLow();
	SI7XX_SDA_WriteHigh(); 						//release SDA-line
	DelayMs(20); 			//wait time to see byte package on scope
	
	return rxByte; //return error code
}

/****************************************************************************
函数介绍：读湿度 *10 温度*100
输入参数：SI7XX_Measure_Type:类型 TH：温湿度值
 输出参数：
 返回值 ：0 写成功 1写失败
****************************************************************************/
uint8_t Sensor_ReadTemperature(uint8_t SI7XX_Measure_Type,int *TH)
{
	uint8_t data[2] = {0},error = 0;
	uint16_t value = 0;

	SI7XX_I2c_StartCondition();
	error |= SI7XX_I2c_WriteByte(SI70XX_WRITE_ADDR);
	if(READ_T == SI7XX_Measure_Type)//读温度
	{
		error |= SI7XX_I2c_WriteByte(CMD_MEASURE_TEMPERATURE_HOLD);
	}
	else if(READ_RH == SI7XX_Measure_Type)//读湿度
	{
		error |= SI7XX_I2c_WriteByte(CMD_MEASURE_HUMIDITY_HOLD);
	}
	SI7XX_I2c_StartCondition();
	error |= SI7XX_I2c_WriteByte(SI70XX_READ_ADDR);

	DelayMs(5000);
	//SI7XX_SCK_WriteHigh();
	
	data[1] = SI7XX_I2c_ReadByte(ACK);
	data[0] = SI7XX_I2c_ReadByte(ACK);
	SI7XX_I2c_ReadByte(NACK);
	SI7XX_I2c_StopCondition();
	value = data[0]|(data[1] << 8);
	value &= ~(0x0003);

	if(READ_T == SI7XX_Measure_Type)		       //扩大100倍
	{
		*TH = 17572*((float)value/65536)-4685;
	}
	else if(READ_RH == SI7XX_Measure_Type)
	{
		if(value>(55575&(~(0x0003))))  return 1;
		
		*TH = (float)value * 1250.0/65536.0-60.0; //扩大10倍
		if(*TH<0)  *TH=0;		
	}
	return error;
}

