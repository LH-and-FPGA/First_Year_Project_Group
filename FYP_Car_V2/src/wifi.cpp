#include "wifi.h"
#include <globals.h>
#include <WiFiManager.h>


static const bool forceResetWiFi = false;

void setupWiFi() {
  WiFiManager wm;

  if (forceResetWiFi) {
    Serial.println("forceResetWiFi=true — resetting WiFi settings and entering AP mode...");
    wm.resetSettings();
    if (!wm.startConfigPortal("ESP8266-ResetAP")) {
      Serial.println("Config portal failed, rebooting...");
      delay(3000);
      ESP.restart();
    }
  }
  else {
    if (!wm.autoConnect("ESP8266-AutoConnect")) {
      Serial.println("AutoConnect failed. Restarting...");
      delay(3000);
      ESP.restart();
    }
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
    WiFi.disconnect();
    WiFi.begin();
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
