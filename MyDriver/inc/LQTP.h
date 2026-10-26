#ifndef __LQTP_H
#define __LQTP_H


// #define LQTP_DEBUG                 1
/* 芯片寄存器地址 */
#define LQTP_PRS_B2              0x00    //压力寄存器1, 高位
#define LQTP_PRS_B1              0x01    //压力寄存器2
#define LQTP_PRS_B0              0x02    //压力寄存器3，低位
#define LQTP_TMP_B2              0x03    //温度寄存器1，高位
#define LQTP_TMP_B1              0x04    //温度寄存器2，
#define LQTP_TMP_B0              0x05    //温度寄存器3，低位
#define LQTP_PRS_CFG             0x06    //压力精度配置寄存器
#define LQTP_TMP_CFG             0x07    //温度精度配置寄存器
#define LQTP_MEAS_CFG            0x08    //传感器模式
#define LQTP_CFG_REG             0x09    //中断及FIFO设置
#define LQTP_INT_STS             0x0A    //中断状态
#define LQTP_PRS_FIFO            0x0B    //连续测量
#define LQTP_PRS_RST             0x0C    //软复位

#define LQTP_COEF_REG            0x10    //校准系数寄存器，0x10-0x21 18个

/* 配置指令 */
#define LQTP_ORDER_Rst           0x89		//软复位1000 1001 ，FIFO清除，复位

typedef struct
{
    short c0;
    short c1;
    int c00;
    int c10;
    short c01;
    short c11;
    short c20;
    short c21;
    short c30;  
} LQTP_Coefficient;

void LQTP_GPIO_Init(void);
void LQTP_Get_Pressure(float *pressure, unsigned char sensor_flag);


#endif /* __LQTP_H */
