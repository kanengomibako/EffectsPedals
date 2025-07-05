#ifndef FX_REVERB_HPP
#define FX_REVERB_HPP

#include "common.h"
#include "calc.hpp"
#include "filter.hpp"
#include "delay_prime_num.hpp"

class fx_reverb : public fx_base
{
private:
  enum paramName {LEVEL, MIX, FBACK, HICUT, LOCUT, HIDUMP,
    P6,P7,P8,P9,P10,P11,P12,P13,P14,P15,P16,P17,P18,P19};
  float param[20] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const uint16_t paramMax[20] = { 70, 100, 99, 99, 99, 99,
      1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  const uint16_t paramMin[20] = { 30, 0, 0, 10, 1, 10,
      0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  const std::string paramName[20] = {
      "LEVEL", "MIX", "F.BACK",
      "HI-CUT", "LO-CUT", "HI-DUMP"
      "","","","","","","","","","","","","",""};
  const uint8_t pageMax = 0;
  const uint8_t dt[10] = { // ディレイタイム配列
      44, 26, 19, 16, 8, 4, 59, 69, 75, 86};

  del_buf_prime_num del[10];
  lpf lpfIn, lpfFB[4];
  hpf hpfOutL, hpfOutR;

public:
  fx_reverb()
  {
    FXnameList[RV] = "PUREVERB"; // Pure Data [rev2~] に基づいたFDNリバーブ
    FXcolorList[RV] = ILI9341_COLOR565(192, 192, 192);
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
    for (int i = 0; i < 10; i++) del[i].set(dt[i]); // 最大ディレイタイム設定
  }

  virtual void deinit()
  {
    for (int i = 0; i < 10; i++) del[i].set(1);
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
        break;
      case HICUT:
        FXparamStr[HICUT] = std::to_string(FXparam[HICUT]);
        FXparamStr[HICUT] += "00Hz";
        break;
      case LOCUT:
        FXparamStr[LOCUT] = std::to_string(FXparam[LOCUT]);
        FXparamStr[LOCUT] += "0Hz";
        break;
      case HIDUMP:
        FXparamStr[HIDUMP] = std::to_string(FXparam[HIDUMP]);
        FXparamStr[HIDUMP] += "00Hz";
        break;
      default:
        break;
    }
  }

  virtual void setParam()
  {
    static uint8_t count = 0;
    count = clipiCirc(count + 1, 0, 12); // 負荷軽減のためパラメータ計算を分散させる
    switch(count)
    {
      case 0:
        param[LEVEL] = dbtovol((float)(FXparam[LEVEL] - 50)); // OUTPUT LEVEL -20...+20dB
        break;
      case 1:
        param[MIX] = mixpot(FXparam[MIX], -20.0f); // MIX
        break;
      case 2:
        param[FBACK] = (float)FXparam[FBACK] / 200.0f; // Feedback 0～0.495
        break;
      case 3:
        param[HICUT] = (float)FXparam[HICUT] * 100.0f; // HI CUT FREQ 1000 ~ 9900 Hz
        break;
      case 4:
        param[LOCUT] = (float)FXparam[LOCUT] * 10.0f; // LOW CUT FREQ 10 ~ 990 Hz
        break;
      case 5:
        param[HIDUMP] = (float)FXparam[HIDUMP] * 100.0f; // Feedback HI CUT FREQ 1000 ~ 9900 Hz
        break;
      case 6:
        lpfIn.set(param[HICUT]);
        break;
      case 7:
        hpfOutL.set(param[LOCUT]);
        break;
      case 8:
        hpfOutR.set(param[LOCUT]);
        break;
      case 9:
        lpfFB[0].set(param[HIDUMP]);
        break;
      case 10:
        lpfFB[1].set(param[HIDUMP]);
        break;
      case 11:
        lpfFB[2].set(param[HIDUMP]);
        break;
      case 12:
        lpfFB[3].set(param[HIDUMP]);
        break;
      default:
        break;
    }
  }

  virtual void process(float xL[], float xR[])
  {
    float fxL[BLOCK_SIZE] = {};
    float fxR[BLOCK_SIZE] = {};

    float ap, am, bp, bm, cp, cm, dp, dm, ep, em,
    fp, fm, gp, gm, hd, id, jd, kd, out_l, out_r;

    setParam();

    for (uint16_t i = 0; i < BLOCK_SIZE; i++)
    {
      fxL[i] = 0.25f * lpfIn.process(xL[i]);

      // Early Reflection
      del[0].write(fxL[i]);
      ap = fxL[i] + del[0].read_fixed();
      am = fxL[i] - del[0].read_fixed();
      del[1].write(am);
      bp = ap + del[1].read_fixed();
      bm = ap - del[1].read_fixed();
      del[2].write(bm);
      cp = bp + del[2].read_fixed();
      cm = bp - del[2].read_fixed();
      del[3].write(cm);
      dp = cp + del[3].read_fixed();
      dm = cp - del[3].read_fixed();
      del[4].write(dm);
      ep = dp + del[4].read_fixed();
      em = dp - del[4].read_fixed();
      del[5].write(em);

      // Late Reflection & High Freq Dumping
      hd = del[6].read_fixed();
      hd = lpfFB[0].process(hd);

      id = del[7].read_fixed();
      id = lpfFB[1].process(id);

      jd = del[8].read_fixed();
      jd = lpfFB[2].process(jd);

      kd = del[9].read_fixed();
      kd = lpfFB[3].process(kd);

      out_l = ep + hd * param[FBACK];
      out_r = del[5].read_fixed() + id * param[FBACK];

      fp = out_l + out_r;
      fm = out_l - out_r;
      gp = jd * param[FBACK] + kd * param[FBACK];
      gm = jd * param[FBACK] - kd * param[FBACK];
      del[6].write(fp + gp);
      del[7].write(fm + gm);
      del[8].write(fp - gp);
      del[9].write(fm - gm);

      fxL[i] = (1.0f - param[MIX]) * xL[i] + param[MIX] * hpfOutL.process(out_l);
      fxR[i] = (1.0f - param[MIX]) * xL[i] + param[MIX] * hpfOutR.process(out_r);

      xL[i] = bypassL(xL[i], param[LEVEL] * fxL[i]);
      xR[i] = bypassR(xL[i], param[LEVEL] * fxR[i]);
    }
  }

};

#endif // FX_REVERB_HPP
