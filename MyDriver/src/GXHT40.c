#include "hk32f0xxa_gpio.h"
#include "GXHT40.h"
#include "IIC_GXHT40.h"

static void Delay20_ms(void)
{
	  long ti;
//		for(ti=0;ti<100;ti++);      //48M
	  for(ti=0;ti<288000;ti++);        //72M

}

static void GXHT40_Soft_Reset()
{
	unsigned char send_buf = GXHT40_Rst;
	I2CSendByte_S(GXHT40_Device_Adderss, &send_buf, 1); // 只用发一个命令
	Delay20_ms();
}

static void GXHT40_Set_Mode(unsigned char mode)
{
	unsigned char send_buf = 0; 

	if(mode == GXHT40_HIGH)
	{
		send_buf = GXHT40_High_Dup_Measure;
	}else if(mode == GXHT40_MID)
	{
		send_buf = GXHT40_Mid_Dup_Measure;
	}else if(mode == GXHT40_LOW)
	{
		send_buf = GXHT40_Low_Dup_Measure;
	}else{
		return;
	}
	I2CSendByte_S(GXHT40_Device_Adderss, &send_buf, 1);
}

void GXHT40_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);  //拉高
	Delay20_ms();

    GXHT40_Soft_Reset();
}

/*
* @name   CRC_8
* @brief  CRC-8校验
* @param  Crc_ptr -> 校验数据首地址
		LEN     -> 校验数据长度
* @retval CRC_Value -> 校验值      
*/
static unsigned char CRC_8(unsigned char *Crc_ptr, unsigned char LEN)
{
	unsigned char CRC_Value = 0xFF;
	unsigned char i = 0,j = 0;
 
	for(i=0;i<LEN;i++)
	{
		CRC_Value ^= *(Crc_ptr+i);
		for(j=0;j<8;j++)
		{
			if(CRC_Value & 0x80)
				CRC_Value = (CRC_Value << 1) ^ 0x31;
			else
				CRC_Value = (CRC_Value << 1);
		}
	}
	return CRC_Value;
}

void GXHT40_ReadT_RH(int *GXHT40_TEMP, int *GXHT40_HUMI)
{
	unsigned short tem, hum;
	unsigned char RHT_ReadBuf[6];  //返回读取的字节数是6
	int temperature, humidity;

	*GXHT40_TEMP = 0;  //ERROR默认值
	*GXHT40_HUMI = 0;  //ERROR默认值
	
	GXHT40_Set_Mode(GXHT40_HIGH);  //设置读取方式，默认高重复率
	
	Delay20_ms();				//等待芯片完成数据转换

	I2CReceiveByte_S(GXHT40_Device_Adderss, RHT_ReadBuf, 6);//读取温湿度原始测量数据
	// printf("buf 1:%d, 2:%d, 3:%d, 4:%d, 5:%d, 6:%d",RHT_ReadBuf[0], RHT_ReadBuf[1],
	// RHT_ReadBuf[2], RHT_ReadBuf[3], RHT_ReadBuf[4], RHT_ReadBuf[5]);
	
			
	if(CRC_8(RHT_ReadBuf, 2) == RHT_ReadBuf[2] && CRC_8(RHT_ReadBuf + 3, 2) == RHT_ReadBuf[5])
	{
		tem = ((RHT_ReadBuf[0]<<8) | RHT_ReadBuf[1]);//温度拼接
		hum = ((RHT_ReadBuf[3]<<8) | RHT_ReadBuf[4]);//湿度拼接
	
		temperature = 175 * tem / 65535 - 45;	// 公式：T = -45 + 175 * tem / (2^16-1)
		humidity = 125 * hum / 65535 - 6;		// 公式：RH = -6 + 125 * hum / (2^16-1)
	
		if((temperature>=-45&&temperature<=125)&&(humidity>=0 && humidity<=100))//过滤错误数据
		{
			*GXHT40_TEMP = temperature;
			*GXHT40_HUMI = humidity;		
		}	
	}
}
