#include "IIC_GXHT40.h"
#include "IIC_LQTP.h"
#include "hk32f0xxa_gpio.h"
#define TIME 2	

/**
 * @brief  芯片及引脚同步替换，HDC1080芯片改成GXHT40
 * @author qinling
 * @date   2026/01/06
 * @param  
 * @return 
 * @note  SDA PA3-->PB6
 */
static void GXHT40_SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief  芯片及引脚同步替换，HDC1080芯片改成GXHT40
 * @author qinling
 * @date   2026/01/06
 * @param  
 * @return 
 * @note  SDA PA3-->PB6
 */
static void GXHT40_SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/********************Pin Definition  用户更改 ************************
	* @1  ： #define 中引脚定义
  * @2  ： 函数内IIC_IN_SHT20(),IIC_OUT_SHT20()，引脚名称的改变
	* version: Version 1.0
	* author : By Tang Yousheng at 2016.10.25
********************Pin Definition  用户更改 *************************/
//#define SI7XX_SCK_WriteHigh()		GPIO_SetBits( GPIO_SCK, GPIO_SCK_PIN )
//#define SI7XX_SCK_WriteLow()		GPIO_ResetBits( GPIO_SCK, GPIO_SCK_PIN )
//#define SI7XX_SDA_WriteHigh()		GPIO_SetBits( GPIO_DATA, GPIO_DATA_PIN )
//#define SI7XX_SDA_WriteLow()		GPIO_ResetBits( GPIO_DATA,GPIO_DATA_PIN )


//#define SI7XX_SDA_ReadPin()		GPIO_ReadInputDataBit(GPIO_DATA,GPIO_DATA_PIN)
//#define SI7XX_SCK_ReadPin()		GPIO_ReadInputDataBit(GPIO_SCK,GPIO_SCK_PIN)
#if 1 
#define SCL_1         GPIO_SetBits( GPIOB, GPIO_Pin_7 )
#define SCL_0         GPIO_ResetBits( GPIOB, GPIO_Pin_7 )
#define SDA_1         GPIO_SetBits( GPIOB, GPIO_Pin_6)
#define SDA_0         GPIO_ResetBits( GPIOB, GPIO_Pin_6 )

#define DIR_IN_GXHT40        GXHT40_SDA_IN()   
#define DIR_OUT_GXHT40       GXHT40_SDA_OUT()
#define SDA_IN_GXHT40        GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6) 

