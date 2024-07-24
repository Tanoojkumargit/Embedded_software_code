#include "main.h"
#include "adc.h"
#include "usart.h"
#include "gpio.h"
#include "i2c.h"
#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "math.h"
#include "rtc.h"
#include "mpu.h"
#include "stdlib.h"
#include "timer.h"
#include "error_handler.h"
#include "globals.h"

volatile uint8_t blink_enabled = 0;
volatile uint8_t low_voltage_flag = 1; // Define the variable here
#define THRESHOLD_MQ135 200 // Example threshold value
#define THRESHOLD_MQ2   150 // Example threshold value
#define THRESHOLD_MQ7   238 // Example threshold value
volatile uint32_t timer_ms = 0;
uint32_t last_time_1s = 0;
//-------------------Functions Prototype---------------------------------------------------------------------------
float Convert_ADC_to_Temperature(uint32_t adc_value);
void FloatToString1(char* buffer, float value, int decimalPlaces);
void motors_temp(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2);
uint32_t Read_ADC_Channel(ADC_HandleTypeDef* hadc, uint32_t channel);
void mq_sensors(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2);
void single_ph_motor(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2);
void UART_Print(UART_HandleTypeDef* huart, char* message);
void print_date_time(UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2);
void SystemClock_Config(void);
void Error_Handler(void); // Ensure prototype is included
void Three_Phase_Motor(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2);
void MPU_6050();
float Convert_ADC_to_MainsVoltage(uint32_t adc_value);
float Convert_ADC_to_Current(uint32_t adc_value);
float Convert_ADC_to_Percentage(uint32_t adc_value);
void TIM2_Init(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void assign_code_word(char code_word[], int index, char value);
void initialize_code_word(char code_word[]);
int code;
char code_word[13]; // 12 for characters + 1 for null terminator

//------------------------------Global Variables---------------------------------------------------------------
char msg[64];
char float_str[20];
uint32_t phase_1, phase_2, phase_3, mains_sup;
uint32_t motor_v, motor_c;
uint32_t temp1, temp2,vibrations;
float temperature_1,temperature_2;
uint32_t mq135_value, mq2_value, mq7_value;
int16_t Accel_X, Accel_Y, Accel_Z;
int16_t prev_Accel_X = 0, prev_Accel_Y = 0, prev_Accel_Z = 0;
float vibration_percentage;
char uart_buf[100];
char float_buf[20];
int uart_buf_len;
float vibration_buffer[MOVING_AVERAGE_SIZE] = {0};
float smoothed_vibration = 0.0;
int buffer_index = 0;
uint32_t last_time = 0;
volatile uint8_t timer_flag = 0;  // Declare this at the top of your `main.c` file.
extern TIM_HandleTypeDef htim3;
    uint8_t hours, minutes, seconds, day, month;
    uint16_t year;
//int code_word [12];//=000000000000;
int main(void)
{

  HAL_Init();
  SystemClock_Config();
 // SysTick_Init();
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  HAL_ADC_Start(&hadc1);
  Timer_Init(); // Initialize timer
  TIM2_Init(); // Initialize the timer
  //RTC_SetTime(15, 27, 10);
  //RTC_SetDate(24, 7, 2024);
  initialize_code_word(code_word);
  int a=0;
  while (1)
  {
	print_date_time(&huart1, &huart2);
	single_ph_motor(&hadc1, &huart1, &huart2);
    Three_Phase_Motor(&hadc1, &huart1,&huart2);//300
    motors_temp(&hadc1, &huart1, &huart2);
    mq_sensors(&hadc1, &huart1, &huart2);
    RTC_GetTime(&hours, &minutes, &seconds);
    RTC_GetDate(&day, &month, &year);
    MPU_6050();
    HAL_Delay(1000);
    a=a+1;
   // software_delay(1000);
    if (timer_flag) {  // Check if the timer flag is set
               timer_flag = 0; // Reset the timer flag
               // Perform tasks that need to run every second
               // Example: Toggle a GPIO, update a display, etc.
               HAL_GPIO_TogglePin(GPIOC, Buzzer_Pin); // Toggle another pin as an example
          }
  }
}

void TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    TIM_HandleTypeDef htim2;
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7999;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 9999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; // Important for automatic reload
    HAL_TIM_Base_Init(&htim2);
    HAL_TIM_Base_Start_IT(&htim2);
}
// Defined in either main.c or timer.c, but not both
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        timer_flag = 1;
    }
}
void software_delay(uint32_t delay_in_ms) {

    uint32_t i, j;
    for (i = 0; i < delay_in_ms; i++) {
        for (j = 0; j < 1000; j++) {
            __NOP();
        }
    }
}
void assign_code_word(char code_word[], int index, char value) {
    // Check if the index is within the bounds
    if (index >= 0 && index < 12) {
        code_word[index] = value;
    }
}

