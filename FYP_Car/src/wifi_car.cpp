#include "wifi_car.h"
#include "globals.h"
#include <WiFi101.h>

char ssid[] = "dianboxi";
char pass[] = "guangsuqulvfeichuan";

void setupWiFi() {
  Serial.println("Connecting to WiFi...");
  
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }
  
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  
  Serial.println("WiFi connected!");
  printWiFiStatus();
}

bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void tryReconnectWiFi() {
  if (!isWiFiConnected()) {
    Serial.println("WiFi lost. Trying to reconnect...");
    WiFi.begin(ssid, pass);
    delay(1000);
  }
}

void printWiFiStatus() {
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}