#ifndef WIFI_CAR_H
#define WIFI_CAR_H

#include <Arduino.h>
#include <WiFi101.h>
#include <WiFiUdp.h>

extern WiFiUDP Udp;

void setupWiFi();
bool isWiFiConnected();
void tryReconnectWiFi();
void printWiFiStatus();

#endif