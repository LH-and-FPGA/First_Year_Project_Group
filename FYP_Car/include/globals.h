#ifndef __GLOABALS_H__
#define __GLOABALS_H__

#include <Arduino.h>
#include <MsgPack.h>

struct MotorCommand {
    int16_t pwm_left;
    int16_t pwm_right;
    int16_t dir_left;
    int16_t dir_right;

    MSGPACK_DEFINE(pwm_left, pwm_right, dir_left, dir_right);
};

extern MotorCommand motorCommand; // Declare the motor command structure

#endif