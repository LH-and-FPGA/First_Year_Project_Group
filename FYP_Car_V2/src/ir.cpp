#include "ir.h"
#include <Arduino.h>

// make these visible everywhere:
volatile unsigned long lastHigh = 0;
volatile int freq = 0;

ICACHE_RAM_ATTR
void onIRRising() {
  unsigned long now = micros();
  // avoid divide-by-zero on the first call
  if (lastHigh != 0) {
    freq = 1000000UL / (now - lastHigh);
  }
  lastHigh = now;
}

void setupIR(const int& pin) {
    pinMode(pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(pin), onIRRising, RISING);
}

void stopIR(const int& pin) {
    detachInterrupt(digitalPinToInterrupt(pin));
}