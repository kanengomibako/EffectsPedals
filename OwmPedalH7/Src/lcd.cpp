#include "lcd.hpp"

// TSC2046 8bit mode
#define READ_X 0b11011000
#define READ_Y 0b10011000

inline void ILI9341_Select()
{
  HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
}

inline void ILI9341_Unselect()
{
  HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

inline void ILI9341_Reset()
{
  HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port, ILI9341_RES_Pin, GPIO_PIN_RESET);
  HAL_Delay(5);
  HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port, ILI9341_RES_Pin, GPIO_PIN_SET);
}

inline void ILI9341_WriteCommand(uint8_t cmd)
{
  HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&ILI9341_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}

inline void ILI9341_WriteData(uint8_t* buff, size_t buff_size)
{
  HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);

  // split data in small chunks because HAL can't send more then 64K at once
  while(buff_size > 0)
  {
    uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
    HAL_SPI_Transmit(&ILI9341_SPI_PORT, buff, chunk_size, HAL_MAX_DELAY);
    buff += chunk_size;
    buff_size -= chunk_size;
  }
}

inline void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  // column address set
  ILI9341_WriteCommand(0x2A); // CASET
  {
    uint8_t data[] = {(uint8_t)((x0 >> 8) & 0xFF), (uint8_t)(x0 & 0xFF), (uint8_t)((x1 >> 8) & 0xFF), (uint8_t)(x1 & 0xFF)};
    ILI9341_WriteData(data, sizeof(data));
  }

  // row address set
  ILI9341_WriteCommand(0x2B); // RASET
  {
    uint8_t data[] = {(uint8_t)((y0 >> 8) & 0xFF), (uint8_t)(y0 & 0xFF), (uint8_t)((y1 >> 8) & 0xFF), (uint8_t)(y1 & 0xFF)};
    ILI9341_WriteData(data, sizeof(data));
  }

  // write to RAM
  ILI9341_WriteCommand(0x2C); // RAMWR
}

