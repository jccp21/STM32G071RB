#include "stm32g0xx_hal.h"
#include "lcd.h" // Nosso driver LCD em 4 bits
#include "stdio.h"
#include "stdint.h"
#include "main.h"

// --- Definições de periféricos ---
TIM_HandleTypeDef htim1;
ADC_HandleTypeDef hadc1;
GPIO_InitTypeDef GPIO_InitStruct = {0};

// --- Variáveis globais ---
uint16_t duty_cycle = 0; // Duty cycle do PWM (0 a 100%)
uint8_t current_screen = 0; // Tela atual do display (0 ou 1)
uint8_t temp_alert_active = 0; // Flag de alerta de temperatura
float temperature = 0.0; // Valor lido do LM35 em °C
uint16_t countdown_timer = 60; // Timer regressivo em segundos

// --- Protótipos ---
void SystemClock_Config(void);
void GPIO_Init(void);
void TIM1_Init(void);
void ADC1_Init(void);
void Buzzer_Beep(uint16_t duration_ms);
void ReadTemperature(void);
void UpdateDisplay(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    TIM1_Init(); // Inicializa PWM com dead-time (canal CH1 e CH1N)
    ADC1_Init();
    LCD_Init();

    // Inicia PWM no canal 1 e seu complementar (CH1N)
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

    LCD_DisplayWelcome();
    Buzzer_Beep(200);
    HAL_Delay(2000);
    LCD_Clear();

    uint32_t last_temp_read = 0;
    uint32_t last_timer_tick = 0;
    uint32_t last_display_update = 0;
    uint32_t last_alarm_toggle = 0;

    while (1)
    {
        // Leitura periódica da temperatura (a cada 100ms)
        if (HAL_GetTick() - last_temp_read >= 100)
        {
            last_temp_read = HAL_GetTick();
            ReadTemperature();
        }

        // Lógica de alarme por temperatura
        if (temperature >= 30.0)
        {
            temp_alert_active = 1;
        }
        else
        {
            temp_alert_active = 0;
            HAL_GPIO_WritePin(ALARM_LED_GPIO_PORT, ALARM_LED, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER, GPIO_PIN_RESET);
        }

        // Pisca LED e buzzer a cada 100ms se em alarme
        if (temp_alert_active && (HAL_GetTick() - last_alarm_toggle >= 100))
        {
            last_alarm_toggle = HAL_GetTick();
            HAL_GPIO_TogglePin(ALARM_LED_GPIO_PORT, ALARM_LED);
            HAL_GPIO_TogglePin(BUZZER_GPIO_PORT, BUZZER);
        }

        // Atualização do display a cada 200ms
        if (HAL_GetTick() - last_display_update >= 200)
        {
            last_display_update = HAL_GetTick();
            UpdateDisplay();
        }

        // Botão UP: aumenta duty cycle em 5%
        if (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_UP) == GPIO_PIN_RESET)
        {
            if (duty_cycle < 100)
            {
                duty_cycle += 5;
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (duty_cycle * (htim1.Init.Period + 1)) / 100);
                Buzzer_Beep(50);
                HAL_Delay(200);
            }
        }

        // Botão DOWN: reduz duty cycle em 5%
        if (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_DOWN) == GPIO_PIN_RESET)
        {
            if (duty_cycle > 0)
            {
                duty_cycle -= 5;
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (duty_cycle * (htim1.Init.Period + 1)) / 100);
                Buzzer_Beep(50);
                HAL_Delay(200);
            }
        }

        // Botão SCREEN: alterna entre telas (PWM/tempo e temperatura)
        if (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_SCREEN) == GPIO_PIN_RESET)
        {
            current_screen ^= 1;
            Buzzer_Beep(50);
            HAL_Delay(300);
            LCD_Clear();
        }

        // Redução do timer regressivo se duty > 0
        if (duty_cycle > 0 && (HAL_GetTick() - last_timer_tick >= 1000))
        {
            last_timer_tick = HAL_GetTick();
            if (countdown_timer > 0)
                countdown_timer--;
            else
            {
                duty_cycle = 0;
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
            }
        }
    }
}

// --- Funções auxiliares ---

void SystemClock_Config(void)
{
    // Configuração feita via STM32CubeMX
}

void GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // LCD (RS, EN, D4–D7)
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // PWM: PA8 (CH1), PA7 (CH1N)
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Botões com pull-up: PA0, PA1, PA6
    GPIO_InitStruct.Pin = BUTTON_UP | BUTTON_DOWN | BUTTON_SCREEN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Buzzer e LED: PA3 e PA4
    GPIO_InitStruct.Pin = BUZZER | ALARM_LED;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Entrada analógica do LM35: PA2
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void TIM1_Init(void)
{
    __HAL_RCC_TIM1_CLK_ENABLE();

    // Configuração para gerar 3 MHz com 8 MHz de clock: PSC = 0, ARR = 2
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 2;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim1);

    // Configuração do canal 1
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 1; // 50% duty inicialmente
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);

    // Configuração do dead time
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 2; // 1 us @ 8 MHz (aproximadamente)
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig);
}

void ADC1_Init(void)
{
    __HAL_RCC_ADC_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        while (1);
    }
}

void ReadTemperature(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = ADC_CHANNEL_2; // LM35 conectado no PA2
    sConfig.Rank = ADC_REGULAR_RANK_1;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint32_t adc_value = HAL_ADC_GetValue(&hadc1);

    temperature = (adc_value * 330.0) / 4095.0; // Conversão para ºC
}

void UpdateDisplay(void)
{
    char buffer[32];

    if (current_screen == 0)
    {
        LCD_SetCursor(0, 0);
        snprintf(buffer, sizeof(buffer), "PWM:%d%% T:%ds", duty_cycle, countdown_timer);
        LCD_Print(buffer);
    }
    else
    {
        LCD_SetCursor(0, 0);
        sprintf(buffer, "Temp: %.1f ", temperature);
        LCD_Print(buffer);
    }
}

void Buzzer_Beep(uint16_t duration_ms)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER, GPIO_PIN_SET);
    HAL_Delay(duration_ms);
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER, GPIO_PIN_RESET);
}
