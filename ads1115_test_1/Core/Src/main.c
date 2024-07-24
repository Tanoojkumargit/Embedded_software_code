
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define ADS1115_ADDRESS 0x48 << 1
#define ADS1115_CONVERSION_REG 0x00
#define ADS1115_CONFIG_REG 0x01
#define ADS1115_CONFIG_MUX_DIFF_0_1 0x0000  // AIN0 - AIN1
#define ADS1115_CONFIG_PGA_6_144V 0x0000    // +/-6.144V range
#define ADS1115_CONFIG_MODE_SINGLE 0x0100   // Single-shot mode
#define ADS1115_CONFIG_DR_128SPS 0x0080     // 128 samples per second
#define ADS1115_CONFIG_OS_SINGLE 0x8000     // Start a single conversion
#define NUM_SAMPLES 100 // Increase the number of samples for better accuracy

//#define NUM_SAMPLES 10

void SystemClock_Config(void);
void Error_Handler(void);
int16_t read_ads1115(void);
void SystemClock_Config(void);
void Error_Handler(void);
int16_t read_ads1115(void);

float voltage_samples[NUM_SAMPLES] = {0};
float current_samples[NUM_SAMPLES] = {0};
int sample_index = 0;


float calculate_average(float* samples, int num_samples)

 {
    float sum = 0;
    for (int i = 0; i < num_samples; i++) {
        sum += samples[i];
    }
    return sum / num_samples;
}

extern I2C_HandleTypeDef hi2c1; // Reference the existing hi2c1 handle

int main(void)
{

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();

  while (1)
  {
      for (int i = 0; i < NUM_SAMPLES; i++) {
          int16_t raw_adc = read_ads1115();
          float voltage = raw_adc * (6.144 / 32768.0); // Convert raw ADC value to voltage
          float current = voltage / 0.1; // SCT013 output (assuming 100A/50mA sensor)

          // Store the latest samples
          voltage_samples[i] = voltage;
          current_samples[i] = current;

          HAL_Delay(10); // Short delay between samples
      }

      // Calculate the moving average
      float avg_voltage = calculate_average(voltage_samples, NUM_SAMPLES);
      float avg_current = calculate_average(current_samples, NUM_SAMPLES);

      // Convert the average values to integers for the integer part and scaled integer for the fractional part
      int voltage_int = (int)avg_voltage;
      int voltage_frac = (int)((avg_voltage - voltage_int) * 10000);
      int current_int = (int)avg_current;
      int current_frac = (int)((avg_current - current_int) * 100);

      char buffer[100];
      // Use integer arithmetic for sprintf
      sprintf(buffer, "Average Voltage: %d.%04d V, Average Current: %d.%02d A\r\n",
              voltage_int, abs(voltage_frac), current_int, abs(current_frac));
      HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), HAL_MAX_DELAY);

      HAL_Delay(100); // Wait before the next batch of samples
  }

}

int16_t read_ads1115(void)
{
    uint16_t config = ADS1115_CONFIG_OS_SINGLE | ADS1115_CONFIG_MUX_DIFF_0_1 |
                      ADS1115_CONFIG_PGA_6_144V | ADS1115_CONFIG_MODE_SINGLE |
                      ADS1115_CONFIG_DR_128SPS;

    uint8_t config_bytes[3];
    config_bytes[0] = ADS1115_CONFIG_REG;
    config_bytes[1] = config >> 8;
    config_bytes[2] = config & 0xFF;

    if (HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS, config_bytes, 3, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_Delay(10);  // Wait for conversion to complete

    uint8_t conversion_reg = ADS1115_CONVERSION_REG;
    if (HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS, &conversion_reg, 1, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    uint8_t adc_bytes[2];
    if (HAL_I2C_Master_Receive(&hi2c1, ADS1115_ADDRESS, adc_bytes, 2, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    int16_t adc_value = (adc_bytes[0] << 8) | adc_bytes[1];
    return adc_value;
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

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
