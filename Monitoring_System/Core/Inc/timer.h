#ifndef TIMER_H
#define TIMER_H

#include "main.h"

extern TIM_HandleTypeDef htim2; // Replace with your timer handler

void Timer_Init(void);
void Timer_Start(void);
void Timer_Stop(void);

#endif /* TIMER_H */