void initialize_code_word(char code_word[]) {
    // Initialize the code_word array with "000000000000"
    for (int i = 0; i < 12; i++) {
        code_word[i] = '0';
    }
    code_word[12] = '\0'; // Null-terminate the string
}

void Three_Phase_Motor(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2) {
    // Read ADC values for the three phases
    phase_1 = Read_ADC_Channel(hadc, ADC_CHANNEL_0);
    phase_2 = Read_ADC_Channel(hadc, ADC_CHANNEL_1);
    phase_3 = Read_ADC_Channel(hadc, ADC_CHANNEL_4);
    mains_sup = Read_ADC_Channel(hadc, ADC_CHANNEL_8);
    // Convert ADC values to mains voltage
    float phase_1_voltage = Convert_ADC_to_MainsVoltage(phase_1);
    float phase_2_voltage = Convert_ADC_to_MainsVoltage(phase_2);
    float phase_3_voltage = Convert_ADC_to_MainsVoltage(phase_3);
    float mains_voltage = Convert_ADC_to_MainsVoltage(mains_sup);
    // Convert and print phase 1 voltage
    FloatToString(float_str, phase_1_voltage, 2);
    sprintf(msg, "M2_Ph1= %s V\r\n", float_str);
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    // Convert and print phase 2 voltage
    FloatToString(float_str, phase_2_voltage, 2);
    sprintf(msg, "M2_Ph2= %s V\r\n", float_str);
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    // Convert and print phase 3 voltage
    FloatToString(float_str, phase_3_voltage, 2);
    sprintf(msg, "M2_Ph3= %s V\r\n", float_str);
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    // Convert and print mains voltage
    FloatToString(float_str, mains_voltage, 2);
    sprintf(msg, "Mains_Voltage= %s V\r\n", float_str);
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    // Check for low voltage or phase/mains failure
    if (phase_1_voltage < 190 || phase_2_voltage < 190 || phase_3_voltage < 190 || mains_voltage < 190) {
        sprintf(msg, "Alert: ");
        if (mains_voltage < 10) {
            strcat(msg, "Mains_Failure ");
            assign_code_word(code_word, 0, '1'); // Set index 0 to '1'
        } else if (mains_voltage < 190) {
            strcat(msg, "Low_Voltage Mains_Voltage ");
            assign_code_word(code_word, 0, '2'); // Set index 0 to '1'
        }
        if (phase_1_voltage < 10) {
            strcat(msg, "Phase_Failure_Phase_1 ");
            assign_code_word(code_word, 1, '1'); // Set index 1 to '1'
        } else if (phase_1_voltage < 190) {
            strcat(msg, "Low_Voltage Phase_1 ");
            assign_code_word(code_word, 1, '2'); // Set index 0 to '1'
        }
        if (phase_2_voltage < 10) {
            strcat(msg, "Phase_Failure_Phase_2 ");
            assign_code_word(code_word, 2, '1'); // Set index 2 to '1'
        } else if (phase_2_voltage < 190) {
            strcat(msg, "Low_Voltage Phase_2 ");
            assign_code_word(code_word, 2, '2'); // Set index 0 to '1'
        }
        if (phase_3_voltage < 10) {
            strcat(msg, "Phase_Failure_Phase_3 ");
            assign_code_word(code_word, 3, '1'); // Set index 3 to '1'
        } else if (phase_3_voltage < 190) {
            strcat(msg, "Low_Voltage Phase_3 ");
            assign_code_word(code_word, 3, '2'); // Set index 0 to '1'
        }
        strcat(msg, "\r\n");
        UART_Print(huart1, msg);
        UART_Print(huart2, msg);
    }

    // Print the code_word via UART2
//    sprintf(msg, "code_word= %s\r\n", code_word);
//    UART_Print(huart2, msg);

    sprintf(msg, "Status= Normal\r\n");
    UART_Print(huart2, msg);
}

