#include <string.h>	
#include <stdio.h>
#include "Linearization.h"
#include "SubFun.h"
#include "flash.h"
#include "CoeffCountMod.h"

float Ck1=0,Cb1=0,Ck2=0,Cb2=0,Ck3=0,Cb3=0,Ck4=0,Cb4=0;
float flowX1=0.0f,flowX2=0.0f,flowX3=0.0f,flowX4=0.0f;

int calibrationO2 = 0;
int calibrationO2Fac = 0;
int calibrationAir = 0;
int calibrationAirFac=0;

int deltaFf = 0;
int deltaFb = 0;

#define SaveAddrO2_4P 0
#define POINT_NUM  20  //最多输入20个点进行线性拟和
typedef struct POINT //点的结构
{
    double x[POINT_NUM];
    double y[POINT_NUM];
	  double H2ox[POINT_NUM];
    double H2oy[POINT_NUM];
}POINTS;

POINTS  points; //存放点数据

unsigned char  debugflag = 0;

int nFxdFair;

float fSlopeFsumN2[3];//温度修正系数
float fSlopeFsumAir[3];	//温度修正系数
float fSlopeFsumO2[3];

// static unsigned char  O2CalibratedFlag;

static  int    tempint;
static  long   templong;


void fnSaveAnchor(SelfSetAnchor anchor)
{
	int  SaveAddr[4] = {60, 200, 0, 150}; //流量、N2、空气、O2
	unsigned char flow_gas = 0;
	printf("type:%d, point:%d, auto:%d, percent:%d, temp:%d, Ff:%d, Fb:%d\r\n",
	anchor.type, anchor.point, anchor.auto_flag, anchor.concentrtion, anchor.temperature, anchor.Ff, anchor.Fb);

	if(anchor.type > 3)
	{
		printf("save anchor failed!\r\n");
		return;
	}

	if(anchor.type == 0)
	{
		flow_gas = 1;
	}else{
		flow_gas = 0;
	}

	ReadFlashAll();
	if(anchor.type == 1) //N2地址分布不一样
	{
		SaveAddr[anchor.type] += anchor.point * 12;
		WriteEepromInt(SaveAddr[anchor.type], 0);
		if(!anchor.auto_flag)
		{
			SaveAddr[anchor.type] += 2;
			WriteEepromInt(SaveAddr[anchor.type], anchor.temperature);
			SaveAddr[anchor.type] += 2;
			WriteEepromLong(SaveAddr[anchor.type], anchor.Ff);
			SaveAddr[anchor.type] += 4;
			WriteEepromLong(SaveAddr[anchor.type], anchor.Fb);
		}else{
			SaveAddr[anchor.type] += 2;
			WriteEepromInt(SaveAddr[anchor.type], mysGasTrait5P[flow_gas].nTemp);
			SaveAddr[anchor.type] += 2;
			WriteEepromLong(SaveAddr[anchor.type], mysGasTrait5P[flow_gas].lFf);
			SaveAddr[anchor.type] += 4;
			WriteEepromLong(SaveAddr[anchor.type], mysGasTrait5P[flow_gas].lFb);
		}
	}else{
		SaveAddr[anchor.type] += anchor.point * 2;
		WriteEepromInt(SaveAddr[anchor.type], anchor.concentrtion);
		if(!anchor.auto_flag)
		{
			SaveAddr[anchor.type] += 8;
			WriteEepromInt(SaveAddr[anchor.type], anchor.temperature);
			SaveAddr[anchor.type] += (anchor.point * 2 + 8);
			WriteEepromLong(SaveAddr[anchor.type], anchor.Ff);
			SaveAddr[anchor.type] += 16;
			WriteEepromLong(SaveAddr[anchor.type], anchor.Fb);
		}else{
			SaveAddr[anchor.type] += 8;
			WriteEepromInt(SaveAddr[anchor.type], mysGasTrait5P[flow_gas].nTemp);
			SaveAddr[anchor.type] += (anchor.point * 2 + 8);
			WriteEepromLong(SaveAddr[anchor.type], mysGasTrait5P[flow_gas].lFf);
			SaveAddr[anchor.type] += 16;
			WriteEepromLong(SaveAddr[anchor.type], mysGasTrait5P[flow_gas].lFb);
		}
	}
	WriteFlashAll();
}

