#ifndef __ADC_H
#define __ADC_H

#include <hk32f0xxa_adc.h>

#define VREFINT_CAL *(volatile unsigned short     int *)(VREFINT_CAL_ADDR)

void  Adc_Init_NTC(void);
unsigned int Get_Adc(void);

#endif
