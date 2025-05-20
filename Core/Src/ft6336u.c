#include "ft6336u.h"
#include "main.h"
#include "stm32l4xx_hal.h"
#include <stdio.h>

static I2C_HandleTypeDef *hi2c3;

#define FT6336_ADDR 0x70

uint16_t touch_x = 0, touch_y = 0;

void FT6336_Reset(void)
{
    HAL_GPIO_WritePin(TOUCH_RST_GPIO_Port, TOUCH_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(TOUCH_RST_GPIO_Port, TOUCH_RST_Pin, GPIO_PIN_SET); // 修正为 SET
    HAL_Delay(100);
}

static uint8_t FT6336_ReadReg(uint8_t reg)
{
    uint8_t val = 0;
    if (HAL_I2C_Mem_Read(hi2c3, FT6336_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &val, 1, 100) != HAL_OK) {
        printf("I2C Read Fail: reg=0x%02X\r\n", reg);
    }
    return val;
}

void FT6336_Init(I2C_HandleTypeDef *hi2c)
{
    hi2c3 = hi2c;
    FT6336_Reset();
    FT6336_ReadReg(0xA6); // 读取厂商ID进行确认
    FT6336_ReadReg(0xA8); // 读取芯片ID进行确认
}

uint8_t FT6336_Scan(uint16_t *x, uint16_t *y)
{

    // 保持芯片处于唤醒模式
    uint8_t mode = 0x00;
    HAL_I2C_Mem_Write(hi2c3, FT6336_ADDR, 0xA4, I2C_MEMADD_SIZE_8BIT, &mode, 1, 100);

    uint8_t buf[4];
    uint8_t tp = FT6336_ReadReg(0x02); // 触点数量

    if (tp > 0 && tp < 6) {
        if (HAL_I2C_Mem_Read(hi2c3, FT6336_ADDR, 0x03, I2C_MEMADD_SIZE_8BIT, buf, 4, 100) != HAL_OK) {
            printf("Touch Data Read Fail\r\n");
            return 0;
        }

        *x = ((buf[0] & 0x0F) << 8) | buf[1];
        *y = ((buf[2] & 0x0F) << 8) | buf[3];

        printf("Touch: x=%d, y=%d\r\n", *x, *y);
        return 1;
    } else {
        return 0;
    }
}