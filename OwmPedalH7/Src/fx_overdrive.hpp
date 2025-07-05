#ifndef FX_OVERDRIVE_HPP
#define FX_OVERDRIVE_HPP

#include "common.h"
#include "calc.hpp"
#include "filter.hpp"

class fx_overdrive : public fx_base
{
private:
  enum paramName {LEVEL, GAIN, BASS, TREBLE,
    P4,P5,P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19};
  float param[20] = {0.0f, 0.0f, 0.0f, 1.0f,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const uint16_t paramMax[20] = {100,100,100,100,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  const uint16_t paramMin[20] = {  0,  0,  0,  0,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const std::string paramName[20] = {
      "LEVEL", "GAIN", "BASS",
      "TREBLE",
      "","","","","","","","","","","","","","","",""};
  const uint8_t pageMax = 0;
  hpf hpf1, hpf2;
  lpf lpf1, lpf2;

public:
  fx_overdrive()
  {
    FXnameList[OD] = "OVERDRIVE";
    FXcolorList[OD] = ILI9341_YELLOW;
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
        FXparamStr[LEVEL] = std::to_string(FXparam[LEVEL]);
        break;
      case GAIN:
        FXparamStr[GAIN] = std::to_string(FXparam[GAIN]);
        break;
      case BASS:
        FXparamStr[BASS] = std::to_string(FXparam[BASS]);
        break;
      case TREBLE:
        FXparamStr[TREBLE] = std::to_string(FXparam[TREBLE]);
        break;
      default:
        break;
    }
  }

  virtual void setParam()
  {
    static uint8_t count = 0;
    count = clipiCirc(count + 1, 0, 5); // 負荷軽減のためパラメータ計算を分散させる
    switch(count)
    {
      case 0:
        param[LEVEL] = logpot(FXparam[LEVEL], -40.0f, 10.0f);  // LEVEL -40...10dB
        break;
      case 1:
        param[GAIN] = logpot(FXparam[GAIN], -6.0f, 34.0f); // GAIN -6...+34dB
        break;
      case 2:
        param[BASS] = 800.0f * logpot(FXparam[BASS], 0.0f, -20.0f); // BASS 80 ~ 800 Hz
        break;
      case 3:
        param[TREBLE] = 5000.0f * logpot(FXparam[TREBLE], -20.0f, 0.0f); // TREBLE 500 ~ 5k Hz
        break;
      case 4:
        hpf1.set(param[BASS]);
        break;
      case 5:
        lpf2.set(param[TREBLE]);
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
      fxL[i] = hpf1.process(xL[i]); // BASS
      fxL[i] = lpf1.process(fxL[i]);
  	  fxL[i] = 5.0f * fxL[i];

  	  if (fxL[i] < -0.5f) fxL[i] = -0.25f; // 2次関数による波形の非対称変形
  	  else fxL[i] = fxL[i] * fxL[i] + fxL[i];

  	  fxL[i] = param[GAIN] * hpf2.process(fxL[i]); // GAIN

  	  if (fxL[i] < -1.0f) fxL[i] = -1.0f; // 2次関数による対称ソフトクリップ
  	  else if (fxL[i] < 0.0f) fxL[i] = fxL[i] * fxL[i] + 2.0f * fxL[i];
      else if (fxL[i] < 1.0f) fxL[i] = 2.0f * fxL[i] - fxL[i] * fxL[i];
      else fxL[i] = 1.0f;

  	  fxL[i] = param[LEVEL] * lpf2.process(fxL[i]); // LEVEL, TREBLE

      xL[i] = bypassL(xL[i], fxL[i]);
    }
  }

};

#endif // FX_OVERDRIVE_HPP