void ILI9341_Init()
{
  ILI9341_Select();
  ILI9341_Reset();

  // command list is based on https://github.com/martnak/STM32-ILI9341

  // SOFTWARE RESET
  ILI9341_WriteCommand(0x01);
  HAL_Delay(100);

  // POWER CONTROL A
  ILI9341_WriteCommand(0xCB);
  {
    uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // POWER CONTROL B
  ILI9341_WriteCommand(0xCF);
  {
    uint8_t data[] = { 0x00, 0xC1, 0x30 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // DRIVER TIMING CONTROL A
  ILI9341_WriteCommand(0xE8);
  {
    uint8_t data[] = { 0x85, 0x00, 0x78 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // DRIVER TIMING CONTROL B
  ILI9341_WriteCommand(0xEA);
  {
    uint8_t data[] = { 0x00, 0x00 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // POWER ON SEQUENCE CONTROL
  ILI9341_WriteCommand(0xED);
  {
    uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // PUMP RATIO CONTROL
  ILI9341_WriteCommand(0xF7);
  {
    uint8_t data[] = { 0x20 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // POWER CONTROL,VRH[5:0]
  ILI9341_WriteCommand(0xC0);
  {
    uint8_t data[] = { 0x23 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // POWER CONTROL,SAP[2:0];BT[3:0]
  ILI9341_WriteCommand(0xC1);
  {
    uint8_t data[] = { 0x10 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // VCM CONTROL
  ILI9341_WriteCommand(0xC5);
  {
    uint8_t data[] = { 0x3E, 0x28 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // VCM CONTROL 2
  ILI9341_WriteCommand(0xC7);
  {
    uint8_t data[] = { 0x86 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // MEMORY ACCESS CONTROL
  ILI9341_WriteCommand(0x36);
  {
    uint8_t data[] = { 0x48 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // PIXEL FORMAT
  ILI9341_WriteCommand(0x3A);
  {
    uint8_t data[] = { 0x55 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // FRAME RATIO CONTROL, STANDARD RGB COLOR
  ILI9341_WriteCommand(0xB1);
  {
    uint8_t data[] = { 0x00, 0x18 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // DISPLAY FUNCTION CONTROL
  ILI9341_WriteCommand(0xB6);
  {
    uint8_t data[] = { 0x08, 0x82, 0x27 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // 3GAMMA FUNCTION DISABLE
  ILI9341_WriteCommand(0xF2);
  {
    uint8_t data[] = { 0x00 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // GAMMA CURVE SELECTED
  ILI9341_WriteCommand(0x26);
  {
    uint8_t data[] = { 0x01 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // POSITIVE GAMMA CORRECTION
  ILI9341_WriteCommand(0xE0);
  {
    uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                       0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
    ILI9341_WriteData(data, sizeof(data));
  }

  // NEGATIVE GAMMA CORRECTION
  ILI9341_WriteCommand(0xE1);
  {
    uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                       0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
    ILI9341_WriteData(data, sizeof(data));
  }

  // EXIT SLEEP
  ILI9341_WriteCommand(0x11);
  HAL_Delay(120);

  // TURN ON DISPLAY
  ILI9341_WriteCommand(0x29);

  // MADCTL
  ILI9341_WriteCommand(0x36);
  {
    uint8_t data[] = { ILI9341_ROTATION };
    ILI9341_WriteData(data, sizeof(data));
  }

  ILI9341_Unselect();
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
  if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;

  ILI9341_Select();

  ILI9341_SetAddressWindow(x, y, x+1, y+1);
  uint8_t data[] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};
  ILI9341_WriteData(data, sizeof(data));

  ILI9341_Unselect();
}

void ILI9341_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
  uint32_t i, b, j;

  ILI9341_SetAddressWindow(x, y, x+font.width-1, y+font.height-1);

  for(i = 0; i < font.height; i++)
  {
    b = font.data[(ch - 32) * font.height + i];
    for(j = 0; j < font.width; j++)
    {
      if((b << j) & 0x8000)
      {
        uint8_t data[] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};
        ILI9341_WriteData(data, sizeof(data));
      }
      else
      {
        uint8_t data[] = {(uint8_t)(bgcolor >> 8), (uint8_t)(bgcolor & 0xFF)};
        ILI9341_WriteData(data, sizeof(data));
      }
    }
  }
}

void ILI9341_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor)
{
  ILI9341_Select();

  while(*str)
  {
    if(x + font.width >= ILI9341_WIDTH)
	  {
      x = 0;
      y += font.height;

      if(y + font.height >= ILI9341_HEIGHT)
	    {
        break;
      }

      if(*str == ' ')
	    {
        // skip spaces in the beginning of the new line
        str++;
        continue;
      }
    }

    ILI9341_WriteChar(x, y, *str, font, color, bgcolor);
    x += font.width;
    str++;
  }

  ILI9341_Unselect();
}

void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
  // clipping
  if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
  if((x + w - 1) >= ILI9341_WIDTH) w = ILI9341_WIDTH - x;
  if((y + h - 1) >= ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;

  ILI9341_Select();
  ILI9341_SetAddressWindow(x, y, x+w-1, y+h-1);

  uint8_t data[] = {(uint8_t)(color >> 8), (uint8_t)(color & 0xFF)};
  HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
  for(y = h; y > 0; y--)
  {
    for(x = w; x > 0; x--)
	  {
      HAL_SPI_Transmit(&ILI9341_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);
    }
  }

  ILI9341_Unselect();
}

void ILI9341_FillScreen(uint16_t color)
{
  ILI9341_FillRectangle(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data)
{
  if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
  if((x + w - 1) >= ILI9341_WIDTH) return;
  if((y + h - 1) >= ILI9341_HEIGHT) return;

  ILI9341_Select();
  ILI9341_SetAddressWindow(x, y, x+w-1, y+h-1);
  ILI9341_WriteData((uint8_t*)data, sizeof(uint16_t)*w*h);
  ILI9341_Unselect();
}

void ILI9341_InvertColors(bool invert)
{
  ILI9341_Select();
  ILI9341_WriteCommand(invert ? 0x21 /* INVON */ : 0x20 /* INVOFF */);
  ILI9341_Unselect();
}

void ILI9341_Circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color)
{
  uint16_t x = r;
  uint16_t y = 0;
  int16_t F = -2 * r + 3;

  while (x >= y)
  {
  ILI9341_DrawPixel(x0 + x, y0 + y, color);
  ILI9341_DrawPixel(x0 - x, y0 + y, color);
  ILI9341_DrawPixel(x0 + x, y0 - y, color);
  ILI9341_DrawPixel(x0 - x, y0 - y, color);
  ILI9341_DrawPixel(x0 + y, y0 + x, color);
  ILI9341_DrawPixel(x0 - y, y0 + x, color);
  ILI9341_DrawPixel(x0 + y, y0 - x, color);
  ILI9341_DrawPixel(x0 - y, y0 - x, color);
    if (F >= 0)
    {
      x--;
      F -= 4 * x;
    }
    y++;
    F += 4 * y + 2;
  }
}

void ILI9341_Line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
  int16_t i, dx, dy, sx, sy;

  /* 二点間の距離 */
  dx = (x1 > x0) ? x1 - x0 : x0 - x1;
  dy = (y1 > y0) ? y1 - y0 : y0 - y1;

  /* 二点の方向 */
  sx = (x1 > x0) ? 1 : -1;
  sy = (y1 > y0) ? 1 : -1;

  if (dx > dy) /* 傾きが1より小さい場合 */
  {
    int16_t E = -dx;
    for (i = 0; i < (dx + 1) >> 1; i++)
    {
      ILI9341_DrawPixel(x0, y0, color);
      ILI9341_DrawPixel(x1, y1, color);
      x0 += sx;
      x1 -= sx;
      E += 2 * dy;
      if (E >= 0)
      {
        y0 += sy;
        y1 -= sy;
        E -= 2 * dx;
      }
    }
    if ((dx % 2) == 0) ILI9341_DrawPixel(x0, y0, color); /* dx + 1 が奇数の場合、残った中央の点を最後に描画 */
  }
  else /* 傾きが1以上の場合 */
  {
  int16_t E = -dy;
    for (i = 0 ;i < (dy + 1) >> 1; i++)
    {
      ILI9341_DrawPixel(x0, y0, color);
      ILI9341_DrawPixel(x1, y1, color);
      y0 += sy;
      y1 -= sy;
      E += 2 * dx;
      if (E >= 0)
      {
        x0 += sx;
        x1 -= sx;
        E -= 2 * dy;
      }
    }
    if ((dy % 2) == 0) ILI9341_DrawPixel(x0, y0, color); /* dy + 1 が奇数の場合、残った中央の点を最後に描画 */
  }
}

inline void ILI9341_TouchSelect()
{
  HAL_GPIO_WritePin(ILI9341_TOUCH_CS_GPIO_Port, ILI9341_TOUCH_CS_Pin, GPIO_PIN_RESET);
}

inline void ILI9341_TouchUnselect()
{
  HAL_GPIO_WritePin(ILI9341_TOUCH_CS_GPIO_Port, ILI9341_TOUCH_CS_Pin, GPIO_PIN_SET);
}

inline bool ILI9341_TouchPressed()
{
  return HAL_GPIO_ReadPin(ILI9341_TOUCH_IRQ_GPIO_Port, ILI9341_TOUCH_IRQ_Pin) == GPIO_PIN_RESET;
}

bool ILI9341_TouchGetCoordinates(uint16_t* x, uint16_t* y)
{
  static const uint8_t cmd_read_x[] = { READ_X };
  static const uint8_t cmd_read_y[] = { READ_Y };
  static const uint8_t zeroes_tx[] = { 0x00, 0x00 };

  ILI9341_TouchSelect();

  uint32_t avg_x = 0;
  uint32_t avg_y = 0;
  uint8_t nsamples = 0;
  for(uint8_t i = 0; i < 16; i++)
  {
    if(!ILI9341_TouchPressed())
        break;

    nsamples++;

    HAL_SPI_Transmit(&ILI9341_TOUCH_SPI_PORT, (uint8_t*)cmd_read_y, sizeof(cmd_read_y), HAL_MAX_DELAY);
    uint8_t y_raw[2];
    HAL_SPI_TransmitReceive(&ILI9341_TOUCH_SPI_PORT, (uint8_t*)zeroes_tx, y_raw, sizeof(y_raw), HAL_MAX_DELAY);

    HAL_SPI_Transmit(&ILI9341_TOUCH_SPI_PORT, (uint8_t*)cmd_read_x, sizeof(cmd_read_x), HAL_MAX_DELAY);
    uint8_t x_raw[2];
    HAL_SPI_TransmitReceive(&ILI9341_TOUCH_SPI_PORT, (uint8_t*)zeroes_tx, x_raw, sizeof(x_raw), HAL_MAX_DELAY);

    avg_x += (((uint16_t)x_raw[0]) << 8) | ((uint16_t)x_raw[1]);
    avg_y += (((uint16_t)y_raw[0]) << 8) | ((uint16_t)y_raw[1]);
  }

  ILI9341_TouchUnselect();

  if(nsamples < 16)
    return false;

  uint32_t raw_x = (avg_x / 16);
  if(raw_x < ILI9341_TOUCH_MIN_RAW_X) raw_x = ILI9341_TOUCH_MIN_RAW_X;
  if(raw_x > ILI9341_TOUCH_MAX_RAW_X) raw_x = ILI9341_TOUCH_MAX_RAW_X;

  uint32_t raw_y = (avg_y / 16);
  if(raw_y < ILI9341_TOUCH_MIN_RAW_X) raw_y = ILI9341_TOUCH_MIN_RAW_Y;
  if(raw_y > ILI9341_TOUCH_MAX_RAW_Y) raw_y = ILI9341_TOUCH_MAX_RAW_Y;

  // orientation: upside down
  *x = ILI9341_TOUCH_SCALE_X - (raw_x - ILI9341_TOUCH_MIN_RAW_X) * ILI9341_TOUCH_SCALE_X / (ILI9341_TOUCH_MAX_RAW_X - ILI9341_TOUCH_MIN_RAW_X);
  *y = (raw_y - ILI9341_TOUCH_MIN_RAW_Y) * ILI9341_TOUCH_SCALE_Y / (ILI9341_TOUCH_MAX_RAW_Y - ILI9341_TOUCH_MIN_RAW_Y);

  return true;
}

void ILI9341_TouchGetXY(uint16_t &x, uint16_t &y)
{
  //static const uint8_t cmd_x[] = {READ_X};
  //static const uint8_t cmd_zero[] = {0x00};
  //static const uint8_t cmd_y[] = {READ_Y};
  static const uint8_t cmd_xy[5] = {READ_X, 0x00, READ_Y, 0x00, 0x00};
  uint8_t raw[5];
  uint16_t avg_x = 0;
  uint16_t avg_y = 0;

  for (int i = 0; i < 8; i++) // 8回平均を算出
  {
    ILI9341_TouchSelect();
    HAL_SPI_TransmitReceive(&ILI9341_TOUCH_SPI_PORT, (uint8_t*)cmd_xy, raw, sizeof(raw), HAL_MAX_DELAY);
    avg_x += raw[1];
    avg_y += raw[3];
    ILI9341_TouchUnselect();
  }

  uint16_t raw_x = avg_x >> 3;
  uint16_t raw_y = avg_y >> 3;

  if(raw_x < ILI9341_TOUCH_MIN_RAW_X) raw_x = ILI9341_TOUCH_MIN_RAW_X;
  if(raw_x > ILI9341_TOUCH_MAX_RAW_X) raw_x = ILI9341_TOUCH_MAX_RAW_X;

  if(raw_y < ILI9341_TOUCH_MIN_RAW_X) raw_y = ILI9341_TOUCH_MIN_RAW_Y;
  if(raw_y > ILI9341_TOUCH_MAX_RAW_Y) raw_y = ILI9341_TOUCH_MAX_RAW_Y;

  // orientation: upside down
  x = ILI9341_TOUCH_SCALE_X - (raw_x - ILI9341_TOUCH_MIN_RAW_X) * ILI9341_TOUCH_SCALE_X / (ILI9341_TOUCH_MAX_RAW_X - ILI9341_TOUCH_MIN_RAW_X);
  y = (raw_y - ILI9341_TOUCH_MIN_RAW_Y) * ILI9341_TOUCH_SCALE_Y / (ILI9341_TOUCH_MAX_RAW_Y - ILI9341_TOUCH_MIN_RAW_Y);
}

uint16_t ILI9341_TouchGetX()
{
  static const uint8_t cmd_read_x[] = { READ_X };
  static const uint8_t zeroes_tx[] = { 0x00, 0x00 };

  ILI9341_TouchSelect();

  HAL_SPI_Transmit(&ILI9341_TOUCH_SPI_PORT, (uint8_t*)cmd_read_x, sizeof(cmd_read_x), HAL_MAX_DELAY);
  uint8_t x_raw[2];
  HAL_SPI_TransmitReceive(&ILI9341_TOUCH_SPI_PORT, (uint8_t*)zeroes_tx, x_raw, sizeof(x_raw), HAL_MAX_DELAY);
  uint32_t raw_x = ((uint16_t)x_raw[0]);

  ILI9341_TouchUnselect();

  //return raw_x;

  if(raw_x < ILI9341_TOUCH_MIN_RAW_X) raw_x = ILI9341_TOUCH_MIN_RAW_X;
  if(raw_x > ILI9341_TOUCH_MAX_RAW_X) raw_x = ILI9341_TOUCH_MAX_RAW_X;

  // orientation: upside down
  return ILI9341_TOUCH_SCALE_X - (raw_x - ILI9341_TOUCH_MIN_RAW_X) * ILI9341_TOUCH_SCALE_X / (ILI9341_TOUCH_MAX_RAW_X - ILI9341_TOUCH_MIN_RAW_X);
}

uint16_t ILI9341_TouchGetY()
{
  static const uint8_t cmd_read_y[] = { READ_Y };
  static const uint8_t zeroes_tx[] = { 0x00, 0x00 };

  ILI9341_TouchSelect();

  HAL_SPI_Transmit(&ILI9341_TOUCH_SPI_PORT, (uint8_t*)cmd_read_y, sizeof(cmd_read_y), HAL_MAX_DELAY);
  uint8_t y_raw[2];
  HAL_SPI_TransmitReceive(&ILI9341_TOUCH_SPI_PORT, (uint8_t*)zeroes_tx, y_raw, sizeof(y_raw), HAL_MAX_DELAY);
  uint32_t raw_y = ((uint16_t)y_raw[0]);

  ILI9341_TouchUnselect();

  //return raw_y;

  if(raw_y < ILI9341_TOUCH_MIN_RAW_X) raw_y = ILI9341_TOUCH_MIN_RAW_Y;
  if(raw_y > ILI9341_TOUCH_MAX_RAW_Y) raw_y = ILI9341_TOUCH_MAX_RAW_Y;

  // orientation: upside down
  return (raw_y - ILI9341_TOUCH_MIN_RAW_Y) * ILI9341_TOUCH_SCALE_Y / (ILI9341_TOUCH_MAX_RAW_Y - ILI9341_TOUCH_MIN_RAW_Y);
}

void ILI9341_Triangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
  ILI9341_Line(x0, y0, x1, y1, color);
  ILI9341_Line(x0, y0, x2, y2, color);
  ILI9341_Line(x1, y1, x2, y2, color);
}

#define TRI_BUF_SIZE 32 // FillTriangle用のバッファ配列の大きさ
void ILI9341_LineBuf(bool buf[TRI_BUF_SIZE*2][TRI_BUF_SIZE*2], int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  int16_t i, dx, dy, sx, sy;

  /* 二点間の距離 */
  dx = (x1 > x0) ? x1 - x0 : x0 - x1;
  dy = (y1 > y0) ? y1 - y0 : y0 - y1;

  /* 二点の方向 */
  sx = (x1 > x0) ? 1 : -1;
  sy = (y1 > y0) ? 1 : -1;

  if (dx > dy) /* 傾きが1より小さい場合 */
  {
    int16_t E = -dx;
    for (i = 0; i < (dx + 1) >> 1; i++)
    {
      buf[y0 + TRI_BUF_SIZE][x0 + TRI_BUF_SIZE] = true;
      buf[y1 + TRI_BUF_SIZE][x1 + TRI_BUF_SIZE] = true;
      x0 += sx;
      x1 -= sx;
      E += 2 * dy;
      if (E >= 0)
      {
        y0 += sy;
        y1 -= sy;
        E -= 2 * dx;
      }
    }
    if ((dx % 2) == 0) buf[y0 + TRI_BUF_SIZE][x0 + TRI_BUF_SIZE] = true; /* dx + 1 が奇数の場合、残った中央の点を最後に描画 */
  }
  else /* 傾きが1以上の場合 */
  {
  int16_t E = -dy;
    for (i = 0 ;i < (dy + 1) >> 1; i++)
    {
      buf[y0 + TRI_BUF_SIZE][x0 + TRI_BUF_SIZE] = true;
      buf[y1 + TRI_BUF_SIZE][x1 + TRI_BUF_SIZE] = true;
      y0 += sy;
      y1 -= sy;
      E += 2 * dx;
      if (E >= 0)
      {
        x0 += sx;
        x1 -= sx;
        E -= 2 * dy;
      }
    }
    if ((dy % 2) == 0) buf[y0 + TRI_BUF_SIZE][x0 + TRI_BUF_SIZE] = true; /* dy + 1 が奇数の場合、残った中央の点を最後に描画 */
  }
}

void ILI9341_FillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
  bool buf[TRI_BUF_SIZE*2][TRI_BUF_SIZE*2] = {}; // バッファ配列
  uint8_t points = 0; // 1列につき交差する辺の数
  uint16_t xa = 0, ya = 0, xb = 0; // 交差する点の座標

  // 原点をx0として移動
  int16_t x01 = x1 - x0;
  int16_t y01 = y1 - y0;
  int16_t x02 = x2 - x0;
  int16_t y02 = y2 - y0;

  // バッファ配列に線分を仮描画
  ILI9341_LineBuf(buf,   0,   0, x01, y01, color);
  ILI9341_LineBuf(buf,   0,   0, x02, y02, color);
  ILI9341_LineBuf(buf, x01, y01, x02, y02, color);

  for (int i = 0; i < TRI_BUF_SIZE*2; i++) // y座標一列ずつ点があるか確認
  {
    points = 0;
    xa = 0;
    xb = 0;
    ya = 0;
    for (int j = 0; j < TRI_BUF_SIZE*2; j++)
    {
      if (buf[i][j])
      {
        if (points) // 2点目以降
        {
          xb = x0 + j - TRI_BUF_SIZE;
        }
        else // 1点目
        {
          xa = x0 + j - TRI_BUF_SIZE;
          ya = y0 + i - TRI_BUF_SIZE;
        }
        points++;
      }
    }
    if (points == 1) ILI9341_DrawPixel(xa, ya, color); // 1点のみ
    else if (points >= 2) ILI9341_FillRectangle(xa, ya, xb - xa + 1, 1, color); // 2点以上あり→線分描画
  }
}
