#ifndef __IIC_LQTP_H
#define __IIC_LQTP_H

/****************** API Interface ************************/
void IICSendByte(unsigned char SendDat);
void IICSendByte_S(unsigned char RegAddr, unsigned char * SendDat, unsigned char Send_Num);
unsigned char IICReceiveByte(unsigned char ack);
void IICReceiveByte_S(unsigned char RegAddr, unsigned char * RcvDat, unsigned char Rcv_Num);

void IIC_GPIO_SET_Init(int flag);
void LQTP_IIC_GPIO_Init(void);

void Delay_Us(unsigned int n);


/****************** API Interface ************************/

#endif /* __IIC_LQTP_H */
