#include "stm32l4xx_hal.h"
#include "main.h"
#include "st7796.h"

static SPI_HandleTypeDef *st7796_hspi;
_lcd_device lcd;


/**
 * 赋值指针变量
 * @param hspi spi
 */
void ST7796_AttachSPI(SPI_HandleTypeDef *hspi) {
	st7796_hspi = hspi;
}

/**
 * LCD发命令
 * @param cmd 命令
 */
static void LCD_WR_REG(const uint8_t cmd) {
	ST7796_CS_LOW();
    ST7796_DC_LOW();
    HAL_SPI_Transmit(st7796_hspi, &cmd, 1, HAL_MAX_DELAY);
    ST7796_CS_HIGH();
}

/**
 * 通用发数据类,(含CS DC)
 * @param data 数据
 * @param size size
 */
static void ST7796_WriteData(const uint8_t *data, const uint16_t size) {
	ST7796_CS_LOW();
    ST7796_DC_HIGH();
    HAL_SPI_Transmit(st7796_hspi, data, size, HAL_MAX_DELAY);
    ST7796_CS_HIGH();
}


/**
 * 写一个字节数据到LCD(含CS DC)
 * @param data 1字节数据
 */
static void LCD_WR_DATA(const uint8_t data) {
    ST7796_WriteData(&data, 1);
}

/**
 * LCD复位
 */
static void LCD_RESET(void) {
    ST7796_RST_LOW();
    HAL_Delay(100);
    ST7796_RST_HIGH();
    HAL_Delay(50);
}

/**
 * 向LCD寄存器写入数据
 * @param LCD_Reg 命令
 * @param LCD_RegValue 数据
 */
void LCD_WriteReg(const uint8_t LCD_Reg, const uint16_t LCD_RegValue)
{
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA(LCD_RegValue);
}

/**
 * SPI发送1字节数据
 * @param hspi spi通道
 * @param byte 1字节数据
 * @return 接收
 */
uint8_t SPI_WriteByte(SPI_HandleTypeDef *hspi, const uint8_t byte)
{
	uint8_t received = 0;
	if (HAL_SPI_TransmitReceive(hspi, &byte, &received, 1, HAL_MAX_DELAY) != HAL_OK)
	{
		return 0xFF;
	}
	return received;
}

/**
 * 设置 ST7796 显示方向（0-3），对应 0°/90°/180°/270° 旋转
 *
 * 写入 ST7796 的 0x36 寄存器（Memory Access Control）来控制
 * 图像的扫描方向和颜色顺序。并同步更新 lcddev 的宽高属性。
 *
 * @param direction 显示方向（0：正常，1：顺时针90度，2：180度，3：270度）
 */
void LCD_direction(const uint8_t direction)
{
    // 初始化 LCD 控制指令
    lcd.setxcmd = 0x2A;   // 设置列地址命令（Column Address Set）
    lcd.setycmd = 0x2B;   // 设置页地址命令（Page Address Set）
    lcd.wramcmd = 0x2C;   // 写显存命令（Memory Write）
    lcd.rramcmd = 0x2E;   // 读显存命令（Memory Read）

    // 方向编号限制在 0~3
    lcd.dir = direction % 4;

    // 根据方向设置 Memory Access Control 寄存器
    switch (lcd.dir) {
        case 0: // 方向0：正常显示（0度）
            lcd.width = ST7796_WIDTH;
            lcd.height = ST7796_HEIGHT;
            LCD_WriteReg(0x36, (1 << 3) | (1 << 6));
            // BGR模式 + 垂直翻转（MV）
            break;

        case 1: // 方向1：顺时针旋转90度
            lcd.width = ST7796_HEIGHT;
            lcd.height = ST7796_WIDTH;
            LCD_WriteReg(0x36, (1 << 3) | (1 << 5));
            // BGR模式 + 横向镜像（ML）
            break;

        case 2: // 方向2：旋转180度
            lcd.width = ST7796_WIDTH;
            lcd.height = ST7796_HEIGHT;
            LCD_WriteReg(0x36, (1 << 3) | (1 << 7));
            // BGR模式 + 水平翻转（MY）
            break;

        case 3: // 方向3：旋转270度
            lcd.width = ST7796_HEIGHT;
            lcd.height = ST7796_WIDTH;
            LCD_WriteReg(0x36, (1 << 3) | (1 << 7) | (1 << 6) | (1 << 5));
            // BGR + 水平翻转 + 垂直翻转 + 镜像（MY | MV | ML）
            break;

        default:
            break;
    }
}

/**
 * 设置绘图区域
 * @param xStar 区域左上角x
 * @param yStar 区域左上角y
 * @param xEnd 区域右下角x
 * @param yEnd 区域右下角y
 */
