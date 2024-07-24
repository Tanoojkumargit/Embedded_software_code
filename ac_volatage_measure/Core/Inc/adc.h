#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx_hal.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;

void MX_ADC1_Init(void);
void MX_ADC3_Init(void);

#endif /* __ADC_H */
