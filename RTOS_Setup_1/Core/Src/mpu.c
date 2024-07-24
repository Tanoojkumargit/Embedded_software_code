// mpu.c

#include "mpu.h"

void MPU6050_Init(I2C_HandleTypeDef *hi2c) {
  uint8_t check, Data;

  // Check device ID WHO_AM_I
  HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 1000);

  if (check == 104) { // 0x68 will be returned by the sensor if everything goes well
    // Power management register 0X6B we should write all 0's to wake the sensor up
    Data = 0;
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, 0x6B, 1, &Data, 1, 1000);

    // Set DATA RATE of 1KHz by writing SMPLRT_DIV register
    Data = 0x07;
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, 0x19, 1, &Data, 1, 1000);

    // Set accelerometer configuration in ACCEL_CONFIG Register
    // XA_ST=0, YA_ST=0, ZA_ST=0, AFS_SEL=0 => ±2g
    Data = 0x00;  // Set AFS_SEL to 0 for ±2g
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, 0x1C, 1, &Data, 1, 1000);

    // Set Gyroscopic configuration in GYRO_CONFIG Register
    // XG_ST=0, YG_ST=0, ZG_ST=0, FS_SEL=0 => ±250 °/s
    Data = 0x00;
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, 0x1B, 1, &Data, 1, 1000);
  }
}

HAL_StatusTypeDef MPU6050_Read_Accel(I2C_HandleTypeDef *hi2c, int16_t *Accel_X, int16_t *Accel_Y, int16_t *Accel_Z) {
  uint8_t Rec_Data[6];
  HAL_StatusTypeDef ret;

  // Read 6 BYTES of data starting from ACCEL_XOUT_H register
  ret = HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR, ACCEL_XOUT_H, 1, Rec_Data, 6, 1000);

  if (ret == HAL_OK) {
    *Accel_X = (int16_t)(Rec_Data[0] << 8 | Rec_Data[1]);
    *Accel_Y = (int16_t)(Rec_Data[2] << 8 | Rec_Data[3]);
    *Accel_Z = (int16_t)(Rec_Data[4] << 8 | Rec_Data[5]);
  }

  return ret;
}

void LowPassFilter(int16_t *current, int16_t *previous, float alpha) {
  *previous = (int16_t)(alpha * (*current) + (1 - alpha) * (*previous));
}

void MovingAverageFilter(float *new_value, float *average, float *buffer, int size, int *index) {
  buffer[*index] = *new_value;
  *index = (*index + 1) % size;
  float sum = 0.0;
  for (int i = 0; i < size; i++) {
    sum += buffer[i];
  }
  *average = sum / size;
}

void Calculate_Vibration_Percentage(int16_t Accel_X, int16_t Accel_Y, int16_t Accel_Z, int16_t *prev_Accel_X, int16_t *prev_Accel_Y, int16_t *prev_Accel_Z, float *vibration_percentage) {
  // Apply low-pass filter
  LowPassFilter(&Accel_X, prev_Accel_X, ALPHA);
  LowPassFilter(&Accel_Y, prev_Accel_Y, ALPHA);
  LowPassFilter(&Accel_Z, prev_Accel_Z, ALPHA);

  // Calculate the difference between the current and previous values
  float diff_X = (float)(Accel_X - *prev_Accel_X) / ACCEL_SCALE;
  float diff_Y = (float)(Accel_Y - *prev_Accel_Y) / ACCEL_SCALE;
  float diff_Z = (float)(Accel_Z - *prev_Accel_Z) / ACCEL_SCALE;

  // Calculate the magnitude of the differences
  float magnitude = sqrtf(diff_X * diff_X + diff_Y * diff_Y + diff_Z * diff_Z);

  // Apply sensitivity multiplier
  magnitude *= SENSITIVITY_MULTIPLIER;

  // Calculate the percentage of the vibration level
  *vibration_percentage = (magnitude / MAX_VIBRATION_LEVEL) * 100.0;  // Convert to percentage

  // Update previous values
  *prev_Accel_X = Accel_X;
  *prev_Accel_Y = Accel_Y;
  *prev_Accel_Z = Accel_Z;
}

void FloatToString(char* buffer, float value, int decimalPlaces) {
  int integerPart = (int)value;
  int fractionPart = (int)((value - integerPart) * pow(10, decimalPlaces));
  sprintf(buffer, "%d.%0*d", integerPart, decimalPlaces, abs(fractionPart));
}
