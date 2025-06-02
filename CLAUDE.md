# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a distributed robotics system for autonomous/remotely controlled robot cars (referred to as "ducks"). The project consists of:
- **FYP_Car/**: Firmware for Adafruit Metro M0 (ARM SAMD21) robot car
- **FYP_Car_V2/**: Firmware for NodeMCU v2 (ESP8266) robot car  
- **First_Year_Project_Host/**: Python control station with GUI

## Build and Run Commands

### Embedded Firmware (FYP_Car and FYP_Car_V2)

Build firmware:
```bash
cd FYP_Car  # or FYP_Car_V2
pio run
```

Upload to device:
```bash
pio run -t upload
```

Monitor serial output:
```bash
pio device monitor
```

Clean build:
```bash
pio run -t clean
```

### Python Host Application

Run the control station:
```bash
cd First_Year_Project_Host
python main.py
```

Test UDP communication:
```bash
python test_send.py    # Send test packets
python test_receive.py # Receive sensor data
```

## Architecture Overview

### Communication Protocol
- **Transport**: UDP over WiFi
- **Control Port**: 1234 (robot receives commands)
- **Sensor Port**: 1235 (host receives telemetry)
- **Data Format**: Custom byte protocol with MsgPack serialization (FYP_Car only)

### Robot Car Architecture
Both versions implement:
- Differential drive motor control (PWM + direction pins)
- WiFi connectivity with UDP packet handling
- Sensor integration (IR frequency detection, ultrasound distance)
- Remote control command processing

Key differences:
- **FYP_Car**: Uses WiFi101 library, MsgPack serialization, integrated architecture
- **FYP_Car_V2**: Uses WiFiManager, modular source files, servo-controlled ultrasound

### Host Application Architecture
- **GUI Framework**: Tkinter with real-time updates
- **Input Methods**: Keyboard, Xbox controller via pygame
- **Threading**: Separate threads for UDP receive and control loops
- **Visualization**: Live sensor data display, control status monitoring

### Control Packet Structure
Commands sent to robot (UDP port 1234):
- Movement commands with forward/angular velocities
- Duck ID for multi-robot support
- Custom binary protocol

Sensor data from robot (UDP port 1235):
- IR frequency readings
- Ultrasound distance measurements
- Status information