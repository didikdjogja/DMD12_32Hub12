# DMD12_32Hub12 Library

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platforms](https://img.shields.io/badge/platforms-ESP8266%20%7C%20ESP32%20%7C%20ESP32--S2%20%7C%20ESP32--C3-green.svg)

Universal library for controlling HUB12/P10 LED matrix displays (32x16 pixels per panel).

## ✨ Why DMD12_32Hub12?

The name explains it all:
- **DMD12**: HUB12 protocol support
- **32**: 32 pixels across per panel
- **Hub12**: Compatible with HUB12 interface displays

##  Wiring Guide untuk Semua Platform

ESP8266 (NodeMCU/Wemos D1):

ESP8266    → P10 HUB12
D2 (GPIO4) → OE
D1 (GPIO5) → A
D3 (GPIO0) → B
D5 (GPIO14)→ CLK (HSPI CLK)
D4 (GPIO2) → LAT
D7 (GPIO13)→ DATA (HSPI MOSI)
GND        → GND
5V/VIN     → VCC (5V)

ESP32-S2 (AI Thinker ESP-12K):

GPIO18 → OE
GPIO17 → A
GPIO16 → B
GPIO36 → CLK (FSPI CLK)
GPIO35 → LAT
GPIO37 → DATA (FSPI MOSI)
GND    → GND
5V     → VCC (5V)

ESP32-C3:

GPIO2 → OE
GPIO3 → A
GPIO4 → B
GPIO6 → CLK (FSPI CLK)
GPIO5 → LAT
GPIO7 → DATA (FSPI MOSI)
GND   → GND
5V    → VCC (5V)

ESP32 (Regular):

GPIO22 → OE
GPIO19 → A
GPIO21 → B
GPIO18 → CLK (VSPI CLK)
GPIO2  → LAT
GPIO23 → DATA (VSPI MOSI)
GND    → GND
5V     → VCC (5V)

## 📋 Features

- ✅ **Multi-platform**: ESP8266, ESP32, ESP32-S2, ESP32-C3
- ✅ **Brightness Control**: 0-255 levels with PWM
- ✅ **Graphics Functions**: Lines, circles, boxes, text
- ✅ **Multiple Panels**: Daisy-chain support
- ✅ **Font Support**: Custom and built-in fonts
- ✅ **Scrolling Text**: Smooth marquee functionality
- ✅ **Auto-detection**: Detects platform automatically

## 🚀 Quick Start

```cpp
#include <DMD12_32Hub12.h>

DMD dmd(1, 1); // 1 panel (32x16)

void setup() {
    dmd.setBrightness(150); // Set brightness
    dmd.clearScreen(true);
    dmd.drawBox(0, 0, 31, 15, GRAPHICS_NORMAL);
}

void loop() {
    dmd.scanDisplayBySPI(); // Call continuously
}
