#include <stdint.h>
#include "ResultCount.h"
#include "SubFun.h"
#include "Linearization.h"
#include "measurement_core.h"
#include "math.h"

/**
 * @brief  卡尔曼滤波器初始化函数
 * @author qinling
 * @date   2026/02/24
 * @param  
 * @return 
 * @note   
 */
void KalmanFilter_Init(KalmanFilter *kf, int init_x, int Q, int R) 
{
    kf->x = init_x;  // 初始状态设为第一个测量值
    kf->P = 1000;    // 初始协方差
    kf->Q = Q;       // 过程噪声（根据传感器实际情况调整，建议100~1000）
    kf->R = R;       // 观测噪声（根据传感器实际情况调整，建议1000~5000）
}

/**
 * @brief  卡尔曼滤波器更新函数
 * @author qinling
 * @date   2026/02/24
 * @param  输入测量值
 * @return 输出滤波后的值
 * @note   
 */
int KalmanFilter_Update(KalmanFilter *kf, int z) 
{
	if (z <= 0) {
        return kf->x; // 直接返回上一次滤波值，不更新
    }
    // 1. 预测步骤
    int x_pred = kf->x;          // 状态预测（假设系统无外部输入，状态不变）
    int P_pred = kf->P + kf->Q;  // 协方差预测

	// 预测协方差保护：避免溢出或过小
    if (P_pred > 50000) P_pred = 50000; // 限制最大值，防止整数溢出
    if (P_pred < 100) P_pred = 100;     // 限制最小值，避免K过小

    // 2. 更新步骤
    // 放大1000倍计算K，避免直接为0
    int K_num = P_pred * 1000;
    int K_den = P_pred + kf->R;
    if (K_den == 0) K_den = 1;
    int K = K_num / K_den; // 卡尔曼增益 K是放大1000倍的结果（如0.4→400）

	// 卡尔曼增益保护：避免K为0或过大
    if (K <= 100) K = 100;               // 最小K=100（对应0.1），保证能跟踪变化
    if (K > 1000) K = 1000;            // 最大K=1000（对应1.0），避免过度信任测量值

    // 状态更新：(z - x_pred)*K 后除以1000，还原比例
    int delta = z - x_pred;
    kf->x = x_pred + (delta * K) / 1000;  // 更新状态估计值  
    // 协方差更新（同样放大避免0）
    kf->P = ((1000 - K) * P_pred) / 1000;   // 更新协方差
	
	// 协方差最终保护
    if (kf->P > 50000) kf->P = 50000;
    if (kf->P < 100) kf->P = 100;

    return kf->x;
}

int nOrderBuffer_5(int *InOrderData)
{
	return Measurement_Median5(InOrderData);
}

//判断两点异常波动
static uint8_t IsAbnormalFluctuation(float tempvalue, float threadshold)
{
	static float old_temp = 0.0f, new_temp = 0.0f;
	float value_div = 0.0f;
	old_temp = new_temp;
	new_temp = tempvalue;
	value_div = fabs(old_temp - new_temp);
	if(value_div > threadshold)
	{
		return 1;
	}
	return 0;
}

// 排序，去掉FliterLen个最大值和FliterLen个最小值 然后取平均  FilterFloatDataTask
float FilterFloatDataTask(float *Tab, unsigned short Len, unsigned short FliterLen)
{
	return Measurement_TrimmedMeanFloat(Tab, Len, FliterLen);
}

// 排序，去掉FliterLen个最大值和FliterLen个最小值 然后取平均  FilterFloatDataTask
int FilterIntDataTask(int *Tab, unsigned short Len, unsigned short FliterLen)
{
	return Measurement_TrimmedSumInt(Tab, Len, FliterLen);
}


int nOrderBuffer_10(int *InOrderData)
{
	return Measurement_Median10(InOrderData);
}

uint16_t AveFilterTask( int *Tab, unsigned short Len)
{
	return Measurement_AverageUnsigned(Tab, Len);
}

int GetAverage(int datain[], unsigned int num)
{
	return Measurement_AverageInt(datain, num);
}

long lRefCountFf;
long lRefCountFb;

