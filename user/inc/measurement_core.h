#ifndef MEASUREMENT_CORE_H
#define MEASUREMENT_CORE_H

int Measurement_Median5(const int *input);
int Measurement_Median10(const int *input);
float Measurement_TrimmedMeanFloat(const float *input,
                                   unsigned short length,
                                   unsigned short trim_length);
int Measurement_TrimmedSumInt(const int *input,
                              unsigned short length,
                              unsigned short trim_length);
unsigned short Measurement_AverageUnsigned(const int *input,
                                           unsigned short length);
int Measurement_AverageInt(const int *input, unsigned int length);

float Measurement_CalculateO2(long ff,
                              long fb,
                              int temperature,
                              const float *air_curve,
                              const float *o2_curve,
                              int calibration_o2,
                              int calibration_o2_factory);
int Measurement_ClampO2(float value);

float Measurement_ApplyFlowSegments(float value,
                                    float primary_k,
                                    float primary_b,
                                    float x1,
                                    float k1,
                                    float b1,
                                    float x2,
                                    float k2,
                                    float b2,
                                    float x3,
                                    float k3,
                                    float b3,
                                    float x4,
                                    float k4,
                                    float b4);
int Measurement_ClampFlow(float value);
unsigned short Measurement_MapPwmDuty(int value);
int Measurement_SelectWindowOutput(unsigned char count,
                                   unsigned char window_length,
                                   int sample,
                                   int stable_average,
                                   unsigned char abnormal);
unsigned char Measurement_NextWindowCount(unsigned char count,
                                          unsigned char window_length,
                                          unsigned char abnormal);
int Measurement_ApplyO2Startup(unsigned char startup_count,
                               unsigned char window_length,
                               int calculated_output);
unsigned char Measurement_NextStartupCount(unsigned char startup_count,
                                           unsigned char window_length);

#endif /* MEASUREMENT_CORE_H */
