#ifndef ultrasound
#define ultrasound

#include <Arduino.h>
#include <Servo.h>

bool readName(String& name);
bool servoRead(String& name, String& last_name, unsigned long& lastMillis, Servo& servoL, Servo& servoR, int& angle, bool& dir);
void setupServos(const int& pinL, const int& pinR, Servo& servoL, Servo& servoR);

#endif