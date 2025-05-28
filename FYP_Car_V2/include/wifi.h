#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

extern WiFiUDP Udp;

void setupWiFi();
bool isWiFiConnected();
void tryReconnectWiFi();
void printWiFiStatus();

#endif
