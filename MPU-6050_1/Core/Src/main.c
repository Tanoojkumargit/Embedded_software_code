#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"
#include "math.h"

#define MPU6050_ADDR (0x68 << 1)
#define WHO_AM_I_REG 0x75
#define ACCEL_XOUT_H 0x3B
#define ACCEL_SCALE 16384.0  // For AFS_SEL = 0

#define MAX_VIBRATION_LEVEL 1.0  // Assuming 1.0 is the maximum vibration level for 100%
#define ALPHA 0.5  // Low-pass filter coefficient
#define SENSITIVITY_MULTIPLIER 10.0  // Multiplier to increase sensitivity
#define MOVING_AVERAGE_SIZE 5  // Size of the moving average window






void SystemClock_Config(void);
int _write(int file, char *data, int len);
void MPU6050_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef MPU6050_Read_Accel(I2C_HandleTypeDef *hi2c, int16_t *Accel_X, int16_t *Accel_Y, int16_t *Accel_Z);
void Calculate_Vibration_Percentage(int16_t Accel_X, int16_t Accel_Y, int16_t Accel_Z, int16_t *prev_Accel_X, int16_t *prev_Accel_Y, int16_t *prev_Accel_Z, float *vibration_percentage);
void FloatToString(char* buffer, float value, int decimalPlaces);
void LowPassFilter(int16_t *current, int16_t *previous, float alpha);
void MovingAverageFilter(float *new_value, float *average, float *buffer, int size, int *index);

int _write(int file, char *data, int len) {
  HAL_UART_Transmit(&huart2, (uint8_t*)data, len, 1000);
  return len;
}

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

int main(void) {

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  MPU6050_Init(&hi2c2);
  int16_t Accel_X, Accel_Y, Accel_Z;
  int16_t prev_Accel_X = 0, prev_Accel_Y = 0, prev_Accel_Z = 0;
  float vibration_percentage;
  char uart_buf[100];
  char float_buf[20];
  int uart_buf_len;
  uint8_t i2c_address = MPU6050_ADDR >> 1; // Print the 7-bit address
  uart_buf_len = sprintf(uart_buf, "I2C Address: 0x%02X\r\n", i2c_address);
  HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);

  // Moving average variables
  float vibration_buffer[MOVING_AVERAGE_SIZE] = {0};
  float smoothed_vibration = 0.0;
  int buffer_index = 0;

  // Read initial accelerometer values to initialize previous values
  if (MPU6050_Read_Accel(&hi2c2, &prev_Accel_X, &prev_Accel_Y, &prev_Accel_Z) != HAL_OK) {
    uart_buf_len = sprintf(uart_buf, "Error reading initial accelerometer data\r\n");
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);
  }

  while (1) {
    if (MPU6050_Read_Accel(&hi2c2, &Accel_X, &Accel_Y, &Accel_Z) == HAL_OK) {

      Calculate_Vibration_Percentage(Accel_X, Accel_Y, Accel_Z, &prev_Accel_X, &prev_Accel_Y, &prev_Accel_Z, &vibration_percentage);

      // Apply moving average filter to vibration percentage
      MovingAverageFilter(&vibration_percentage, &smoothed_vibration, vibration_buffer, MOVING_AVERAGE_SIZE, &buffer_index);

      uart_buf_len = sprintf(uart_buf, "Accel_X: %d, Accel_Y: %d, Accel_Z: %d\r\n", Accel_X, Accel_Y, Accel_Z);
      HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);

      FloatToString(float_buf, smoothed_vibration, 2);
      uart_buf_len = sprintf(uart_buf, "Vibration: %s%%\r\n", float_buf);
      HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);

    } else {
      uart_buf_len = sprintf(uart_buf, "Error reading accelerometer data\r\n");
      HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);
    }

    HAL_Delay(1000); // Delay of 1 second for more frequent readings
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{

}
#endif
