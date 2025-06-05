#include <Arduino.h>
#include "ir.h"

void onIRRising() {
    extern unsigned long lastIRHigh;
    extern int IRfreq;
    unsigned long time = micros();
    IRfreq = 1e6 / (time - lastIRHigh);
    lastIRHigh = time;
}

void setupIR(const int& pin, unsigned long& lastIRHigh) {
    attachInterrupt(digitalPinToInterrupt(pin), onIRRising, RISING);
}

void stopIR(const int& pin) {
    detachInterrupt(digitalPinToInterrupt(pin));
}
