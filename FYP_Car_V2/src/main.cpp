#include <Arduino.h>
#include "udp_comm.h"
#include "motor.h"
#include "wifi.h"


// put function declarations here:
int myFunction(int, int);
const char* ssid = "KChou";
const char* pass = "KCHOU0604";
bool debug = false; // Set to true for debugging without WiFi

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(9600);
  while (!Serial);

  if (!debug) {
    setupWiFi();
    udpInit();
    initMotors();
  }
}

void loop() {
  checkWiFiResetButton();

  if (!debug) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi lost. Stopping motors and retrying...");
      stopMotors();
      WiFi.begin();
      delay(1000);
      return;
    }

    handleUdpPacket([](uint8_t pwmL, bool dirL, uint8_t pwmR, bool dirR) {
      setMotorPWM(pwmL, dirL, pwmR, dirR);
    });

    if (udpTimeout(500)) {
      Serial.println("UDP timeout — stopping motors");
      stopMotors();
    }
  }

  if (debug) {

  }

  delay(20);
}


// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}