/**
 * @note: Pin Out OV7670
 * ************************************************************************
    * OV7670 Pin Out
    * -----------------------------------------------------------------------
    * PB9 -> I2C SDA
    * PB7 -> VSYNC
    * PB6 -> SCL
    * PD3 -> D5
    * PC11 -> D4
    * PA10 -> D1
    * PA9 -> D0
    * PA8 -> XLK
    * PC9 -> D3
    * PC8 -> D2
    * PA6 -> PLK
    * PA5 -> RET
    * PA4 -> HS
    * PE6 -> D7
    * PE6 -> D6
 * ************************************************************************
 */
#include "main.h"
#include "ov7670.h"
#include <stdio.h>

extern char _flash_start, _flash_end;
extern char _ram_start, _r2am_end;

void check_memory_usage() {
    size_t flash_total = (size_t)(&_flash_end - &_flash_start);
    size_t ram_total = (size_t)(&_ram_end - &_ram_start);

    printf("Flash: %lu bytes total\n", flash_total);
    printf("RAM: %lu bytes total\n", ram_total);
}

#define FRAME_BUFFER_SIZE (120 * 120 * 3) // 120x120, RGB
uint8_t frameBuffer[FRAME_BUFFER_SIZE];

I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim1;
DCMI_HandleTypeDef hdcmi;
DMA_HandleTypeDef hdma_dcmi;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_DCMI_Init(void);

int main(void) {
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_DMA_Init();
    MX_I2C1_Init();
    MX_TIM1_Init();
    MX_DCMI_Init();
    printf("Hello World");
    // Start TIM1 to provide XCLK for OV7670
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

    // Initialize OV7670 camera
    OV7670_Init(&hi2c1, OV7670_QVGA);

    while (1) {
        // Start capturing frame to buffer
        HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)frameBuffer, FRAME_BUFFER_SIZE / 4);

        // Simulate streaming by processing or sending the buffer here
        // For example, you can send frameBuffer via UART or store it

        HAL_DCMI_Stop(&hdcmi); // Stop DCMI after frame capture
        check_memory_usage();

    }
}

void SystemClock_Config(void) {
    // Configure the system clock to 64 MHz
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 8;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // Configure GPIO for OV7670 and other peripherals
    // Example: GPIO_InitStruct.Pin = GPIO_PIN_X; GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
}

static void MX_DMA_Init(void) {
    __HAL_RCC_DMA1_CLK_ENABLE();

    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
}

static void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00303D5B;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    HAL_I2C_Init(&hi2c1);
}

static void MX_TIM1_Init(void) {
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 7;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    HAL_TIM_PWM_Init(&htim1);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 4;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
}

static void MX_DCMI_Init(void) {
    hdcmi.Instance = DCMI;
    hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
    hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
    hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
    hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_HIGH;
    hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
    HAL_DCMI_Init(&hdcmi);
}
