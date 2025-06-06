#include <Arduino.h>
#include "ir.h"

volatile int fir[10], iir = 0;
volatile unsigned long lir;

void onIRRising() {
  extern int lastIRHigh;
  extern int IRfreq;
  unsigned long t = micros();
  fir[iir++] = 1e6 / (t - lir);
  lir = t;
  if (iir == 10) {
    iir = 0;
    int a[10];
    for (int j = 0; j < 10; ++j) a[j] = fir[j];
    for (int j = 0; j < 9; ++j)
      for (int k = 0; k < 9 - j; ++k)
        if (a[k] > a[k + 1]) {
          int tmp = a[k]; a[k] = a[k + 1]; a[k + 1] = tmp;
        }
    IRfreq = (a[4] + a[5]) / 2;
  }
  lastIRHigh = t;
}

void setupIR(const int& pin, unsigned long& lastIRHigh) {
  attachInterrupt(digitalPinToInterrupt(pin), onIRRising, RISING);
}

void stopIR(const int& pin) {
  detachInterrupt(digitalPinToInterrupt(pin));
}