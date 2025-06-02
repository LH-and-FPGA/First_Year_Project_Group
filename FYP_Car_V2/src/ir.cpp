#include "ir.h"
#include <climits>

volatile unsigned long IRFrequency::lastRiseTime = 0;
volatile unsigned long IRFrequency::pulseInterval = 0;
volatile bool IRFrequency::newPulse = false;

IRFrequency::IRFrequency() : count(0), lastDetectedTime(0), currentFrequency(0.0), detectedType("None") {
    memset(intervals, 0, sizeof(intervals));
}

void IRFrequency::begin() {
    pinMode(PULSE_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PULSE_PIN), onRisingEdge, RISING);
}

void IRAM_ATTR IRFrequency::onRisingEdge() {
    unsigned long currentTime = micros();
    pulseInterval = currentTime - lastRiseTime;
    lastRiseTime = currentTime;
    newPulse = true;
}

void IRFrequency::update() {
    if (newPulse) {
        noInterrupts();
        unsigned long interval = pulseInterval;
        newPulse = false;
        interrupts();

        if (interval > 0 && count < BUFFER_SIZE) {
            intervals[count++] = interval;
        }

        if (count == BUFFER_SIZE) {
            // Sort the array
            for (int i = 0; i < BUFFER_SIZE - 1; i++) {
                for (int j = i + 1; j < BUFFER_SIZE; j++) {
                    if (intervals[i] > intervals[j]) {
                        unsigned long temp = intervals[i];
                        intervals[i] = intervals[j];
                        intervals[j] = temp;
                    }
                }
            }

            // Find median
            unsigned long medianInterval;
            if (BUFFER_SIZE % 2 == 0) {
                medianInterval = (intervals[BUFFER_SIZE/2 - 1] + intervals[BUFFER_SIZE/2]) / 2;
            } else {
                medianInterval = intervals[BUFFER_SIZE/2];
            }

            currentFrequency = 1e6 / (float)medianInterval;

            if (currentFrequency > 440 && currentFrequency < 480) { // aiming for 457 Hz
                detectedType = "Wibbo";
                lastDetectedTime = millis();
            } else if (currentFrequency > 270 && currentFrequency < 310) { // aiming for 293 Hz
                detectedType = "Snorkle";
                lastDetectedTime = millis();
            } else {
                detectedType = "None";
            }
            
            count = 0; // Reset for next batch
        }
    }
}

unsigned long IRFrequency::getTimeSinceLastDetected() const {
    if (lastDetectedTime == 0) return ULONG_MAX;
    return millis() - lastDetectedTime;
}