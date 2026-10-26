#include "IIC_LQTP.h"
#include "hk32f0xxa_gpio.h"
#include <stdint.h>

static uint16_t IIC_SCL_PIN;
static GPIO_TypeDef * IIC_SCL_GPIO_PORT;
static uint32_t IIC_SCL_GPIO_RCC;

static uint16_t IIC_SDA_PIN;
static GPIO_TypeDef * IIC_SDA_GPIO_PORT;
static uint32_t IIC_SDA_GPIO_RCC;

static uint8_t SENSOR_ADDRESS = 0x77;

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：Delay roughly						
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
void Delay_Us(unsigned int n)
{
	n *= (SystemCoreClock / 1000000);
	while(n--);
}

/**
  * @file   I2C_init
  * @brief  管脚配置, 两个芯片，引脚切换
  * @param  无
  * @retval 无
  */
void IIC_GPIO_SET_Init(int flag)
{
  if(flag==0)
  {
    IIC_SCL_PIN      		 =    GPIO_Pin_7             ; //PA2-->PB7
    IIC_SCL_GPIO_PORT    =  	GPIOB                  ;
    IIC_SCL_GPIO_RCC     = 		RCC_AHBPeriph_GPIOB    ;
                                                   
    IIC_SDA_PIN      		 =    GPIO_Pin_6             ;   //PA3-->PB6
    IIC_SDA_GPIO_PORT    =    GPIOB                  ;
    IIC_SDA_GPIO_RCC     = 		RCC_AHBPeriph_GPIOB    ;
	SENSOR_ADDRESS       =   0x77;
  }                                                  
  else
  {
     IIC_SCL_PIN      		=   GPIO_Pin_11             ;  //PA5-->PA11
     IIC_SCL_GPIO_PORT   	=  	GPIOA                  ;
     IIC_SCL_GPIO_RCC     =   RCC_AHBPeriph_GPIOA    ;
                                                     
     IIC_SDA_PIN      		=   GPIO_Pin_12             ;   //PB1-->PA12
     IIC_SDA_GPIO_PORT   	=   GPIOA                  ;
     IIC_SDA_GPIO_RCC     = 	RCC_AHBPeriph_GPIOA    ;
	 SENSOR_ADDRESS       =   0x76;
  }
}

static void LQTP_SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(IIC_SDA_GPIO_RCC, ENABLE);
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(IIC_SDA_GPIO_PORT, &GPIO_InitStructure);
}

static void LQTP_SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHBPeriphClockCmd(IIC_SDA_GPIO_RCC, ENABLE);
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(IIC_SDA_GPIO_PORT, &GPIO_InitStructure);
}

#define SCL_1         	 GPIO_SetBits(  IIC_SCL_GPIO_PORT , IIC_SCL_PIN)   /*GPIOB->BSRR = GPIO_Pin_6*/
#define SCL_0            GPIO_ResetBits(IIC_SCL_GPIO_PORT , IIC_SCL_PIN)   /*GPIOB->BRR  = GPIO_Pin_6 */
   
#define SDA_1         	 GPIO_SetBits(IIC_SDA_GPIO_PORT ,   IIC_SDA_PIN)   /*GPIOB->BSRR = GPIO_Pin_7*/
#define SDA_0         	 GPIO_ResetBits(IIC_SDA_GPIO_PORT , IIC_SDA_PIN)   /*GPIOB->BRR  = GPIO_Pin_7*/

#define DIR_IN_LQTP      LQTP_SDA_IN()   
#define DIR_OUT_LQTP     LQTP_SDA_OUT()
#define SDA_IN_LQTP      GPIO_ReadInputDataBit(IIC_SDA_GPIO_PORT , IIC_SDA_PIN)/* GPIOB->IDR  & GPIO_Pin_7	  */

