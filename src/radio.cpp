#include "radio.h"
#include <Arduino.h>

volatile int fradio[10], iradio = 0;
volatile unsigned long lradio;

void onRadioRising() {
  extern int lastRadioHigh;
  extern int radioFreq;
  unsigned long t = micros();
  fradio[iradio++] = 1e6 / (t - lradio);
  lradio = t;
  if (iradio == 10) {
    iradio = 0;
    int a[10];
    for (int j = 0; j < 10; ++j) a[j] = fradio[j];
    for (int j = 0; j < 9; ++j)
      for (int k = 0; k < 9 - j; ++k)
        if (a[k] > a[k + 1]) {
          int tmp = a[k]; a[k] = a[k + 1]; a[k + 1] = tmp;
        }
    radioFreq = (a[4] + a[5]) / 2;
  }
  lastRadioHigh = t;
}

void setupRadio(const int& pin, int& radioFreq, unsigned long& lastRadioHigh) {
  attachInterrupt(digitalPinToInterrupt(pin), onRadioRising, RISING);
}

void stopRadio(const int& pin) {
  detachInterrupt(digitalPinToInterrupt(pin));
}