//-------------------------------------------------------------------------------
void single_ph_motor(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2) {

    motor_v = Read_ADC_Channel(hadc, ADC_CHANNEL_15);
    motor_c = Read_ADC_Channel(hadc, ADC_CHANNEL_10);
    float m_v = Convert_ADC_to_MainsVoltage(motor_v);
    float motor_current = Convert_ADC_to_Current(motor_c);
    FloatToString(float_str, m_v, 2);
    sprintf(msg, "M1_Voltage= %s V\r\n", float_str); // Use %lu for uint32_t
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    // Voltage alert and control for LED blinking
    if (m_v < 10) {
            sprintf(msg, "Alert= Low Voltage\r\n");
            assign_code_word(code_word, 10, '1'); // Assuming index 3 for low voltage alert
            UART_Print(huart1, msg);
            UART_Print(huart2, msg);
        } else if (m_v < 190) {
            sprintf(msg, "Alert= Low Voltage\r\n");
            assign_code_word(code_word, 10, '2'); // Assuming index 3 for low voltage alert
            UART_Print(huart1, msg);
            UART_Print(huart2, msg);
        }
    if (m_v < 200) {
        blink_enabled = 1; // Enable LED blinking
        Timer_Start(); // Start timer for LED blinking
    } else {
        blink_enabled = 0; // Disable LED blinking
        Timer_Stop(); // Stop timer if voltage is above 200V
        HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET); // Ensure LED is off
    }
    // Convert and print motor current
    FloatToString(float_str, motor_current, 2);
    sprintf(msg, "M1_Current= %sA\r\n", float_str);
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    sprintf(msg, "mc= %sA\r\n", float_str);
    //sprintf(msg, "M1_Current: %lu Amps\r\n", motor_c); // Use %lu for uint32_t
    UART_Print(huart1, msg);
    //UART_Print(huart2, msg);

    if (motor_current >= 20.0 && motor_current <= 23.21) {
        sprintf(msg, "Alert= Normal\r\n");
        assign_code_word(code_word, 4, '0');
    } else if (motor_current > 23.21 && motor_current <= 28.21) {
        sprintf(msg, "Alert= Load_Effect\r\n");
        assign_code_word(code_word, 4, '1');
    } else if (motor_current > 28.21 && motor_current <= 30.01) {
        sprintf(msg, "Alert= Overload\r\n");
        assign_code_word(code_word, 4, '2');
    } else {
        sprintf(msg, "Alert= V_Load\r\n");
    }
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);
}

