#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

void initMotors();

void setMotorPWM(uint8_t pwmL, bool dirL, uint8_t pwmR, bool dirR);

void stopMotors();

#endif