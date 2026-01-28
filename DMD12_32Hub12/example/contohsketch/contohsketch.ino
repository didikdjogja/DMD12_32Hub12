/*
  DMD12_32Hub12 - Universal Example
  New library name: DMD12_32Hub12
*/

#include <DMD12_32Hub12.h>

DMD dmd(1, 1); // 1 panel

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("DMD12_32Hub12 - Universal Test");
  Serial.println("=================================");
  
  // Display chip info
  Serial.print("Platform: ");
  Serial.println(dmd.getChipType());
  
  // Set initial brightness
  #ifdef ESP8266
    dmd.setBrightness(200); // ESP8266 needs higher brightness
  #else
    dmd.setBrightness(150); // ESP32 family
  #endif
  
  // Clear and draw test pattern
  dmd.clearScreen(true);
  
  // Draw border
  dmd.drawBox(0, 0, 31, 15, GRAPHICS_NORMAL);
  
  // Draw diagonal lines
  dmd.drawLine(0, 0, 31, 15, GRAPHICS_NORMAL);
  dmd.drawLine(31, 0, 0, 15, GRAPHICS_NORMAL);
  
  // Draw "12" in the center (representing DMD12_32)
  dmd.drawFilledBox(8, 4, 11, 11, GRAPHICS_NORMAL);
  dmd.drawFilledBox(20, 4, 23, 11, GRAPHICS_NORMAL);
  
  Serial.println("Setup complete!");
  Serial.print("Initial brightness: ");
  Serial.println(dmd.getBrightness());
}

void loop() {
  // Must call this continuously
  dmd.scanDisplayBySPI();
  
  // Demo brightness animation
  static unsigned long lastChange = 0;
  static uint8_t brightness = 150;
  static int direction = 3;
  
  if (millis() - lastChange > 30) {
    brightness += direction;
    if (brightness >= 250 || brightness <= 50) {
      direction = -direction;
    }
    dmd.setBrightness(brightness);
    lastChange = millis();
  }
}