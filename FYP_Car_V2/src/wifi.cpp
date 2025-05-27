#include "wifi.h"
#include <globals.h>
#include <WiFiManager.h>

#define RESET_WIFI_PIN 0  // GPIO0 = FLASH 按钮
#define LONG_PRESS_TIME 2000  // 毫秒，长按2秒触发
unsigned long buttonPressStart = 0;
bool buttonHeld = false;


void setupWiFi() {
  pinMode(RESET_WIFI_PIN, INPUT_PULLUP);  // FLASH 按钮为下拉低有效

  WiFiManager wm;

  if (digitalRead(RESET_WIFI_PIN) == LOW) {
    Serial.println("FLASH button held — resetting WiFi settings...");
    wm.resetSettings();
    bool res = wm.startConfigPortal("ESP8266-ResetAP");
    if (!res) {
      Serial.println("Failed to connect via config portal. Restarting...");
      delay(3000);
      ESP.restart();
    }
  } else {
    bool res = wm.autoConnect("ESP8266-AutoConnect");
    if (!res) {
      Serial.println("AutoConnect failed. Restarting...");
      delay(3000);
      ESP.restart();
    }
  }

  Serial.println("WiFi connected!");
  printWiFiStatus();
}

void checkWiFiResetButton() {
  static bool resetting = false; // 防止重复触发

  if (resetting) return;

  if (digitalRead(RESET_WIFI_PIN) == LOW) {
    if (!buttonHeld) {
      buttonPressStart = millis();
      buttonHeld = true;
    } else if (millis() - buttonPressStart >= LONG_PRESS_TIME) {
      Serial.println("FLASH button long-pressed — resetting WiFi...");
      resetting = true;  // 只触发一次

      WiFiManager wm;
      wm.resetSettings();
      delay(500);
      ESP.restart(); // 重启后会自动进入配置模式
    }
  } else {
    buttonHeld = false;
  }
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
