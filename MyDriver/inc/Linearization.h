/********************************************************************** 
*  Copyright (C), WuHan Cubic Optoelectronics
*  FileName:  .H
*  Author:shipingjing      Version :2.0     Date: 2006/06/09
*  Email:shipingjing@yahoo.com.cn	QQ:164658701
*  Description: 

*  Others:  

*  Function List:  
	1.
	2.

*  History:       
    1. Date:20060808
       Author:shipingjing
       Modification:建立
    2. 
***********************************************************************/

#ifndef _LINEARIZATION_H_
#define _LINEARIZATION_H_
	#ifndef uchar
		#define uchar unsigned char
	#endif
	#ifndef uint
		#define uint  unsigned int
	#endif
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	~                    PUBLIC VARIABLE 
	~           (公共的全局变量,可以供其它模块使用的(extern)变量)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
extern int calibrationO2;
extern int calibrationO2Fac;
 extern int calibrationAir;
extern int calibrationAirFac;   
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	~                    FUNCTION PROTOTYPES
	~            (外部函数,可以供其它模块使用的(extern)函数)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
extern int deltaFf;
extern int deltaFb;
extern float Ck1,Cb1,Ck2,Cb2,Ck3,Cb3,Ck4,Cb4;
extern float flowX1,flowX2,flowX3,flowX4;

extern unsigned char  debugflag;

extern  float fSlopeFsumN2[3];//温度修正系数
extern  float fSlopeFsumAir[3];	//温度修正系数
extern  float fSlopeFsumO2[3];

extern int nFxdFair;

typedef struct
{
	char type;  //标定类型：流量、N2、Air、O2
	unsigned char point; //标定点
	short auto_flag; //是否自动应用温度及飞行时间
	int concentrtion; //浓度
	int temperature; //温度
	long Ff;  //飞行时间正
	long Fb;  //飞行时间负
} SelfSetAnchor;

void FnPointSave(uchar tGasNUM,uchar tStep,int tInPut);
void FnLinearization(uchar tGasNUM);	
void fnPointRevert(uchar tGasNUM);
void FnPointSaveT(uchar tGasNUM, int tInPut);
void FnLinearization1(void);
void fnSaveAnchor(SelfSetAnchor anchor);

	
	//int nFnO2CorrectionT(int nInTemp,int nInO2);
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	~                    PRIVATE VARIABLE 
	~                (私有变量,仅在MAIN中定义一次)
	~    (本模块中的一些参数的设置,在模块内部都使用的外部变量extern)	
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/	
#ifdef	_MAIN_	//进行原型定义

#endif	
#endif