void ResultCount5P_Flow(long myFf, long myFb, SensorValue sensor_value)
{
	float tempValue = 0;
	int aveTempFfFb = 0;
	int aveFfFbZero = 0;
	int tempFlow=0;
	static uint8_t firstcount_flow = 0;
	static int FlowResultFlip[FLOW_RESULT_FLIP_POINT];
	static int FlowResultFlipPoint = 0;
	static uint8_t Flow_Count = 0;
	uint8_t abnormal = 0;
	int stable_average = tempFlow;
	float flow_err = 0;
	mysGasTrait5P[1].lFf = myFf;
	mysGasTrait5P[1].lFb = myFb;
	mysGasTrait5P[1].nTemp = sensor_value.temperature_NTC;
	mysGasTrait5P[1].CountLong = (myFf-myFb); 	
	if(FlowResultFlipPoint >= FLOW_RESULT_FLIP_POINT)
	{
		FlowResultFlipPoint = 0;
	}
	aveFfFbZero = (deltaFb + deltaFf)/2;
	aveTempFfFb = (mysGasTrait5P[1].lFf/10 + mysGasTrait5P[1].lFb/10)/2;
	aveTempFfFb = aveTempFfFb - aveFfFbZero;		
	tempValue = (1.0 / (mysGasTrait5P[1].lFf/10.0 - aveTempFfFb) - 1.0 / (mysGasTrait5P[1].lFb/10.0 - aveTempFfFb));
    tempValue = Measurement_ApplyFlowSegments(tempValue,
                                              mysGasTrait5P[1].Coefficient[0],
                                              mysGasTrait5P[1].Coefficient[1],
                                              flowX1, Ck1, Cb1,
                                              flowX2, Ck2, Cb2,
                                              flowX3, Ck3, Cb3,
                                              flowX4, Ck4, Cb4);
	tempFlow = Measurement_ClampFlow(tempValue);

	FlowResultFlip[FlowResultFlipPoint] = tempFlow;
	//printf("tempflowfast=%f,Flow_k=%f,Flow_b=%f\n",tempflowfast,mysGasTrait5P[1].Coefficient[0],mysGasTrait5P[1].Coefficient[1]);
	//mysGasTrait5P[1].Result = (int)tempflowfast;
//	printf("flow:ff=%d,fb=%d,tmep=%d,result=%d",mysGasTrait5P[1].lFf,mysGasTrait5P[1].lFb,
//	mysGasTrait5P[1].nTemp,mysGasTrait5P[1].Result);
//	printf("mysGasTrait5P[1].Coefficient[0]=%f,mysGasTrait5P[1].Coefficient[1]=%f,",mysGasTrait5P[1].Coefficient[0],mysGasTrait5P[1].Coefficient[1]);
//	printf("Flow_k=%f,Flow_b=%fm",Flow_k,Flow_b);

	if(firstcount_flow < FLOW_RESULT_FLIP_POINT)  //开机未填满窗口 → 直接输出
	{
		mysGasTrait5P[1].Result = tempFlow;
		firstcount_flow++;
	}

	if(Flow_Count >= FLOW_RESULT_FLIP_POINT)
	{
		abnormal = IsAbnormalFluctuation(tempFlow, 8);
		if (!abnormal)
		{
			stable_average = GetAverage(FlowResultFlip, FLOW_RESULT_FLIP_POINT);
		}
	}
	mysGasTrait5P[1].Result = Measurement_SelectWindowOutput(
		Flow_Count,
		FLOW_RESULT_FLIP_POINT,
		tempFlow,
		stable_average,
		abnormal);
	Flow_Count = Measurement_NextWindowCount(Flow_Count,
	                                              FLOW_RESULT_FLIP_POINT,
	                                              abnormal);

	FlowResultFlipPoint++;

//	if(mydebugflag)
//	{
//		printf("Flow_Div=%d,",Flow_Div);
//		printf("flowfast=%f,flowslow=%f,",tempflowfast,tempflowslow);
//		printf("flow_Result=%d,",mysGasTrait5P[1].Result);
//	}
	//环境压力修正（相较于浮子）
	mysGasTrait5P[1].AmbientResult = mysGasTrait5P[1].Result;
	if(mysGasTrait5P[1].Result > 50) //0.5L以上才修正
	{
		if(sensor_value.press_out >= 900) //90kpa以上
		{
			mysGasTrait5P[1].AmbientResult = mysGasTrait5P[1].Result + 10; //0.5L以上 + 0.1L
		}
		else
		{
			if(mysGasTrait5P[1].Result > 400)  //4L以上
			{
				flow_err = -0.345 * sensor_value.press_out + 311;
			}
			else if(mysGasTrait5P[1].Result > 200) //2~4L
			{
				flow_err = -0.239 * sensor_value.press_out + 200;
			}
			else  //2L以下
			{
				flow_err = -0.055 * sensor_value.press_out + 28;
			}

			mysGasTrait5P[1].AmbientResult = mysGasTrait5P[1].Result - (int)flow_err;
		}
	} 

	//背压修正
    mysGasTrait5P[1].PressResult = (int)((float)mysGasTrait5P[1].AmbientResult * sensor_value.press_in / sensor_value.press_out);
  	// if(sensor_value.press_out >= 9500)
    // {
    //  mysGasTrait5P[1].PressResult =  (int)((float)mysGasTrait5P[1].Result*(sensor_value.press_out +(sensor_value.press_in-sensor_value.press_out))/sensor_value.press_out);
    // }
    // else if(sensor_value.press_out >= 8800 && sensor_value.press_out < 9500)
    // {
    //   mysGasTrait5P[1].PressResult = (int)((float)mysGasTrait5P[1].Result *(sensor_value.press_out -(sensor_value.press_in-sensor_value.press_out))/sensor_value.press_out)/2+
	//    (int)((float)mysGasTrait5P[1].Result*(sensor_value.press_out +(sensor_value.press_in-sensor_value.press_out))/sensor_value.press_out)/2;
    // }
    // else
    // {
    //   mysGasTrait5P[1].PressResult = (int)((float)mysGasTrait5P[1].Result *(sensor_value.press_out -(sensor_value.press_in-sensor_value.press_out))/sensor_value.press_out);
    // }

}

