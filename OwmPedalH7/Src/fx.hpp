#ifndef FX_HPP
#define FX_HPP

#include "common.h"

void FXinit();

void FXdeinit();

void FXsetParamStr(uint8_t paramNum);

void FXprocess(float xL[], float xR[]);

#endif // FX_HPP
