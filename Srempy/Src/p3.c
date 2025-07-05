#include "main.h"

extern uint8_t pot[];

static void mode(uint8_t);
static float LFO1(uint8_t);
static float potRate(uint8_t);
static float potLevel(uint8_t);
static float bqEQ1(float,float,float,float);
static float bqEQ2(float,float,float,float);
static float bqEQ3(float,float,float,float);

static float bw1 = 1.0f; // EQ Bandwidth
static float bw2 = 1.0f;
static float bw3 = 1.0f;
static float  d1 = 0.0f; // EQ Depth(dB Gain)
static float  d2 = 0.0f;
static float  d3 = 0.0f;
static float vol = 1.0f; // âπó ï‚ê≥

float p3(float x)
{
  static float x1 = 0.0f;
  float fc = LFO1(pot[1]);
  x = bqEQ1(x, fc, bw1, d1);        // 100...1200 Hz
  x = bqEQ2(x, 6.0f * fc, bw2, d2); // 600...7200 Hz
  x = bqEQ3(x, 2.5f * fc, bw3, d3); // 250...3000 Hz
  if (x1 < 0.0f && x >= 0.0f) mode(pot[2]); // É[ÉçÉNÉçÉXéûMODEêÿë÷
  x1 = x;
  return potLevel(pot[0]) * vol * x;
}

void mode(uint8_t pot) // pot[2] 0...4
{
  if (pot == 0) // STD
  {
    d1  = -40.0f;
    bw1 =   0.3f;
    d2  = -15.0f;
    bw2 =   1.0f;
    d3  =   9.0f;
    bw3 =   2.0f;
    vol =   1.6f;
  }
  if (pot == 1) // SOFT
  {
    d1  = -30.0f;
    bw1 =   0.4f;
    d2  = -30.0f;
    bw2 =   0.4f;
    d3  =   0.0f;
    bw3 =   1.0f;
    vol =  1.97f;
  }
  if (pot == 2) // HIGH
  {
    d1  = -30.0f;
    bw1 =   0.4f;
    d2  =   7.0f;
    bw2 =   1.5f;
    d3  =   0.0f;
    bw3 =   1.0f;
    vol =  1.28f;
  }
  if (pot == 3) // LOW
  {
    d1  =   6.0f;
    bw1 =   1.5f;
    d2  = -30.0f;
    bw2 =   0.4f;
    d3  =   0.0f;
    bw3 =   1.0f;
    vol =   1.3f;
  }
  if (pot == 4) // INV
  {
    d1  =  20.0f;
    bw1 =   1.0f;
    d2  =  20.0f;
    bw2 =   1.0f;
    d3  = -10.0f;
    bw3 =   1.0f;
    vol =  0.47f;
  }
}

float LFO1(uint8_t pot) // 0...0.5 Triangle Wave
{
  uint32_t LFOrate = potRate(pot) * fs;
  static uint32_t LFOcount = 0;
  LFOcount++;
  if (LFOcount >= LFOrate) LFOcount = 0;
  float y = (float) LFOcount / (float) LFOrate;
  if (LFOcount > LFOrate/2) y = 1.0f - y;
  y = 100.0f * expf(4.9698f * y); // 100...1200 Hz
  return y;
}

float potRate(uint8_t pot) // pot[1] 0...100
{
  static float p = 1.0f;
  static uint8_t last_pot = 111;
  if (last_pot != pot)
  {
    last_pot = pot;
    p = 0.025f * expf(0.053f * (100.0f - (float) pot)) + 0.025f; // 0.05...5sec
  }
  return p;
}

float potLevel(uint8_t pot) // pot[0] 0...20
{
  static float p = 1.0f;
  static uint8_t last_pot = 111;
  if (last_pot != pot)
  {
    last_pot = pot;
    p = powf(10.0f, ((float) pot - 10.0f) / 20.0f); // -10...+10dB
  }
  return p;
}

float bqEQ1(float x,float fc,float bw,float gain)
{
  float alpha = bqAlphaBW(fc,bw);
  float cos = bqCosOmega(fc);
  float A = bqA(gain);

  float a0 =  1.0f + alpha / A;
  float a1 = -2.0f * cos;
  float a2 =  1.0f - alpha / A;
  float b0 =  1.0f + alpha * A;
  float b1 = -2.0f * cos;
  float b2 =  1.0f - alpha * A;

  static float x1 = 0.0f;
  static float x2 = 0.0f; 
  static float y1 = 0.0f;
  static float y2 = 0.0f;

  float y = b0 / a0 * x + b1 / a0 * x1 + b2 / a0 * x2
                        - a1 / a0 * y1 - a2 / a0 * y2;
  x2 = x1;
  x1 = x;
  y2 = y1;
  y1 = y;
  return y;
}

float bqEQ2(float x,float fc,float bw,float gain)
{
  float alpha = bqAlphaBW(fc,bw);
  float cos = bqCosOmega(fc);
  float A = bqA(gain);

  float a0 =  1.0f + alpha / A;
  float a1 = -2.0f * cos;
  float a2 =  1.0f - alpha / A;
  float b0 =  1.0f + alpha * A;
  float b1 = -2.0f * cos;
  float b2 =  1.0f - alpha * A;

  static float x1 = 0.0f;
  static float x2 = 0.0f;
  static float y1 = 0.0f;
  static float y2 = 0.0f;

  float y = b0 / a0 * x + b1 / a0 * x1 + b2 / a0 * x2
                        - a1 / a0 * y1 - a2 / a0 * y2;
  x2 = x1;
  x1 = x;
  y2 = y1;
  y1 = y;
  return y;
}

float bqEQ3(float x,float fc,float bw,float gain)
{
  float alpha = bqAlphaBW(fc,bw);
  float cos = bqCosOmega(fc);
  float A = bqA(gain);

  float a0 =  1.0f + alpha / A;
  float a1 = -2.0f * cos;
  float a2 =  1.0f - alpha / A;
  float b0 =  1.0f + alpha * A;
  float b1 = -2.0f * cos;
  float b2 =  1.0f - alpha * A;

  static float x1 = 0.0f;
  static float x2 = 0.0f;
  static float y1 = 0.0f;
  static float y2 = 0.0f;

  float y = b0 / a0 * x + b1 / a0 * x1 + b2 / a0 * x2
                        - a1 / a0 * y1 - a2 / a0 * y2;
  x2 = x1;
  x1 = x;
  y2 = y1;
  y1 = y;
  return y;
}
