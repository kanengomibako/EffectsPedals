#ifndef LFO_HPP
#define LFO_HPP

#include "common.h"
#include "calc.hpp"

class triangleWave
{
private:
  uint32_t count = 0, rateCount = 20000;

public:
  triangleWave()
  {
  }

  void set(float rate) // 周期（秒）設定
  {
    rateCount = rate * SAMPLING_FREQ;
  }

  void set(float rate, float phase) // 周期（秒）、位相（0～1）設定
  {
    rateCount = rate * SAMPLING_FREQ;
    count = (uint32_t)((float)rateCount * phase);
  }

  float output() // 周期指定
  {
    count++;
    if (count >= rateCount) count = 0;
    float y = (float)count / (float)rateCount;
    if (count > rateCount / 2) y = 1.0f - y;
    return 2 * y; // 0 ... 1 Triangle Wave
  }

};

#endif // LFO_HPP
