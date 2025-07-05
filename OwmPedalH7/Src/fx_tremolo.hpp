#ifndef FX_TREMOLO_HPP
#define FX_TREMOLO_HPP

#include "common.h"
#include "lfo.hpp"
#include "calc.hpp"

class fx_tremolo : public fx_base
{
private:
  enum paramName {LEVEL, RATE, WAVE, DEPTH,
    P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19};
  float param[20] = {0.0f, 1.0f, 1.0f, 1.0f,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const uint16_t paramMax[20] = { 70,100,100,100,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  const uint16_t paramMin[20] = { 30,  0,  0,  0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const std::string paramName[20] = {
      "LEVEL", "RATE", "WAVE",
      "DEPTH",
      "","","","","","","","","","","","","","","",""};
  const uint8_t pageMax = 0;

  triangleWave tri;

public:
  fx_tremolo()
  {
    FXnameList[TR] = "TREMOLO";
    FXcolorList[TR] = ILI9341_GREEN;
  }

  virtual void init()
  {
    FXpageMax = pageMax;
    for (int i = 0; i < 20; i++)
    {
      FXparamName[i] = paramName[i];
      FXparamMax[i] = paramMax[i];
      FXparamMin[i] = paramMin[i];
      FXparam[i] = clipi(allData[FXnum][i], paramMin[i], paramMax[i]);
    }
  }

  virtual void deinit()
  {
  }

  virtual void setParamStr(uint8_t paramNum)
  {
    switch(paramNum)
    {
      case LEVEL:
        FXparamStr[LEVEL] = std::to_string(FXparam[LEVEL] - 50);
        if (FXparam[LEVEL] > 50) FXparamStr[LEVEL].insert(0, "+");
        FXparamStr[LEVEL] += "dB";
        break;
      case RATE:
        FXparamStr[RATE] = std::to_string(FXparam[RATE]);
        break;
      case WAVE:
        FXparamStr[WAVE] = std::to_string(FXparam[WAVE]);
        break;
      case DEPTH:
        FXparamStr[DEPTH] = std::to_string(FXparam[DEPTH]);
        break;
      default:
        break;
    }
  }

  virtual void setParam()
  {
    static uint8_t count = 0;
    count = clipiCirc(count + 1, 0, 4); // 負荷軽減のためパラメータ計算を分散させる
    switch(count)
    {
      case 0:
        param[LEVEL] = dbtovol((float)(FXparam[LEVEL] - 50)); // LEVEL -20...+20dB
        break;
      case 1:
        param[RATE] = 0.01f * (105.0f - (float)FXparam[RATE]); // Rate 1s
        break;
      case 2:
        param[WAVE] = logpot(FXparam[WAVE], 0.0f, 50.0f); // Wave 三角波～矩形波変形
        break;
      case 3:
        param[DEPTH] = (float)FXparam[DEPTH] * 0.1f; // Depth ±10dB
        break;
      case 4:
        tri.set(param[RATE]);
        break;
      default:
        break;
    }
  }

  virtual void process(float xL[], float xR[])
  {
    float fxL[BLOCK_SIZE] = {};

    setParam();

    for (uint16_t i = 0; i < BLOCK_SIZE; i++)
    {
      float gain = 2.0f * (tri.output() - 0.5f); // -1dB ... 1dB LFO
      gain = clipf(gain * param[WAVE], -1.0f, 1.0f);
      gain = dbtovol(gain * param[DEPTH]);
      fxL[i] = param[LEVEL] * xL[i] * gain;
      xL[i] = bypassL(xL[i], fxL[i]);
    }
  }

};

#endif // FX_TREMOLO_HPP
