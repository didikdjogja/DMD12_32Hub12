# DMD12_32Hub12 Library

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platforms](https://img.shields.io/badge/platforms-ESP8266%20%7C%20ESP32%20%7C%20ESP32--S2%20%7C%20ESP32--C3-green.svg)

Universal library for controlling HUB12/P10 LED matrix displays (32x16 pixels per panel).

## ✨ Why DMD12_32Hub12?

The name explains it all:
- **DMD12**: HUB12 protocol support
- **32**: 32 pixels across per panel
- **Hub12**: Compatible with HUB12 interface displays

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