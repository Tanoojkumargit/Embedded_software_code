#include "timer.h"

TIM_HandleTypeDef htim2; // Replace with your timer handler

void Timer_Init(void) {
    // Initialize TIM2
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = (uint32_t)(SystemCoreClock / 1000) - 1; // Timer ticks every 1ms
    htim2.Init.Period = 300; // Interrupt every 300ms
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);
}

void Timer_Start(void) {
    HAL_TIM_Base_Start_IT(&htim2); // Start timer with interrupt
}

void Timer_Stop(void) {
    HAL_TIM_Base_Stop_IT(&htim2); // Stop timer and interrupt
}

// TIM2 interrupt handler
void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim2);
}

// Timer callback function
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
//    if (htim->Instance == TIM2) {
//        // Toggle LED or perform blinking operation here
//        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // Example: Toggling GPIOA Pin 5 (LED)
//    }
//}
