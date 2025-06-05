#include "radio.h"
#include <Arduino.h>

void onRadioRising() {
    extern unsigned long lastRadioHigh;
    extern int radioFreq;
    unsigned long time = micros();
    radioFreq = 1e6/(time - lastRadioHigh);
    lastRadioHigh = time;
}

void setupRadio(const int& radioPin, int& radioFreq, unsigned long& lastRadioHigh) {
    attachInterrupt(digitalPinToInterrupt(radioPin), onRadioRising, RISING);
}

void stopRadio(const int& radioPin) {
    detachInterrupt(digitalPinToInterrupt(radioPin));
}