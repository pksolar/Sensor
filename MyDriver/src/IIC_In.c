#include "IIC_In.h"
#include "lps22hb_reg_In.h"
#include <hk32f0xxa_gpio.h>
#include <string.h>

char ADDRESS_WRITE = 0;
char ADDRESS_READ = 0;

uint16_t I2C_SCL_PIN;
GPIO_TypeDef * I2C_SCL_GPIO_PORT;
uint32_t I2C_SCL_GPIO_RCC;

uint16_t I2C_SDA_PIN;
GPIO_TypeDef * I2C_SDA_GPIO_PORT;
uint32_t I2C_SDA_GPIO_RCC;
//以下代码是硬件相关的，移植时候需要修改
GPIO_InitTypeDef  GPIO_InitForSCL;
GPIO_InitTypeDef  GPIO_InitForSDAOut;
GPIO_InitTypeDef  GPIO_InitForSDAIn;

/**
  * @file   I2C_init
  * @brief  管脚配置
  * @param  无
  * @retval 无
  */
void SHT20_GPIO_SET_Init(int flag)
{
  if(flag==0)
  {
    I2C_SCL_PIN      		 =    GPIO_Pin_7             ; //PA2-->PB7
    I2C_SCL_GPIO_PORT    =  	GPIOB                  ;
    I2C_SCL_GPIO_RCC     = 		RCC_AHBPeriph_GPIOB    ;
                                                   
    I2C_SDA_PIN      		 =    GPIO_Pin_6             ;   //PA3-->PB6
    I2C_SDA_GPIO_PORT    =    GPIOB                  ;
    I2C_SDA_GPIO_RCC     = 		RCC_AHBPeriph_GPIOB    ;
  }                                                  
  else
  {
     I2C_SCL_PIN      		=   GPIO_Pin_11             ;  //PA5-->PA11
     I2C_SCL_GPIO_PORT   	=  	GPIOA                  ;
     I2C_SCL_GPIO_RCC     =   RCC_AHBPeriph_GPIOA    ;
                                                     
     I2C_SDA_PIN      		=   GPIO_Pin_12             ;   //PB1-->PA12
     I2C_SDA_GPIO_PORT   	=   GPIOA                  ;
     I2C_SDA_GPIO_RCC     = 	RCC_AHBPeriph_GPIOA    ;
  }
}

  void SHT20_GPIO_Init(void)
{
	/* Configure I2C2 pins: PA6->SCL  and  PA7->SDA */
	RCC_AHBPeriphClockCmd( I2C_SCL_GPIO_RCC | I2C_SDA_GPIO_RCC, ENABLE);
	
	GPIO_InitForSCL.GPIO_Pin =  I2C_SCL_PIN ;
	GPIO_InitForSCL.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitForSCL.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitForSCL.GPIO_OType = GPIO_OType_PP;
	GPIO_InitForSCL.GPIO_PuPd = GPIO_PuPd_UP;
	
	
	GPIO_Init(I2C_SCL_GPIO_PORT, &GPIO_InitForSCL);

	
	GPIO_InitForSDAIn.GPIO_Pin =  I2C_SDA_PIN;
	GPIO_InitForSDAIn.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitForSDAIn.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitForSDAIn.GPIO_OType = GPIO_OType_PP;
	GPIO_InitForSDAIn.GPIO_PuPd = GPIO_PuPd_UP;
	
	
	GPIO_InitForSDAOut.GPIO_Pin =  I2C_SDA_PIN;
	GPIO_InitForSDAOut.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitForSDAOut.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitForSDAIn.GPIO_OType = GPIO_OType_PP;
	GPIO_InitForSDAIn.GPIO_PuPd = GPIO_PuPd_UP;
	
	
	GPIO_Init(I2C_SDA_GPIO_PORT, &GPIO_InitForSDAOut);
	
//	SCL_L;
//	SDA_L;
	SCL_H;
	SDA_H;
}

static void Delay_us(unsigned int n)
{
	while(n--);
}

static void SHT_SDA_OUT(void)
{
  GPIO_Init(I2C_SDA_GPIO_PORT, &GPIO_InitForSDAOut);
}

/*----------------------------------
函数名：Si70xx_DATA_IN
描述：Si70xx  DATA(PC15)设置为输入
----------------------------------*/
static void SHT_SDA_IN(void)
{
   GPIO_Init(I2C_SDA_GPIO_PORT, &GPIO_InitForSDAIn);
}

