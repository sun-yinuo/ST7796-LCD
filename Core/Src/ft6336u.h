#ifndef FT6336U_H
#define FT6336U_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

// I2C地址 (7-bit 地址左移1位)
#define FT6336_ADDR (0x38 << 1)

// 寄存器定义
#define FT6336_DEVICE_MODE    0x00
#define FT6336_GEST_ID        0x01
#define FT6336_TD_STATUS      0x02
#define FT6336_TOUCH1_XH      0x03
#define FT6336_TOUCH1_XL      0x04
#define FT6336_TOUCH1_YH      0x05
#define FT6336_TOUCH1_YL      0x06
#define FT6336_ID_G_LIB_VER   0xA1
#define FT6336_ID_G_MODE      0xA4
#define FT6336_ID_G_THGROUP   0x80
#define FT6336_ID_G_PERIODACTIVE 0x88
#define FT6336_ID_G_AUTO_CLB  0xA0
#define FT6336_ID_G_FIRMID    0xA6
#define FT6336_ID_G_CHIPID    0xA3
#define FT6336_ID_G_RELEASE   0xAF

// API函数声明
void FT6336_Init(I2C_HandleTypeDef *hi2c);
void FT6336_Reset(void);
uint8_t FT6336_Scan(uint16_t *x, uint16_t *y);

extern uint16_t touch_x, touch_y;


#endif





