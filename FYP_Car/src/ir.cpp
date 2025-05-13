#include "ir.h"

void detectIR(int ProxSensor) {
    int sensorValue = analogRead(ProxSensor);
    Serial.print("Proximity Sensor Value: ");
    Serial.println(sensorValue);

    if (sensorValue < 100) {
        Serial.println("Obstacle detected!");
    } else {
        Serial.println("No obstacle detected.");
    }
}