void FnPointSave(unsigned char tGasNUM,unsigned char tStep,int tInPut)
{
	int  SaveAddr;
	
	if(tGasNUM>=2)	return;
	if(tStep>=6)	return;
	
ReadFlashAll();
	if(tGasNUM==0)
	{
		SaveAddr = tGasNUM * 60 + tStep * 2 + 0;
		WriteEepromInt(SaveAddr, tInPut);
		//	tTemp
		//	SaveAddr = tGasNUM*60 +  tStep*2+8;
		SaveAddr += 8;
		WriteEepromInt(SaveAddr, mysGasTrait5P[tGasNUM].nTemp);
		// Ff
		//	SaveAddr = tGasNUM*60 + tStep*4+16;
		SaveAddr += (tStep * 2 + 8);
		WriteEepromLong(SaveAddr, mysGasTrait5P[tGasNUM].lFf);
		// Fb
		//	SaveAddr = tGasNUM*60 + tStep*4+32;
		SaveAddr += 16;
		WriteEepromLong(SaveAddr, mysGasTrait5P[tGasNUM].lFb);
	}
	else
	{
		//Y[i]
		SaveAddr = tGasNUM*46 + tStep*2+0;
		WriteEepromInt(SaveAddr,tInPut);	 				
	//	tTemp
	//	SaveAddr = tGasNUM*60 +  tStep*2+8;
		SaveAddr += 12;
		WriteEepromInt(SaveAddr,mysGasTrait5P[tGasNUM].nTemp);		 				 			
	// Ff
	//	SaveAddr = tGasNUM*60 + tStep*4+16;
		SaveAddr += (tStep*2+12);
		WriteEepromLong(SaveAddr,mysGasTrait5P[tGasNUM].lFf);		
	// Fb
	//	SaveAddr = tGasNUM*60 + tStep*4+32;
		SaveAddr += 24;
		WriteEepromLong(SaveAddr,mysGasTrait5P[tGasNUM].lFb);		
	}

WriteFlashAll();

	/***************标定时间存储*****************/
	/*******************************************/	
}

void fnPointRevert(unsigned char tGasNUM)
{
	unsigned int  ReadAddr;
	unsigned char i;
		
	ReadFlashAll();
	for(i=0;i<2;i++)
	{
	//	tInput
		ReadAddr = tGasNUM*60 + i*2+0;
			tempint = ReadEepromInt(ReadAddr);
			WriteEepromInt(ReadAddr+4,tempint);
		//	tTempK
	//	ReadAddr = tGasNUM*60 + i*2+8;
		ReadAddr += 8;
			tempint = ReadEepromInt(ReadAddr);
			WriteEepromInt(ReadAddr+4,tempint);			
		// Ff	
	//	ReadAddr = tGasNUM*60 + i*4+16;
		ReadAddr += i*2+8;
			templong = ReadEepromLong(ReadAddr);
			WriteEepromLong(ReadAddr+8,templong);					
		// Fb	
	//	ReadAddr = tGasNUM*60 + i*4+32;
		ReadAddr += 16;
			templong = ReadEepromLong(ReadAddr);
			WriteEepromLong(ReadAddr+8,templong);			
	}		
	WriteFlashAll();		
}

