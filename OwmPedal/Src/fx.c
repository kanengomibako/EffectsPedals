#include "main.h"

extern uint16_t pot[];
extern uint8_t sw[];

/* エフェクト選択 複数かけることもできる -----------------------------------------------*/
static float fx(float x)
{
  x = overdrive(x);
  return x;
}

/* バイパス ポップノイズ対策のため、0.01ずつ音量操作しエフェクト切り替えする--------------*/
float bypass(float x)
{
  static uint16_t count = 0;

  if (sw[3] != 0) // エフェクトON
  {
    if (count < 100) // バイパス音量ダウン
    {
      x = (1.0f - (float) count * 0.01f) * x;
      count++;
    }
    else if (count < 200) // エフェクト音量アップ
    {
      x = ((float) count * 0.01f - 1.0f) * fx(x);
      count++;
    }
    else // count終了 (count: 200)
    {
      x = fx(x);
    }
  }
  else // エフェクトOFF
  {
    if (count > 100) // エフェクト音量ダウン
    {
      x = ((float) count * 0.01f - 1.0f) * fx(x);
      count--;
    }
    else if (count > 0) // バイパス音量アップ
    {
      x = (1.0f - (float) count * 0.01f) * x;
      count--;
    }
  }
  return x;
}

/* 1次LPF、HPF用の係数計算 fs=48000、50～10kHzでの近似曲線 -----------------------------*/
float lpfCoef(float fc)
{
  return 0.000000005146361f * fc * fc - 0.0001185165f * fc + 0.9943665f;
}
