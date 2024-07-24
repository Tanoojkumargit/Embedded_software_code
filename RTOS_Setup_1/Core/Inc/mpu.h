// mpu.h

#ifndef MPU_H
#define MPU_H

#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

#define MPU6050_ADDR (0x68 << 1)
#define WHO_AM_I_REG 0x75
#define ACCEL_XOUT_H 0x3B
#define ACCEL_SCALE 16384.0  // For AFS_SEL = 0

#define MAX_VIBRATION_LEVEL 1.0  // Assuming 1.0 is the maximum vibration level for 100%
#define ALPHA 0.5  // Low-pass filter coefficient
#define SENSITIVITY_MULTIPLIER 10.0  // Multiplier to increase sensitivity
#define MOVING_AVERAGE_SIZE 5  // Size of the moving average window

void MPU6050_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef MPU6050_Read_Accel(I2C_HandleTypeDef *hi2c, int16_t *Accel_X, int16_t *Accel_Y, int16_t *Accel_Z);
void Calculate_Vibration_Percentage(int16_t Accel_X, int16_t Accel_Y, int16_t Accel_Z, int16_t *prev_Accel_X, int16_t *prev_Accel_Y, int16_t *prev_Accel_Z, float *vibration_percentage);
void FloatToString(char* buffer, float value, int decimalPlaces);
void LowPassFilter(int16_t *current, int16_t *previous, float alpha);
void MovingAverageFilter(float *new_value, float *average, float *buffer, int size, int *index);

#endif // MPU_H
