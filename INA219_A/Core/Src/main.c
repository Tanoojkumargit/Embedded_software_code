#include "stm32f4xx_hal.h"
#include "string.h"
#include "stdio.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"

#define INA219_ADDRESS 0x80  // Adjust this to your INA219's actual address

#define INA219_REG_CURRENT 0x04
#define INA219_REG_BUS_VOLTAGE 0x02

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_I2C1_Init(void);
void MX_USART2_UART_Init(void);
void INA219_Init(void);  // Function prototype

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_USART2_UART_Init();
    INA219_Init();  // Initialize the INA219 sensor

    char buffer[64];
    char *testMessage = "Hello UART\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)testMessage, strlen(testMessage), HAL_MAX_DELAY);


    while (1) {
        uint16_t current_raw = read_INA219(INA219_REG_CURRENT);
        uint16_t voltage_raw = read_INA219(INA219_REG_BUS_VOLTAGE);

        // Debug output to UART
        char debug_msg[100];
        snprintf(debug_msg, sizeof(debug_msg), "Debug - Raw Current: %u, Raw Voltage: %u\n", current_raw, voltage_raw);
        HAL_UART_Transmit(&huart2, (uint8_t *)debug_msg, strlen(debug_msg), HAL_MAX_DELAY);

        float current = get_current_mA(current_raw);
        float voltage = get_voltage_V(voltage_raw);

       // char buffer[64];
        snprintf(buffer, sizeof(buffer), "Voltage: %.2f V, Current: %.2f mA\r\n", voltage, current);
        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

        HAL_Delay(1000);
    }

}

uint16_t read_INA219(uint8_t reg) {
    uint8_t buf[2];
    if (HAL_I2C_Mem_Read(&hi2c1, INA219_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, buf, 2, HAL_MAX_DELAY) != HAL_OK) {
        printf("I2C read error!\n");
        return 0;
    }
    return (buf[0] << 8) | buf[1];
}

float get_current_mA(uint16_t value) {
    return value * 0.1;  // Adjust according to your calibration
}

float get_voltage_V(uint16_t value) {
    return (value >> 3) * 0.004;  // Based on datasheet
}

void SystemClock_Config(void) {
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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}
