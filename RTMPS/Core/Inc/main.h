/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define Current_Sen_M1_Pin GPIO_PIN_0
#define Current_Sen_M1_GPIO_Port GPIOC
#define Temp_Ref_Pin GPIO_PIN_1
#define Temp_Ref_GPIO_Port GPIOC
#define Vibration_Sen_Pin GPIO_PIN_2
#define Vibration_Sen_GPIO_Port GPIOC
#define Temp_Sen_Pin GPIO_PIN_3
#define Temp_Sen_GPIO_Port GPIOC
#define Phase_1_Pin GPIO_PIN_0
#define Phase_1_GPIO_Port GPIOA
#define Phase_2_Pin GPIO_PIN_1
#define Phase_2_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define Phase_3_Pin GPIO_PIN_4
#define Phase_3_GPIO_Port GPIOA
#define Mains_Ref_Pin GPIO_PIN_5
#define Mains_Ref_GPIO_Port GPIOA
#define MQ_135_Pin GPIO_PIN_6
#define MQ_135_GPIO_Port GPIOA
#define MQ_2_Pin GPIO_PIN_7
#define MQ_2_GPIO_Port GPIOA
#define MQ_7_Pin GPIO_PIN_4
#define MQ_7_GPIO_Port GPIOC
#define V_Sen_M1_Pin GPIO_PIN_0
#define V_Sen_M1_GPIO_Port GPIOB
#define Buzzer_Alert_Pin GPIO_PIN_1
#define Buzzer_Alert_GPIO_Port GPIOB
#define AC_Current_SCL_Pin GPIO_PIN_10
#define AC_Current_SCL_GPIO_Port GPIOB
#define Led_Red_Pin GPIO_PIN_6
#define Led_Red_GPIO_Port GPIOC
#define Led_green_Pin GPIO_PIN_7
#define Led_green_GPIO_Port GPIOC
#define Buzzer_Pin GPIO_PIN_8
#define Buzzer_GPIO_Port GPIOC
#define MPU_SDA_Pin GPIO_PIN_9
#define MPU_SDA_GPIO_Port GPIOC
#define MPU_SCL_Pin GPIO_PIN_8
#define MPU_SCL_GPIO_Port GPIOA
#define ESP_TX_Pin GPIO_PIN_9
#define ESP_TX_GPIO_Port GPIOA
#define ESP_RX_Pin GPIO_PIN_10
#define ESP_RX_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define AC_Currrent_SDA_Pin GPIO_PIN_12
#define AC_Currrent_SDA_GPIO_Port GPIOC
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define RTC_SCL_Pin GPIO_PIN_8
#define RTC_SCL_GPIO_Port GPIOB
#define RTC_SDA_Pin GPIO_PIN_9
#define RTC_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
