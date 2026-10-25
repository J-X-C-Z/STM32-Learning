/* USER CODE BEGIN Header */
/**
  * @file    main.c
  * @brief   Homework 2: EXTI-driven LED state machine.
  */
/* USER CODE END Header */
#include "main.h"

typedef enum
{
  LED_OFF = 0,
  LED_ON,
  LED_BREATHING
} LedState;

static volatile uint8_t button_pressed = 0U;
static volatile uint32_t last_button_irq = 0U;
static LedState led_state = LED_OFF;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

static void SetLeds(GPIO_PinState level)
{
  HAL_GPIO_WritePin(GPIOA, RED_LED_Pin | GREEN_LED_Pin | BLUE_LED_Pin, level);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  uint32_t last_breath_step = HAL_GetTick();
  uint8_t brightness = 0U;
  int8_t direction = 1;

  while (1)
  {
    uint32_t now = HAL_GetTick();
    __disable_irq();
    uint8_t pressed = button_pressed;
    button_pressed = 0U;
    __enable_irq();
    if (pressed != 0U)
    {
      led_state = (LedState)((led_state + 1U) % 3U);
      if (led_state == LED_OFF)
      {
        SetLeds(GPIO_PIN_SET);
      }
      else if (led_state == LED_ON)
      {
        SetLeds(GPIO_PIN_RESET);
      }
      if (led_state == LED_BREATHING) brightness = 0U;
      last_breath_step = now;
    }

    if (led_state == LED_BREATHING)
    {
      if ((uint32_t)(now - last_breath_step) >= 20U)
      {
        last_breath_step = now;
        if (brightness == 0U)
        {
          direction = 1;
        }
        else if (brightness == 20U)
        {
          direction = -1;
        }
        brightness = (uint8_t)(brightness + direction);
      }
      SetLeds(((now % 20U) < brightness) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  uint32_t now = HAL_GetTick();
  if ((GPIO_Pin == BUTTON_Pin) && ((uint32_t)(now - last_button_irq) >= 200U))
  {
    last_button_irq = now;
    button_pressed = 1U;
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef osc = {0};
  RCC_ClkInitTypeDef clk = {0};
  osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  osc.HSIState = RCC_HSI_ON;
  osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  osc.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();
  clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV1;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef gpio = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, BUTTON_GND_Pin, GPIO_PIN_RESET);
  gpio.Pin = BUTTON_GND_Pin;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &gpio);

  SetLeds(GPIO_PIN_SET);
  gpio.Pin = RED_LED_Pin | GREEN_LED_Pin | BLUE_LED_Pin;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = BUTTON_Pin;
  gpio.Mode = GPIO_MODE_IT_FALLING;
  gpio.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &gpio);
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2U, 0U);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1) { }
}
