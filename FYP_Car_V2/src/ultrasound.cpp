#include "ultrasound.h"
#include "globals.h"

void setupServos(const int& pinR, const int& pinL, Servo& servoL, Servo& servoR) {
    servoL.attach(pinL);
    servoR.attach(pinR);
    servoL.write(90);
    servoR.write(90);
}

bool readName(String& name) {
    if (Serial1.available() > 0) {
      name = "";
      char incomingByte = Serial1.read();
      if (incomingByte == '#') {
        int i = 0;
        while (i < 3) {
            if (Serial1.available() > 0) {
                incomingByte = Serial1.read();
                name += incomingByte;
                i++;
            }
        }
        Serial.print("FOUND: ");Serial.println(name);
        Serial1.flush();
        return true;
      }
      Serial1.flush();
    }
    return false;
}

bool servoRead(String& name, String& last_name, unsigned long& lastMillis, Servo& servoL, Servo& servoR, int& angle, bool& dir) {

    if (readName(name)) {
        if (last_name == name) return true;
        last_name = name;
        if (readName(name)) {
            if (name == last_name) return true;
        }
    }
    unsigned long m = millis();

    if (m > lastMillis + 300 && last_name != name) {
        if (dir) {
            if (angle > 0) angle -= 10;
            else {
                dir = !dir;
                angle += 10;
            }
        } else {
            if (angle < 40) angle += 10;
            else {
                dir = !dir;
                angle -= 10;
            }
        }
        servoL.write(95+angle);
        servoR.write(95-angle);

        lastMillis = m;
    }
    return false;
}