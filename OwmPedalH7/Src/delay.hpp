#ifndef DELAY_HPP
#define DELAY_HPP

#include "common.h"

/* ディレイバッファ ----------------------------------------*/
class del_buf
{
private:
  int16_t *del_array;
  uint32_t wpos = 0;
  uint32_t max_delay_sample = 1;

public:
  del_buf()
  {
    set(1.0f);
  }

  ~del_buf()
  {
    erase();
  }

  void set(float max_delay_time) // 最大ディレイ時間 ms
  {
    erase();
    max_delay_sample = (uint32_t)(SAMPLING_FREQ * max_delay_time / 1000.0f); // 最大サンプル数計算
    del_array = new int16_t[max_delay_sample]; // バッファ配列メモリ確保
    for (uint32_t i = 0; i < max_delay_sample; i++) del_array[i] = 0; // 配列0埋め
  }

  void erase()
  {
    delete[] del_array; // バッファ配列メモリ削除
  }

  void write(float x)
  {
    del_array[wpos] = (int16_t)(x * 32767.0f); // バッファ配列へ16ビット整数で書き込み
    wpos++; // 書込位置を1つ進める
    if (wpos == max_delay_sample) wpos = 0; // 最大サンプル数までで0に戻す
  }

  float read(float del_time) // 通常のサンプル単位での読み出し
  {
    uint32_t interval = (uint32_t) (0.001f * del_time * SAMPLING_FREQ); // 書込位置と読出位置の間隔を計算
    if (interval > max_delay_sample) interval = max_delay_sample;
    uint32_t rpos;
    if (wpos >= interval) rpos = wpos - interval;  // 読出位置を取得
    else rpos = wpos - interval + max_delay_sample;
    return ((float) del_array[rpos]) / 32767.0f; // バッファ配列からfloatで読み出し
  }

  float read_lerp(float del_time) // 線形補間して読み出し コーラス等に利用
  {
    float interval_f = 0.001f * del_time * SAMPLING_FREQ; // 書込位置と読出位置の間隔をfloatで計算
    if (interval_f > (float)max_delay_sample) interval_f = (float)max_delay_sample;
    float rpos_f;
    if ((float)wpos >= interval_f) rpos_f = (float)wpos - interval_f; // 読出位置をfloatで取得
    else rpos_f = (float)wpos - interval_f + (float)max_delay_sample;
    uint32_t rpos0 = (uint32_t)rpos_f; // 読出位置の前後の整数値を取得
    uint32_t rpos1 = rpos0 + 1;
    if (rpos1 == max_delay_sample) rpos1 = 0;
    float t = rpos_f - (float)rpos0;  // 線形補間用係数
    return ((float)del_array[rpos0] + t * (float)(del_array[rpos1] - del_array[rpos0])) / 32767.0f;
  }

  float read_fixed() // 固定時間（最大ディレイタイム）で読み出し
  {
    return ((float) del_array[wpos]) / 32767.0f; // バッファ配列からfloatで読み出し
  }

};

#endif // DELAY_HPP



































