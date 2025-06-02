#include <Arduino.h>
#include "globals.h"
#include "udp_comm.h"
#include "motor.h"
#include "wifi.h"
#include "ultrasound.h"
#include "radio_freq.h"


// put function declarations here:
int myFunction(int, int);
const char* ssid = "KChou";
const char* pass = "KCHOU0604";
bool debug = false; // Set to true for debugging without WiFi

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 500;

/// ultrasound stuff
int servoPinR = D5;
int servoPinL = D6;
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

// Radio frequency detector
RadioFrequency radioFreq;

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
    radioFreq.begin();
  }

  if (debug) {
    radioFreq.begin();
  }
}

void loop() {
  if (!debug) {
    /// Ultrasound handling
    if (servoRead(name, last_name, lastMillis, servoL, servoR, angle, dir)) {
      oh_my_duck_name_tilde = name;
      Serial.print("Duck name: ");
      Serial.println(oh_my_duck_name_tilde);
    }

    /// IR handling
    if (lastHigh + 1000000 > micros()) {
      Serial.println(freq);
      irFrequency = freq;
    }

    /// Radio frequency handling
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

      // rfFrequency is now updated by radio frequency detector
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


// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}