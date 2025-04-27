#include "lcd.h"
#include "stm32g0xx_hal.h"

// Funções internas
void LCD_SendNibble(uint8_t nibble);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_EnablePulse(void);

void LCD_Init(void)
{
    // Inicializa os pinos
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure todos como saída
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    GPIO_InitStruct.Pin = LCD_RS_PIN;
    HAL_GPIO_Init(LCD_RS_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_EN_PIN;
    HAL_GPIO_Init(LCD_EN_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_D4_PIN;
    HAL_GPIO_Init(LCD_D4_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_D5_PIN;
    HAL_GPIO_Init(LCD_D5_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_D6_PIN;
    HAL_GPIO_Init(LCD_D6_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LCD_D7_PIN;
    HAL_GPIO_Init(LCD_D7_PORT, &GPIO_InitStruct);

    // Sequência de inicialização do LCD
    HAL_Delay(40);
    LCD_SendNibble(0x03);
    HAL_Delay(5);
    LCD_SendNibble(0x03);
    HAL_Delay(1);
    LCD_SendNibble(0x03);
    HAL_Delay(1);
    LCD_SendNibble(0x02); // Modo 4 bits

    LCD_SendCommand(0x28); // 2 linhas, 5x8 pontos
    LCD_SendCommand(0x0C); // Display ON, cursor OFF
    LCD_SendCommand(0x06); // Incrementa cursor
    LCD_Clear();
}

void LCD_Clear(void)
{
    LCD_SendCommand(0x01);
    HAL_Delay(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t address[] = {0x00, 0x40}; // Linha 1 e linha 2
    LCD_SendCommand(0x80 | (address[row] + col));
}

void LCD_Print(char *str)
{
    while (*str)
    {
        LCD_SendData((uint8_t)(*str));
        str++;
    }
}

void LCD_DisplayWelcome(void)
{
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Sistema Iniciado");
    LCD_SetCursor(1, 0);
    LCD_Print("Aguardando...");
}

// --- Internas ---
void LCD_SendCommand(uint8_t cmd)
{
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_RESET);
    LCD_SendNibble(cmd >> 4);
    LCD_SendNibble(cmd & 0x0F);
    HAL_Delay(1);
}

void LCD_SendData(uint8_t data)
{
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, GPIO_PIN_SET);
    LCD_SendNibble(data >> 4);
    LCD_SendNibble(data & 0x0F);
    HAL_Delay(1);
}

void LCD_SendNibble(uint8_t nibble)
{
    HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    LCD_EnablePulse();
}

void LCD_EnablePulse(void)
{
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
}
