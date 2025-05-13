#include <Arduino.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <SPI.h>

// —— 硬件接线参数 ——
const int pwmPin_right = 9;
const int dirPin_right = 8;
const int pwmPin_left  = 0;
const int dirPin_left  = 1;
const int ProxSensor   = 3;

// —— WiFi & UDP 设置 ——
char ssid[] = "Marios";
char pass[] = "gamomanes123";
WiFiUDP Udp;
const unsigned int localUdpPort = 1234;

// —— 超时停机设置 ——
unsigned long lastRxTime = 0;
const unsigned long RX_TIMEOUT = 500;  // ms

void printWiFiStatus() {
  IPAddress ip = WiFi.localIP();
  Serial.print("IP: "); Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("RSSI: "); Serial.print(rssi); Serial.println(" dBm");
}

void setup() {
  Serial.begin(9600);
  while (!Serial);  // 等待 USB 串口就绪（仅原生 USB 板）

  // 连接 WiFi
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }
  Serial.println("WiFi connected!");
  printWiFiStatus();

  // 开始监听 UDP
  if (!Udp.begin(localUdpPort)) {
    Serial.println("UDP begin failed!");
    while (1) delay(1000);
  }
  Serial.printf("Listening UDP on port %u\n", localUdpPort);

  // 设置输出脚
  pinMode(pwmPin_left,  OUTPUT);
  pinMode(dirPin_left,  OUTPUT);
  pinMode(pwmPin_right, OUTPUT);
  pinMode(dirPin_right, OUTPUT);

  lastRxTime = millis();

  pinMode(3 , OUTPUT);
}

void loop() {
  // —— 1. WiFi 掉线检测（可选） ——  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost. Stopping motors and retrying WiFi...");
    // 立刻停机
    analogWrite(pwmPin_left,  0);
    digitalWrite(dirPin_left,  LOW);
    analogWrite(pwmPin_right, 0);
    digitalWrite(dirPin_right, LOW);
    // 然后尝试重连
    if (WiFi.begin(ssid, pass) == WL_CONNECTED) {
      Serial.println("WiFi reconnected!");
      printWiFiStatus();
      lastRxTime = millis();
    }
    delay(1000);
    return;  // this iteration done
  }

  // —— 2. 检查有没有 UDP 包 ——  
  int packetSize = Udp.parsePacket();
  Serial.print("parsePacket() -> "); Serial.println(packetSize);

  if (packetSize > 0) {
    // 收到包，更新超时计时
    lastRxTime = millis();

    // 读数据
    uint8_t buf[32];
    int len = Udp.read(buf, sizeof(buf));
    Serial.printf("Read %d bytes: ", len);
    for (int i = 0; i < len; i++) {
      Serial.printf("%02X ", buf[i]);
    }
    Serial.println();

    // 找起始字节 0x0F
    int idx = 0;
    while (idx < len && buf[idx] != 0x0F) idx++;
    if (idx + 4 < len) {
      uint8_t pwmL = buf[idx+1];
      uint8_t pwmR = buf[idx+2];
      uint8_t dL   = buf[idx+3];
      uint8_t dR   = buf[idx+4];
      Serial.printf("Cmd -> L_pwm:%u R_pwm:%u dL:%u dR:%u\n",
                    pwmL, pwmR, dL, dR);
      // 驱动
      analogWrite(pwmPin_left,  pwmL);
      digitalWrite(dirPin_left,  dL ? HIGH : LOW);
      analogWrite(pwmPin_right, pwmR);
      digitalWrite(dirPin_right, dR ? HIGH : LOW);
    } else {
      Serial.println("Malformed payload! (no 0x0F+4 bytes)");
    }
  }
  // —— 3. 超时停机 ——  
  else if (millis() - lastRxTime > RX_TIMEOUT) {
    Serial.println("RX timeout — stopping motors");
    analogWrite(pwmPin_left,  0);
    digitalWrite(dirPin_left,  LOW);
    analogWrite(pwmPin_right, 0);
    digitalWrite(dirPin_right, LOW);
    // 注意：不要重置 lastRxTime！让它继续算
  }

  // —— 4. 小延时 ——  
  delay(20);
}
