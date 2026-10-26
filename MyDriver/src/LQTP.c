#include "LQTP.h"
#include "IIC_LQTP.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

static LQTP_Coefficient coef0;
static LQTP_Coefficient coef1;
static int press_raw;
static int temp_raw;

static float lqtp_pressure0 = 0.0f;
static float lqtp_pressure1 = 0.0f;

static void Delay_ms(unsigned int n)
{
	Delay_Us(n * 1000);
}

static void LQTP_Soft_Reset()
{
	unsigned char send_buf = LQTP_ORDER_Rst;
	IICSendByte_S(LQTP_PRS_RST, &send_buf, 1); // 只用发一个命令
    Delay_ms(10);
}

/**
 * @brief  等待传感器初始化完成，硬件可用，系数可读
 * @author qinling
 * @date   2026/01/26
 * @param  
 * @return 
 * @note   
 */
static unsigned char LQTP_Wait_All_Ready()
{
    unsigned char status = 0;
    int timeout = 72000;
    
    while(timeout > 0)
    {
        IICReceiveByte_S(LQTP_MEAS_CFG, &status, 1);  //获取传感器状态
        if((status & 0xC0) == 0xC0)  //全部就绪 1100 XXXX
        {
            return 0;
        }
        timeout--;
    }
    printf("LQTP sensor timeout, sensor status:%x\r\n", status);
    return 1;
}

/**
 * @brief  设置传感器的基础配置，精度，采样等
 * @author qinling
 * @date   2026/01/07
 * @param  
 * @return 
 * @note   
 */
static void LQTP_Reg_Init_Set()
{
    unsigned char send_buf = 0;
    send_buf = 0x74; //0111 0100 标准采样16/s
    IICSendByte_S(LQTP_PRS_CFG, &send_buf, 1);
    Delay_ms(10);

    send_buf = 0; //0000 0000 标准采样1/s, 温度无需高精度
    send_buf |= 0x80; //温度需要最高位置1
    IICSendByte_S(LQTP_TMP_CFG, &send_buf, 1);
    Delay_ms(10);

    send_buf = 0x07;  //0000 0111 连续测量压力+温度
    IICSendByte_S(LQTP_MEAS_CFG, &send_buf, 1);
    Delay_ms(10);

    send_buf = 0x04;  //0000 0100 关中断+压力过采样+关FIFO
    IICSendByte_S(LQTP_CFG_REG, &send_buf, 1);
    Delay_ms(10);
}

/**
 * @brief  获取校准系数
 * @author qinling
 * @date   2026/01/07
 * @param  
 * @return 
 * @note   
 */
static void LQTP_Read_Coefficient(unsigned char sensor_flag)
{
    LQTP_Coefficient coef = {0};
    unsigned short temp_16 = 0;
    unsigned int temp_32 = 0;
    memset(&coef, 0, sizeof(LQTP_Coefficient)); //初始化0
    unsigned char cal_coef[18] = {0};
    IICReceiveByte_S(LQTP_COEF_REG, cal_coef, 18); //系数原始数据

    temp_16 |= (cal_coef[0]<<4);
    temp_16 |= ((cal_coef[1] >> 4) & 0x0F);
    coef.c0 = (temp_16 & 0x0800) ? (temp_16 | 0xF000) : temp_16; // 12位补码转int16_t

    temp_16 = 0;
    temp_16 |= ( (cal_coef[1] & 0x0F) << 8 ); 
    temp_16 |= cal_coef[2];  
    coef.c1 = (temp_16 & 0x0800) ? (temp_16 | 0xF000) : temp_16; // 12位补码转int16_t

    temp_32 |= ( (unsigned int)cal_coef[3] << 12 );
    temp_32 |= ( (unsigned int)cal_coef[4] << 4 );
    temp_32 |= ( (cal_coef[5] >> 4) & 0x0F );
    coef.c00 = (temp_32 & 0x080000) ? (temp_32 | 0xFFF00000) : temp_32; // 20位补码转int32_t

    temp_32 = 0;
    temp_32 |= ( (unsigned int)(cal_coef[5] & 0x0F) << 16 );
    temp_32 |= ( (unsigned int)cal_coef[6] << 8 );
    temp_32 |= cal_coef[7];
    coef.c10 = (temp_32 & 0x080000) ? (temp_32 | 0xFFF00000) : temp_32; // 20位补码转int32_t

    coef.c01 = ( (short)cal_coef[8] << 8 ) | cal_coef[9];

    coef.c11 = ( (short)cal_coef[10] << 8 ) | cal_coef[11];

    coef.c20 = ( (short)cal_coef[12] << 8 ) | cal_coef[13];

    coef.c21 = ( (short)cal_coef[14] << 8 ) | cal_coef[15];

    coef.c30 = ( (short)cal_coef[16] << 8 ) | cal_coef[17];

    if(sensor_flag == 0)
    {
        memcpy(&coef0, &coef, sizeof(LQTP_Coefficient));
    }else{
        memcpy(&coef1, &coef, sizeof(LQTP_Coefficient));
    }
}

/**
 * @brief  获取温度&压力原始数据
 * @author qinling
 * @date   2026/01/07
 * @param  
 * @return 
 * @note   
 */
