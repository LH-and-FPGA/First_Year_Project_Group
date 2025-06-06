#ifndef __IR_H__
#define __IR_H__

#include <Arduino.h>

void setupIR(const int& pin, unsigned long& lastHigh, int& freq);
void stopIR(const int& pin);
void onIRRising();

#endif