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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>

#include <string.h>


#define ADS1115_ADDRESS 0x48  // Update this according to your setup
// Change this in your declarations at the top of the file
uint16_t read_ADS1115(uint8_t channel);  // Match this with the definition

void send_hello_world(void);
float read_voltage_from_ADS1115(void);
void SystemClock_Config(void);




//#define ADS1115_ADDRESS 0x90  // Check if your ADS1115 address matches
#define ADS1115_CONV_REG 0x00
#define ADS1115_CONFIG_REG 0x01
uint8_t config_data[3] = {0x01, 0xC1, 0x83};  // Example configuration for single-ended input



int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();
    MX_USART3_UART_Init();

    while (1) {
           uint16_t adc_value = read_ADS1115();
           float current = adc_value * (4.096 / 32768.0) / 0.1;  // Conversion factor depends on your setup

           char message[64];
           snprintf(message, sizeof(message), "Measured Current: %.2f A\r\n", current);

           send_uart_message(&huart1, message);
           send_uart_message(&huart3, message);

           HAL_Delay(1000);  // Delay between reads
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}






uint16_t read_ADS1115(uint8_t channel)
{
    uint8_t config[3] = {0x01, 0x40 | (channel << 4), 0x83};  // Config for single-ended input on selected channel
    uint8_t data[2];

    HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1, config, sizeof(config), 1000);
    HAL_Delay(10);  // Delay for conversion time
    HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1, (uint8_t[]){0x00}, 1, 1000);  // Point to conversion register
    HAL_I2C_Master_Receive(&hi2c1, ADS1115_ADDRESS << 1, data, 2, 1000);

    return (data[0] << 8) | data[1];
}


float read_voltage_from_ADS1115(void) {
    uint8_t config[3] = {0x01, 0xC3, 0x83};  // Config register: MUX set for A0, Gain for ±4.096V, Single-shot mode
    uint8_t data[2];
    uint16_t adc_value;

    // Start single conversion
    HAL_I2C_Master_Transmit(&hi2c1, (ADS1115_ADDRESS << 1), config, sizeof(config), 1000);
    HAL_Delay(8);  // Wait for conversion to complete

    // Read conversion result
    HAL_I2C_Master_Transmit(&hi2c1, (ADS1115_ADDRESS << 1), (uint8_t[]){0x00}, 1, 1000);  // Point to conversion register
    HAL_I2C_Master_Receive(&hi2c1, (ADS1115_ADDRESS << 1), data, 2, 1000);

    // Calculate voltage
    adc_value = (data[0] << 8) | data[1];
    return adc_value * 4.096 / 32768.0;  // Convert to voltage based on gain
}







void send_uart_message(UART_HandleTypeDef *huart, const char *message)
{
    HAL_UART_Transmit(huart, (uint8_t *)message, strlen(message), HAL_MAX_DELAY);
}




void send_hello_world(void)
{
  const char *msg = "Communicating with cloud....\n\n";
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart3, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}




void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{

}
#endif
