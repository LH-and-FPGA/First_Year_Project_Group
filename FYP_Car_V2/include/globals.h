#pragma once

#include <Arduino.h>
#include <wifi.h>
#include <udp_comm.h>
#include <motor.h>

extern WiFiUDP Udp;
extern const unsigned int localUdpPort;

extern float rfFrequency;
extern float irFrequency;
extern int magneticDirection;
extern String oh_my_duck_name_tilde;

