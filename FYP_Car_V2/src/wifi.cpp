#include "wifi.h"
#include <globals.h>
#include <WiFiManager.h>


static const bool forceResetWiFi = true;

void setupWiFi() {
  WiFiManager wm;

  if (forceResetWiFi) {
    Serial.println("forceResetWiFi=true — resetting WiFi settings and entering AP mode...");
    wm.resetSettings();
    // 这里启动一个配置热点，SSID 你可以根据需要改
    if (!wm.startConfigPortal("ESP8266-ResetAP")) {
      Serial.println("Config portal failed, rebooting...");
      delay(3000);
      ESP.restart();
    }
  }
  else {
    // 正常流程：先尝试自动连接
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
