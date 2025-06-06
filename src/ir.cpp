#include <Arduino.h>
#include "ir.h"

volatile int f[10], i = 0;
volatile unsigned long l;

void onIRRising() {
  extern int lastIRHigh;
  extern int IRfreq;
  unsigned long t = micros();
  f[i++] = 1e6 / (t - l);
  l = t;
  if (i == 10) {
    i = 0;
    int a[10];
    for (int j = 0; j < 10; ++j) a[j] = f[j];
    for (int j = 0; j < 9; ++j)
      for (int k = 0; k < 9 - j; ++k)
        if (a[k] > a[k + 1]) {
          int tmp = a[k]; a[k] = a[k + 1]; a[k + 1] = tmp;
        }
    IRfreq = (a[4] + a[5]) / 2;
  }
  lastIRHigh = t;
}

void setupIR(int p) {
  attachInterrupt(digitalPinToInterrupt(p), onIRRising, RISING);
}

void stopIR(int p) {
  detachInterrupt(digitalPinToInterrupt(p));
}