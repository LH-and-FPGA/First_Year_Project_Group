#include <Arduino.h>
#include <ir.h>
#include "globals.h"
#include "udp_comm.h"
#include "motor.h"
#include "wifi.h"
#include "ultrasound.h"


// put function declarations here:
int myFunction(int, int);
const char* ssid = "KChou";
const char* pass = "KCHOU0604";
bool debug = false; // Set to true for debugging without WiFi

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 500; // 每 1000ms 发送一次

/// ultrasound stuff
int servoPinR = 2;
int servoPinL = 3;
Servo servoL;
Servo servoR;
bool dir = false;
int angle = 0;
unsigned long lastMillis = 0;
String name = "arb1";
String last_name = "arb2";

const int irPin = 2;
int freq;
unsigned long lastHigh = micros();

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(9600);
  Serial1.begin(600);
  while (!Serial);

  if (!debug) {
    setupWiFi();
    udpInit();
    initMotors();
    setupServos(servoPinL, servoPinR, servoL, servoR);
  }

  Serial.begin(115200);
  setupIR(irPin, lastHigh, freq);
}

void loop() {
  if (!debug) {
    /// Ultrasound handling
    if (servoRead(name, last_name, lastMillis, servoL, servoR, angle, dir)) {
      oh_my_duck_name_tilde = name;
      Serial.print("Duck name: ");
      Serial.println(oh_my_duck_name_tilde);
    }

    /// WiFi and UDP handling
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
    unsigned long now = millis();
    if (now - lastSendTime >= sendInterval) {
      lastSendTime = now;

      rfFrequency += 0.1f;
      irFrequency += 0.2f;
      magneticDirection = (magneticDirection + 5) % 360;

      sendStatusPacket();
    }
  }

  if (debug) {

  }

  if (lastHigh + 1000000 > micros()) Serial.println(freq);

  delay(20);
}


// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}