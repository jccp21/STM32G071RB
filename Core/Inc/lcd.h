#ifndef __LCD_H
#define __LCD_H

#include "stm32g0xx_hal.h"

// Define os pinos do LCD
#define LCD_RS_PORT GPIOC
#define LCD_RS_PIN  GPIO_PIN_5

#define LCD_EN_PORT GPIOC
#define LCD_EN_PIN  GPIO_PIN_4

#define LCD_D4_PORT GPIOA
#define LCD_D4_PIN  GPIO_PIN_10

#define LCD_D5_PORT GPIOB
#define LCD_D5_PIN  GPIO_PIN_3

#define LCD_D6_PORT GPIOB
#define LCD_D6_PIN  GPIO_PIN_5

#define LCD_D7_PORT GPIOB
#define LCD_D7_PIN  GPIO_PIN_4

// Funções públicas
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(char *str);
void LCD_DisplayWelcome(void);

#endif
