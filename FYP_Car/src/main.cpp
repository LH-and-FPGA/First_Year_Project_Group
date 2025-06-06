#include <Arduino.h>
#include <WiFi101.h>
#include "globals.h"
#include "udp_comm.h"
#include "motor.h"
#include "wifi_car.h"
#include "ultrasound.h"
#include "radio_freq.h"

bool debug = false;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 500;

int servoPinR = 5;
int servoPinL = 6;
Servo servoL;
Servo servoR;
bool dir = false;
int angle = 0;
unsigned long lastMillis = 0;
String name = "arb1";
String last_name = "arb2";

const int irPin = 0;
int freq;
unsigned long lastHigh = micros();

RadioFrequency radioFreq;

void setup() {
  Serial.begin(9600);
  Serial1.begin(600);
  while (!Serial && millis() < 3000);

  if (!debug) {
    setupWiFi();
    udpInit();
    initMotors();
    setupServos(servoPinL, servoPinR, servoL, servoR);
    radioFreq.begin();
  }

  if (debug) {
    radioFreq.begin();
  }
}

void loop() {
  if (!debug) {
    if (servoRead(name, last_name, lastMillis, servoL, servoR, angle, dir)) {
      oh_my_duck_name_tilde = name;
      Serial.print("Duck name: ");
      Serial.println(oh_my_duck_name_tilde);
    }

    if (lastHigh + 1000000 > micros()) {
      Serial.println(freq);
      irFrequency = freq;
    }

    radioFreq.update();
    if (radioFreq.isDetected()) {
      rfFrequency = radioFreq.getFrequency();
      Serial.print("Radio Freq: ");
      Serial.print(radioFreq.getDetectedType());
      Serial.print(" (");
      Serial.print(rfFrequency, 2);
      Serial.println(" Hz)");
    } else {
      rfFrequency = 0.0;
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi lost. Stopping motors and retrying...");
      stopMotors();
      tryReconnectWiFi();
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
      magneticDirection = (magneticDirection + 5) % 360;
      sendStatusPacket();
    }
  }

  if (debug) {
    radioFreq.update();
    if (radioFreq.isDetected()) {
      rfFrequency = radioFreq.getFrequency();
      Serial.print("Radio Freq: ");
      Serial.print(radioFreq.getDetectedType());
      Serial.print(" (");
      Serial.print(rfFrequency, 2);
      Serial.println(" Hz)");
    } else {
      rfFrequency = 0.0;
      Serial.println("No radio frequency detected");
    }
  }
  
  delay(20);
}