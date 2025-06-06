#include "motor.h"

const int pwmPin_left  = 0;
const int dirPin_left  = 1;
const int pwmPin_right = 9;
const int dirPin_right = 8;

void initMotors() {
  pinMode(pwmPin_left, OUTPUT);
  pinMode(dirPin_left, OUTPUT);
  pinMode(pwmPin_right, OUTPUT);
  pinMode(dirPin_right, OUTPUT);
  stopMotors();
}

void setMotorPWM(uint8_t pwmL, bool dirL, uint8_t pwmR, bool dirR) {
  analogWrite(pwmPin_left, pwmL);
  digitalWrite(dirPin_left, dirL ? HIGH : LOW);

  analogWrite(pwmPin_right, pwmR);
  digitalWrite(dirPin_right, dirR ? HIGH : LOW);
}

void stopMotors() {
  analogWrite(pwmPin_left, 0);
  digitalWrite(dirPin_left, LOW);
  analogWrite(pwmPin_right, 0);
  digitalWrite(dirPin_right, LOW);
}