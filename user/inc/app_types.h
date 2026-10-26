#ifndef APP_TYPES_H
#define APP_TYPES_H

typedef struct
{
    int temperature;       /* 0.01 degree C when humidity sensor is enabled */
    int temperature_NTC;   /* 0.01 degree C */
    int nRH;               /* historical protocol scaling */
    float press_in;        /* historical LQTP scaling, currently kPa */
    float press_out;       /* historical LQTP scaling, currently kPa */
    int h2o_press_factor;  /* 0.1 percent O2 correction */
} SensorValue;

typedef struct
{
    int K_O2;
    int B_O2;
    int Cn_O2;
} CalibrationCoefficient;

typedef struct
{
    char SendOrder;
    unsigned char cENAutoSendFlag;
    int nWarnVale[3];
    CalibrationCoefficient coef;
} FlashData;

typedef struct
{
    long lFf;
    long lFb;
    long CountLong;
    int nTemp;
    int Result;
    float Coefficient[2];
    int AmbientResult;
    int PressResult;
    int ReResult;
} MyStructTrait5P;

#endif /* APP_TYPES_H */
