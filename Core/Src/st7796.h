

#ifndef ST7796_H
#define ST7796_H

#include "stm32l4xx_hal.h"
#include "main.h"

typedef struct
{
    uint16_t width;
    uint16_t height;
    uint16_t id;
    uint8_t  dir;
    uint16_t wramcmd;
    uint16_t  rramcmd;
    uint16_t  setxcmd;
    uint16_t  setycmd;
}_lcd_device;

extern _lcd_device lcd;

#define ST7796_WIDTH  320
#define ST7796_HEIGHT 480
#define USE_HORIZONTAL  0

// 引脚宏定义
#define ST7796_CS_LOW()     HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET)
#define ST7796_CS_HIGH()    HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET)

#define ST7796_DC_LOW()     HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET)
#define ST7796_DC_HIGH()    HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET)

#define ST7796_RST_LOW()    HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET)
#define ST7796_RST_HIGH()   HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET)

//颜色
#define   BLACK     0x0000       //   黑色
#define   NAVY      0x000F      //    深蓝色
#define   DGREEN    0x03E0        //  深绿色
#define   DCYAN     0x03EF       //   深青色
#define   MAROON    0x7800       //   深红色
#define   PURPLE    0x780F       //   紫色
#define   OLIVE     0x7BE0       //   橄榄绿
#define   LGRAY     0xC618        //  灰白色
#define   DGRAY     0x7BEF        //  深灰色
#define   BLUE      0x001F        //  蓝色
#define   GREEN     0x07E0        //  绿色
#define   CYAN      0x07FF        //  青色
#define   RED       0xF800        //  红色
#define   MAGENTA   0xF81F        //  品红
#define   YELLOW    0xFFE0        //  黄色
#define   WHITE     0xFFFF        //  白色



//指针初始化
void ST7796_AttachSPI(SPI_HandleTypeDef *hspi);
//初始化函数
void ST7796_Init(void);
//画点
void LCD_DrawPoint(const uint16_t x, const uint16_t y, const uint16_t color);


void LCD_FlushArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, const uint8_t *color_p);

#endif
