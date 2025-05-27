#ifndef UDP_COMM_H
#define UDP_COMM_H

#include <Arduino.h>
#include <WiFiUdp.h>

// UDP 端口
extern const unsigned int localUdpPort;

// UDP 初始化函数
void udpInit();

// 处理 UDP 数据包（需放在 loop 中）
void handleUdpPacket(void (*callback)(uint8_t pwmL, bool dirL, uint8_t pwmR, bool dirR));

// 更新时间戳检查（是否超时）
bool udpTimeout(unsigned long timeout_ms);

// 停止接收（可选）
void udpStop();

void sendStatusPacket();

#endif
