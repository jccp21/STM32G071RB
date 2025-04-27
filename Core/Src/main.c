#include "stm32g0xx_hal.h"
#include "lcd.h" // Nosso driver LCD em 4 bits
#include "stdio.h"
#include "stdint.h"

// --- Definições de periféricos ---
TIM_HandleTypeDef htim1;
ADC_HandleTypeDef hadc1;
GPIO_InitTypeDef GPIO_InitStruct = {0};

// --- Definições de pinos ---
#define BUTTON_UP        GPIO_PIN_0
#define BUTTON_DOWN      GPIO_PIN_1
#define BUTTON_SCREEN    GPIO_PIN_5
#define BUTTON_GPIO_PORT      GPIOA

#define BUZZER           GPIO_PIN_3
#define BUZZER_GPIO_PORT      GPIOA

#define ALARM_LED       GPIO_PIN_4
#define ALARM_LED_GPIO_PORT  GPIOA

#define PWM_OUTPUT      GPIO_PIN_8

// --- Variáveis globais ---
uint16_t duty_cycle = 0;
uint8_t current_screen = 0;
uint8_t temp_alert_active = 0;
float temperature = 0.0;
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
    TIM1_Init();
    ADC1_Init();
    LCD_Init();

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
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
        // Leitura da temperatura
        if (HAL_GetTick() - last_temp_read >= 100)
        {
            last_temp_read = HAL_GetTick();
            ReadTemperature();
        }

        // Controle de alarme de temperatura
        if (temperature >= 50.0)
        {
            temp_alert_active = 1;
        }
        else
        {
            temp_alert_active = 0;
            HAL_GPIO_WritePin(ALARM_LED_GPIO_PORT, ALARM_LED, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER, GPIO_PIN_RESET);
        }

        // Alarme (pisca LED e buzzer)
        if (temp_alert_active && (HAL_GetTick() - last_alarm_toggle >= 100))
        {
            last_alarm_toggle = HAL_GetTick();
            HAL_GPIO_TogglePin(ALARM_LED_GPIO_PORT, ALARM_LED);
            HAL_GPIO_TogglePin(BUZZER_GPIO_PORT, BUZZER);
        }

        // Atualiza o display
        if (HAL_GetTick() - last_display_update >= 200)
        {
            last_display_update = HAL_GetTick();
            UpdateDisplay();
        }

        // Controle de botões
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

        if (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_SCREEN) == GPIO_PIN_RESET)
        {
            current_screen ^= 1;
            Buzzer_Beep(50);
            HAL_Delay(300);
            LCD_Clear();
        }

        // Controle do timer regressivo
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
    // Deixe configurado pelo CubeMX.
}

void GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PWM Output (PA8)
    GPIO_InitStruct.Pin = PWM_OUTPUT;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

   // Configura botões como entrada
GPIO_InitStruct.Pin = BUTTON_UP | BUTTON_DOWN | BUTTON_SCREEN;
GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
GPIO_InitStruct.Pull = GPIO_PULLUP;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

// Configura buzzer e LED como saída
GPIO_InitStruct.Pin = BUZZER | ALARM_LED;
GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
GPIO_InitStruct.Pull = GPIO_NOPULL;
HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

void TIM1_Init(void)
{
    __HAL_RCC_TIM1_CLK_ENABLE();

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 1;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 3199;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    HAL_TIM_PWM_Init(&htim1);

    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
}

void ADC1_Init(void)
{
    // Habilita o clock do ADC
    __HAL_RCC_ADC_CLK_ENABLE();

    // --- Habilita o clock do sistema para o ADC e configura o clock do ADC ---
    // (opcional se quiser garantir o melhor funcionamento)

    // Coloca o ADC em modo de inicialização
    if ((ADC1->CR & ADC_CR_ADEN) != 0)
    {
        ADC1->CR |= ADC_CR_ADDIS; // Desliga o ADC se estiver ligado
        while ((ADC1->CR & ADC_CR_ADEN) != 0); // Espera desligar
    }

    // Ativa o clock interno do ADC (14 MHz)
    ADC1_COMMON->CCR|= ADC_CCR_VREFEN; // Ativa referência VREFINT (se quiser usar futuramente)

    // Faz calibração
    ADC1->CR |= ADC_CR_ADCAL;
    while ((ADC1->CR & ADC_CR_ADCAL) != 0); // Espera a calibração terminar

    // --- Configuração da estrutura HAL ---
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4; // Divide PCLK
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

    HAL_ADC_Init(&hadc1);
}


void ReadTemperature(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = ADC_CHANNEL_2; // Conecte o LM35 no PA2
    sConfig.Rank = ADC_REGULAR_RANK_1;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint32_t adc_value = HAL_ADC_GetValue(&hadc1);

    // Convertendo para temperatura em °C
    temperature = ((adc_value * 3.3) / 4095.0) * 100.0;
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
        sprintf(buffer, "Temp: %.1f C", temperature);
        LCD_Print(buffer);
    }
}

void Buzzer_Beep(uint16_t duration_ms)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER, GPIO_PIN_SET);
    HAL_Delay(duration_ms);
    HAL_GPIO_WritePin(BUZZER_GPIO_PORT, BUZZER, GPIO_PIN_RESET);
}
