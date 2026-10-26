#include "RXExecute.h"
#include "SubFun.h"
#include "Linearization.h"
#include <stdio.h>
#include "usart.h"
#include "flash.h"
#include "protocol_codec.h"
#include "persistent_layout.h"
#include <string.h>

#if SENSOR_VERSION == YY
	const unsigned char version[] = {"3.DY.001"};  // 丹阳鱼跃，有压力芯片
	const unsigned char sn[] = {"Cubic20260410 004-DY-F"};
#elif SENSOR_VERSION == MT
	const unsigned char version[] = {"3.MT.001"};
	const unsigned char sn[] = {"Cubic20260510 004-MT-F"};
#endif

static unsigned char temprx[35];

static SelfSetAnchor anchor = {0};   //流量4个点，浓度3个点

unsigned char cAlarmRS232Count;    //无串口计数

int nOrderFf;		//
int nOrderFb;		//T1计数

unsigned char  CaliFlag;

static void SendResponse(const unsigned char *frame)
{
	unsigned short length;
	unsigned short index;
	unsigned char checksum;

	length = frame[1] + 2;
	checksum = Protocol_ChecksumByte(frame, length);
	for (index = 0; index < length; index++)
	{
		putchar(frame[index]);
	}
	putchar(checksum);
}

static unsigned char SerialJudge(unsigned char L)
{
	if(temprx[1]!=L)
	{	//接受命令长度不对
		Protocol_EncodeLengthError(temprx);
	}
	else
	{
		return	1;
	}
	return	0;
}

void AutoSendData(SensorValue sensor_value)
{
	#if SENSOR_VERSION == YY
		Protocol_EncodeLiveDataYY(temprx,
		                         mysGasTrait5P[0].ReResult,
		                         mysGasTrait5P[1].ReResult,
		                         mysGasTrait5P[0].Result,
		                         sensor_value.temperature_NTC);
	#elif SENSOR_VERSION == MT
		Protocol_EncodeLiveDataMT(temprx,
		                         mysGasTrait5P[0].ReResult,
		                         mysGasTrait5P[1].ReResult,
		                         mysGasTrait5P[0].Result,
		                         sensor_value.temperature_NTC,
		                         sensor_value.press_in,
		                         sensor_value.press_out);
	#endif
					
	SendResponse(temprx);
}

