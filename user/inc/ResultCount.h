#ifndef __ResultCount_H
#define __ResultCount_H
#include "main.h"

typedef struct {
    int x;  // 状态估计值（滤波后的飞行时间）
    int P;  // 状态估计协方差
    int Q;  // 过程噪声协方差
    int R;  // 观测噪声协方差
} KalmanFilter;

#define O2_RESULT_FLIP_POINT   12
#define FLOW_RESULT_FLIP_POINT 30
#define O2_INITIAL_VALUE  209

void KalmanFilter_Init(KalmanFilter *kf, int init_x, int Q, int R);
int KalmanFilter_Update(KalmanFilter *kf, int z);

float FilterFloatDataTask(float *Tab, unsigned short Len, unsigned short FliterLen);
int FilterIntDataTask(int *Tab, unsigned short Len, unsigned short FliterLen);

int nOrderBuffer_5(int *InOrderData);
int nOrderBuffer_10(int *InOrderData);

void ResultCount5P_Flow(long myFf, long myFb, SensorValue sensor_value);
void ResultCount5P_O2(long myFf, long myFb, SensorValue sensor_value);

extern long lRefCountFf;
extern long lRefCountFb;

#endif /* __ResultCount_H */
