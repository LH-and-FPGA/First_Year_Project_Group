#include <Arduino.h>
#include <MsgPack.h>
#include <WiFi101.h>
#include <SPI.h>

const int pwmPin_right = 9;     // 用于PWM输出的引脚
const int highPin_right = 8;    // 持续输出HIGH的引脚
const int pwmPin_left = 10;     // 用于PWM输出的引脚
const int highPin_left = 11;    // 持续输出HIGH的引脚

char ssid[] = "Fred's iPhone"; // your network SSID (name)
char pass[] = "20242024"; // your network password

WiFiServer server(1234); // Using port 1234 for testing purposes

int status = WL_IDLE_STATUS; // the WiFi radio's status

// put function declarations here:
int myFunction(int, int);
void printWiFiStatus();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Check for the WiFi shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("Fuck you Wifi");
    while (true);
  }
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
    delay(1000);
  }
  Serial.println("Gooooooooooooood!");
  server.begin();
  printWiFiStatus();


  int result = myFunction(2, 3);


  pinMode(pwmPin_right, OUTPUT);
  pinMode(highPin_right, OUTPUT);

  pinMode(pwmPin_left, OUTPUT);
  pinMode(highPin_left, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  // analogWrite(pwmPin_right, 255);  // 127/255 ≈ 50% 占空比
  WiFiClient client = server.available();
  if (client) {
    Serial.print("Connected!");

    const size_t bufSize = 64;
    uint8_t buffer[bufSize];
    size_t bytesRead = 0;
    while (client.connected() && client.available() == 0);
    while (client.available() && bytesRead < bufSize) {
      buffer[bytesRead++] = client.read();
    }    


  }
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

// 打印连接状态信息
void printWiFiStatus() {
  // 打印IP地址
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // 打印信号强度
  long rssi = WiFi.RSSI();
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");
}