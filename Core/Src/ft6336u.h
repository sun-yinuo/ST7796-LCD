#ifndef FT6336U_H
#define FT6336U_H

#include "stm32l4xx_hal.h"
#include <stdint.h>


void FT6336_Init(I2C_HandleTypeDef *hi2c);
void FT6336_Reset(void);
uint8_t FT6336_Scan(uint16_t *x, uint16_t *y);


#endif





