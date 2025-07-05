#ifndef LCD_HPP
#define LCD_HPP

#include "common.h"

#include "stm32h7xx_hal.h"
#include "fonts.h"

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04

/*** Redefine if necessary ***/
#define ILI9341_SPI_PORT hspi1
#define ILI9341_TOUCH_SPI_PORT hspi1
extern SPI_HandleTypeDef ILI9341_SPI_PORT;

#define ILI9341_RES_Pin       GPIO_PIN_11
#define ILI9341_RES_GPIO_Port GPIOD
#define ILI9341_CS_Pin        GPIO_PIN_12
#define ILI9341_CS_GPIO_Port  GPIOD
#define ILI9341_DC_Pin        GPIO_PIN_10
#define ILI9341_DC_GPIO_Port  GPIOD
#define ILI9341_TOUCH_IRQ_Pin       GPIO_PIN_14
#define ILI9341_TOUCH_IRQ_GPIO_Port GPIOB
#define ILI9341_TOUCH_CS_Pin        GPIO_PIN_15
#define ILI9341_TOUCH_CS_GPIO_Port  GPIOB

// orientation: upside down
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define ILI9341_ROTATION (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR)

// change depending on screen orientation
#define ILI9341_TOUCH_SCALE_X 240
#define ILI9341_TOUCH_SCALE_Y 320

// touchscreen calibration
#define ILI9341_TOUCH_MIN_RAW_X 12
#define ILI9341_TOUCH_MAX_RAW_X 115
#define ILI9341_TOUCH_MIN_RAW_Y 10
#define ILI9341_TOUCH_MAX_RAW_Y 120

void ILI9341_Init(void);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);
void ILI9341_InvertColors(bool invert);

void ILI9341_Circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void ILI9341_Line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

//bool ILI9341_TouchPressed();
//bool ILI9341_TouchGetCoordinates(uint16_t* x, uint16_t* y);
void ILI9341_TouchGetXY(uint16_t& x, uint16_t& y);
uint16_t ILI9341_TouchGetX();
uint16_t ILI9341_TouchGetY();

void ILI9341_Triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
void ILI9341_FillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);

#endif // LCD_HPP
