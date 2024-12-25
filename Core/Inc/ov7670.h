#ifndef OV7670_H
#define OV7670_H

#include "stm32h7xx_hal.h"

// Supported resolutions
typedef enum {
    OV7670_QVGA,  // 320x240
    OV7670_VGA    // 640x480
} OV7670_Resolution;

// Function prototypes
void OV7670_Init(I2C_HandleTypeDef *hi2c, OV7670_Resolution resolution);
void OV7670_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t value);
uint8_t OV7670_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg);

#endif // OV7670_H
