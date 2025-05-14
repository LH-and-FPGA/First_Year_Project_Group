#include <freq.h>

void freq_detection(int SIGNAL_PIN) {
  unsigned long highTime = pulseIn(SIGNAL_PIN, HIGH, 1000000L);
  unsigned long lowTime  = pulseIn(SIGNAL_PIN, LOW, 1000000L);

  if (highTime == 0 || lowTime == 0) {
    Serial.println("Signal timeout");
  } else {
    unsigned long period = highTime + lowTime;
    float frequency = 1000000.0f / period;
    Serial.print("Freq: ");
    Serial.print(frequency, 2);
    Serial.println(" Hz");
  }
  // 要做一下非阻塞
}