void FnPointSaveT(unsigned char tGasNUM, int tInPut)
{
	int  SaveAddr;		

	ReadFlashAll();
	//Y[i]
	SaveAddr = 150+tGasNUM*12;
	WriteEepromInt(SaveAddr,tInPut); 				
	//	tTemp
	SaveAddr += 2;
	WriteEepromInt(SaveAddr,mysGasTrait5P[0].nTemp); 				 			
	// Ff
	SaveAddr += 2;
	WriteEepromLong(SaveAddr,mysGasTrait5P[0].lFf);		
	// Fb
	SaveAddr += 4;
	WriteEepromLong(SaveAddr,mysGasTrait5P[0].lFb);
	
	WriteFlashAll();	
}

/*
参数说明：
        P[POINT_NUM]：传入要线性拟合的点数据（结构体数组）
        N：线性拟合的点的数量
        K0:直线斜率参数存放地址
        b0:直线截距参数存放地址
*/
void Least_square_method(double  *X,double *Y,int N,double *K0,double *b0)
{
    int i=0;
    double K=0,b=0,A=0,B=0,C=0,D=0;
    for(i=0;i<N;i++)
    {
        A+=X[i]*Y[i];
        B+=X[i];
        C+=Y[i];
        D+=X[i]*X[i];
    }
    K=(N*A-B*C)/(N*D-B*B);
    b=C/N-K*B/N;
    /*将计算得到的直线参数通过指针传递到函数外部*/
    *K0=K;
    *b0=b;
    
/*    printf("A=%f\nB=%f\nC=%f\nD=%f\n",A,B,C,D);
    if(b>0) printf("y=%fx+%f\n",K,b);
    else if(b==0)printf("y=%fx\n",K);
    else printf("y=%fx%f\n",K,b);*/
}

