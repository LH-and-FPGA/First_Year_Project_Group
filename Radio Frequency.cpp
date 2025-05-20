#include <Arduino.h>

const byte pulsePin = 2;

volatile unsigned long lastRiseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile bool newPulse = false;

const int bufferSize = 10;
unsigned long intervals[bufferSize];
int count = 0;
unsigned long lastMillis = 0;

void onRisingEdge() {
  unsigned long currentTime = micros();
  pulseInterval = currentTime - lastRiseTime;
  lastRiseTime = currentTime;
  newPulse = true;
}

void detected() {
  lastMillis = millis();
}

unsigned long lastDetected() {
  return millis() - lastMillis;
}

void setup() {
  pinMode(pulsePin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsePin), onRisingEdge, RISING);
  Serial.begin(9600);
}

void loop() {
  if (newPulse) {
    noInterrupts();
    unsigned long interval = pulseInterval;
    newPulse = false;
    interrupts();

    if (interval > 0 && count < bufferSize) {
      intervals[count++] = interval;
    }

    if (count == bufferSize) {
      // Sort the array
      for (int i = 0; i < bufferSize - 1; i++) {
        for (int j = i + 1; j < bufferSize; j++) {
          if (intervals[i] > intervals[j]) {
            unsigned long temp = intervals[i];
            intervals[i] = intervals[j];
            intervals[j] = temp;
          }
        }
      }

      // Find median
      unsigned long medianInterval;
      if (bufferSize % 2 == 0) {
        medianInterval = (intervals[bufferSize/2 - 1] + intervals[bufferSize/2]) / 2;
      } else {
        medianInterval = intervals[bufferSize/2];
      }

      float frequency = 1e6 / (float)medianInterval;

      if (frequency > 90 && frequency < 110) {
        Serial.println("Gribbit (" + String(frequency, 2) + " Hz)");
        detected();
      }

      else if (frequency > 140 && frequency < 160) {
        Serial.println("Zapple (" + String(frequency, 2) + " Hz)");
        detected();
      }


      else{
        Serial.println("No frequency match");
      }
      count = 0; // Reset for next batch
    }
  }
}
