#include <Arduino.h>
#include <ir2.h>
#define SAMPLE_RATE 4000      // 4 kHz sampling rate (sufficient for <1kHz)
#define N 128                 // Number of samples (32ms window)
#define TARGET1 293            // IR signal 1
#define TARGET2 457            // IR signal 2




float goertzel(float* data, int N, float targetFreq) {
  float omega = 2.0 * PI * targetFreq / SAMPLE_RATE;
  float coeff = 2.0 * cos(omega);
  float q0 = 0, q1 = 0, q2 = 0;

  for (int i = 0; i < N; i++) {
    q0 = coeff * q1 - q2 + data[i];
    q2 = q1;
    q1 = q0;
  }

  return q1 * q1 + q2 * q2 - coeff * q1 * q2;
}


void detect(int analogPin) {
  // Sample the signal
  for (int i = 0; i < N; i++) {
    samples[i] = analogRead(analogPin);
    delayMicroseconds(1000000 / SAMPLE_RATE);
  }

  // Run Goertzel for both target frequencies
  float power293 = goertzel(samples, N, TARGET1);
  float power457 = goertzel(samples, N, TARGET2);

  // Decide which one is stronger
  if (power293 > power457 && power293 > 1000) {
    Serial.println("Detected: 293 Hz");
  } else if (power457 > power293 && power457 > 1000) {
    Serial.println("Detected: 457 Hz");
  } else {
    Serial.println("No strong signal");
  }

  delay(300);  // Brief pause
}