#endif 

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C start		
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void I2CStart(void)
{
	DIR_OUT_GXHT40;
	SDA_1;
	Delay_Us(TIME);
	SCL_1;
	Delay_Us(TIME);
	SDA_0;
	Delay_Us(TIME);
	SCL_0;
}

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler sent stop	 signal
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void I2CStop(void)
{
	DIR_OUT_GXHT40;
	SCL_0;  //补充完整时序
	Delay_Us(TIME);
	SDA_0;
	Delay_Us(TIME);
	SCL_1;
	Delay_Us(TIME);
	SDA_1;
}

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler wait for ackownlege of device	
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static unsigned char I2C_Wait_ACK(void)
{	
	unsigned char flag = 0;
	unsigned int time = 0;
	DIR_IN_GXHT40; 
	Delay_Us(TIME);	
	SDA_1;
	SCL_1;
	Delay_Us(TIME);
	while(SDA_IN_GXHT40)
	{
			time++;
			if(time > 250) //msp430 16Mhz 150uS
			{
				flag =1; //应答失败
				break;
			}
	}
	SCL_0;	
	Delay_Us(TIME);
	DIR_OUT_GXHT40;
	return flag;
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Sent ackownlege to device	
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void I2C_Ack(void)
{
	SCL_0;
	DIR_OUT_GXHT40;
	SDA_0;		
	Delay_Us(2);
	SCL_1;		
	Delay_Us(5);
	SCL_0;
}

/**
  * @author ：Add by qin ling at 2026.01.07 
  * @brief  ：I2C controler Sent without ackownlege to device	
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void I2C_NAck(void)
{
	SCL_0;
	DIR_OUT_GXHT40;
	SDA_1;		
	Delay_Us(2);
	SCL_1;		
	Delay_Us(5);
	SCL_0;
}

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Sent Byte to device	
	* @param  ：SendDat :the Data sent by I2C
  * @note   ：When called ，the sender should wait for ACK from receriver
  * @retval ：No retval
  */
void I2CSendByte(unsigned char SendDat)
{
	unsigned int i;
	DIR_OUT_GXHT40;
    
	for(i = 0;i < 8;i++)
	{
		SCL_0;
		Delay_Us(TIME);
		if ((SendDat&0x80)==0x80)	
		{ SDA_1;}//最高位是否为1,如果是则 MDO= 1
		else 
        { SDA_0;} //否则 MDO=0
			Delay_Us(TIME);
		SCL_1;
			Delay_Us(TIME);
		SendDat <<= 1; //数据左移一位,进入下一轮送数
	}
	SCL_0; //
	//SDA_1; //等待确认信号，MDI = 0
	Delay_Us(TIME);
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Sent Byte to device	
	* @param  ：SendDat  :the pointer of the Datas sent by I2C
							Send_Num :the number of the data to be sent
							SlaAdd   :the slave divice address 
  * @note   ：
  * @retval ：No retval
  */
void I2CSendByte_S(unsigned char SlaAdd, unsigned char * SendDat, unsigned char Send_Num)
{
	unsigned char i;
	unsigned char flag = 0;
	
	I2CStart();
	I2CSendByte(( SlaAdd << 1) |0x00 );  //地址 + write(0x00)
	flag = I2C_Wait_ACK();
	if(flag)
	{
		printf("sensor write ack failed!\r\n");
		flag = 0;
	}
	
	for ( i = 0; i < Send_Num; i++ )
	{			
		I2CSendByte(SendDat[i]);
		flag = I2C_Wait_ACK();
		if(flag)
		{
			printf("send data ack failed!\r\n");
			flag = 0;
		}
	}
	I2CStop();
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Receive Byte sent by device	
	* @param  ：No
  * @retval ：unsigned char ：the Received  Data
  * @note   ：接收到的第一个字节为要接收的数据字节数，
							如果接收数据数组容量不够则返回0，否则为1。
  */
unsigned char I2CReceiveByte(unsigned char ack)
{
	unsigned char i;
	unsigned char RcvData = 0;
					
	DIR_IN_GXHT40;
	for(i = 0;i < 8;i++)
	{
		SCL_0;
		Delay_Us(TIME);
		SCL_1;		                			
		Delay_Us(TIME);
		RcvData<<=1;
		if(SDA_IN_GXHT40){
			RcvData++;
		}   
		Delay_Us(TIME);
	}
	if(ack)  //坑：放在字节循环内部了
	{
		I2C_Ack();
	}else{
		I2C_NAck();
	}
	return RcvData;
}


/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Receive Byte sent by device	
	* @param  ：SlaAdd   ：从器件地址，包含读写位
							RcvDat   ：接受的地址，pchar 类型
							Rcv_Num  ：接受的数据个数
  * @retval ：No
  * @note   ：1.    Master发送I2C addr（7bit）和w操作1（1bit），等待ACK
							2.    Slave发送ACK
							3.    Master发送reg addr（8bit），等待ACK
							4.    Slave发送ACK
							5.    Master发起START
							6.    Master发送I2C addr（7bit）和r操作1（1bit），等待ACK
							7.    Slave发送ACK
							8.    Slave发送data（8bit），即寄存器里的值
							9.    Master发送ACK
						 10.    第8步和第9步可以重复多次，即顺序读多个寄存器
  */
void I2CReceiveByte_S(unsigned char SlaAdd, unsigned char * RcvDat, unsigned char Rcv_Num)
{
	unsigned char i;
	unsigned char flag = 0;
	I2CStart();
	I2CSendByte(( SlaAdd << 1) |0x01 );
	flag = I2C_Wait_ACK();
	if(flag)
	{
		printf("sensor read ack failed!\r\n");
		flag = 0;
		return;
	}

	for (i = 0;i < Rcv_Num; i++)
	{
		if(i != ( Rcv_Num - 1 ) )
		{
			RcvDat[i] = I2CReceiveByte(1);  //前面都要回ack
		}else{
			RcvDat[i] = I2CReceiveByte(0);  //最后一个不回
		}
	}
	I2CStop();
}
