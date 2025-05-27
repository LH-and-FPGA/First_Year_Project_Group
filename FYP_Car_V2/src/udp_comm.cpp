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

void sendStatusPacket() { // msgpack
  // 打包数据
  uint8_t buffer[64];
  int index = 0;

  // 添加识别符
  buffer[index++] = 0xA5;

  // RF Frequency (float)
  memcpy(&buffer[index], &rfFrequency, sizeof(float));
  index += sizeof(float);

  // IR Frequency (float)
  memcpy(&buffer[index], &irFrequency, sizeof(float));
  index += sizeof(float);

  // Magnetic Direction (int)
  memcpy(&buffer[index], &magneticDirection, sizeof(int));
  index += sizeof(int);

  // Device Name (String, 16 bytes max)
  int nameLen = deviceName.length();
  nameLen = nameLen > 15 ? 15 : nameLen;
  memcpy(&buffer[index], deviceName.c_str(), nameLen);
  index += nameLen;
  buffer[index++] = '\0';  // null terminator

  // 发送数据（目标IP和端口需设置，示例为广播）
  IPAddress remoteIp = IPAddress(255, 255, 255, 255);
  unsigned int remotePort = 1235;

  Udp.beginPacket(remoteIp, remotePort);
  Udp.write(buffer, index);
  Udp.endPacket();
}
