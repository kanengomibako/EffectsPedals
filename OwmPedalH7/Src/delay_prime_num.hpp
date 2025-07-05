#ifndef DELAY_PRIME_NUM_HPP
#define DELAY_PRIME_NUM_HPP

#include "common.h"
#include "sosu.h"

/* 素数サンプル数ディレイバッファ 255msまで float仕様----------------------------------------*/
class del_buf_prime_num
{
private:
  float *del_array;
  uint16_t wpos = 0;
  uint16_t max_delay_sample = 1;

public:
  del_buf_prime_num()
  {
    set(1);
  }

  ~del_buf_prime_num()
  {
    erase();
  }

  void set(uint8_t max_delay_time) // 最大ディレイ時間 ms
  {
    erase();
    max_delay_sample = sosu[max_delay_time]; // 最大サンプル数計算
    del_array = new float[max_delay_sample]; // バッファ配列メモリ確保
    for (uint16_t i = 0; i < max_delay_sample; i++) del_array[i] = 0.0f; // 配列0埋め
  }

  void erase()
  {
    delete[] del_array; // バッファ配列メモリ削除
  }

  void write(float x)
  {
    del_array[wpos] = x; // バッファ配列へfloatで書き込み
    wpos++; // 書込位置を1つ進める
    if (wpos == max_delay_sample) wpos = 0; // 最大サンプル数までで0に戻す
  }

  float read(uint8_t del_time)
  {
    uint16_t interval = sosu[del_time]; // 書込位置と読出位置の間隔を計算
    if (interval > max_delay_sample) interval = max_delay_sample;
    uint16_t rpos;
    if (wpos >= interval) rpos = wpos - interval;  // 読出位置を取得
    else rpos = wpos - interval + max_delay_sample;
    return del_array[rpos]; // バッファ配列から読み出し
  }

  float read_fixed() // 最大ディレイ時間で読み出し
  {
    return del_array[wpos];
  }

};

#endif // DELAY_PRIME_NUM_HPP
