#ifndef __LCD_H
#define __LCD_H

#include "stm32g0xx_hal.h"
#include "main.h"


// Funções públicas
void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t col, uint8_t row);
void LCD_Print(char *str);
void LCD_DisplayWelcome(void);

#endif
