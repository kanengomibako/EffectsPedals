#include "main.h"

extern uint8_t sw[];
extern uint8_t mute;

/* エフェクト選択　複数かけることもできる  -----------------------------------------------*/
static float fx(float x)
{
  x = p3(x);
  return x;
}

/* バイパス ポップノイズ対策のため、0.01ずつ音量操作しエフェクト切り替えする----------------*/
float bypass(float x)
{
  static uint16_t count = 0;
  
  if (mute == 1) return 0.0f; // データ保存時ノイズが出るためミュートする
  
  if ( sw[0] != 0 ) // エフェクトON
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
    else if ( count > 0) // バイパス音量アップ
    {
      x = (1.0f - (float) count * 0.01f) * x;
      count--;
    }
  }
  return x;
}

/* BiQuadフィルタ係数 近似計算 ----------------------------------------------------------*/
float bqSinOmega(float fc)
{
  float fc2 = fc * fc;
  return fc * ( 0.0001308996939f - 3.738218157e-13f * fc2 + 3.202667914e-22f * fc2 * fc2 );
  //return sinf( 2.0f * PI * fc / fs );
}

float bqCosOmega(float fc)
{
  float fc2 = fc * fc;
  return 1.0f - 8.567364932e-9f * fc2 + 1.223329031e-17f * fc2 * fc2;
  //return cosf( 2.0f * PI * fc / fs );
}

float bqAlphaQ(float fc,float q)
{
  return bqSinOmega(fc) / ( 2.0f * q );
}

float bqAlphaBW(float fc,float bw)
{
  return ( 4.66355e-6f * bw * bw + 4.15256e-5f * bw + 7.04497e-7f) * fc;
}

float bqA(float gain)
{
  return expf( 0.05756462732f * gain );
}

float bqBeta(float gain,float q)
{
  return expf( 0.02878231366f * gain ) / q;
}