void LQTP_IIC_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	/* Configure I2C2 pins: PA6->SCL  and  PA7->SDA */
	RCC_AHBPeriphClockCmd( IIC_SCL_GPIO_RCC | IIC_SDA_GPIO_RCC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin =  IIC_SCL_PIN ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(IIC_SCL_GPIO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  IIC_SDA_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(IIC_SDA_GPIO_PORT, &GPIO_InitStructure);

	GPIO_SetBits(IIC_SCL_GPIO_PORT, IIC_SCL_PIN | IIC_SDA_PIN);  //拉高
	Delay_Us(20000);
}

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C start		
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void IICStart(void)
{
	DIR_OUT_LQTP;
	SDA_1;
	Delay_Us(5);
	SCL_1;
	Delay_Us(5);
	SDA_0;
	Delay_Us(5);
	SCL_0;
}

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler sent stop	 signal
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void IICStop(void)
{
	DIR_OUT_LQTP;
	SCL_0;  //补充完整时序
	Delay_Us(5);
	SDA_0;
	Delay_Us(5);
	SCL_1;
	Delay_Us(5);
	SDA_1;
}

/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler wait for ackownlege of device	
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static unsigned char IIC_Wait_ACK(void)
{	
	unsigned char flag = 0;
	unsigned int time = 0;
	DIR_IN_LQTP; 
	Delay_Us(2);
	SDA_1;
	SCL_1;
	Delay_Us(5); 
	while(SDA_IN_LQTP)
	{
		time++;
		if(time > 250) //msp430 16Mhz 150uS
		{
			flag =1; //应答失败
			IICStop();
			break;
		}
		Delay_Us(2);
	}
	SCL_0;	
	Delay_Us(5);
	DIR_OUT_LQTP;
	return flag;
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Sent ackownlege to device	
  * @param  ：No parameters
  * @note   ：
  * @retval ：No retval
  */
static void IIC_Ack(void)
{
	SCL_0;
	DIR_OUT_LQTP;
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
static void IIC_NAck(void)
{
	SCL_0;
	DIR_OUT_LQTP;
	SDA_1;		
	Delay_Us(5);
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
void IICSendByte(unsigned char SendDat)
{
	unsigned int i;
	DIR_OUT_LQTP;
    
	for(i = 0;i < 8;i++)
	{
		SCL_0;
		Delay_Us(5);
		if ((SendDat&0x80)==0x80)	
		{ SDA_1;}//最高位是否为1,如果是则 MDO= 1
		else 
        { SDA_0;} //否则 MDO=0
			Delay_Us(5);
		SCL_1;
			Delay_Us(5);
		SendDat <<= 1; //数据左移一位,进入下一轮送数
	}
	SCL_0; //
	//SDA_1; //等待确认信号，MDI = 0
	Delay_Us(5);
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
void IICSendByte_S(unsigned char RegAddr, unsigned char * SendDat, unsigned char Send_Num)
{
	unsigned char i;
	unsigned char flag = 0;
	
	IICStart();
	IICSendByte((SENSOR_ADDRESS << 1) |0x00 );  //芯片地址 + write(0x00)
	flag = IIC_Wait_ACK();
	if(flag)
	{
		printf("sensor write ack failed!\r\n");
		flag = 0;
	}

    IICSendByte(RegAddr);  //寄存器地址
    flag = IIC_Wait_ACK();
	if(flag)
	{
		printf("register write ack failed!\r\n");
		flag = 0;
	}
	
	for ( i = 0; i < Send_Num; i++ )
	{			
		IICSendByte(SendDat[i]);
		flag = IIC_Wait_ACK();
		if(flag)
		{
			printf("send data ack failed!\r\n");
			flag = 0;
		}
	}
	IICStop();
}
/**
  * @author ：Add by Tang Yousheng at 2016.11.20 
  * @brief  ：I2C controler Receive Byte sent by device	
	* @param  ：No
  * @retval ：unsigned char ：the Received  Data
  * @note   ：接收到的第一个字节为要接收的数据字节数，
							如果接收数据数组容量不够则返回0，否则为1。
  */
unsigned char IICReceiveByte(unsigned char ack)
{
	unsigned char i;
	unsigned char RcvData = 0;
					
	DIR_IN_LQTP;
	for(i = 0;i < 8;i++)
	{
		SCL_0;
		Delay_Us(2);
		SCL_1;		                			
		Delay_Us(2);
		RcvData<<=1;
		if(SDA_IN_LQTP){
			RcvData++;
		}   
		Delay_Us(2);
	}
	if(ack) //坑：放在字节循环内部了
	{
		IIC_Ack();
	}else{
		IIC_NAck();
	}
	return RcvData;
}

//寄存器地址自动递增
void IICReceiveByte_S(unsigned char RegAddr, unsigned char * RcvDat, unsigned char Rcv_Num)
{
	unsigned char i;
	unsigned char flag = 0;
    IICStart();
	IICSendByte((SENSOR_ADDRESS << 1) |0x00 );  //1. 先写芯片
	flag = IIC_Wait_ACK();
	if(flag)
	{
		printf("sensor write ack failed!\r\n");
		flag = 0;
	}

    IICSendByte(RegAddr);  //2.发送寄存器地址
    flag = IIC_Wait_ACK();
	if(flag)
	{
		printf("register write ack failed!\r\n");
		flag = 0;
	}

	IICStart(); //重复IIC起始信号
	IICSendByte((SENSOR_ADDRESS << 1) |0x01 );  //3.读芯片地址+read(0x01)
	flag = IIC_Wait_ACK();
	if(flag)
	{
		printf("sensor read ack failed!\r\n");
		flag = 0;
	}
	
	for (i = 0;i < Rcv_Num; i++)
	{
		if(i != ( Rcv_Num - 1 ) )
		{
			RcvDat[i] = IICReceiveByte(1);  //前面都要回ack
		}else{
			RcvDat[i] = IICReceiveByte(0);  //最后一个不回
		}
	}
	IICStop();
}