/****************************************************************************
函数介绍：初始化I2C
输入参数：
输出参数：
返回值 ：
****************************************************************************/
void IIC_Init(void)
{
	SHT_IIC_SDA_H;
	SHT_IIC_SCL_H;
}

//产生 IIC 起始信号
void SHT_IIC_Start(void)
{
	SHT_IIC_SDA_H;
	SHT_IIC_SCL_H;
	Delay_us(5);
	SHT_IIC_SDA_L;//START:when CLK is high,DATAchange form high to low
	Delay_us(5);
	SHT_IIC_SCL_L;//锁住 IIC 总线，准备发送或接收数据
	Delay_us(4);
}

//产生 IIC 停止信号
void SHT_IIC_Stop(void)
{

	SHT_IIC_SCL_H;
	SHT_IIC_SDA_L;//STOP:when CLK is high DATAchange form low to high
	Delay_us(5);
	SHT_IIC_SDA_H;//发送 IIC 总线结束信号
	Delay_us(5);
}

//等待应答信号到来
//返回值：1，接收应答失败
// 0，接收应答成功
unsigned char SHT_IIC_Wait_Ack(void)
{
	unsigned char ucErrTime=0;
	unsigned char ReturnValue=0;
	SHT_IIC_SDA_H;
	SHT_IIC_SCL_H;
	Delay_us(5);
	SHT_SDA_IN(); //SDA 设置为输入
	while(SHT_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			ReturnValue=1;
			break;
		}
	}
	SHT_IIC_SCL_L;//时钟输出为 0
	SHT_SDA_OUT(); //SDA 设置为输出
	return ReturnValue;	
}

//产生 ACK 应答
void SHT_IIC_Ack(void)
{
	SHT_IIC_SCL_L;
	SHT_SDA_OUT();
	SHT_IIC_SDA_L;
	Delay_us(2);
	SHT_IIC_SCL_H;
	Delay_us(2);
	SHT_IIC_SCL_L;
}

//不产生 ACK 应答
void SHT_IIC_NAck(void)
{
	SHT_IIC_SCL_L;
	SHT_SDA_OUT();
	SHT_IIC_SDA_H;
	Delay_us(2);
	SHT_IIC_SCL_H;
	Delay_us(2);
	SHT_IIC_SCL_L;	
}

//IIC 发送一个字节
//返回从机 有 无应答
void SHT_IIC_Send_Byte(unsigned char txd)
{
	unsigned char t;

	SHT_IIC_SCL_L;//拉低时钟开始数据传输
	for(t=0;t<8;t++)
	{
              if((txd&0x80)>>7)  SHT_IIC_SDA_H;
              else               SHT_IIC_SDA_L;
		/////SHT_IIC_SDA=(txd&0x80)>>7;
		txd<<=1;
		//Delay_us(2);
		SHT_IIC_SCL_H;
		Delay_us(2);
		SHT_IIC_SCL_L;
		Delay_us(2);
	}	
}

//读一个字节，ack=1 时，发送 ACK，ack=0 时，发送 nACK；
unsigned char SHT_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SHT_SDA_IN();//SDA 设置为输入
	for(i=0;i<8;i++ )
	{
		SHT_IIC_SCL_L;
		Delay_us(2);
		SHT_IIC_SCL_H;
		receive<<=1;
		if(SHT_READ_SDA)receive++;
		Delay_us(1);
	}
	if (!ack)  SHT_IIC_NAck();//发送 nACK
	else       SHT_IIC_Ack(); //发送 ACK
	return receive;	
}

// 检查 IIC 器件是否响应成功
unsigned char IIC_CheckDevice(unsigned char adress)
{
	unsigned char CdAck;
	IIC_Init();
	SHT_IIC_Start();
	SHT_IIC_Send_Byte(adress);
	CdAck = SHT_IIC_Wait_Ack();
	return CdAck;
}

void SI7XX_Init(void)
{
  SHT20_GPIO_Init();
}

//读取一个字节
uint8_t IIC_ReadOneByte(uint8_t readaddr)
{
	uint8_t temp = 0;
	SHT_IIC_Start(); 
	SHT_IIC_Send_Byte(ADDRESS_WRITE); 
	SHT_IIC_Wait_Ack();
	SHT_IIC_Send_Byte(readaddr);
	SHT_IIC_Wait_Ack();
	SHT_IIC_Start(); 
	SHT_IIC_Send_Byte(ADDRESS_READ); 
	SHT_IIC_Wait_Ack();
	temp = SHT_IIC_Read_Byte(0);
	SHT_IIC_Stop();
	return temp;
}