//-----------------Single Phase motor ----------------------------------------------
void motors_temp(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2) {
    // Read ADC values for motor temperature and vibrations
    uint32_t temp1 = Read_ADC_Channel(hadc, ADC_CHANNEL_11);
    uint32_t temp2 = Read_ADC_Channel(hadc, ADC_CHANNEL_13);
    uint32_t vibrations = Read_ADC_Channel(hadc, ADC_CHANNEL_12);
    // Convert ADC values to temperature and vibration percentage
    float temperature_1 = Convert_ADC_to_Temperature(temp1);
    float temperature_2 = Convert_ADC_to_Temperature(temp2);
    float vibration_percentage = Convert_ADC_to_Percentage(vibrations);
    // Convert values to string for printing
    char temperature_str1[20];
    char temperature_str2[20];
    char vibration_str[20];
    FloatToString1(temperature_str1, temperature_1, 2);
    FloatToString1(temperature_str2, temperature_2, 2);
    FloatToString1(vibration_str, vibration_percentage, 2);
    // Print motor temperatures
    sprintf(msg, "M1_Temp= %s °C\r\n", temperature_str1);
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    sprintf(msg, "M2_Temp= %s °C\r\n", temperature_str2);
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    // Print motor vibrations
    sprintf(msg, "Motor1_Vibrations= %s%%\r\n", vibration_str);
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);

    // Check and print temperature conditions for temperature_1
    if (temperature_1 >= 0 && temperature_1 <= 60) {
        sprintf(msg, "Temperature Status: Normal Range\r\n");
        UART_Print(huart2, "Debug: if loop\r\n");
    } else if (temperature_1 > 60 && temperature_1 <= 70) {
        sprintf(msg, "Temperature Status: Poor Ventilation\r\n");
        assign_code_word(code_word, 5, '1');
    } else if (temperature_1 > 70 && temperature_1 <= 80) {
        sprintf(msg, "Temperature Status: Load on Motor\r\n");
        assign_code_word(code_word, 5, '2');
    } else if (temperature_1 > 80 && temperature_1 <= 90) {
        sprintf(msg, "Temperature Status: Overload on Motor\r\n");
        assign_code_word(code_word, 5, '3');
    }
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    // Check and print vibration conditions
    if (vibration_percentage >= 0 && vibration_percentage <= 10) {
        sprintf(msg, "Vibration Status: Normal Range\r\n");
    } else if (vibration_percentage > 10 && vibration_percentage <= 13) {
        sprintf(msg, "Vibration Status: Imbalance Load\r\n");
        assign_code_word(code_word, 6, '1');
    } else if (vibration_percentage > 13 && vibration_percentage <= 20) {
        sprintf(msg, "Vibration Status: Coupling or Faults\r\n");
        assign_code_word(code_word, 6, '2');
    } else if (vibration_percentage > 21 && vibration_percentage <= 30) {
        sprintf(msg, "Vibration Status: Bearing Worn and Shaft Issue\r\n");
        assign_code_word(code_word, 6, '3');
    }
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);

//    UART_Print(huart1, "Debug: Printed vibration status\r\n");
}
//-----------------------------Motors temperatures---------------------------------------------------------


//-----------------------------Mq Sensors------------------------------------
void mq_sensors(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2) {
    mq135_value = Read_ADC_Channel(hadc, ADC_CHANNEL_6); // PA6
    mq2_value = Read_ADC_Channel(hadc, ADC_CHANNEL_7);   // PA7
    mq7_value = Read_ADC_Channel(hadc, ADC_CHANNEL_14);  // Pc4

    sprintf(msg, "MQ135= %lu\r\n", mq135_value);
    //UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    sprintf(msg, "MQ2= %lu\r\n", mq2_value);
    //UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    sprintf(msg, "MQ7= %lu\r\n", mq7_value);
    //UART_Print(huart1, msg);
    UART_Print(huart2, msg);
    // Assuming msg is adequately sized to hold the entire string
    sprintf(msg, "MQ135=%lu; MQ2=%lu; MQ7=%lu;\r\n", mq135_value, mq2_value, mq7_value);
    UART_Print(huart1, msg);  // Print on UART1
   // UART_Print(huart2, msg);  // Print on UART2

    if (mq135_value >= THRESHOLD_MQ135) {
        UART_Print(huart1, "Alert= Ammonia leakage detected\r\n");
        UART_Print(huart2, "Ammonia leakage detected=YES\r\n");
        assign_code_word(code_word, 7, '2');
        //UART_Print(huart2, "MQ=135\r\n");
    }
    if (mq2_value >= THRESHOLD_MQ2) {
        UART_Print(huart1, "Smoke detected!\r\n");
        UART_Print(huart2, "Smoke detected=YES\r\n");
        assign_code_word(code_word, 8, '2');
        //UART_Print(huart2, "MQ=2\r\n");
    }

    if (mq7_value >= THRESHOLD_MQ7) {
        UART_Print(huart1, "Flammable gas detected!\r\n");
        UART_Print(huart2, "Flammable gas detected=YES\r\n");
        assign_code_word(code_word, 9, '2');
        //UART_Print(huart2, "MQ=7\r\n");
    }
    if (mq135_value <= THRESHOLD_MQ135) {
            UART_Print(huart1, "No Ammonia leakage \r\n");
            UART_Print(huart2, "Ammonia leakage=NO\r\n");
            assign_code_word(code_word, 7, '1');

            //UART_Print(huart2, "MQ=135\r\n");
        }

        if (mq2_value <= THRESHOLD_MQ2) {
            UART_Print(huart1, "No Smoke detected!\r\n");
            UART_Print(huart2, "Smoke detected=NO\r\n");
            assign_code_word(code_word, 8, '1');
            //UART_Print(huart2, "MQ=2\r\n");
        }

        if (mq7_value <=THRESHOLD_MQ7) {
            UART_Print(huart1, "No Flammable gas detected!\r\n");
            UART_Print(huart2, "Flammable gas detected=NO\r\n");
            assign_code_word(code_word, 9, '1');
            //UART_Print(huart2, "MQ=7\r\n");
        }
        sprintf(msg, "code_word= %s\r\n", code_word);
        UART_Print(huart2, msg);

}
//------------------------------------------------------------------------------------------



