#include "measurement_core.h"

int Measurement_Median5(const int *input)
{
    int temp;
    int ordered[5];
    unsigned char i;
    unsigned char j;

    for (i = 0; i < 5; i++)
    {
        ordered[i] = input[i];
    }

    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 4 - i; j++)
        {
            if (ordered[j] > ordered[j + 1])
            {
                temp = ordered[j];
                ordered[j] = ordered[j + 1];
                ordered[j + 1] = temp;
            }
        }
    }
    return ordered[2];
}

int Measurement_Median10(const int *input)
{
    int temp;
    int ordered[10];
    unsigned char i;
    unsigned char j;

    for (i = 0; i < 10; i++)
    {
        ordered[i] = input[i];
    }

    for (i = 0; i < 10; i++)
    {
        for (j = 0; j < 9 - i; j++)
        {
            if (ordered[j] > ordered[j + 1])
            {
                temp = ordered[j];
                ordered[j] = ordered[j + 1];
                ordered[j + 1] = temp;
            }
        }
    }
    return (ordered[4] + ordered[5]) / 2;
}

float Measurement_TrimmedMeanFloat(const float *input,
                                   unsigned short length,
                                   unsigned short trim_length)
{
    unsigned char i;
    unsigned char j;
    unsigned char maximum_index;
    float sum = 0;
    float temp = 0;
    float buffer[20];
    unsigned short result_length;

    for (i = 0; i < length; i++)
    {
        buffer[i] = input[i];
    }

    for (i = 0; i < length - 1; i++)
    {
        maximum_index = i;
        for (j = i + 1; j < length; j++)
        {
            if (buffer[j] > buffer[maximum_index])
            {
                maximum_index = j;
            }
        }
        if (i != maximum_index)
        {
            temp = buffer[i];
            buffer[i] = buffer[maximum_index];
            buffer[maximum_index] = temp;
        }
    }

    result_length = length - (trim_length * 2);
    sum = 0;
    for (i = trim_length; i < length - trim_length; i++)
    {
        sum += buffer[i];
    }
    sum /= result_length;
    return sum;
}

int Measurement_TrimmedSumInt(const int *input,
                              unsigned short length,
                              unsigned short trim_length)
{
    unsigned char i;
    unsigned char j;
    unsigned char maximum_index;
    int sum = 0;
    int temp = 0;
    int buffer[length];

    for (i = 0; i < length; i++)
    {
        buffer[i] = input[i];
    }

    for (i = 0; i < length - 1; i++)
    {
        maximum_index = i;
        for (j = i + 1; j < length; j++)
        {
            if (buffer[j] > buffer[maximum_index])
            {
                maximum_index = j;
            }
        }
        if (i != maximum_index)
        {
            temp = buffer[i];
            buffer[i] = buffer[maximum_index];
            buffer[maximum_index] = temp;
        }
    }

    sum = 0;
    for (i = trim_length; i < length - trim_length; i++)
    {
        sum += buffer[i];
    }
    return sum;
}

unsigned short Measurement_AverageUnsigned(const int *input,
                                           unsigned short length)
{
    unsigned char i;
    unsigned int sum = 0;

    for (i = 0; i < length; i++)
    {
        sum += input[i];
    }
    return (unsigned short)(sum / length);
}

int Measurement_AverageInt(const int *input, unsigned int length)
{
    unsigned int i;
    long sum = 0;

    for (i = 0; i < length; i++)
    {
        sum += input[i];
    }
    return (int)(sum / length);
}

float Measurement_CalculateO2(long ff,
                              long fb,
                              int temperature,
                              const float *air_curve,
                              const float *o2_curve,
                              int calibration_o2,
                              int calibration_o2_factory)
{
    float value;
    float air_value;
    float o2_value;
    float calibration_k;
    float calibration_b;

    value = ff + fb;
    air_value = air_curve[0] * temperature * temperature
              + air_curve[1] * temperature + air_curve[2];
    o2_value = o2_curve[0] * temperature * temperature
             + o2_curve[1] * temperature + o2_curve[2];

    calibration_k = (calibration_o2 - 209) / (o2_value - air_value);
    calibration_b = 209 - calibration_k * air_value;
    value = calibration_k * value + calibration_b;

    calibration_k = (float)(calibration_o2_factory - 209)
                  / ((float)calibration_o2 - 209);
    calibration_b = 209 - calibration_k * 209;
    value = calibration_k * value + calibration_b;
    return value;
}

int Measurement_ClampO2(float value)
{
    int result;

    result = (int)(value + 0.5);
    if (result < 0)
    {
        result = 0;
    }
    if (result > 1200)
    {
        result = 1200;
    }
    return result;
}

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
                                    float b4)
{
    if (value > x1)
    {
        value = primary_k * value + primary_b;
    }
    else if (value > x2)
    {
        value = k1 * value + b1;
    }
    else if (value > x3)
    {
        value = k2 * value + b2;
    }
    else if (value > x4)
    {
        value = k3 * value + b3;
    }
    else
    {
        value = k4 * value + b4;
    }
    return value;
}

int Measurement_ClampFlow(float value)
{
    int result;

    value = value * 10;
    result = (int)(value + 0.5);
    if ((result > 9999) || (result < 0))
    {
        result = 0;
    }
    return result;
}

unsigned short Measurement_MapPwmDuty(int value)
{
    float converted;
    unsigned int span_data;

    span_data = 764;
    if (value > 1000)
    {
        value = 1000;
    }
    else if (value < 0)
    {
        value = 0;
    }

    converted = value;
    value = (int)(converted * span_data / 1000.0 + 0.5);
    return (unsigned short)value;
}

int Measurement_SelectWindowOutput(unsigned char count,
                                   unsigned char window_length,
                                   int sample,
                                   int stable_average,
                                   unsigned char abnormal)
{
    if (count < window_length)
    {
        return sample;
    }
    if (abnormal)
    {
        return sample;
    }
    return stable_average;
}

unsigned char Measurement_NextWindowCount(unsigned char count,
                                          unsigned char window_length,
                                          unsigned char abnormal)
{
    if (count < window_length)
    {
        return (unsigned char)(count + 1);
    }
    if (abnormal)
    {
        return 0;
    }
    return count;
}

int Measurement_ApplyO2Startup(unsigned char startup_count,
                               unsigned char window_length,
                               int calculated_output)
{
    if (startup_count < window_length)
    {
        return 209;
    }
    return calculated_output;
}

unsigned char Measurement_NextStartupCount(unsigned char startup_count,
                                           unsigned char window_length)
{
    if (startup_count < window_length)
    {
        return (unsigned char)(startup_count + 1);
    }
    return startup_count;
}
