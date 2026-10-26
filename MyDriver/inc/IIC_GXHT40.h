#ifndef _IIC_GXHT40_H
#define _IIC_GXHT40_H
	
/****************** API Interface ************************/
void I2CSendByte(unsigned char SendDat);
void I2CSendByte_S(unsigned char SlaAdd, unsigned char * SendDat, unsigned char Send_Num);
unsigned char I2CReceiveByte(unsigned char ack);
void I2CReceiveByte_S(unsigned char SlaAdd, unsigned char * RcvDat, unsigned char Rcv_Num);

/****************** API Interface ************************/


#endif