///-----------------------------------UART  print----------------------------------
void UART_Print(UART_HandleTypeDef* huart, char* message) {
    HAL_UART_Transmit(huart, (uint8_t*)message, strlen(message), HAL_MAX_DELAY);
}

//----------------------------Read ADC Value Fun----------------------------------------
uint32_t Read_ADC_Channel(ADC_HandleTypeDef* hadc, uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    if (HAL_ADC_ConfigChannel(hadc, &sConfig) != HAL_OK) {
        Error_Handler();
    }
    uint32_t total = 0;
    const int num_samples = 10; // Number of samples for averaging
    for (int i = 0; i < num_samples; i++) {
        HAL_ADC_Start(hadc);
        HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
        total += HAL_ADC_GetValue(hadc);
        HAL_ADC_Stop(hadc);
    }
    return total / num_samples;
}

//--------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------
float Convert_ADC_to_Percentage(uint32_t adc_value) {
    const float ADC_RESOLUTION = 1023.0;
    return (adc_value / ADC_RESOLUTION) * 100.0f;
}
//---------------------------------------------------------------------------------
float Convert_ADC_to_Current(uint32_t adc_value) {
    // ADC resolution and reference voltage
    const float ADC_RESOLUTION = 1023.0;
    const float VREF = 3.3;
    const float MAX_ADC_VOLTAGE = 3.06;
    const float MAX_CURRENT = 32.0;
    // Convert ADC value to voltage
    float adc_voltage = (adc_value / ADC_RESOLUTION) * VREF;
    // Scale the voltage to represent the current (0 to 32 amps)
    float current = (adc_voltage / MAX_ADC_VOLTAGE) * MAX_CURRENT;
    return current;
}
//-------------------------------------------------------------------------------
float Convert_ADC_to_Temperature(uint32_t adc_value) {
  float voltage = adc_value * (3.3f / 1023.0f);  // Assuming 3.3V reference and 12-bit ADC resolution
  return voltage * 100.0f;  // LM35 gives 10mV per degree Celsius
}
//-------------------------------------------------------------------------
void FloatToString1(char* buffer, float value, int decimalPlaces) {
  int integerPart = (int)value;
  int fractionPart = (int)((value - integerPart) * pow(10, decimalPlaces));
  sprintf(buffer, "%d.%0*d", integerPart, decimalPlaces, abs(fractionPart));
}

//----------------------------------------------------------------------------------------------------
float Convert_ADC_to_MainsVoltage(uint32_t adc_value) {
    // ADC resolution and reference voltage
    const float ADC_RESOLUTION = 1023.0;
    const float VREF = 3.3;
    const float MAX_ADC_VOLTAGE = 3.0;
    const float MAX_MAINS_VOLTAGE = 230.0;
    // Convert ADC value to voltage
    float adc_voltage = (adc_value / ADC_RESOLUTION) * VREF;
    // Scale the voltage to represent 230V
    float mains_voltage = (adc_voltage / MAX_ADC_VOLTAGE) * MAX_MAINS_VOLTAGE;
    return mains_voltage;
}

