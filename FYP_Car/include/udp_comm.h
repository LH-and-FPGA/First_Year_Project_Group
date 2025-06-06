#ifndef UDP_COMM_H
#define UDP_COMM_H

#include <Arduino.h>
#include <WiFiUdp.h>

extern const unsigned int localUdpPort;

void udpInit();

void handleUdpPacket(void (*callback)(uint8_t pwmL, bool dirL, uint8_t pwmR, bool dirR));

bool udpTimeout(unsigned long timeout_ms);

void udpStop();

void sendStatusPacket();

#endif