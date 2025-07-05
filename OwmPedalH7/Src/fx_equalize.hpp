#ifndef FX_EQUALIZE_HPP
#define FX_EQUALIZE_HPP

#include "common.h"
#include "calc.hpp"
#include "filter.hpp"

class fx_equalize : public fx_base
{
private:
  enum paramName {LEVEL, P1, P2, FREQ1, Q1, GAIN1, FREQ2, Q2, GAIN2, FREQ3, Q3, GAIN3, FREQ4, Q4, GAIN4, FREQ5, Q5, GAIN5,
    P18,P19};
  float param[20] = {0.0f,0.0f,0.0f,9.9f,1.0f,0.0f,9.9f,1.0f,0.0f,9.9f,1.0f,0.0f,9.9f,1.0f,0.0f,9.9f,1.0f,0.0f,
      0,0};
  const uint16_t paramMax[20] = {70,1,1,99,99,65,99,99,65,99,99,65,99,99,65,99,99,65,
      1,1};
  const uint16_t paramMin[20] = {30,0,0,1,1,35,1,1,35,1,1,35,1,1,35,1,1,35,
      0,0};
  const std::string paramName[20] = {
      "LEVEL", "", "", "FREQ1", "Q1", "GAIN1", "FREQ2", "Q2", "GAIN2", "FREQ3", "Q3", "GAIN3", "FREQ4", "Q4", "GAIN4", "FREQ5", "Q5", "GAIN5",
      "",""};
  const uint8_t pageMax = 2;

