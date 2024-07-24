/*
 * rtc.h
 *
 *  Created on: Jun 19, 2024
 *      Author: tanoo
 */



#ifndef __RTC_H
#define __RTC_H

#include "stm32f4xx_hal.h"

void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
void RTC_SetDate(uint8_t day, uint8_t month, uint16_t year);
void RTC_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
void RTC_GetDate(uint8_t *day, uint8_t *month, uint16_t *year);

#endif /* __RTC_H */
