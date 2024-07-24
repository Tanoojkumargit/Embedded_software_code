/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
#include "stdio.h"
#include <math.h>
#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SAMPLES_COUNT 200  // Increased number of samples

/* Calibration values */
#define ADC_ZERO_VOLT 196  // Example value for 0V with 8-bit resolution (needs calibration)
#define ADC_MAX_VOLT 170   // Example value for 250V with 8-bit resolution (needs calibration)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* Private user code ---------------------------------------------------------*/
extern UART_HandleTypeDef huart2;

int _write(int file, char *data, int len) {
  HAL_UART_Transmit(&huart2, (uint8_t*)data, len, 1000);
  return len;
}

uint32_t Read_ADC(void) {
  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
    return HAL_ADC_GetValue(&hadc1);
  }
  return 0;
}

void sort_array(uint32_t* data, int size) {
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (data[j] > data[j + 1]) {
        uint32_t temp = data[j];
        data[j] = data[j + 1];
        data[j + 1] = temp;
      }
    }
  }
}

float median_filter(uint32_t* data, int size) {
  sort_array(data, size);
  if (size % 2 == 0) {
    return (data[size / 2 - 1] + data[size / 2]) / 2.0;
  } else {
    return data[size / 2];
  }
}

float running_average_filter(uint32_t* data, int size) {
  uint32_t sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i];
  }
  return sum / (float)size;
}

float Convert_ADC_to_Voltage(uint32_t adc_value) {
  float voltage = 0.0;

  if (adc_value > ADC_ZERO_VOLT) {
    voltage = 0.0;
  } else if (adc_value < ADC_MAX_VOLT) {
    voltage = 250.0;
  } else {
    voltage = 250.0 * ((float)(ADC_ZERO_VOLT - adc_value) / (ADC_ZERO_VOLT - ADC_MAX_VOLT));
  }

  return voltage;
}

void FloatToString(char* buffer, float value, int decimalPlaces) {
  int integerPart = (int)value;
  int fractionPart = (int)((value - integerPart) * pow(10, decimalPlaces));
  sprintf(buffer, "%d.%0*d", integerPart, decimalPlaces, abs(fractionPart));
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();

  char uart_buf[100];
  int uart_buf_len;
  uint32_t adc_values[SAMPLES_COUNT];
  uint32_t adc_value;
  float voltage;
  char voltage_str[20];

  while (1)
  {
    for (int i = 0; i < SAMPLES_COUNT; i++) {
      adc_values[i] = Read_ADC();
      HAL_Delay(10); // Small delay between samples to avoid rapid fluctuation
    }

    // Apply median filter to remove outliers
    adc_value = (uint32_t)median_filter(adc_values, SAMPLES_COUNT);
    // Apply running average filter to smooth the result
    adc_value = (uint32_t)running_average_filter(adc_values, SAMPLES_COUNT);

    voltage = Convert_ADC_to_Voltage(adc_value);

    // Convert float to string
    FloatToString(voltage_str, voltage, 2);

    // Print the voltage and ADC value
    uart_buf_len = sprintf(uart_buf, "ADC Value: %lu, Voltage: %sV\r\n", adc_value, voltage_str);
    HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);

    HAL_Delay(1000);  // 1 second delay
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
