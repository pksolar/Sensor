#ifndef __RXExecute_H
#define __RXExecute_H

#include "main.h"
#include "SubFun.h"

/*~~~~~~~~~~~~串口通讯~~~~~~~~~~~~*/
//erroor code
#define LengthError 			0X01
#define NotCommand				0x02
#define EorrCommandStatu 	    0x03
#define CantExeCmd				0x04

#define BUSH_RX	0X05 		//机器在调零，校准，预热，捡漏
#define ACK_RX	0x16
#define NACK_RX	0X06

#define YY      1
#define MT      2

#define SENSOR_VERSION    YY



void RXExecute(SensorValue sensor_value, FlashData *flash_data);
void AutoSendData(SensorValue sensor_value);

extern unsigned char cAlarmRS232Count;    //无串口计数
extern int nOrderFf;		//
extern int nOrderFb;		//T1计数
extern unsigned char  CaliFlag;
extern const unsigned char version[];
extern const unsigned char sn[];


#endif /* __RXExecute_H */