//--------------------------------------RTC Module-------------------------------------------------------
void print_date_time(UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2) {
//    uint8_t hours, minutes, seconds, day, month;
//    int16_t year;
    char msg[64];
    // Get the current time and date from the RTC
    RTC_GetTime(&hours, &minutes, &seconds);
    RTC_GetDate(&day, &month, &year);
    // Format the date and time into a string
    sprintf(msg, "Date: %02d/%02d/%04d Time: %02d:%02d:%02d\r\n", day, month, year, hours, minutes, seconds);
    // Print the formatted string via UART1 and UART2
    UART_Print(huart1, msg);
    UART_Print(huart2, msg);

}
//-------------------------------------------MPU6050-------------------------------------------------
void MPU_6050(){
	//  uint8_t i2c_address = MPU6050_ADDR >> 1; // Print the 7-bit address
//	  uart_buf_len = sprintf(uart_buf, "I2C Address: 0x%02X\r\n", i2c_address);
//	  HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);
	  // Moving average variables
	  if (MPU6050_Read_Accel(&hi2c2, &prev_Accel_X, &prev_Accel_Y, &prev_Accel_Z) != HAL_OK) {
	     uart_buf_len = sprintf(uart_buf, "Error reading initial accelerometer data\r\n");
	     HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);
	   }
    if (MPU6050_Read_Accel(&hi2c2, &Accel_X, &Accel_Y, &Accel_Z) == HAL_OK) {
          Calculate_Vibration_Percentage(Accel_X, Accel_Y, Accel_Z, &prev_Accel_X, &prev_Accel_Y, &prev_Accel_Z, &vibration_percentage);
          // Apply moving average filter to vibration percentage
          MovingAverageFilter(&vibration_percentage, &smoothed_vibration, vibration_buffer, MOVING_AVERAGE_SIZE, &buffer_index);
//          uart_buf_len = sprintf(uart_buf, "Accel_X: %d, Accel_Y: %d, Accel_Z: %d\r\n", Accel_X, Accel_Y, Accel_Z);
//          HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);
//          FloatToString(float_buf, smoothed_vibration, 2);
//          uart_buf_len = sprintf(uart_buf, "Motor_Vibration: %s%%\r\n", float_buf);
//          HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);
          uart_buf_len = sprintf(uart_buf, "--------------------------------------------\r\n");
          HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);

        } else {
          uart_buf_len = sprintf(uart_buf, "Error reading accelerometer data\r\n");
          HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, uart_buf_len, 100);
        }
}
//----------------------------------------------------------------------------------------------------
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */

