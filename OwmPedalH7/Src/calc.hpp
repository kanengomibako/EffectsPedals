#ifndef CALC_HPP
#define CALC_HPP

#include "common.h"
#include "dbtovolTable.h"
#include <math.h>

inline float voltodb(float x) // 使用範囲0.00001(-100dB)～1(0dB) 最大誤差0.016dB
{
  x = sqrtf(sqrtf(sqrtf(sqrtf(sqrtf(x)))));
  return - 559.57399f + 995.83468f * x
         - 591.85129f * x * x + 155.60596f * x * x * x;
}

inline float dbtovol(float x) // 使用範囲±128dB 最大誤差0.015dB
{
  return dbtovolTable[(uint8_t)(x + 128.0f)]
         + (dbtovolTable[(uint8_t)(x + 128.0f) + 1] - dbtovolTable[(uint8_t)(x + 128.0f)])
           * ((x + 128.0f) - (float)((uint8_t)(x + 128.0f)));
}

inline float clipf(float x, float min, float max) // 最大最小値でクリップ float
{
  if (x > max) return max;
  else if (x < min) return min;
  else return x;
}

inline int32_t clipi(int32_t x, int32_t min, int32_t max) // 最大最小値でクリップ int
{
  if (x > max) return max;
  else if (x < min) return min;
  else return x;
}

inline int32_t clipiCirc(int32_t x, int32_t min, int32_t max) // 最大値←→最小値で循環 int
{
  if (x > max) return min;
  else if (x < min) return max;
  else return x;
}

inline int8_t invert(int8_t x) // 「!」でもよいが確実に0と1にしたい場合に使用
{
  if (x) return 0;
  else return 1;
}

inline float logpot(uint16_t pot, float dBmin, float dBmax)
{
  // POTの値0～100を最小dB～最大dB倍率へ割り当てる
  float p = (dBmax - dBmin) * (float)pot * 0.01f + dBmin;
  return dbtovol(p); // dBから倍率へ変換
}

inline float mixpot(uint16_t pot, float dBmin)
{
  // POTの値0～100をMIX倍率へ割り当てる dBminは-6以下の負の値
  float a = (-6.0f - dBmin) * 0.02f; // dB増加の傾きを計算
  if      (pot ==   0) return 0.0f;
  else if (pot <=  50) return dbtovol((float)pot * a + dBmin); // dBmin ～ -6dB
  else if (pot <  100) return 1.0f - dbtovol((float)(100 - pot) * a + dBmin);
  else                 return 1.0f;
}

/* バイパス ポップノイズ対策のため、0.01ずつ音量操作しエフェクト切り替えする--------------*/
inline float bypassL(float x, float fx)
{
  static uint8_t count = 0;

  if (footSW) // エフェクトON
  {
    if (count < 100) // バイパス音量ダウン
    {
      count++;
      return (1.0f - (float)count * 0.01f) * x; // (count: 1～100)
    }
    else if (count < 200) // エフェクト音量アップ
    {
      count++;
      return ((float)count * 0.01f - 1.0f) * fx; // (count: 101～200)
    }
    else // count終了 (count: 200)
    {
      return fx;
    }
  }
  else // エフェクトOFF
  {
    if (count > 100) // エフェクト音量ダウン
    {
      count--;
      return ((float)count * 0.01f - 1.0f) * fx; // (count: 199～100)
    }
    else if (count > 0) // バイパス音量アップ
    {
      count--;
      return (1.0f - (float)count * 0.01f) * x; // (count: 99～0)
    }
    else // count終了 (count: 0)
    {
      return x;
    }
  }
}

inline float bypassR(float x, float fx)
{
  static uint8_t count = 0;

  if (footSW) // エフェクトON
  {
    if (count < 100) // バイパス音量ダウン
    {
      count++;
      return (1.0f - (float)count * 0.01f) * x; // (count: 1～100)
    }
    else if (count < 200) // エフェクト音量アップ
    {
      count++;
      return ((float)count * 0.01f - 1.0f) * fx; // (count: 101～200)
    }
    else // count終了 (count: 200)
    {
      return fx;
    }
  }
  else // エフェクトOFF
  {
    if (count > 100) // エフェクト音量ダウン
    {
      count--;
      return ((float)count * 0.01f - 1.0f) * fx; // (count: 199～100)
    }
    else if (count > 0) // バイパス音量アップ
    {
      count--;
      return (1.0f - (float)count * 0.01f) * x; // (count: 99～0)
    }
    else // count終了 (count: 0)
    {
      return x;
    }
  }
}

#endif // CALC_HPP
