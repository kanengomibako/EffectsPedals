#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <string>

/* 各定数設定 --------------------------*/
#define PEDAL_NAME "Owm Pedal H7 v0.02"

// ブロックサイズ
#define BLOCK_SIZE 32

// 円周率
#define PI 3.14159265359f

// サンプリング周波数
#define SAMPLING_FREQ 44100.0f

// 最大エフェクト数
#define MAX_FX_NUM 6
// エフェクト番号割当
enum FXtype {OD, DD, TR, EQ, CE, RV};

// ILI9341 Color definitions
#define	ILI9341_BLACK   0x0000
#define	ILI9341_BLUE    0x001F
#define	ILI9341_RED     0xF800
#define	ILI9341_GREEN   0x07E0
#define ILI9341_CYAN    0x07FF
#define ILI9341_MAGENTA 0xF81F
#define ILI9341_YELLOW  0xFFE0
#define ILI9341_WHITE   0xFFFF
#define ILI9341_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

/* グローバル変数 --------------------------*/

// user_main.cpp で定義
extern uint8_t footSW;
extern uint16_t FXparam[];
extern uint16_t FXparamMax[];
extern uint16_t FXparamMin[];
extern std::string FXparamName[];
extern std::string FXparamStr[];
extern uint8_t FXnum;
extern uint16_t allData[MAX_FX_NUM][20];
extern uint8_t FXpageMax;

// fx.cpp で定義
extern std::string FXnameList[MAX_FX_NUM];
extern uint16_t FXcolorList[MAX_FX_NUM];

#endif // COMMON_H
