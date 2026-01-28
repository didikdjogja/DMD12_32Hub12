/*--------------------------------------------------------------------------------------
 DMD12_32Hub12.h - Universal library for HUB12/P10 LED displays
 Supports: ESP8266, ESP32, ESP32-S2, ESP32-C3
 --------------------------------------------------------------------------------------*/
#ifndef DMD12_32_HUB12_H_
#define DMD12_32_HUB12_H_

#include "Arduino.h"
#include <SPI.h>
#include "DMDContainer.h"
#include "constants.h"

// ===== CHIP DETECTION =====
#if defined(ESP8266)
  #define DMD_TARGET_ESP8266
  #warning "DMD12_32Hub12: Compiling for ESP8266"
  
#elif defined(ARDUINO_ESP32S2_DEV) || defined(ARDUINO_ESP32S2) || \
      defined(ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S2)
  #define DMD_TARGET_ESP32S2
  #warning "DMD12_32Hub12: Compiling for ESP32-S2"
  
#elif defined(ARDUINO_ESP32C3_DEV) || defined(ARDUINO_ESP32C3) || \
      defined(ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C3)
  #define DMD_TARGET_ESP32C3
  #warning "DMD12_32Hub12: Compiling for ESP32-C3"
  
#elif defined(ESP32)
  #define DMD_TARGET_ESP32
  #warning "DMD12_32Hub12: Compiling for ESP32"
  
#else
  #error "DMD12_32Hub12: Unsupported platform!"
#endif

// ===== DEFAULT PIN CONFIGURATION =====
#ifdef DMD_TARGET_ESP8266
  // ESP8266 (NodeMCU, Wemos D1) pins
  #define PIN_DMD_nOE D2     // GPIO4  - Output Enable
  #define PIN_DMD_A D1       // GPIO5  - Row Select A
  #define PIN_DMD_B D3       // GPIO0  - Row Select B
  #define PIN_DMD_CLK D5     // GPIO14 - SPI CLK (HSPI)
  #define PIN_DMD_LAT D4     // GPIO2  - Latch
  #define PIN_DMD_R_DATA D7  // GPIO13 - SPI MOSI (HSPI)
  #define PIN_OTHER_SPI_nCS D8 // GPIO15
  
#elif defined(DMD_TARGET_ESP32S2)
  // ESP32-S2 (AI Thinker ESP-12K) - FSPI pins
  #define PIN_DMD_nOE 18    // GPIO18 - Output Enable
  #define PIN_DMD_A 17      // GPIO17 - Row Select A
  #define PIN_DMD_B 16      // GPIO16 - Row Select B
  #define PIN_DMD_CLK 36    // GPIO36 - FSPI CLK (SCK)
  #define PIN_DMD_LAT 35    // GPIO35 - Latch
  #define PIN_DMD_R_DATA 37 // GPIO37 - FSPI MOSI (Data)
  #define PIN_OTHER_SPI_nCS 34
  
#elif defined(DMD_TARGET_ESP32C3)
  // ESP32-C3 - FSPI pins
  #define PIN_DMD_nOE 2     // GPIO2 - Output Enable
  #define PIN_DMD_A 3       // GPIO3 - Row Select A
  #define PIN_DMD_B 4       // GPIO4 - Row Select B
  #define PIN_DMD_CLK 6     // GPIO6 - FSPI CLK (SCK)
  #define PIN_DMD_LAT 5     // GPIO5 - Latch
  #define PIN_DMD_R_DATA 7  // GPIO7 - FSPI MOSI (Data)
  #define PIN_OTHER_SPI_nCS 8
  
#else
  // ESP32 (VSPI pins)
  #define PIN_DMD_nOE 22    // GPIO22 - Output Enable
  #define PIN_DMD_A 19      // GPIO19 - Row Select A
  #define PIN_DMD_B 21      // GPIO21 - Row Select B
  #define PIN_DMD_CLK 18    // GPIO18 - VSPI CLK (SCK)
  #define PIN_DMD_LAT 2     // GPIO2  - Latch
  #define PIN_DMD_R_DATA 23 // GPIO23 - VSPI MOSI (Data)
  #define PIN_OTHER_SPI_nCS 5
#endif

