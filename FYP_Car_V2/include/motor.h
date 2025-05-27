#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>

// 初始化电机引脚
void initMotors();

// 设置电机速度和方向
void setMotorPWM(uint8_t pwmL, bool dirL, uint8_t pwmR, bool dirR);

// 停止所有电机
void stopMotors();

#endif