void ResultCount5P_O2(long myFf, long myFb, SensorValue sensor_value)
{
	int tempO2 = 0;
	float tempvalue=0;
	static uint8_t O2ResultFlipPoint = 0;
	static uint8_t firstcount_o2=0;
	static uint8_t  O2_Count=0;
	static int O2ResultFlip[O2_RESULT_FLIP_POINT];
	uint8_t abnormal = 0;
	int stable_average = tempO2;
	mysGasTrait5P[0].lFf = myFf;
	mysGasTrait5P[0].lFb = myFb;

	mysGasTrait5P[0].CountLong= mysGasTrait5P[0].lFf + mysGasTrait5P[0].lFb;
	mysGasTrait5P[0].nTemp = sensor_value.temperature_NTC;
	//浓度滑动窗口指针
	if(O2ResultFlipPoint >= O2_RESULT_FLIP_POINT)
	{
		O2ResultFlipPoint = 0;
	}
	tempvalue = Measurement_CalculateO2(mysGasTrait5P[0].lFf,
	                                    mysGasTrait5P[0].lFb,
	                                    mysGasTrait5P[0].nTemp,
	                                    fSlopeFsumAir,
	                                    fSlopeFsumO2,
	                                    calibrationO2,
	                                    calibrationO2Fac);
	tempO2 = Measurement_ClampO2(tempvalue);

	O2ResultFlip[O2ResultFlipPoint] = tempO2;

	if(O2_Count >= O2_RESULT_FLIP_POINT)
	{
		abnormal = IsAbnormalFluctuation(tempvalue, 100);
		if (!abnormal)
		{
			stable_average = GetAverage(O2ResultFlip, O2_RESULT_FLIP_POINT);
		}
	}
	mysGasTrait5P[0].Result = Measurement_SelectWindowOutput(
		O2_Count,
		O2_RESULT_FLIP_POINT,
		tempO2,
		stable_average,
		abnormal);
	O2_Count = Measurement_NextWindowCount(O2_Count,
	                                          O2_RESULT_FLIP_POINT,
	                                          abnormal);

	mysGasTrait5P[0].Result = Measurement_ApplyO2Startup(
		firstcount_o2,
		O2_RESULT_FLIP_POINT,
		mysGasTrait5P[0].Result);
	firstcount_o2 = Measurement_NextStartupCount(firstcount_o2,
	                                               O2_RESULT_FLIP_POINT);
	
	O2ResultFlipPoint++;
	
}
