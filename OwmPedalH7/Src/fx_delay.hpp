#ifndef FX_DELAY_HPP
#define FX_DELAY_HPP

#include "common.h"
#include "calc.hpp"
#include "filter.hpp"
#include "delay.hpp"

class fx_delay : public fx_base
{
private:
  enum paramName {OUTPUT, ELEVEL, FBACK, DELAYTIME1, DELAYTIME2, HICUT,
    P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19};
  float param[20] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const uint16_t paramMax[20] = { 70, 70, 99, 10, 99, 99,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  const uint16_t paramMin[20] = { 30, 30,  0,  0,  0, 10,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const std::string paramName[20] = {
      "OUTPUT", "E.LEVEL", "F.BACK",
			"  DELAY", "TIME   ", "HI-CUT"
      "","","","","","","","","","","","","",""};
  const float max_del_time = 1500.0f;
  const uint8_t pageMax = 0;
  del_buf del1;
  lpf2nd lpf2nd1;

public:
  fx_delay()
  {
    FXnameList[DD] = "DELAY";
    FXcolorList[DD] = ILI9341_WHITE;
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
    del1.set(max_del_time); // 最大ディレイタイム設定
  }

  virtual void deinit()
  {
    del1.set(1.0f);
  }

  virtual void setParamStr(uint8_t paramNum)
  {
    switch(paramNum)
    {
      case OUTPUT:
        FXparamStr[OUTPUT] = std::to_string(FXparam[OUTPUT] - 50);
        if (FXparam[OUTPUT] > 50) FXparamStr[OUTPUT].insert(0, "+");
        FXparamStr[OUTPUT] += "dB";
        break;
      case ELEVEL:
        FXparamStr[ELEVEL] = std::to_string(FXparam[ELEVEL] - 50);
        if (FXparam[ELEVEL] > 50) FXparamStr[ELEVEL].insert(0, "+");
        FXparamStr[ELEVEL] += "dB";
        break;
      case FBACK:
        FXparamStr[FBACK] = std::to_string(FXparam[FBACK]);
        FXparamStr[FBACK] += "%";
        break;
      case DELAYTIME1:
        FXparamStr[DELAYTIME1] = std::to_string(FXparam[DELAYTIME1]);
        if (FXparam[DELAYTIME1] < 10) FXparamStr[DELAYTIME1].insert(0, "      ");
        else if (FXparam[DELAYTIME1] < 100) FXparamStr[DELAYTIME1].insert(0, "     ");
        else FXparamStr[DELAYTIME1].insert(0, "    ");
        break;
      case DELAYTIME2:
        FXparamStr[DELAYTIME2] = std::to_string(FXparam[DELAYTIME2]);
        if (FXparam[DELAYTIME2] < 10) FXparamStr[DELAYTIME2].insert(0, "0");
        FXparamStr[DELAYTIME2] += "ms   ";
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
    count = clipiCirc(count + 1, 0, 6); // 負荷軽減のためパラメータ計算を分散させる
    switch(count)
    {
      case 0:
        param[OUTPUT] = dbtovol((float)(FXparam[OUTPUT] - 50)); // OUTPUT LEVEL -20...+20dB
        break;
      case 1:
        param[ELEVEL] = dbtovol((float)(FXparam[ELEVEL] - 50)); // EFFECT LEVEL -20...+20dB
        break;
      case 2:
        param[FBACK] = (float)FXparam[FBACK] / 100.0f; // Feedback 0～0.99
        break;
      case 3:
        param[DELAYTIME1] = (float)FXparam[DELAYTIME1] * 100.0f; // DELAYTIME 0 ～ 1000
        break;
      case 4:
        param[DELAYTIME2] = (float)FXparam[DELAYTIME2]; // DELAYTIME 細かい値 0 ～ 99
        break;
      case 5:
        param[HICUT] = (float)FXparam[HICUT] * 100.0f; // HI CUT FREQ 1000 ~ 9900 Hz
        break;
      case 6:
        lpf2nd1.set(param[HICUT]);
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
      fxL[i] = del1.read(param[DELAYTIME1] + param[DELAYTIME2]);
      fxL[i] = lpf2nd1.process(fxL[i]);
      del1.write(xL[i] + param[FBACK] * fxL[i]);
      fxL[i] = param[OUTPUT] * (xL[i] + fxL[i] * param[ELEVEL]);
      xL[i] = bypassL(xL[i], fxL[i]);
    }
  }
};

#endif // FX_DELAY_HPP