//写入一个字节
void IIC_WriteOneByte(uint8_t writeaddr,uint8_t writedata)
{
	SHT_IIC_Start();
	SHT_IIC_Send_Byte(ADDRESS_WRITE); 
	SHT_IIC_Wait_Ack();
	SHT_IIC_Send_Byte(writeaddr);
	SHT_IIC_Wait_Ack();
	SHT_IIC_Send_Byte(writedata);
	SHT_IIC_Wait_Ack();
	SHT_IIC_Stop();
	Delay_us(10000);
}

//读取多个字节
void IIC_Read(uint8_t readaddr,uint8_t *pbuff,uint8_t num)
{
	while(num--)
	{
		*pbuff++ = IIC_ReadOneByte(readaddr++);
//		num--;
	}
}

//写入多个字节
void IIC_Write(uint8_t writeaddr,uint8_t *pbuff,uint8_t num)
{
	while(num--)
	{
		IIC_WriteOneByte(writeaddr++,*pbuff++);
//		writeaddr++;
//		pbuff++;	
	}
}

//压力传感器读写函数 为了使用官方驱动库
//官方驱动是使用硬件IIC
static int32_t I2C_write(void *handle, uint8_t reg, uint8_t *bufp,uint16_t len)
{
	IIC_Write(reg,bufp,len);
	return 0;
}

static int32_t I2C_read(void *handle, uint8_t reg, uint8_t *bufp,uint16_t len)
{
	IIC_Read(reg,bufp,len);
	return 0;
}

void test_iic(void)
{
	uint8_t ret=0;
	ret = IIC_ReadOneByte(0x0f);
	
	printf("%d\r\n",ret);	
}

float pressure_hPa = 66535;
static stmdev_ctx_t dev_ctx;
float temperature_degC = 64934;
static uint8_t whoamI, rst;

/*******************************************************************************
 * Code
 ******************************************************************************/

void User_lps22hb_Init(void)
{
	  /*
   *  Initialize mems driver interface
   */
    dev_ctx.write_reg = I2C_write;
    dev_ctx.read_reg = I2C_read;
//    dev_ctx.handle = I2C0;
	
//    User_I2C0_Init();
	  SHT20_GPIO_Init();
    whoamI = 0;
	
    //	Check device ID
    lps22hh_device_id_get(&dev_ctx, &whoamI);
    if (whoamI != LPS22HH_ID )
    {
        return ;
    }
//	printf("%d\r\n",whoamI);
    //  Restore default configuration
    lps22hh_reset_set(&dev_ctx, PROPERTY_ENABLE);
    do{
        lps22hh_reset_get(&dev_ctx, &rst);
    }while (rst);
	
    /* Enable Block Data Update */
    lps22hh_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

    /* Set Output Data Rate */
    lps22hh_data_rate_set(&dev_ctx, LPS22HH_10_Hz_LOW_NOISE);
}


void User_lps22hb_Task(void)// Task use time 2ms
{
//    static unsigned short IIC_2000msTimingCnt = 0;
    uint32_t data_raw_pressure;
    int16_t data_raw_temperature;
    lps22hh_reg_t reg;
//    if(TIME_TRUE == OsTimer_CheckTimeOut(IIC_2000msTimingCnt, osTimer_MSec, 2000))
    {
//        IIC_2000msTimingCnt = osTimer_MSec;
            
        // Read output only if new value is available
        lps22hh_read_reg(&dev_ctx, LPS22HH_STATUS, (uint8_t *)&reg, 1);
//		IIC_Read(LPS22HH_STATUS,(uint8_t *)&reg,1);
		
        if ( reg.status.p_da )
        {
            //read pressure_hPa
            memset(&data_raw_pressure, 0x00, sizeof(int32_t));
            lps22hh_pressure_raw_get(&dev_ctx, &data_raw_pressure);	
            pressure_hPa = lps22hh_from_lsb_to_hpa(data_raw_pressure);
            //read temperature_degC  
            memset(&data_raw_temperature, 0x00, sizeof(int16_t));
            lps22hh_temperature_raw_get(&dev_ctx, &data_raw_temperature);
            temperature_degC = lps22hh_from_lsb_to_celsius( data_raw_temperature );
        }	
    }
//	printf("压力%.3f 温度%.3f\r\n",pressure_hPa,temperature_degC);
}