static void LQTP_Read_Sensor_Raw()
{
    unsigned char prs_read_data[3] = {0};
    unsigned char tmp_read_data[3] = {0};
    int raw_data = 0;
 
    IICReceiveByte_S(LQTP_PRS_B2, prs_read_data, 3); //压力原始数据
    IICReceiveByte_S(LQTP_TMP_B2, tmp_read_data, 3); //温度原始数据
   

#ifdef LQTP_DEBUG
    printf("Press: 0x00: %02x, 0x01: %02x, 0x02: %02x\r\n", prs_read_data[0], prs_read_data[1], prs_read_data[2]);
    printf("Temp: 0x03: %02x, 0x04: %02x, 0x05: %02x\r\n", tmp_read_data[0], tmp_read_data[1], tmp_read_data[2]);
#endif

    //压力原始值
    raw_data = 0;
    raw_data |= ( (unsigned int)prs_read_data[0] << 16 );  // B2：第23~16位
    raw_data |= ( (unsigned int)prs_read_data[1] << 8 );   // B1：第15~8位
    raw_data |= prs_read_data[2];                      // B0：第7~0位
    raw_data &= 0X007FFFFF;  //屏蔽23位以上
    press_raw = (raw_data & 0x00400000) ? (raw_data | 0xFF800000) : raw_data;

    //温度原始值
    raw_data = 0;
    raw_data |= ( (unsigned int)tmp_read_data[0] << 16 );  // B2：第23~16位
    raw_data |= ( (unsigned int)tmp_read_data[1] << 8 );   // B1：第15~8位
    raw_data |= tmp_read_data[2];                      // B0：第7~0位
    raw_data &= 0X007FFFFF;  //屏蔽23位以上
    temp_raw = (raw_data & 0x00400000) ? (raw_data | 0xFF800000) : raw_data;
}

/**
 * @brief  计算压力值
 * @author qinling
 * @date   2026/01/07
 * @param  
 * @return 
 * @note   
 */
static void LQTP_Calculate_Pressure(unsigned char sensor_flag)
{
    float temp0 = 0.0f;
    float temp1 = 0.0f;
    LQTP_Coefficient coef = {0};
    if(sensor_flag == 0)
    {
        memcpy(&coef, &coef0, sizeof(LQTP_Coefficient));
    }else{
        memcpy(&coef, &coef1, sizeof(LQTP_Coefficient));
    }
    LQTP_Read_Sensor_Raw();  //读测量原始值

    float kP = 253952.0f; //16times
    float kT = 524288.0f; //single
    float press = (float)press_raw;
    float temp = (float)temp_raw;
    float c0 = (float)coef.c0;
    float c1 = (float)coef.c1;
    float c00 = (float)coef.c00;
    float c10 = (float)coef.c10;
    float c01 = (float)coef.c01;
    float c11 = (float)coef.c11;
    float c20 = (float)coef.c20;
    float c21 = (float)coef.c21;
    float c30 = (float)coef.c30;

#ifdef LQTP_DEBUG
    if(sensor_flag == 0)
    {
        printf("P_in:");
    }else{
        printf("P_out:"); 
    }
    printf("p_raw:%f, temo_raw:%f, kP:%f, kT:%f\r\n", press, temp, kP, kT);
    printf("c0:%f, c1:%f, c00:%f, c10:%f, c01:%f, c11:%f, c20:%f, c21:%f, c30:%f, ", 
    c0, c1, c00, c10, c01, c11, c20, c21, c30);
#endif

    press = press/kP;
    temp = temp/kT;
   
    if(sensor_flag == 0)
    {
        lqtp_pressure0 = c00 + press * (c10 + press * (c20 + press * c30)) + temp * c01 + 
                            temp * press * (c11 + press * c21);  //公式计算
        temp0 = c0 * 0.5 + c1 * temp;
    }else
    {
        lqtp_pressure1 = c00 + press * (c10 + press * (c20 + press * c30)) + temp * c01 + 
                            temp * press * (c11 + press * c21);  //公式计算
        temp1 = c0 * 0.5 + c1 * temp;
    }

#ifdef LQTP_DEBUG
    if(sensor_flag == 0)
    {
        printf("Press_in:%f, temp_in:%f\r\n", lqtp_pressure0, temp0);
    }else{
        printf("Press_out:%f, temp_out:%f\r\n", lqtp_pressure1, temp1); 
    }
#endif
    
}

/**
 * @brief  获取压力值（避免使用全局变量）
 * @author qinling
 * @date   2026/01/23
 * @param  
 * @return 
 * @note   
 */
void LQTP_Get_Pressure(float *pressure, unsigned char sensor_flag)
{
    LQTP_Calculate_Pressure(sensor_flag);
    if(sensor_flag == 0)
    {
        *pressure = lqtp_pressure0 / 100.0f; //保持位数一致
    }else
    {
        *pressure = lqtp_pressure1 / 100.0f; //保持位数一致
    } 
}


void LQTP_GPIO_Init(void)
{
    //两个压力芯片分别初始化
    IIC_GPIO_SET_Init(0);
    LQTP_IIC_GPIO_Init();
    LQTP_Soft_Reset();
    LQTP_Reg_Init_Set();
    LQTP_Wait_All_Ready();
    printf("Chip0 initial\r\n");

    LQTP_Read_Coefficient(0);

    IIC_GPIO_SET_Init(1);
    LQTP_IIC_GPIO_Init();
    LQTP_Soft_Reset();
    LQTP_Reg_Init_Set();
    LQTP_Wait_All_Ready();
    printf("Chip1 initial\r\n");

    LQTP_Read_Coefficient(1);

}