//void Three_Phase_Motor(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2) {
//    char msg[64];
//    char float_str[20];
//    uint32_t phase_1, phase_2, phase_3, mains_sup;
//
//    // Read ADC values for the three phases
//    phase_1 = Read_ADC_Channel(hadc, ADC_CHANNEL_0);
//    phase_2 = Read_ADC_Channel(hadc, ADC_CHANNEL_1);
//    phase_3 = Read_ADC_Channel(hadc, ADC_CHANNEL_4);
//    mains_sup = Read_ADC_Channel(hadc, ADC_CHANNEL_8); //5 od
//
//    // Convert ADC values to mains voltage
//    float phase_1_voltage = Convert_ADC_to_MainsVoltage(phase_1);
//    float phase_2_voltage = Convert_ADC_to_MainsVoltage(phase_2);
//    float phase_3_voltage = Convert_ADC_to_MainsVoltage(phase_3);
//    float mains_voltage = Convert_ADC_to_MainsVoltage(mains_sup);
//
//    // Convert float values to strings
//    FloatToString(float_str, phase_1_voltage, 2);
//    sprintf(msg, "M2_Ph1= %s V\r\n", float_str);
//    UART_Print(huart1, msg);
//    UART_Print(huart2, msg);
//
//    FloatToString(float_str, phase_2_voltage, 2);
//    sprintf(msg, "M2_Ph2= %s V\r\n", float_str);
//    UART_Print(huart1, msg);
//    UART_Print(huart2, msg);
//
//    FloatToString(float_str, phase_3_voltage, 2);
//    sprintf(msg, "M2_Ph3= %s V\r\n", float_str);
//    UART_Print(huart1, msg);
//    UART_Print(huart2, msg);
//
//    FloatToString(float_str, mains_voltage, 2);
//    sprintf(msg, "Mains_Voltage= %s V\r\n", float_str);
//    UART_Print(huart1, msg);
//    UART_Print(huart2, msg);
//    sprintf(msg, "Status= Normal\r\n");
//      //UART_Print(huart1, msg);
//      UART_Print(huart2, msg);
//
//      if (mains_voltage < 200) {
//              sprintf(msg, "Alert= Low Voltage %s V\r\n", float_str);
//              UART_Print(huart1, msg);
//              UART_Print(huart2, msg);
//      }
//
//}



//void single_ph_motor(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2) {
//    // Read ADC values for motor voltage and current
//    motor_v = Read_ADC_Channel(hadc, ADC_CHANNEL_15);
//    motor_c = Read_ADC_Channel(hadc, ADC_CHANNEL_10);
//    float m_v = Convert_ADC_to_MainsVoltage(motor_v);
//    FloatToString(float_str, m_v, 2);
//    // Print motor voltage to both UARTs
//    sprintf(msg, "M1_Voltage= %s V\r\n", float_str); // Use %lu for uint32_t
//    UART_Print(huart1, msg);
//    UART_Print(huart2, msg);
//    //HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
//    if (m_v < 200) {
//               blink_enabled = 1; // Enable LED blinking
//               Timer_Start(); // Start timer for LED blinking
//           } else {
//               blink_enabled = 0; // Disable LED blinking
//               Timer_Stop(); // Stop timer if voltage is above 200V
//               HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET); // Ensure LED is off
//           }
//    float motor_current =Convert_ADC_to_Current(motor_c);
//    FloatToString(float_str, motor_current, 2);
//    // Print motor current to both UARTs
//    sprintf(msg, "mc= %sA\r\n", float_str);
//    //sprintf(msg, "M1_Current: %lu Amps\r\n", motor_c); // Use %lu for uint32_t
//    UART_Print(huart1, msg);
//    //UART_Print(huart2, msg);
//    sprintf(msg, "M1_Current= %sA\r\n", float_str);
//    UART_Print(huart2, msg);
//
//}


