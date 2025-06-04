#include "lcd.h"
#include "string.h"
#include "stdint.h"
#include "stm32g0xx_hal.h"
#include "main.h"

static void LCD_EnablePulse(void);
static void LCD_Send4Bits(uint8_t data);
static void LCD_SendCommand(uint8_t cmd);
static void LCD_SendData(uint8_t data);

static void LCD_EnablePulse(void)
{
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
}

static void LCD_Send4Bits(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, (data >> 0) & 0x01);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, (data >> 1) & 0x01);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, (data >> 2) & 0x01);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, (data >> 3) & 0x01);
    LCD_EnablePulse();
}

static void LCD_SendCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    LCD_Send4Bits(cmd >> 4);
    LCD_Send4Bits(cmd & 0x0F);
    HAL_Delay(2);
}

static void LCD_SendData(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
    LCD_Send4Bits(data >> 4);
    LCD_Send4Bits(data & 0x0F);
    HAL_Delay(1);
}

void LCD_Init(void)
{
    HAL_Delay(50); // Espera após power-up
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

    // Inicialização modo 4 bits
    LCD_Send4Bits(0x03);
    HAL_Delay(5);
    LCD_Send4Bits(0x03);
    HAL_Delay(5);
    LCD_Send4Bits(0x03);
    HAL_Delay(5);
    LCD_Send4Bits(0x02); // 4 bits
    HAL_Delay(1);

    LCD_SendCommand(0x28); // 2 linhas, 4 bits, 5x8 dots
    LCD_SendCommand(0x0C); // Display ON, cursor OFF
    LCD_SendCommand(0x06); // Incremento automático
    LCD_SendCommand(0x01); // Clear
    HAL_Delay(2);
}

void LCD_Clear(void)
{
    LCD_SendCommand(0x01);
    HAL_Delay(2);
}

void LCD_SetCursor(uint8_t col, uint8_t row)
{
    uint8_t addr = (row == 0) ? 0x00 : 0x40;
    addr += col;
    LCD_SendCommand(0x80 | addr);
}

void LCD_Print(char* str)
{
    while (*str)
    {
        LCD_SendData(*str++);
    }
}

void LCD_DisplayWelcome(void)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("  Sistema Pronto ");
    LCD_SetCursor(0, 1);
    LCD_Print("   STM32 + LCD   ");
}
