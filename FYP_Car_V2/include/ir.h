#ifndef IR_FREQ_H
#define IR_FREQ_H

#include <Arduino.h>

class IRFrequency {
private:
    static const byte PULSE_PIN = D1;
    static const int BUFFER_SIZE = 10;
    
    static volatile unsigned long lastRiseTime;
    static volatile unsigned long pulseInterval;
    static volatile bool newPulse;
    
    unsigned long intervals[BUFFER_SIZE];
    int count;
    unsigned long lastDetectedTime;
    
    float currentFrequency;
    String detectedType;
    
public:
    IRFrequency();
    void begin();
    void update();
    
    float getFrequency() const { return currentFrequency; }
    String getDetectedType() const { return detectedType; }
    unsigned long getTimeSinceLastDetected() const;
    bool isDetected() const { return lastDetectedTime > 0 && getTimeSinceLastDetected() < 1000; }
    
    static void IRAM_ATTR onRisingEdge();
};

#endif