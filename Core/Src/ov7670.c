#include "ov7670.h"

#define OV7670_I2C_ADDRESS 0x42

void OV7670_Init(I2C_HandleTypeDef *hi2c, OV7670_Resolution resolution) {
    // Reset camera
    OV7670_WriteRegister(hi2c, 0x12, 0x80);
    HAL_Delay(100);

    // Configure resolution
    if (resolution == OV7670_QVGA) {
        OV7670_WriteRegister(hi2c, 0x12, 0x14); // QVGA mode, RGB565
    } else if (resolution == OV7670_VGA) {
        OV7670_WriteRegister(hi2c, 0x12, 0x00); // VGA mode, RGB565
    }

    // Configure for 120x120 resolution using scaler and cropping
    OV7670_WriteRegister(hi2c, 0x72, 0x22); // Horizontal scaling factor (approx. 1/3)
    OV7670_WriteRegister(hi2c, 0x73, 0x22); // Vertical scaling factor (1/2)
    OV7670_WriteRegister(hi2c, 0x17, 0x3F); // Horizontal start
    OV7670_WriteRegister(hi2c, 0x18, 0xC0); // Horizontal stop
    OV7670_WriteRegister(hi2c, 0x19, 0x03); // Vertical start
    OV7670_WriteRegister(hi2c, 0x1A, 0x7B); // Vertical stop

    // Additional settings for color format, gamma, etc.
    OV7670_WriteRegister(hi2c, 0x3A, 0x04); // Set RGB565 format
    OV7670_WriteRegister(hi2c, 0x40, 0xD0); // RGB scaling
    OV7670_WriteRegister(hi2c, 0x11, 0x01); // Configure clock prescaler
}

void OV7670_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg, uint8_t value) {
    HAL_I2C_Mem_Write(hi2c, OV7670_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}

uint8_t OV7670_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg) {
    uint8_t value;
    HAL_I2C_Mem_Read(hi2c, OV7670_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
    return value;
}

void Convert_Frame_RGB565_to_RGB888(uint16_t *frameBuffer565, uint8_t *frameBuffer888, size_t width, size_t height) {
    for (size_t i = 0; i < width * height; i++) {
        uint8_t r = (frameBuffer565[i] >> 8) & 0xF8; // R: 5 MSBs -> 8 bits
        uint8_t g = (frameBuffer565[i] >> 3) & 0xFC; // G: 6 bits -> 8 bits
        uint8_t b = (frameBuffer565[i] << 3) & 0xF8; // B: 5 LSBs -> 8 bits
        frameBuffer888[i * 3] = r;
        frameBuffer888[i * 3 + 1] = g;
        frameBuffer888[i * 3 + 2] = b;
    }
}
