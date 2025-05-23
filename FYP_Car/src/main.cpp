#include <Arduino.h>
// #include <Servo.h>
// #include "ultrasound.h"
#include "ir.h"
// #include "radio.h"

// /// ultrasound stuff
// int servoPinR = 2;
// int servoPinL = 3;
// Servo servoL;
// Servo servoR;
// bool dir = false;
// int angle = 0;
// unsigned long lastMillis = 0;
// String name = "arb1";
// String last_name = "arb2";

int irPin = 2;
int freq;
unsigned long lastHigh = micros();


void setup() {
  // setupServos(servoPinL, servoPinR, servoL, servoR);
  Serial.begin(115200);
  setupIR(irPin, lastHigh, freq);
  // Serial1.begin(600);
}
  
void loop() {
  // if (servoRead(name, last_name, lastMillis, servoL, servoR, angle, dir)) {
  //   Serial.println(name);
  // }
  if (lastHigh + 1000000 > micros()) Serial.println(freq);

}
