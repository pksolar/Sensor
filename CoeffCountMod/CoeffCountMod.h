/********************************************************************** 
*  Copyright (C), WuHan Cubic Optoelectronics
*  FileName:  COEFFCOUNT.H
*  Author:shipingjing      Version :0.01     Date: 2016/10/18
*  Email:13871216116@163.com	QQ:164658701
*  Description: 

*  Others:  

*  Function List:  
	1.
	2.

*  History:       
    1. Date:20161018
       Author:shipingjing
       Modification:建立
    2. 
***********************************************************************/



#ifndef _COEFFCOUNT_H_
#define _COEFFCOUNT_H_
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
	
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	~                    FUNCTION PROTOTYPES
	~            (外部函数,可以供其它模块使用的(extern)函数)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
		/******************************************************************** 
		*
		* 名称: FnInverse
		* 说明: 
		* 功能: 求方程根
				注意。有可能取两个根，不是离fInputX最近的。
			
		* 调用:	printf()
		* 输入: 
			fInputX, fInputY,
			*Coefficient[4] 三次方程系数
			
		* 返回值: OutPutX
			由	fInputY,和	*Coefficient[4] 三次方程系数
			求的方程根
				 fInputY=*(Coefficient+0)*OutPutX*OutPutX*OutPutX
								+*(Coefficient+1)*OutPutX*OutPutX
								+*(Coefficient+2)*OutPutX;
								+*(Coefficient+3);
		***********************************************************************/	
		float FnInverse(float *Coefficient,float fInputX,float fInputY);

		/******************************************************************** 
		*
		* 名称: fnCoeffCount3P
		* 说明: 
		* 功能: 计算系数
		* 调用:	printf()

		* 输入: fInputX[3],fInputY[3],
		* 返回值:
				float *Coefficient[3]  二次方程系数 

			 fInputY=*(Coefficient+0)*fInputX*fInputX
							+*(Coefficient+1)*fInputX
							+*(Coefficient+2);
		***********************************************************************/	
		void fnCoeffCount3P(float fInputX[3],float fInputY[3],float *Coefficient);


		/******************************************************************** 
		*
		* 名称: fnCoeffCountModDebugEn
		* 说明: 
		* 功能: 允许输出调试信息
		* 调用:	
		* 输入: 
		      INENFlag =0 禁止输出调试信息
					INENFlag =1     输出调试信息	 并且输出版本号	
		* 返回值: 无
		***********************************************************************/
		void fnCoeffCountModDebugEn(unsigned char INENFlag);
				
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	~                    PRIVATE VARIABLE 
	~                (私有变量,仅在MAIN中定义一次)
	~    (本模块中的一些参数的设置,在模块内部都使用的外部变量extern)	
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/	
	#ifdef	_MAIN_	//进行原型定义


	#endif	
#endif
