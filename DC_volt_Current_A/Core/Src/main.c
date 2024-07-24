#include "main.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>
#include <string.h> // Include for strlen

// Function prototypes
uint32_t Read_ADC(ADC_HandleTypeDef* hadc, uint32_t Channel);
void SystemClock_Config(void);
void FloatToString(char *buffer, float value, int decimal_digits);

// Main function
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC2_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();

    uint32_t adc_value_voltage;
    uint32_t adc_value_current;
    float voltage, current;
    char msg[50];

    while (1)
    {
        adc_value_voltage = Read_ADC(&hadc2, ADC_CHANNEL_7); // PA7
        adc_value_current = Read_ADC(&hadc2, ADC_CHANNEL_8); // PB0

        // Convert ADC value to actual voltage and current
        voltage = (adc_value_voltage * 3.3f / 4096.0f) * ((20.0f + 6.8f) / 6.8f); // Considering the voltage divider
        current = (adc_value_current * 3.3f / 4096.0f - 2.5f) / 0.066f; // ACS712 with 66mV/A sensitivity and 2.5V zero current offset

        // Convert float values to string
        char voltage_str[20];
        char current_str[20];
        FloatToString(voltage_str, voltage, 2); // 2 decimal places for voltage
        FloatToString(current_str, current, 2); // 2 decimal places for current

        // Prepare message to send via UART
        snprintf(msg, sizeof(msg), "Voltage: %s V, Current: %s A\r\n", voltage_str, current_str);
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        HAL_Delay(1000); // 1 second delay
    }
}

void FloatToString(char *buffer, float value, int decimal_digits)
{
    int int_part = (int)value;
    float remainder = value - (float)int_part;

    // Handle negative numbers
    if (value < 0)
    {
        remainder = -remainder;
    }

    // Convert integer part to string
    sprintf(buffer, "%d.", int_part);

    // Convert decimal part to string
    for (int i = 0; i < decimal_digits; i++)
    {
        remainder *= 10;
        int digit = (int)remainder;
        sprintf(buffer + strlen(buffer), "%d", digit);
        remainder -= (float)digit;
    }
}

uint32_t Read_ADC(ADC_HandleTypeDef* hadc, uint32_t Channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = Channel;
    sConfig.Rank = 1; // Adjust rank assignment for ADC channel configuration
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    HAL_ADC_ConfigChannel(hadc, &sConfig);

    HAL_ADC_Start(hadc);
    HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
    uint32_t adc_value = HAL_ADC_GetValue(hadc);
    HAL_ADC_Stop(hadc);

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
    // User can add his own implementation to report the HAL assert_param error return state
}
#endif