void RXExecute(SensorValue sensor_value, FlashData *flash_data)
{
 unsigned char tSetStep;
 unsigned char SaveGasFlag;
 unsigned char OutputFlag;
 int tempint;

 Get_OutputFlag(&OutputFlag);

	for(int i=0;i<(temprx[1]+3);i++)
	{	
		temprx[i]=rxBuffer[i];
		rxBuffer[i]=0;
//		printf("%d ",(int)temprx[i]);
//		putchar(temprx[i]);
	}	
// ****************指令说明******************


// *****************************************
	if((temprx[0]==0x11)
	   && Protocol_IsChecksumValid(temprx, (unsigned short)(temprx[1] + 3)))//IPadd)
	{				
		cAlarmRS232Count = 0;
		
		switch(temprx[2])	
		{
//输出实时数据
        case 1:	
        if(SerialJudge(1))
        {
		#if SENSOR_VERSION == YY
			Protocol_EncodeLiveDataYY(temprx,
			                         mysGasTrait5P[0].ReResult,
			                         mysGasTrait5P[1].ReResult,
			                         mysGasTrait5P[0].Result,
			                         sensor_value.temperature_NTC);
		#elif SENSOR_VERSION == MT
			Protocol_EncodeLiveDataMT(temprx,
			                         mysGasTrait5P[0].ReResult,
			                         mysGasTrait5P[1].ReResult,
			                         mysGasTrait5P[0].Result,
			                         sensor_value.temperature_NTC,
			                         sensor_value.press_in,
			                         sensor_value.press_out);
		#endif
		}				
		break;
			
//读取电压	
			case 2:
				if(SerialJudge(2))
				{
					temprx[0]	= ACK_RX;		//回答类型
					temprx[1]	= 12;			//长度	
		//			temprx[2]	= 1;			//命令号
				
					if(temprx[3] == 0)
					{
						temprx[4]  = ((nOrderFf)>>8)&0xff;
						temprx[5]  =  (nOrderFf)    &0xff;	

						temprx[6]  = ((nOrderFb)>>8)&0xff;
						temprx[7]  =  (nOrderFb)    &0xff;	

											
					  tempint = sensor_value.temperature_NTC;
					  tempint = (int)((tempint /10.0)+0.5);
						
						temprx[8]  = ((tempint)>>8)&0xff;
						temprx[9]  =  (tempint)    &0xff;	

						temprx[10]  = ((mysGasTrait5P[0].Result)>>8)&0xff;
						temprx[11]  =  (mysGasTrait5P[0].Result)    &0xff;	

						temprx[12]  = ((mysGasTrait5P[1].Result)>>8)&0xff;
						temprx[13]  =  (mysGasTrait5P[1].Result)    &0xff;				
					}
					else if(temprx[3] == 1)		//同时输出O2和 制氧机氧气
					{
						temprx[4]  = ((flash_data->coef.Cn_O2)>>8)&0xff;
						temprx[5]  =  (flash_data->coef.Cn_O2)    &0xff;	

						temprx[6]  = ((0)>>8)&0xff;
						temprx[7]  =  (0)    &0xff;	

											
						tempint = sensor_value.temperature_NTC;
						tempint = (int)((tempint /10.0)+0.5);
						
						temprx[8]  = ((tempint)>>8)&0xff;
						temprx[9]  =  (tempint)    &0xff;	

						temprx[10]  = ((mysGasTrait5P[0].Result)>>8)&0xff;
						temprx[11]  =  (mysGasTrait5P[0].Result)    &0xff;	

						temprx[12]  = ((mysGasTrait5P[1].Result)>>8)&0xff;
						temprx[13]  =  (mysGasTrait5P[1].Result)    &0xff;				
					}
					else 
					{
						temprx[0]	=NACK_RX;		//回答类型
						temprx[1]	=2;				//长度	
					//  temprx[2]	=1;				//命令号
						temprx[3] = CantExeCmd;	//参数		
					}
				 }
			break;
								 
				case 3:
				if(SerialJudge(1))
				{
					temprx[0]	= ACK_RX;		//回答类型
					temprx[1]	= 9;			//长度	
					temprx[2]	= 3;			//命令号
					
					tempint = sensor_value.temperature_NTC;//温度
					tempint = (int)((tempint /10.0)+0.5);
					
					temprx[3]=(tempint)>>8;
					temprx[4]=(tempint)& 0xff;
					
					tempint = sensor_value.nRH;//湿度
					tempint = (int)((tempint));
					
					temprx[5]=(tempint)>>8;
					temprx[6]=(tempint)& 0xff;
					
					tempint = (int)sensor_value.press_in;//压力测试结果
//					tempint = (int)((tempint )+0.5);
					temprx[7]=(tempint)>>8;
					temprx[8]=(tempint)& 0xff;
					
					tempint = (int)sensor_value.press_out;//压力测试结果
					temprx[9]=(tempint)>>8;
					temprx[10]=(tempint)& 0xff;
				}				
			break;
/*
//带泵操作阀调零
			case 3:
				if(SerialJudge(1,temprx))
				{
					temprx[0]	= ACK_RX;		//回答类型
					temprx[1]	= 1;			//长度	
				//	temprx[2]	=  ;			//命令号
									
					TICCount = 0;
			//		ZeroFlag = 1;
					fnGasZeroN2();
					fnGasZeroO2();
				}				
			break;
*/
//更改触发顺序	11 01 05 E9
			case 5:
				if(SerialJudge(1))
				{
					temprx[0]	= ACK_RX;		//回答类型
					temprx[1]	= 2;			//长度	
		//			temprx[2]	= 1;			//命令号
		
					if(flash_data->SendOrder == 0)
						flash_data->SendOrder = 1;
					else
						flash_data->SendOrder = 0;
					
					ReadFlashAll();
							WriteEeprom(PERSISTENT_ADDR_SEND_ORDER, flash_data->SendOrder);
  				WriteFlashAll();	

					temprx[3] = flash_data->SendOrder;		
					
				 }
			break;
				 
				 
//自动发送设置 11 01 07 E7
			case 7:
				if(SerialJudge(1))
				{
					temprx[0]	= ACK_RX;		//回答类型
					temprx[1]	= 2;			//长度	
		//		temprx[2]	= 1;			//命令号
		
					if(flash_data->cENAutoSendFlag == 0)
						flash_data->cENAutoSendFlag = 1;
					else
						flash_data->cENAutoSendFlag = 0;
					
					ReadFlashAll();
							WriteEeprom(PERSISTENT_ADDR_AUTO_SEND, flash_data->cENAutoSendFlag);
  				WriteFlashAll();	

					temprx[3] = flash_data->cENAutoSendFlag;						
				 }
			break;

			case 8:
				if(SerialJudge(2))
				{
					temprx[0]	= ACK_RX;		//回答类型
					temprx[1]	= 2;			//长度	
					//			temprx[2]	= 1;			//命令号
					ReadFlashAll();
					WriteEeprom(PERSISTENT_ADDR_BAUD_RATE, temprx[3]);
			
					WriteFlashAll();	
					}
			break;	
//温度标定 
// 11 02 09 00 e4
// 11 02 09 01 e3
// 11 02 09 02 e2
			case 9:	
				if(SerialJudge(2))
				{
					temprx[0]	=ACK_RX;		//回答类型
					temprx[1]	=1;				//长度	
				//	temprx[2]	=1;				//命令号
				
					if(temprx[3] < 3)
					{
						FnPointSaveT(temprx[3], 209);		//Air标定				
					}					
					CaliFlag =1;			//更新标定系数				
				}
			break;
		
//设置/查询参数	
//11 04 0d 00 ff ff e0
//11 04 0d 01 ff ff df
//11 04 0d 02 ff ff de		
				
			case 13:
				if(SerialJudge(4))
				{
					temprx[0]	= ACK_RX;		//回答类型
					temprx[1]	= 4;			//长度	
		//			temprx[2]	= 13;			//命令号
		
					SaveGasFlag = temprx[3] ;
					tempint = temprx[4]*256+temprx[5];
					if(SaveGasFlag < 3)
					{
						if(tempint != -1)
						{
							flash_data->nWarnVale[SaveGasFlag] = tempint;
							ReadFlashAll();
								WriteEeprom((PERSISTENT_ADDR_WARN_VALUES + SaveGasFlag * 2), temprx[5]);
							WriteEeprom((PERSISTENT_ADDR_WARN_VALUES + SaveGasFlag * 2 + 1), temprx[4]);
							WriteFlashAll();
						}	
						temprx[4] = (flash_data->nWarnVale[SaveGasFlag] >>8  );
						temprx[5] = (flash_data->nWarnVale[SaveGasFlag] &0xff);
					}
					else if((SaveGasFlag ==3))
					{
						if(tempint != -1)
						{
							flash_data->coef.K_O2 = tempint;
							
							ReadFlashAll();
								WriteEeprom(PERSISTENT_ADDR_O2_K, temprx[5]);
							WriteEeprom(PERSISTENT_ADDR_O2_K + 1, temprx[4]);
							WriteFlashAll();
						}	
						temprx[4] = (flash_data->coef.K_O2 >>8  );
						temprx[5] = (flash_data->coef.K_O2 &0xff);
					}
					else if((SaveGasFlag ==4))
					{
						if(tempint != -1)
						{
							flash_data->coef.B_O2 = tempint;
							ReadFlashAll();
								WriteEeprom(PERSISTENT_ADDR_O2_B, temprx[5]);
							WriteEeprom(PERSISTENT_ADDR_O2_B + 1, temprx[4]);
							WriteFlashAll();
						}	
						temprx[4] = (flash_data->coef.B_O2 >>8  );
						temprx[5] = (flash_data->coef.B_O2 &0xff);
					}
					else
					{
						temprx[0]	=NACK_RX;		//回答类型
						temprx[1]	=2;				//长度	
					//  temprx[2]	=1;				//命令号
						temprx[3] = CantExeCmd;	//参数								
					}	
				 }
			break;			
			
//=====================系统信息============================
			case	30:		//查询软件版本
			case	31:		//查询仪器编号
				if(SerialJudge(1))
				{
					temprx[0]	=ACK_RX;		//回答类型
				//	temprx[1]	=1;				//长度	
				//  temprx[2]	=1;				//命令号

					if(temprx[2] == 30)
					{	
						temprx[1]	= 9;
						for(int i =0;i<8; i++)
						{
							temprx[3+i] = version[i];
						}
					}
					else if(temprx[2] == 31)
					{
						temprx[1]	= 11;
						for(int i =0;i<10; i++)
						{			
							temprx[3+i] = ReadEeprom(PERSISTENT_ADDR_SERIAL_NUMBER + i);
						}
					}
					else
					{
						temprx[0]	=NACK_RX;		//回答类型
						temprx[1]	=2;				//长度	
					//  temprx[2]	=1;				//命令号
						temprx[3] = CantExeCmd;	//参数	
					}
				}
			break;

			case 32: //写入SN码
				if(SerialJudge(9))
				{
					temprx[0]	=ACK_RX;		//回答类型
					temprx[1]	=1;				//长度
					ReadFlashAll();
					for(int i=0;i<8;i++)
					{
						WriteEeprom(PERSISTENT_ADDR_SERIAL_NUMBER + i, temprx[3+i]);
					}
					WriteEeprom(PERSISTENT_ADDR_SERIAL_NUMBER + 8, 0);
					WriteEeprom(PERSISTENT_ADDR_SERIAL_NUMBER + 9, 0);
					WriteFlashAll();
				}
/*
//设置编号
//01 仪器编号
//02 主板编号
//03 红外平台编号
//04 流量计编号
			case	35:
				if(SerialJudge( 12,temprx))
				{
					temprx[0]	=ACK_RX;		//回答类型
					temprx[1]	=1;				//长度	
				//	temprx[2]	=1;				//命令号
					if(temprx[3] == 0)
					{
						temprx[3]= 0;
						for(i =0;i<5;i++)
						{
							ListNumber[i] = temprx[4+i+i] *256+temprx[5+i+i];
							if((ListNumber[i]<0)&&(ListNumber[i]>9999))
							{
								temprx[3]= 1;
							}
						}

						if(temprx[3] == 0)
						{
							tempchar[0]=ListNumber[0]>>8;
							tempchar[1]=ListNumber[0]&0xff;
							tempchar[2]=ListNumber[1]>>8;
							tempchar[3]=ListNumber[1]&0xff;
							fnflashsave(5,tempchar);
			
							tempchar[0]=ListNumber[2]>>8;
							tempchar[1]=ListNumber[2]&0xff;
							tempchar[2]=ListNumber[3]>>8;
							tempchar[3]=ListNumber[3]&0xff;
							fnflashsave(6,tempchar);
			
							tempchar[0]=ListNumber[4]>>8;
							tempchar[1]=ListNumber[4]&0xff;
							fnflashsave(7,tempchar);
						}
						else
						{
							temprx[0]	=NACK_RX;		//回答类型
							temprx[1]	=2;				//长度	
						//  temprx[2]	=1;				//命令号
							temprx[3] = CantExeCmd;	//参数		
						}
					}
				}
			break;
*/

//=====================两点标定============================
//11 04 32 00 00 00 b9
//11 04 33 00 00 00 b8
/*			case 50	:
			case 51	:
				if(SerialJudge(4,temprx))
				{
					temprx[0]	=ACK_RX;		//回答类型
					temprx[1]	=1;				//长度	
				//  temprx[2]	=1;				//命令号

					//判断标定第几步
					tSetStep = temprx[2] - 50;
					//判断标定气体种类
					SaveGasFlag = temprx[3] ;

					if( SaveGasFlag  == 0x00 )
					//判断气体是否标定有效
					{

					}
					else
					{
						temprx[0]	=NACK_RX;		//回答类型
						temprx[1]	=2;				//长度	
					//  temprx[2]	=1;				//命令号
						temprx[3] = CantExeCmd;	//参数
					}
					SaveGasFlag=0;
				}
			break;
*/
//11 0A 46 00 00 00 A0
//11 0A 47 FF 00 00 00 00 00 00 00 00 9F
//11 0A 48 FF 00 00 00 00 00 00 00 00 9E
//11 0A 49 FF 00 00 00 00 00 00 00 00 9D
//11 0A 4A FF 00 00 00 00 00 00 00 00 9C

//11 0C 4B FF 00 00 00 00 00 00 00 00 00 00  99
//11 0C 4C FF 00 00 00 00 00 00 00 00 00 00  98

//五气分析仪标定
			case 70	:	//零点 	 46  4b
			  case 71	:	//空
			  case 72	:	//空
			  case 73	:	//空
			case 74	:	//量程   4a 4c
			case 75	:	//零点
			case 76	:	//高量程
				if(SerialJudge(4))
				{
					temprx[0]	=ACK_RX;		//回答类型
					temprx[1]	=1;				//长度	
				//	temprx[2]	=1;				//命令号

                //判断标定第几步
                //	tSetStep = temprx[2] - 70;
                //	tSetStep = temprx[2] - 75;
                if(temprx[2] == 70)//46
                    tSetStep = 0;
					else if(temprx[2] == 71)//47
                    tSetStep = 1;
					else if(temprx[2] == 72)//48
                    tSetStep = 2;
					else if(temprx[2] == 73)//49
                    tSetStep = 3;
					else if(temprx[2] == 74)//4A
                    tSetStep = 4;
					else if(temprx[2] == 75)//4B
                    tSetStep = 5;

                //判断标定气体种类
                SaveGasFlag = temprx[3] ;
                if(( SaveGasFlag == 1)) //流量标定
                {
                    tempint = temprx[4] * 256 + temprx[5];
                    //参数存储
                    FnPointSave(SaveGasFlag, tSetStep, tempint);
                    //厂家标定时同时标定用户
////////                    if((tSetStep == 0))
////////                    {
////////                        FnPointSave(SaveGasFlag, 2, tempint);
////////                    }
////////                    else if(tSetStep == 1)
////////                    {
////////                        FnPointSave(SaveGasFlag, 3, tempint);
////////                    }
                    CaliFlag = 1;
                }
                if(( SaveGasFlag == 2))//标定空气点
                {
                    tempint = temprx[4] * 256 + temprx[5];
                    //参数存储
                    //							if(tempint > 500)
                    //							{
                    if(sensor_value.temperature_NTC < 1800)
                    {
                        //FnPointSave(0,0,tempint);
						FnPointSaveT(0, 209);
                    }
                    else
                    {
						if(sensor_value.temperature_NTC<4000)
                        {
                            //FnPointSave(0,1,tempint);
                            FnPointSaveT(1, 209);
                        }
                        else
                        {
                            FnPointSaveT(2, 209);
//                                FnPointSave(0, 3, tempint);
                        }
                    }
					CaliFlag =1;
				}
				if(( SaveGasFlag == 3))//标定氧气点
				{
					tempint = temprx[4]*256+temprx[5];
					//参数存储	
					if(tempint > 500)
					{
						if(sensor_value.temperature_NTC<1800)
						{
							FnPointSave(0,0,tempint);
						}
						else
						{
							if(sensor_value.temperature_NTC<4000)
							{
								FnPointSave(0,1,tempint);
							}
							else
							{
								FnPointSave(0,2,tempint);
////										FnPointSave(2,3,tempint);
							}
						}
					}
                    CaliFlag = 1;
                }
                //						if(( SaveGasFlag == 3))//标定氧气点的第二个温度点
                //						{
                //							tempint = temprx[4]*256+temprx[5];
                //							//参数存储
                //							FnPointSave(0,1,tempint);
                //							CaliFlag =1;
                //						}
                //						if(( SaveGasFlag == 4))//标定氧气点的第三个温度点
                //						{
                //							tempint = temprx[4]*256+temprx[5];
                //							//参数存储
                //							FnPointSave(0,2,tempint);
                //							CaliFlag =1;
                //						}
                SaveGasFlag = 0;
            }
            break;

//恢复用户/厂家
//11 02 4d 00 a0
//11 02 4d 01 9f
			case 77://11 03 4d AF [TVM]ef		//恢复出厂备份数据
				if(SerialJudge(2))
				{
					temprx[0]	=ACK_RX;		//回答类型
					temprx[1]	=1;				//长度	
			//		temprx[2]	=1;				//命令号

					SaveGasFlag = temprx[3];
					if(SaveGasFlag <= 2)	//五点标定
					{
						fnPointRevert(SaveGasFlag);	
					}
					else
					{
						temprx[0]	 =NACK_RX;			//回答类型
					 	temprx[1]	 =2;				//长度	
				//		temprx[2]	 =1;				//命令号
						temprx[3]=CantExeCmd;		//参数
					} 
				 }
			break;

			case 100: //自定义标定
			if(SerialJudge(21))
				{
					temprx[0]	=ACK_RX;		      //回答类型
					temprx[1]	= 1;		//长度	
					memcpy(&anchor, (SelfSetAnchor *)&temprx[3], sizeof(SelfSetAnchor));
					fnSaveAnchor(anchor);
				}
			break;

			case 101: //读取当前飞行时间
			if(SerialJudge(1))
				{
					temprx[0]	=0x16;    //回答类型
					temprx[1]	= 7;		//长度	
					temprx[3] = nT1CountLeft >> 8;
					temprx[4] = nT1CountLeft;
					temprx[5] = nT1CountRight >> 8;
					temprx[6] = nT1CountRight;
					temprx[7] = sensor_value.temperature_NTC >> 8;
					temprx[8] = sensor_value.temperature_NTC;
				}
			break;

//读取flash的值
//11 03 CMD [ADD1][ADD2][CS]	
//11 04 CMD [ADD1][ADD2][DF][CS]
			case	230:
				if(SerialJudge(3))
				{
					temprx[0]	=ACK_RX;		//回答类型
					temprx[1]	=4;				//长度	
				//	temprx[2]	=;				//命令号
				
					tempint = temprx[3]*256+temprx[4];	
					if(tempint < 256)
					{
						temprx[5] = ReadEeprom(tempint);
					}
					else
					{
						temprx[0]	=	NACK_RX;	//回答类型
						temprx[1]	=2;				//长度	
					//  temprx[2]	=1;				//命令号
						temprx[3] = CantExeCmd;	//参数						
					}
					SaveGasFlag=0;
				}				
			break;
						
//写入flash的值	
//11 04 CMD [ADD1][ADD2][DA][CS]
			case	231:
				if(SerialJudge(4))
				{
					temprx[0]	=ACK_RX;		//回答类型
					temprx[1]	=1;				//长度	
				//	temprx[2]	=1;				//命令号

					tempint = temprx[3]*256+temprx[4];
					if(tempint < 256)
					{
						ReadFlashAll();
							WriteEeprom(tempint,temprx[5]);
						WriteFlashAll();	
					}										
					else
					{
						temprx[0]	=	NACK_RX;	//回答类型
						temprx[1]	=2;				//长度	
					//  temprx[2]	=1;				//命令号
						temprx[3] = CantExeCmd;	//参数						
					}
				}
			break;
//==================厂家参数的设置命令区==============================
//			case	249:
////88	7791	7782
////282	7608	7600
////492	7431	7425

//	mysGasTrait5P[0].lFf = 74310+22320;
//	mysGasTrait5P[0].lFb = 74250+22320;
//	mysGasTrait5P[0].nTemp = 492;	
//	FnPointSaveT(2,209);
//			
//	mysGasTrait5P[0].lFf = 76080+22320;
//	mysGasTrait5P[0].lFb = 76000+22320;
//	mysGasTrait5P[0].nTemp = 282;	
//	FnPointSaveT(1,209);
//	FnPointSave(0,0,209);	
//	FnPointSave(0,2,209);
//	
//	mysGasTrait5P[0].lFf = 77910+22320;
//	mysGasTrait5P[0].lFb = 77820+22320;
//	mysGasTrait5P[0].nTemp = 88;	
//	FnPointSaveT(0,209);

////281	7931	7925
//	
//	mysGasTrait5P[0].lFf = 79310+22320;
//	mysGasTrait5P[0].lFb = 79250+22320;
//	mysGasTrait5P[0].nTemp = 281;	
//	FnPointSave(0,1,939);	
//	FnPointSave(0,3,939);		
//	
//			break;
		
			case    250:
				temprx[0]	=ACK_RX;		//回答类型
				temprx[1]	=1;				//长度	
			//	temprx[2]	=1;				//命令号
			
				CaliFlag = 1;
				debugflag = 1;
	//				while(1);
			break;

/*			case    251:
				if(debugflag)
				{
					for(i=0;i<ALL_5P;i++)
					{
						for(j=0;j<4;j++)
						{
							printf("%18.14f, ",mysGasTrait5P[i].Coefficient[j]);					
						}
						printf("\n");
						for(j=4;j<6;j++)
						{
							printf("%18.14f, ",mysGasTrait5P[i].Coefficient[j]);					
						}
						printf("\n");
					}
				}
			break;
*/
			case    252:		//注意标定时会有数据存入
				OutputFlag = !OutputFlag;
				Set_OutputFlag(OutputFlag);
				printf("\nOutputFlag= ");
				if(OutputFlag)
					printf("1\n");
				else	
					printf("0\n");
			break;
	
			case    253:		//注意标定时会有数据存入
				debugflag=~debugflag;
				printf("\ndebugflag= ");
				if(debugflag)
					printf("1\n");
				else	
					printf("0\n");
			break;				
			case	255:
				for(int i=0;i<24;i++) putchar(sn[i]);
			break;

			default:  //恢复正常
					temprx[0]		=NACK_RX;			//回答类型
					temprx[1]		=2;				//长度	
//					temprx[2]		=9;				//命令号
					temprx[3]	=NotCommand;	//参数
			break;
		}
		SendResponse(temprx);
		
	// ***************************************				
	}
}