void LCD_SetWindows(const uint16_t xStar, const uint16_t yStar, const uint16_t xEnd, const uint16_t yEnd)
{
	// 设置 X 方向的范围（列地址）
	LCD_WR_REG(lcd.setxcmd);       // 写入列地址设置命令（通常是 0x2A）
	LCD_WR_DATA(xStar >> 8);       // 起始列高8位
	LCD_WR_DATA(xStar & 0x00FF);   // 起始列低8位
	LCD_WR_DATA(xEnd >> 8);        // 结束列高8位
	LCD_WR_DATA(xEnd & 0x00FF);    // 结束列低8位

	// 设置 Y 方向的范围（页地址）
	LCD_WR_REG(lcd.setycmd);       // 写入页地址设置命令（通常是 0x2B）
	LCD_WR_DATA(yStar >> 8);       // 起始行高8位
	LCD_WR_DATA(yStar & 0x00FF);   // 起始行低8位
	LCD_WR_DATA(yEnd >> 8);        // 结束行高8位
	LCD_WR_DATA(yEnd & 0x00FF);    // 结束行低8位

	// 设置完窗口后，准备开始向该区域写数据
	LCD_WR_REG(lcd.wramcmd);       // 发送 0x2C，进入写显存状态
}

/**
 * 清屏填充颜色
 * @param color color
 */
void LCD_Clear(const uint16_t color)
{
	//设置整个屏幕的画布
	LCD_SetWindows(0,0,lcd.width-1,lcd.height-1);
	ST7796_CS_LOW();
	ST7796_DC_HIGH();
	for(unsigned int i = 0;i<lcd.height;i++)
	{
		for(unsigned int m = 0;m<lcd.width;m++)
		{
			//每个像素写颜色数据
			SPI_WriteByte(st7796_hspi,color>>8);
			SPI_WriteByte(st7796_hspi,color);
		}
	}
	ST7796_CS_HIGH();
}

/**
 * 写16位数据
 * @param data 16bit数据(通常是RGB565格式的颜色值)
 */
void LCD_WriteData_16Bit(const uint16_t data)
{
	ST7796_CS_LOW();
	ST7796_DC_HIGH();

	//发高8位
	SPI_WriteByte(st7796_hspi,data>>8);
	//发低8位
	SPI_WriteByte(st7796_hspi,data);

	ST7796_CS_HIGH();
}

/**
 * 画点函数依赖,创建1x1的绘图区域
 * @param Xpos X
 * @param Ypos Y
 */
void LCD_SetCursor(const uint16_t Xpos, const uint16_t Ypos)
{
	LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);
}

/**
 * 画点
 * @param x x
 * @param y y
 * @param color color
 */
void LCD_DrawPoint(const uint16_t x, const uint16_t y, const uint16_t color)
{
	LCD_SetCursor(x,y);
	LCD_WriteData_16Bit(color);
}

/**
 * 初始化
 */
void ST7796_Init(void) {


	LCD_RESET();


	LCD_WR_REG(0x11);

	HAL_Delay(120);

	LCD_WR_REG(0x36);     // Memory Data Access Control MY,MX~~
	LCD_WR_DATA(0x48);

	LCD_WR_REG(0x3A);
	LCD_WR_DATA(0x55);

	LCD_WR_REG(0xF0);     // Command Set Control
	LCD_WR_DATA(0xC3);

	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x96);

	LCD_WR_REG(0xB4);
	LCD_WR_DATA(0x02);

	LCD_WR_REG(0xB7);
	LCD_WR_DATA(0xC6);

	LCD_WR_REG(0xC0);
	LCD_WR_DATA(0xC0);
	LCD_WR_DATA(0x00);

	LCD_WR_REG(0xC1);
	LCD_WR_DATA(0x13);

	LCD_WR_REG(0xC2);
	LCD_WR_DATA(0xA7);

	LCD_WR_REG(0xC5);
	LCD_WR_DATA(0x21);

	LCD_WR_REG(0xE8);
	LCD_WR_DATA(0x40);
	LCD_WR_DATA(0x8A);
	LCD_WR_DATA(0x1B);
	LCD_WR_DATA(0x1B);
	LCD_WR_DATA(0x23);
	LCD_WR_DATA(0x0A);
	LCD_WR_DATA(0xAC);
	LCD_WR_DATA(0x33);

	LCD_WR_REG(0xE0);
	LCD_WR_DATA(0xD2);
	LCD_WR_DATA(0x05);
	LCD_WR_DATA(0x08);
	LCD_WR_DATA(0x06);
	LCD_WR_DATA(0x05);
	LCD_WR_DATA(0x02);
	LCD_WR_DATA(0x2A);
	LCD_WR_DATA(0x44);
	LCD_WR_DATA(0x46);
	LCD_WR_DATA(0x39);
	LCD_WR_DATA(0x15);
	LCD_WR_DATA(0x15);
	LCD_WR_DATA(0x2D);
	LCD_WR_DATA(0x32);

	LCD_WR_REG(0xE1);
	LCD_WR_DATA(0x96);
	LCD_WR_DATA(0x08);
	LCD_WR_DATA(0x0C);
	LCD_WR_DATA(0x09);
	LCD_WR_DATA(0x09);
	LCD_WR_DATA(0x25);
	LCD_WR_DATA(0x2E);
	LCD_WR_DATA(0x43);
	LCD_WR_DATA(0x42);
	LCD_WR_DATA(0x35);
	LCD_WR_DATA(0x11);
	LCD_WR_DATA(0x11);
	LCD_WR_DATA(0x28);
	LCD_WR_DATA(0x2E);

	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x3C);
	LCD_WR_REG(0xF0);
	LCD_WR_DATA(0x69);
	HAL_Delay(120);
	LCD_WR_REG(0x21);
	LCD_WR_REG(0x29);
	//设置位默认方向
	LCD_direction(USE_HORIZONTAL);
	//填充白色清屏
	LCD_Clear(0xFFFF);
}

