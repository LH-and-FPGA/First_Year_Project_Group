#include "radio.h"
#include <Arduino.h>

volatile int f[10], i = 0;
volatile unsigned long l;

void onRadioRising() {
  extern int lastRadioHigh;
  extern int radioFreq;
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
    radioFreq = (a[4] + a[5]) / 2;
  }
  lastRadioHigh = t;
}

void setupRadio(int p) {
  attachInterrupt(digitalPinToInterrupt(p), onRadioRising, RISING);
}

void stopRadio(int p) {
  detachInterrupt(digitalPinToInterrupt(p));
}
