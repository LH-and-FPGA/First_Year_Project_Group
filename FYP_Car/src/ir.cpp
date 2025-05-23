#include <Arduino.h>
#include "ir.h"

void onIRRising() {
    extern unsigned long lastHigh;
    extern int freq;
    unsigned long time = micros();
    freq = 1e6/(time - lastHigh);
    lastHigh = time;
}

void setupIR(const int& pin, unsigned long& lastHigh, int& freq) {
    attachInterrupt(digitalPinToInterrupt(pin), onIRRising, RISING);
}

void stopIR(const int& pin) {
    detachInterrupt(digitalPinToInterrupt(pin));
}