  biquadfilter bqf1, bqf2, bqf3, bqf4, bqf5;

public:
  fx_equalize()
  {
    FXnameList[EQ] = "5PARA-EQ";
    FXcolorList[EQ] = ILI9341_COLOR565(128, 128, 128);
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
      case FREQ1:
        FXparamStr[FREQ1] = std::to_string(FXparam[FREQ1]);
        FXparamStr[FREQ1] += "00Hz";
        break;
      case FREQ2:
        FXparamStr[FREQ2] = std::to_string(FXparam[FREQ2]);
        FXparamStr[FREQ2] += "00Hz";
        break;
      case FREQ3:
        FXparamStr[FREQ3] = std::to_string(FXparam[FREQ3]);
        FXparamStr[FREQ3] += "00Hz";
        break;
      case FREQ4:
        FXparamStr[FREQ4] = std::to_string(FXparam[FREQ4]);
        FXparamStr[FREQ4] += "00Hz";
        break;
      case FREQ5:
        FXparamStr[FREQ5] = std::to_string(FXparam[FREQ5]);
        FXparamStr[FREQ5] += "00Hz";
        break;
      case Q1:
        FXparamStr[Q1] = std::to_string(FXparam[Q1]);
        if (FXparam[Q1] < 10) FXparamStr[Q1].insert(0, "0.");
        else FXparamStr[Q1].insert(1, ".");
        break;
      case Q2:
        FXparamStr[Q2] = std::to_string(FXparam[Q2]);
        if (FXparam[Q2] < 10) FXparamStr[Q2].insert(0, "0.");
        else FXparamStr[Q2].insert(1, ".");
        break;
      case Q3:
        FXparamStr[Q3] = std::to_string(FXparam[Q3]);
        if (FXparam[Q3] < 10) FXparamStr[Q3].insert(0, "0.");
        else FXparamStr[Q3].insert(1, ".");
        break;
      case Q4:
        FXparamStr[Q4] = std::to_string(FXparam[Q4]);
        if (FXparam[Q4] < 10) FXparamStr[Q4].insert(0, "0.");
        else FXparamStr[Q4].insert(1, ".");
        break;
      case Q5:
        FXparamStr[Q5] = std::to_string(FXparam[Q5]);
        if (FXparam[Q5] < 10) FXparamStr[Q5].insert(0, "0.");
        else FXparamStr[Q5].insert(1, ".");
        break;
      case GAIN1:
        FXparamStr[GAIN1] = std::to_string(FXparam[GAIN1] - 50);
        if (FXparam[GAIN1] > 50) FXparamStr[GAIN1].insert(0, "+");
        FXparamStr[GAIN1] += "dB";
        break;
      case GAIN2:
        FXparamStr[GAIN2] = std::to_string(FXparam[GAIN2] - 50);
        if (FXparam[GAIN2] > 50) FXparamStr[GAIN2].insert(0, "+");
        FXparamStr[GAIN2] += "dB";
        break;
      case GAIN3:
        FXparamStr[GAIN3] = std::to_string(FXparam[GAIN3] - 50);
        if (FXparam[GAIN3] > 50) FXparamStr[GAIN3].insert(0, "+");
        FXparamStr[GAIN3] += "dB";
        break;
      case GAIN4:
        FXparamStr[GAIN4] = std::to_string(FXparam[GAIN4] - 50);
        if (FXparam[GAIN4] > 50) FXparamStr[GAIN4].insert(0, "+");
        FXparamStr[GAIN4] += "dB";
        break;
      case GAIN5:
        FXparamStr[GAIN5] = std::to_string(FXparam[GAIN5] - 50);
        if (FXparam[GAIN5] > 50) FXparamStr[GAIN5].insert(0, "+");
        FXparamStr[GAIN5] += "dB";
        break;
      default:
        break;
    }
  }

  virtual void setParam()
  {
    static uint8_t count = 0;
    count = clipiCirc(count + 1, 0, 20); // 負荷軽減のためパラメータ計算を分散させる
    switch(count)
    {
      case 0:
        param[LEVEL] = dbtovol((float)(FXparam[LEVEL] - 50)); // OUTPUT LEVEL -20...+20dB
        break;
      case 1:
        param[FREQ1] = (float)FXparam[FREQ1] * 100.0f; // FREQ 100 ~ 9900 Hz
        break;
      case 2:
        param[FREQ2] = (float)FXparam[FREQ2] * 100.0f; // FREQ 100 ~ 9900 Hz
        break;
      case 3:
        param[FREQ3] = (float)FXparam[FREQ3] * 100.0f; // FREQ 100 ~ 9900 Hz
        break;
      case 4:
        param[FREQ4] = (float)FXparam[FREQ4] * 100.0f; // FREQ 100 ~ 9900 Hz
        break;
      case 5:
        param[FREQ5] = (float)FXparam[FREQ5] * 100.0f; // FREQ 100 ~ 9900 Hz
        break;
      case 6:
        param[Q1] = (float)FXparam[Q1] * 0.1f; // Q 0.1 ~ 9.9
        break;
      case 7:
        param[Q2] = (float)FXparam[Q2] * 0.1f; // Q 0.1 ~ 9.9
        break;
      case 8:
        param[Q3] = (float)FXparam[Q3] * 0.1f; // Q 0.1 ~ 9.9
        break;
      case 9:
        param[Q4] = (float)FXparam[Q4] * 0.1f; // Q 0.1 ~ 9.9
        break;
      case 10:
        param[Q5] = (float)FXparam[Q5] * 0.1f; // Q 0.1 ~ 9.9
        break;
      case 11:
        param[GAIN1] = (float)(FXparam[GAIN1] - 50); // GAIN -15...+15dB
        break;
      case 12:
        param[GAIN2] = (float)(FXparam[GAIN2] - 50); // GAIN -15...+15dB
        break;
      case 13:
        param[GAIN3] = (float)(FXparam[GAIN3] - 50); // GAIN -15...+15dB
        break;
      case 14:
        param[GAIN4] = (float)(FXparam[GAIN4] - 50); // GAIN -15...+15dB
        break;
      case 15:
        param[GAIN5] = (float)(FXparam[GAIN5] - 50); // GAIN -15...+15dB
        break;
      case 16:
        bqf1.setPF(param[FREQ1], param[Q1], param[GAIN1]);
        break;
      case 17:
        bqf2.setPF(param[FREQ2], param[Q2], param[GAIN2]);
        break;
      case 18:
        bqf3.setPF(param[FREQ3], param[Q3], param[GAIN3]);
        break;
      case 19:
        bqf4.setPF(param[FREQ4], param[Q4], param[GAIN4]);
        break;
      case 20:
        bqf5.setPF(param[FREQ5], param[Q5], param[GAIN5]);
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
      fxL[i] = bqf1.process(xL[i]);
      fxL[i] = bqf2.process(fxL[i]);
      fxL[i] = bqf3.process(fxL[i]);
      fxL[i] = bqf4.process(fxL[i]);
      fxL[i] = bqf5.process(fxL[i]);
  	  xL[i] = bypassL(xL[i], param[LEVEL] * fxL[i]);
    }
  }

};

#endif // FX_EQUALIZE_HPP
