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


void loop() {
  float magnet_direction;
  magnet_direction = get_magnet_direction();
  if(magnet_direction>=ambient+25){
    if ((radioFreq < 160) && (radioFreq > 140)){
      Serial.println("Zapple\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nMagnet Up");
    }
    else if((IRfreq < 303) && (IRfreq > 283)){
      Serial.println("Snorkle\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nMagnet Up");
    }else{
      Serial.println("Nothing detected\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nMagnet Up(False Positive)");
    }
  }else if(magnet_direction<=ambient-15){
    if ((radioFreq < 110) && (radioFreq > 90)){
      Serial.println("Gribbit\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nMagnet Down");
    }
    else if((IRfreq < 467) && (IRfreq > 447)){
      Serial.println("Wibbo\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nMagnet Down");
    }else{
        Serial.println("Nothing detected\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nMagnet Down(False Positive)");
    }
  }else{
     if ((radioFreq < 110) && (radioFreq > 90)){
      Serial.println("Gribbit\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nNo magnetic field detected");
    }
    else if((radioFreq < 160) && (radioFreq > 140)){
      Serial.println("Zapple\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nNo magnetic field detected");
    }else if((IRfreq < 303) && (IRfreq > 283)){
      Serial.println("Snorkle\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nNo magnetic field detected");
    }else if((IRfreq < 467) && (IRfreq > 447)){
      Serial.println("Wibbo\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nNo magnetic field detected");
    }else{
      Serial.println("Nothing detected\nRadio: " + String(radioFreq) + "\nInfrared: " + String(IRfreq) + "\nNo magnetic field detected");
    }
  }

  if (micros() - lastRadioHigh > 100000) {
    radioFreq = 0;
  }
  if (micros() - lastIRHigh > 100000){
    IRfreq = 0;
  }
}
