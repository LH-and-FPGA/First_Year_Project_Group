#include <Arduino.h>
#include "radio.h"
#include <Arduino.h>
#include <vector>
#include "ir.h"

int radioPin = 1;
int radioFreq=0;
int MagnetPin = 0;
unsigned long lastRadioHigh=micros();
float ambient = 0.0;
int irPin = 2;
int IRfreq;
unsigned long lastIRHigh = micros();

void get_ambient_reading(){
  for (int i=0; i<50; i++){
    ambient += 10*analogRead(MagnetPin);
    delay(1);
  }
  ambient=ambient/50;
}

void setup() {
  get_ambient_reading();
  setupRadio(radioPin, radioFreq, lastRadioHigh);
  setupIR(irPin, lastIRHigh);
  Serial.begin(9600);
}

float get_magnet_direction(){
  float val1 = 0;
  for (int i=0; i<50; i++){
    val1 += 10*analogRead(0);
    delay(1);
  }
  val1=val1/50;
  return val1;
}

void printDetectionResult(const String& signal, const String& field) {
  Serial.println(signal);
  Serial.println("Radio: " + String(radioFreq));
  Serial.println("Infrared: " + String(IRfreq));
  Serial.println("Magnet: " + field);
}

void loop() {
  float magnet_direction = get_magnet_direction();
  String field;

  if (magnet_direction >= ambient + 25) {
    field = "Up";
  } else if (magnet_direction <= ambient - 15) {
    field = "Down";
  } else {
    field = "None";
  }

  // Determine signal
  if ((radioFreq > 140 && radioFreq < 160)) {
    printDetectionResult("Zapple", field);
  } else if ((IRfreq > 273 && IRfreq < 313)) {
    printDetectionResult("Snorkle", field);
  } else if ((radioFreq > 90 && radioFreq < 110)) {
    printDetectionResult("Gribbit", field);
  } else if ((IRfreq > 437 && IRfreq < 477)) {
    printDetectionResult("Wibbo", field);
  } else {
    printDetectionResult("Nothing detected", field + (field == "None" ? "" : " (False Positive)"));
  }

  // Timeout checks
  if (micros() - lastRadioHigh > 100000) radioFreq = 0;
  if (micros() - lastIRHigh > 100000) IRfreq = 0;
}
