
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"
#include "stdio.h"

void SystemClock_Config(void);
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

float Convert_ADC_to_Temperature(uint32_t adc_value) {
  float voltage = adc_value * (3.3f / 4096.0f);  // Assuming 3.3V reference and 12-bit ADC resolution
  return voltage * 100.0f;  // LM35 gives 10mV per degree Celsius
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
  char uart_buf[50];
  int uart_buf_len;
  uint32_t adc_value;
  float temperature;
  char temperature_str[20];

  while (1)
  {

    adc_value = Read_ADC();
    temperature = Convert_ADC_to_Temperature(adc_value);

    // Convert float to string
    FloatToString(temperature_str, temperature, 2);

    // Print the temperature
    uart_buf_len = sprintf(uart_buf, "Temperature: %s°C\r\n", temperature_str);
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