//流量的系数计算
void FnLinearization(unsigned char tGasNUM)
{

int  	tInput;			//浓度
int  	tTemp;			//温度
unsigned int aveFfFb = 0;
int aveTempFfFb = 0;
unsigned char  firstLineOrThirdLine = 0;

unsigned int  ReadAddr;
float x[6];
float y[6];

	if(tGasNUM>=2)	return;
	
	ReadAddr = tGasNUM*60;
	
	if(debugflag)
	{
		printf("\nFf   Fb   IN   Temp  X  Y \n");	
	}		

	for(int i=0; i<6; i++)
	{
		//	tInput
		ReadAddr = tGasNUM*46 + i*2+0;
		tInput = ReadEepromInt(ReadAddr);
													 			
		//	tTempK
	//	ReadAddr = tGasNUM*60 + i*2+8;
		ReadAddr += 12;
		tTemp = ReadEepromInt(ReadAddr);
		
		// Ff	
	//	ReadAddr = tGasNUM*60 + i*4+16;
		ReadAddr += (12 + i*2);	
		templong = ReadEepromLong(ReadAddr);
		x[i] = templong;
			
		if(debugflag)
		{
			printf("%ld, ",templong);	
		}				
		// Fb	
	//	ReadAddr = tGasNUM*60 + i*4+32;
		ReadAddr += 24 ;
		templong = ReadEepromLong(ReadAddr);
			
//============================================================
			if(tGasNUM == 0)	//O2
			{
				x[i] += templong;				
				if(i == 0)
				{
		//      	    nFxdFair =	(int)(x[i]/20-(-0.9088933223*tTemp+7884.8)); 					  //add20131205
										
					nFxdFair =	(int)(x[i]/20-( USFCountAirTemp(tTemp)));   //CHG20140920  7500B
										//printf("nFxdFair=%d,x[i]=%f,\n",nFxdFair,x[i]);
					if((nFxdFair >200*72.0/20.0)||(nFxdFair<-200*72.0/20.0))
					{
						nFxdFair = -38*72.0/20.0; //没有标定时候，给个初始值   
					}    		    		
				}  	
				
            x[i] -= (fSlopeFsumAir[0] * tTemp * tTemp + fSlopeFsumAir[1] * tTemp );
			y[i]  = tInput;				
			}	
			else 				//FLOW	
			{
//				x[i] -= templong;
				if(i==0)
				{
					//printf("x[0]=%ld,%ld,",x[0],templong);
					deltaFf = (x[0]/10);
					deltaFb = (templong/10);
					aveFfFb = (deltaFf+deltaFb)/2;
				}
				aveTempFfFb = (x[i]/10.0 + templong/10.0)/2;
				aveTempFfFb = aveTempFfFb - aveFfFb;
				x[i] = (1.0/( x[i]/10.0 - aveTempFfFb) -1.0/(templong/10.0 - aveTempFfFb));		//时间差 同计算部分
				y[i] = tInput;       
			}

//============================================================										
		if(debugflag)
		{
			printf("%ld, ",templong);//Fb 中间没有进行赋值
			printf("%d, %d, %.1f, %.2f \n",tInput,tTemp,x[i],y[i]);	
		}	
        if(tGasNUM == 0)
        {
            if(y[0] > (150*72/48) && y[0] < (1200*72/48) && y[1] > (150*72/48) && y[1] < (1200*72/48))
            {
                firstLineOrThirdLine = 1;
//                O2CalibratedFlag = 1;
//                average20Ff = (x[0] + x[1]) / 2;
            }
            if(y[2] > (150*72/48) && y[2] < (1200*72/48) && y[3] > (150*72/48) && y[3] < (1200*72/48))
            {
                firstLineOrThirdLine = 3;
//                O2CalibratedFlag = 3;
//                average20Ff = (x[0] + x[1]) / 2;
            }
        }
    }
    if(tGasNUM == 0)
    {
        if(firstLineOrThirdLine == 1)
        {
            mysGasTrait5P[0].Coefficient[0] = (y[1] - y[0]) / (x[1] - x[0]);
            mysGasTrait5P[0].Coefficient[1] =  y[0] - mysGasTrait5P[0].Coefficient[0] * x[0];
        }
        if(firstLineOrThirdLine == 3)
        {
            mysGasTrait5P[0].Coefficient[0] = (y[3] - y[2]) / (x[3] - x[2]);
            mysGasTrait5P[0].Coefficient[1] =  y[2] - mysGasTrait5P[0].Coefficient[0] * x[2];
        }
    }
    else
    {
		mysGasTrait5P[tGasNUM].Coefficient[0] = (y[1]-y[0])/(x[1]-x[0]);
		mysGasTrait5P[tGasNUM].Coefficient[1] =  y[0]-mysGasTrait5P[tGasNUM].Coefficient[0]*x[0];
		Ck1 = (y[1]-y[2])/(x[1]-x[2]);
		Cb1 =  y[1]-Ck1*x[1];
		Ck2 = (y[2]-y[3])/(x[2]-x[3]);
		Cb2 =  y[2]-Ck2*x[2];
		Ck3 = (y[3]-y[4])/(x[3]-x[4]);
		Cb3 =  y[3]-Ck3*x[3];
		Ck4 = (y[5]-y[4])/(x[5]-x[4]);
		Cb4 =  y[4]-Ck4*x[4];
		flowX1 = x[1];
		flowX2 = x[2];
		flowX3 = x[3];
		flowX4 = x[4];
    }
    if(debugflag)
    {
        if(tGasNUM == 0)
            printf("O2:\n");
        else
            printf("Flow:\n");
        //		printf("x0=%.2f ,y0=%.2f\n",x[2],y[2]);
        //		printf("x1=%.2f ,y1=%.2f\n",x[3],y[3]);
        {
          printf("%18.14f  %18.14f\n", mysGasTrait5P[tGasNUM].Coefficient[0] , mysGasTrait5P[tGasNUM].Coefficient[1]);
          printf("%18.14f  %18.14f\n", Ck1 , Cb1);  
        }
    }
}