// ===== SPI SETTINGS =====
// SPI clock speed (adjust based on platform)
#ifdef DMD_TARGET_ESP8266
  #define DMD_SPI_CLOCK 8000000  // 8 MHz for ESP8266
#else
  #define DMD_SPI_CLOCK 4000000  // 4 MHz for ESP32 family
#endif

// Pixel/graphics writing modes
#define GRAPHICS_NORMAL 0
#define GRAPHICS_INVERSE 1
#define GRAPHICS_TOGGLE 2
#define GRAPHICS_OR 3
#define GRAPHICS_NOR 4

// Test patterns
#define PATTERN_ALT_0 0
#define PATTERN_ALT_1 1
#define PATTERN_STRIPE_0 2
#define PATTERN_STRIPE_1 3

// Display sizing
#define DMD_PIXELS_ACROSS 32
#define DMD_PIXELS_DOWN 16
#define DMD_BITSPERPIXEL 1
#define DMD_RAM_SIZE_BYTES ((DMD_PIXELS_ACROSS * DMD_BITSPERPIXEL / 8) * DMD_PIXELS_DOWN)

// Pixel lookup table
static byte bPixelLookupTable[8] = {
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

// Main DMD class
class DMD
{
public:
  // Brightness Control
  void setBrightness(uint8_t brightness);
  uint8_t getBrightness();
  
  // Platform info
  const char* getChipType();
  
  // Constructors
  DMD(byte panelsWide, byte panelsHigh);
  DMD(byte panelsWide, byte panelsHigh,
      uint8_t nOEPin, uint8_t aPin, uint8_t bPin, 
      uint8_t clkPin, uint8_t latPin, uint8_t rDataPin);
  
  // Graphics functions
  void writePixel(unsigned int bX, unsigned int bY, byte bGraphicsMode, byte bPixel);
  void drawString(int bX, int bY, const char *bChars, byte length, byte bGraphicsMode);
  void selectFont(const uint8_t *font);
  int drawChar(const int bX, const int bY, const unsigned char letter, byte bGraphicsMode);
  int charWidth(const unsigned char letter);
  void drawMarquee(const char *bChars, byte length, int left, int top);
  boolean stepMarquee(int amountX, int amountY);
  void clearScreen(byte bNormal);
  void drawLine(int x1, int y1, int x2, int y2, byte bGraphicsMode);
  void drawCircle(int xCenter, int yCenter, int radius, byte bGraphicsMode);
  void drawBox(int x1, int y1, int x2, int y2, byte bGraphicsMode);
  void drawFilledBox(int x1, int y1, int x2, int y2, byte bGraphicsMode);
  void drawTestPattern(byte bPattern);
  void scanDisplayBySPI();
  void drawContainer(DMDContainer *container);
  
private:
  uint8_t _brightness;
  uint8_t _nOEPin, _aPin, _bPin, _clkPin, _latPin, _rDataPin;
  
  void init(byte panelsWide, byte panelsHigh);
  void drawCircleSub(int cx, int cy, int x, int y, byte bGraphicsMode);
  
  byte *bDMDScreenRAM;
  char marqueeText[256];
  byte marqueeLength;
  int marqueeWidth, marqueeHeight, marqueeOffsetX, marqueeOffsetY;
  const uint8_t *Font;
  byte DisplaysWide, DisplaysHigh, DisplaysTotal;
  int row1, row2, row3;
  volatile byte bDMDByte;
  SPIClass *spi = NULL;
  
  // Helper functions
  void lightRow_01_05_09_13() { digitalWrite(_aPin, LOW); digitalWrite(_bPin, LOW); }
  void lightRow_02_06_10_14() { digitalWrite(_aPin, HIGH); digitalWrite(_bPin, LOW); }
  void lightRow_03_07_11_15() { digitalWrite(_aPin, LOW); digitalWrite(_bPin, HIGH); }
  void lightRow_04_08_12_16() { digitalWrite(_aPin, HIGH); digitalWrite(_bPin, HIGH); }
  void latchShiftRegToOutput() { digitalWrite(_latPin, HIGH); digitalWrite(_latPin, LOW); }
  void oeRowsOff() { digitalWrite(_nOEPin, LOW); }
  void oeRowsOn() { digitalWrite(_nOEPin, HIGH); }
};

#endif /* DMD12_32_HUB12_H_ */