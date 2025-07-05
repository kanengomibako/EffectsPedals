#ifndef FX_CHORUS_HPP
#define FX_CHORUS_HPP

#include "common.h"
#include "calc.hpp"
#include "filter.hpp"
#include "lfo.hpp"

class fx_chorus : public fx_base
{
private:
  enum paramName {LEVEL, MIX, FBACK, RATE, DEPTH, HICUT,
    P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19};
  float param[20] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const uint16_t paramMax[20] = { 70,100,99,100,100, 99,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  const uint16_t paramMin[20] = { 30, 0,  0,  0,  0, 10,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const std::string paramName[20] = {
      "LEVEL", "MIX", "F.BACK",
      "RATE", "DEPTH", "HI-CUT"
      "","","","","","","","","","","","","",""};
  const uint8_t pageMax = 0;

  triangleWave tri1;
  del_buf del1;
  hpf hpf1;
  lpf2nd lpf2nd1, lpf2nd2;

public:
  fx_chorus()
  {
    FXnameList[CE] = "CHORUS";
    FXcolorList[CE] = ILI9341_COLOR565(64, 64, 255);
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
    del1.set(20.0f); // 最大ディレイタイム設定
    hpf1.set(100.0f); // ウェット音のローカット設定
  }

  virtual void deinit()
  {
    del1.set(1.0f);
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
      case MIX:
        FXparamStr[MIX] = std::to_string(FXparam[MIX]);
        break;
      case FBACK:
        FXparamStr[FBACK] = std::to_string(FXparam[FBACK]);
        FXparamStr[FBACK] += "%";
        break;
      case RATE:
        FXparamStr[RATE] = std::to_string(FXparam[RATE]);
        break;
      case DEPTH:
        FXparamStr[DEPTH] = std::to_string(FXparam[DEPTH]);
        break;
      case HICUT:
        FXparamStr[HICUT] = std::to_string(FXparam[HICUT]);
        FXparamStr[HICUT] += "00Hz";
        break;
      default:
        break;
    }
  }

  virtual void setParam()
  {
    static uint8_t count = 0;
    count = clipiCirc(count + 1, 0, 8); // 負荷軽減のためパラメータ計算を分散させる
    switch(count)
    {
      case 0:
        param[LEVEL] = dbtovol((float)(FXparam[LEVEL] - 50)); // OUTPUT LEVEL -20...+20dB
        break;
      case 1:
        param[MIX] = mixpot(FXparam[MIX], -20.0f);  // MIX
        break;
      case 2:
        param[FBACK] = (float)FXparam[FBACK] / 100.0f; // Feedback 0～0.99
        break;
      case 3:
        param[RATE] = 0.02f * (105.0f - (float)FXparam[RATE]); // Rate 2s
        break;
      case 4:
        param[DEPTH] = 0.1f * (float)FXparam[DEPTH]; // Depth 10ms
        break;
      case 5:
        param[HICUT] = (float)FXparam[HICUT] * 100.0f; // HI CUT FREQ 1000 ~ 9900 Hz
        break;
      case 6:
        lpf2nd1.set(param[HICUT]);
        break;
      case 7:
        lpf2nd2.set(param[HICUT]);
        break;
      case 8:
        tri1.set(param[RATE]);
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
      float dtime = param[DEPTH] * tri1.output() + 5.0f; // ディレイタイム5~15ms
      fxL[i] = del1.read_lerp(dtime);
      fxL[i] = lpf2nd1.process(fxL[i]);
      fxL[i] = lpf2nd2.process(fxL[i]);
      del1.write(hpf1.process(xL[i]) + param[FBACK] * fxL[i]);
      fxL[i] = (1.0f - param[MIX]) * xL[i] + param[MIX] * fxL[i];
      xL[i] = bypassL(xL[i], fxL[i] * param[LEVEL]);
    }
  }

};

#endif // FX_CHORUS_HPP
