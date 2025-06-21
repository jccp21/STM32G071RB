/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32g0xx_hal.h"

// --- Pinos dos botões ---
#define BUTTON_UP        GPIO_PIN_0
#define BUTTON_DOWN      GPIO_PIN_1
#define BUTTON_SCREEN    GPIO_PIN_6
#define BUTTON_GPIO_PORT GPIOA

// --- Buzzer e LED de alarme ---
#define BUZZER           GPIO_PIN_3
#define BUZZER_GPIO_PORT GPIOA
#define ALARM_LED        GPIO_PIN_4
#define ALARM_LED_GPIO_PORT GPIOA

// --- Saída de PWM (canal CH1 e CH1N) ---
#define PWM_OUTPUT       GPIO_PIN_8   // TIM1_CH1 (PA8)
#define PWM_COMPLEMENTAR GPIO_PIN_7   // TIM1_CH1N (PA7)

// --- Pinos do LCD 16x2 (modo 4 bits) ---
#define LCD_EN_Pin       GPIO_PIN_4
#define LCD_EN_GPIO_Port GPIOC
#define LCD_RS_Pin       GPIO_PIN_5
#define LCD_RS_GPIO_Port GPIOC
#define LCD_D4_Pin       GPIO_PIN_10
#define LCD_D4_GPIO_Port GPIOA
#define LCD_D5_Pin       GPIO_PIN_3
#define LCD_D5_GPIO_Port GPIOB
#define LCD_D6_Pin       GPIO_PIN_5
#define LCD_D6_GPIO_Port GPIOB
#define LCD_D7_Pin       GPIO_PIN_4
#define LCD_D7_GPIO_Port GPIOB

// --- Pinos SWD ---
#define TMS_Pin          GPIO_PIN_13
#define TMS_GPIO_Port    GPIOA
#define TCK_Pin          GPIO_PIN_14
#define TCK_GPIO_Port    GPIOA

// --- Prototipação ---
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
