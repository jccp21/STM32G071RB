#ifndef __LCD_H
#define __LCD_H

#include "stm32g0xx_hal.h"

// Configurações dos pinos
#define LCD_RS_GPIO_Port GPIOC
#define LCD_RS_Pin       GPIO_PIN_5

#define LCD_EN_GPIO_Port GPIOC
#define LCD_EN_Pin       GPIO_PIN_4

#define LCD_D4_GPIO_Port GPIOA
#define LCD_D4_Pin       GPIO_PIN_10

#define LCD_D5_GPIO_Port GPIOB
#define LCD_D5_Pin       GPIO_PIN_3

#define LCD_D6_GPIO_Port GPIOB
#define LCD_D6_Pin       GPIO_PIN_5

#define LCD_D7_GPIO_Port GPIOB
#define LCD_D7_Pin       GPIO_PIN_4

// Funções públicas
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t col, uint8_t row);
void LCD_Print(char* str);
void LCD_DisplayWelcome(void);

#endif
