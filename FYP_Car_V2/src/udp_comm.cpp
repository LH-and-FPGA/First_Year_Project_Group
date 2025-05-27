#include "udp_comm.h"
#include <globals.h>

static unsigned long lastRxTime = 0;

void udpInit() {
  if (!Udp.begin(localUdpPort)) {
    Serial.println("UDP begin failed!");
    while (1) delay(1000);
  }
  Serial.printf("Listening UDP on port %u\n", localUdpPort);
  lastRxTime = millis();
}

void handleUdpPacket(void (*callback)(uint8_t pwmL, bool dirL, uint8_t pwmR, bool dirR)) {
  int packetSize = Udp.parsePacket();
  if (packetSize > 0) {
    lastRxTime = millis();

    uint8_t buf[32];
    int len = Udp.read(buf, sizeof(buf));
    Serial.printf("Read %d bytes: ", len);
    for (int i = 0; i < len; i++) {
      Serial.printf("%02X ", buf[i]);
    }
    Serial.println();

    int idx = 0;
    while (idx < len && buf[idx] != 0x0F) idx++;
    if (idx + 4 < len) {
      uint8_t pwmL = buf[idx+1];
      uint8_t pwmR = buf[idx+2];
      uint8_t dL   = buf[idx+3];
      uint8_t dR   = buf[idx+4];
      Serial.printf("Cmd -> L_pwm:%u R_pwm:%u dL:%u dR:%u\n", pwmL, pwmR, dL, dR);

      // 调用用户提供的回调
      if (callback) {
        callback(pwmL, dL, pwmR, dR);
      }
    } else {
      Serial.println("Malformed payload! (no 0x0F+4 bytes)");
    }
  }
}

bool udpTimeout(unsigned long timeout_ms) {
  return millis() - lastRxTime > timeout_ms;
}

void udpStop() {
  Udp.stop();
}
