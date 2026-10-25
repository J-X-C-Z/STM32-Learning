/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
   HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  uint32_t now = HAL_GetTick();
  uint32_t last_blink_tick = now;
  uint32_t last_chase_tick = now;
  uint32_t button_change_tick = now;
  uint32_t press_start_tick = 0;
  GPIO_PinState button_candidate = GPIO_PIN_SET;
  GPIO_PinState button_stable = GPIO_PIN_SET;
  uint8_t chasing = 0;
  uint8_t chase_index = 0;
  const uint16_t led_pins[] = {RED_LED_Pin, GREEN_LED_Pin, BLUE_LED_Pin};
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    now = HAL_GetTick();  // 获取系统启动以来的毫秒数，用于非阻塞计时

    GPIO_PinState button_raw = HAL_GPIO_ReadPin(BUTTON_GPIO_Port, BUTTON_Pin);  // 读取按键当前的原始电平
    if (button_raw != button_candidate)  // 原始读数变化时，重新开始按键消抖计时
    {
      button_candidate = button_raw;  // 原始电平变化，更新候选状态
      button_change_tick = now;  // 记录这次变化发生的时间，开始消抖计时
    }
    if ((button_candidate != button_stable) && (now - button_change_tick >= 20U))  // 候选状态与稳定状态不同且已持续 20 ms
    {
      button_stable = button_candidate;  // 候选状态持续至少 20 ms，确认为稳定状态
      if (button_stable == GPIO_PIN_RESET)  // RESET 表示按键按下（输入使用上拉）
      {
        press_start_tick = now;  // 按键确认按下，记录按下起始时间
      }
      else if (chasing != 0U)  // 只有流水灯运行中，松开按键才停止该模式
      {
        chasing = 0U;  // 按键确认松开，停止流水灯模式
        HAL_GPIO_WritePin(GPIOA, RED_LED_Pin | GREEN_LED_Pin | BLUE_LED_Pin, GPIO_PIN_RESET);  // 将三个 LED 引脚都设为低电平
        last_blink_tick = now;  // 重置普通闪烁计时，避免松开后立即翻转
      }
    }

    if ((button_stable == GPIO_PIN_RESET) && (chasing == 0U) &&  // 按键仍按下且流水灯尚未启动
        (now - press_start_tick >= 500U))  // 本次按下持续时间达到 500 ms
    {
      chasing = 1U;  // 按键持续按下达到 500 ms，进入流水灯模式
      chase_index = 0U;  // 将流水灯索引设为 0（RED_LED_Pin）
      last_chase_tick = now;  // 记录流水灯启动时间
      HAL_GPIO_WritePin(GPIOA, RED_LED_Pin | GREEN_LED_Pin | BLUE_LED_Pin, GPIO_PIN_RESET);  // 三个 LED 引脚先全部设为低电平
      HAL_GPIO_WritePin(GPIOA, led_pins[chase_index], GPIO_PIN_SET);  // 将当前索引对应的 LED 引脚设为高电平
    }

    if (chasing != 0U)  // 流水灯模式下只处理流水灯计时
    {
      if (now - last_chase_tick >= 500U)  // 距离上次切换已达到 500 ms
      {
        last_chase_tick = now;  // 更新上次切换时间，每 500 ms 切换一次
        HAL_GPIO_WritePin(GPIOA, RED_LED_Pin | GREEN_LED_Pin | BLUE_LED_Pin, GPIO_PIN_SET);  // 三个 LED 引脚先全部设为高电平
        chase_index = (uint8_t)((chase_index + 1U) % 3U);  // 索引加一，并用取模让 0、1、2 循环
        HAL_GPIO_WritePin(GPIOA, led_pins[chase_index], GPIO_PIN_RESET);  // 将新索引对应的 LED 引脚设为低电平
      }
    }
    else if (now - last_blink_tick >= 1000U)  // 非流水灯模式下，每 1000 ms 执行一次普通闪烁
    {
      last_blink_tick = now;  // 更新普通闪烁的上次翻转时间
      HAL_GPIO_TogglePin(GPIOA, RED_LED_Pin | GREEN_LED_Pin | BLUE_LED_Pin);  // 每隔 1000 ms 翻转三个 LED 引脚的电平
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* PA9 supplies the button's low level; the button connects it to PA10. */
  HAL_GPIO_WritePin(GPIOA, BUTTON_GND_Pin, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = BUTTON_GND_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RED_LED_Pin | GREEN_LED_Pin | BLUE_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA3 PA5 PA7 */
  GPIO_InitStruct.Pin = RED_LED_Pin | GREEN_LED_Pin | BLUE_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BUTTON_Pin */
  GPIO_InitStruct.Pin = BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BUTTON_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
#ifdef USE_FULL_ASSERT
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
