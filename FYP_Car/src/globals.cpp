#include "globals.h"

WiFiUDP Udp;
const unsigned int localUdpPort = 1234;

float rfFrequency = 0.0f;
float irFrequency = 0.0f;
int magneticDirection = 0;
String oh_my_duck_name_tilde = "Metro";