//void motors_temp(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2) {
//    char msg[64];
//
//    // Read ADC values for motor temperature and vibrations
//    uint32_t temp1 = Read_ADC_Channel(hadc, ADC_CHANNEL_11);
//    uint32_t temp2 = Read_ADC_Channel(hadc, ADC_CHANNEL_13);
//    uint32_t vibrations = Read_ADC_Channel(hadc, ADC_CHANNEL_12);
//
//    // Convert ADC values to temperature and vibration percentage
//    float temperature_1 = Convert_ADC_to_Temperature(temp1);
//    float temperature_2 = Convert_ADC_to_Temperature(temp2);
//    float vibration_percentage = Convert_ADC_to_Percentage(vibrations);
//
//    // Convert values to string for printing
//    char temperature_str1[20];
//    char temperature_str2[20];
//    char vibration_str[20];
//    FloatToString1(temperature_str1, temperature_1, 2);
//    FloatToString1(temperature_str2, temperature_2, 2);
//    FloatToString1(vibration_str, vibration_percentage, 2);
//
//    // Print motor temperatures
//    sprintf(msg, "M1_Temp= %s °C\r\n", temperature_str1);
//    UART_Print(huart1, msg);
//    UART_Print(huart2, msg);
//    sprintf(msg, "M2_Temp= %s °C\r\n", temperature_str2);
//    UART_Print(huart1, msg);
//    UART_Print(huart2, msg);
//
//    // Print motor vibrations
//    sprintf(msg, "Motor1_Vibrations= %s%%\r\n", vibration_str);
//    UART_Print(huart1, msg);
//    UART_Print(huart2, msg);
//
//    // Check and print temperature conditions for temperature_1
//    if (temperature_1 >= 0 && temperature_1 <= 60) {
//        sprintf(msg, "Temperature Status: Normal Range\r\n");
//    } else if (temperature_1 > 60 && temperature_1 <= 70) {
//        sprintf(msg, "Temperature Status: Poor Ventilation\r\n");
//        assign_code_word(code_word, 5, '1');
//    } else if (temperature_1 > 70 && temperature_1 <= 80) {
//        sprintf(msg, "Temperature Status: Load on Motor\r\n");
//        assign_code_word(code_word, 5, '2');
//    } else if (temperature_1 > 80 && temperature_1 <= 90) {
//        sprintf(msg, "Temperature Status: Overload on Motor\r\n");
//        assign_code_word(code_word, 5, '3');
//    }
//    UART_Print(huart1, msg);
//    UART_Print(huart2, msg);
//
//    // Check and print vibration conditions
//    if (vibration_percentage >= 0 && vibration_percentage <= 10) {
//        sprintf(msg, "Vibration Status: Normal Range\r\n");
//    } else if (vibration_percentage > 10 && vibration_percentage <= 13) {
//        sprintf(msg, "Vibration Status: Imbalance Load\r\n");
//        assign_code_word(code_word, 6, '1');
//    } else if (vibration_percentage > 13 && vibration_percentage <= 15) {
//        sprintf(msg, "Vibration Status: Coupling or Faults\r\n");
//        assign_code_word(code_word, 6, '2');
//    } else if (vibration_percentage > 15 && vibration_percentage <= 18) {
//        sprintf(msg, "Vibration Status: Bearing Worn and Shaft Issue\r\n");
//        assign_code_word(code_word, 6, '3');
//    }
//    UART_Print(huart1, msg);
//    UART_Print(huart2, msg);
//
//    // Print the code_word via UART2
//    sprintf(msg, "code_word= %s\r\n", code_word);
//    UART_Print(huart2, msg);
//}



//void motors_temp(ADC_HandleTypeDef* hadc, UART_HandleTypeDef* huart1, UART_HandleTypeDef* huart2) {
//    // Read ADC values for motor voltage and current
//    temp1 = Read_ADC_Channel(hadc, ADC_CHANNEL_11);
//    temp2 = Read_ADC_Channel(hadc, ADC_CHANNEL_13);
//    vibrations= Read_ADC_Channel(hadc, ADC_CHANNEL_12);
//    temperature_1 = Convert_ADC_to_Temperature(temp1);
//    temperature_2= Convert_ADC_to_Temperature(temp2);
//    char temperature_str1[20];
//    char temperature_str2[20];
//    char vibration_str[20];
//        FloatToString1(temperature_str1, temperature_1, 2);
//        FloatToString1(temperature_str2, temperature_2, 2);
//        sprintf(msg, "M1_Temp= %s °C\r\n", temperature_str1); // Use %lu for uint32_t
//           UART_Print(huart1, msg);
//           UART_Print(huart2, msg);
//           //UART_Print(huart2, "MQ=135\r\n");
//           sprintf(msg, "M2_Temp= %s °C\r\n", temperature_str2); // Use %lu for uint32_t
//                  UART_Print(huart1, msg);
//                  UART_Print(huart2, msg);
//                  float vibration_percentage = Convert_ADC_to_Percentage(vibrations);
//                  FloatToString1(vibration_str, vibration_percentage, 2);
//                  sprintf(msg, "Motor1_Vibrations= %s%%\r\n", vibration_str);
//                  //sprintf(msg, "Motor1_Vibrations: %d\r\n", vibrations); // Use %lu for uint32_t
//                             UART_Print(huart1, msg);
//                             UART_Print(huart2, msg);
//}
