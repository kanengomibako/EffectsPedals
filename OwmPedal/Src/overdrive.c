#include "main.h"

extern uint16_t pot[];
extern uint8_t sw[];

static float lpf1(float,float);
static float lpf2(float,float);
static float hpf1(float,float);
static float hpf2(float,float);
static float potVol1(uint16_t,float,float);
static float potVol2(uint16_t,float,float);

float overdrive(float x)
{
  x = hpf1(x, 200.0f);
  x = lpf1(x, 4000.0f);
  x = 5.0f * x;

  if (x < -0.5f) x = -0.25f; // 2次関数による波形の非対称変形
  else x = x * x + x;

  x = hpf2(x, 10.0f);

  x = x * potVol1(pot[2], -6.0f, 34.0f); // GAIN -6...+34dB

  if (x < -1.0f) x = -1.0f; // 2次関数による対称ソフトクリップ
  else if (x < 0.0f) x = x * x + 2.0f * x;
  else if (x < 1.0f) x = 2.0f * x - x * x;
  else x = 1.0f;

  x = lpf2(x, 2000.0f);

  x = x * potVol2(pot[0], -40.0f, 0.0f);  // LEVEL -40...0dB

  return x;
}

float lpf1(float x,float fc)
{
  static float y1 = 0.0f;
  float a1 = lpfCoef(fc);
  float b0 = 1.0f - a1;
  float y = b0 * x + a1 * y1;
  y1 = y;
  return y;
}

float lpf2(float x,float fc)
{
  static float y1 = 0.0f;
  float a1 = lpfCoef(fc);
  float b0 = 1.0f - a1;
  float y = b0 * x + a1 * y1;
  y1 = y;
  return y;
}

float hpf1(float x,float fc)
{
  static float x1 = 0.0f;
  static float y1 = 0.0f;
  float a1 = lpfCoef(fc);
  float b0 = 0.5f * (1.0f + a1);
  float y = b0 * x - b0 * x1 + a1 * y1;
  x1 = x;
  y1 = y;
  return y;
}

float hpf2(float x,float fc)
{
  static float x1 = 0.0f;
  static float y1 = 0.0f;
  float a1 = lpfCoef(fc);
  float b0 = 0.5f * (1.0f + a1);
  float y = b0 * x - b0 * x1 + a1 * y1;
  x1 = x;
  y1 = y;
  return y;
}

float potVol1(uint16_t pot, float min, float max)
{
  static float p = 0.0f;
  static uint16_t last_pot = 0;
  if (abs(last_pot - pot) > POT_CHANGE) // ポットが動いた時のみ処理
  {
    last_pot = pot;
    p = (float) pot / 4095.0f;
    p = (max - min) * p + min;
    p = powf(10.0f, p / 20.0f); // dBから倍率へ変換
  }
  return p;
}

float potVol2(uint16_t pot, float min, float max)
{
  static float p = 0.0f;
  static uint16_t last_pot = 0;
  if (abs(last_pot - pot) > POT_CHANGE) // ポットが動いた時のみ処理
  {
    last_pot = pot;
    p = (float) pot / 4095.0f;
    p = (max - min) * p + min;
    p = powf(10.0f, p / 20.0f); // dBから倍率へ変換
  }
  return p;
}