static void stdFf(float AirFf[3], int CnBuffer[3], float O2Ff[3])
{
	float kk=0,bb=0;
	kk = (O2Ff[0]-AirFf[0])/((float)(CnBuffer[0]-209));
	bb = AirFf[0]-kk*209;
	O2Ff[0] = kk*CnBuffer[1]+bb; //用常温（30℃）修正5℃飞行时间
//	printf("L %f %f %d\n",O2Ff[0],AirFf[0],CnBuffer[0]);
//	printf("L1 %f %f\n",kk,bb);
	kk = (O2Ff[2]-AirFf[2])/((float)(CnBuffer[2]-209));
	bb = AirFf[2]-kk*209;
	O2Ff[2] = kk*CnBuffer[1]+bb; //用常温（30℃）修正50℃飞行时间
//	printf("L2 %f %f\n",kk,bb);
}

//浓度的系数计算
void FnLinearization1(void)
{
	int tInput;			//浓度
	int	tTemp;			//温度
	int CnBuffer[4] = {0};
	float FfFbBuffer[4] = {0};
	unsigned int  ReadAddr;   	
	
	float x[3];
	float y[3];
//=========================读取数据================================	   	
	for(int i=0; i<3; i++)
	{
		//	tInput
		ReadAddr = 150 + i*12;	
		tInput =ReadEepromInt(ReadAddr);
										 			
		//	tTempK
		ReadAddr += 2;
		tTemp =ReadEepromInt(ReadAddr);
	
		// Ff
		ReadAddr += 2;			
		templong =ReadEepromLong(ReadAddr);
		y[i] = templong;			
		if(debugflag)
		{
			printf("%ld, ",templong);	
		}				
		// Fb		
		ReadAddr += 4;
		templong =ReadEepromLong(ReadAddr);		
				
//============================================================	
		y[i] += templong;		//浓度								
		x[i] = tTemp;			//			
        FfFbBuffer[i] = y[i];
		if(debugflag)
		{
			printf("%ld, ",templong);//Fb 中间没有进行赋值
			printf("%d, %d, %.1f, %.2f \n",tInput,tTemp,x[i],y[i]);				
		}
	}   	
  fnCoeffCount3P(x,y,fSlopeFsumAir);	
	if(debugflag)
	{
		printf("%18.14f  %18.14f %18.14f\n",fSlopeFsumAir[0] ,fSlopeFsumAir[1],fSlopeFsumAir[2]);
	}
	//下面是氧气点的温度标定
	for(int i=0;i<3;i++)
	{
		//	tInput
		ReadAddr = i*2+0;
		tInput = ReadEepromInt(ReadAddr);	
		CnBuffer[i] = tInput;
		if(i==1)
		{
			calibrationO2 = tInput;
		}
		if(i==2)
		{
			calibrationO2Fac = tInput;
		}
		//	tTempK
		ReadAddr += 8;
		tTemp = ReadEepromInt(ReadAddr);
		// Ff	
		ReadAddr += (8 + i*2);	
		templong = ReadEepromLong(ReadAddr);
		y[i] = templong;			
		if(debugflag)
		{
				printf("%ld, ",templong);	
		}				
		// Fb		
		ReadAddr += 16 ;
		templong = ReadEepromLong(ReadAddr);		
    //============================================================	
		y[i] += templong;		//浓度								
		x[i] = tTemp;			//				
		if(debugflag)
		{
			printf("%ld, ",templong);//Fb 中间没有进行赋值
			printf("%d, %d, %.1f, %.2f \n",tInput,tTemp,x[i],y[i]);				
		}
	}
	stdFf(FfFbBuffer,CnBuffer,y);
	//printf("%.2f %.2f %.2f \n",y[0],y[1],y[2]);
  fnCoeffCount3P(x,y,fSlopeFsumO2);	
	if(debugflag)
	{
		printf("%18.14f  %18.14f %18.14f\n",fSlopeFsumO2[0] ,fSlopeFsumO2[1],fSlopeFsumO2[2]);
	}
}



