#include "radio_freq.h"
#include <climits>

volatile unsigned long RadioFrequency::lastRiseTime = 0;
volatile unsigned long RadioFrequency::pulseInterval = 0;
volatile bool RadioFrequency::newPulse = false;

RadioFrequency::RadioFrequency() : count(0), lastDetectedTime(0), currentFrequency(0.0), detectedType("None") {
    memset(intervals, 0, sizeof(intervals));
}

void RadioFrequency::begin() {
    pinMode(PULSE_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PULSE_PIN), onRisingEdge, RISING);
}

void RadioFrequency::onRisingEdge() {
    unsigned long currentTime = micros();
    pulseInterval = currentTime - lastRiseTime;
    lastRiseTime = currentTime;
    newPulse = true;
}

void RadioFrequency::update() {
    if (newPulse) {
        noInterrupts();
        unsigned long interval = pulseInterval;
        newPulse = false;
        interrupts();

        if (interval > 0 && count < BUFFER_SIZE) {
            intervals[count++] = interval;
        }

        if (count == BUFFER_SIZE) {
            for (int i = 0; i < BUFFER_SIZE - 1; i++) {
                for (int j = i + 1; j < BUFFER_SIZE; j++) {
                    if (intervals[i] > intervals[j]) {
                        unsigned long temp = intervals[i];
                        intervals[i] = intervals[j];
                        intervals[j] = temp;
                    }
                }
            }

            unsigned long medianInterval;
            if (BUFFER_SIZE % 2 == 0) {
                medianInterval = (intervals[BUFFER_SIZE/2 - 1] + intervals[BUFFER_SIZE/2]) / 2;
            } else {
                medianInterval = intervals[BUFFER_SIZE/2];
            }

            currentFrequency = 1e6 / (float)medianInterval;

            if (currentFrequency > 90 && currentFrequency < 110) {
                detectedType = "Gribbit";
                lastDetectedTime = millis();
            } else if (currentFrequency > 140 && currentFrequency < 160) {
                detectedType = "Zapple";
                lastDetectedTime = millis();
            } else {
                detectedType = "None";
            }
            
            count = 0;
        }
    }
}

unsigned long RadioFrequency::getTimeSinceLastDetected() const {
    if (lastDetectedTime == 0) return ULONG_MAX;
    return millis() - lastDetectedTime;
}