#include "main.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"

#define THRESHOLD_MQ135 500 // Example threshold value
#define THRESHOLD_MQ2   500 // Example threshold value
#define THRESHOLD_MQ7   700 // Example threshold value

void SystemClock_Config(void);
uint32_t Read_ADC_Channel(ADC_HandleTypeDef* hadc, uint32_t channel);

void UART_Print(UART_HandleTypeDef* huart, char* message) {
    HAL_UART_Transmit(huart, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_ADC1_Init();

    char msg[64];
    uint32_t mq135_value, mq2_value, mq7_value;

    HAL_ADC_Start(&hadc1);
    while (1) {
        // Read ADC values
        mq135_value = Read_ADC_Channel(&hadc1, ADC_CHANNEL_0); // PA0
        mq2_value = Read_ADC_Channel(&hadc1, ADC_CHANNEL_1);   // PA1
        mq7_value = Read_ADC_Channel(&hadc1, ADC_CHANNEL_4);   // PA4

        // Print the ADC values
        sprintf(msg, "MQ-135: %lu\r\n", mq135_value);
        UART_Print(&huart2, msg);

        sprintf(msg, "MQ-2: %lu\r\n", mq2_value);
        UART_Print(&huart2, msg);

        sprintf(msg, "MQ-7: %lu\r\n", mq7_value);
        UART_Print(&huart2, msg);

        // Check for specific conditions and print messages
        if (mq135_value > THRESHOLD_MQ135) {
            UART_Print(&huart2, "Ammonia leakage detected!\r\n");
        }

        if (mq2_value > THRESHOLD_MQ2) {
            UART_Print(&huart2, "Smoke detected!\r\n");
        }

        if (mq7_value > THRESHOLD_MQ7) {
            UART_Print(&huart2, "Flammable gas detected!\r\n");
        }

        HAL_Delay(1000); // Delay for readability
    }
}

uint32_t Read_ADC_Channel(ADC_HandleTypeDef* hadc, uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = 1; // Use integer 1 directly
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;

    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_ADC_Start(hadc);
    HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
    uint32_t value = HAL_ADC_GetValue(hadc);
    HAL_ADC_Stop(hadc);

    return value;
}






void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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

  /** Initializes the CPU, AHB and APB buses clocks
  */
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

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
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
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
