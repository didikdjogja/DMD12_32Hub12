/*--------------------------------------------------------------------------------------
 DMD12_32Hub12.cpp - Universal library for HUB12/P10 LED displays
 Supports: ESP8266, ESP32, ESP32-S2, ESP32-C3
--------------------------------------------------------------------------------------*/
#include "DMD12_32Hub12.h"
#include "utils.h"

/*--------------------------------------------------------------------------------------
 Setup and instantiation of DMD library
--------------------------------------------------------------------------------------*/

DMD::DMD(byte panelsWide, byte panelsHigh)
{
    _aPin = PIN_DMD_A;
    _bPin = PIN_DMD_B;
    _latPin = PIN_DMD_LAT;
    _nOEPin = PIN_DMD_nOE;

    _clkPin = PIN_DMD_CLK;
    _rDataPin = PIN_DMD_R_DATA;

    init(panelsWide, panelsHigh);
}

DMD::DMD(
    byte panelsWide, byte panelsHigh, 
    uint8_t nOEPin, uint8_t aPin, uint8_t bPin, uint8_t clkPin, uint8_t latPin, uint8_t rDataPin
) {
    _aPin = aPin;
    _bPin = bPin;
    _latPin = latPin;
    _nOEPin = nOEPin;

    _clkPin = clkPin;
    _rDataPin = rDataPin;

    init(panelsWide, panelsHigh);
}

void DMD::init(byte panelsWide, byte panelsHigh)
{
    uint16_t ui;
    DisplaysWide = panelsWide;
    DisplaysHigh = panelsHigh;
    DisplaysTotal = DisplaysWide * DisplaysHigh;
    row1 = DisplaysTotal << 4;
    row2 = DisplaysTotal << 5;
    row3 = ((DisplaysTotal << 2) * 3) << 2;
    bDMDScreenRAM = (byte *)malloc(DisplaysTotal * DMD_RAM_SIZE_BYTES);   

    // BRIGHTNESS CONTROL INIT
    _brightness = 255; // Default full brightness
    
    // ==== SPI INIT BERDASARKAN PLATFORM ====
    #ifdef DMD_TARGET_ESP8266
        // ESP8266 menggunakan HSPI
        spi = new SPIClass();
        Serial.println("[DMD12_32Hub12] Initializing for ESP8266 (HSPI)");
        
    #elif defined(DMD_TARGET_ESP32S2)
        // ESP32-S2 menggunakan HSPI (alias FSPI)
        spi = new SPIClass(HSPI);
        Serial.println("[DMD12_32Hub12] Initializing for ESP32-S2 (HSPI/FSPI)");
        
    #elif defined(DMD_TARGET_ESP32C3)
        // ESP32-C3 menggunakan HSPI (SPI2)
        spi = new SPIClass(HSPI);
        Serial.println("[DMD12_32Hub12] Initializing for ESP32-C3 (HSPI)");
        
    #elif defined(DMD_TARGET_ESP32)
        // ESP32 regular menggunakan VSPI
        spi = new SPIClass(VSPI);
        Serial.println("[DMD12_32Hub12] Initializing for ESP32 (VSPI)");
        
    #endif
    
    // Inisialisasi SPI dengan pin yang sesuai
    // ESP8266: harus panggil spi->begin() tanpa parameter
    #ifdef DMD_TARGET_ESP8266
        spi->begin();
    #else
        // ESP32 family: bisa kasih parameter pin
        spi->begin(_clkPin, -1, _rDataPin, -1);
    #endif
    
    // setup pin mode
    pinMode(_aPin, OUTPUT);
    pinMode(_bPin, OUTPUT);
    pinMode(_latPin, OUTPUT); 
    pinMode(_nOEPin, OUTPUT);
    pinMode(PIN_OTHER_SPI_nCS, INPUT);
    
    // initial GPIO state
    digitalWrite(_aPin, LOW);
    digitalWrite(_bPin, LOW);
    digitalWrite(_latPin, LOW);
    digitalWrite(_nOEPin, LOW); 
    
    clearScreen(true);
    
    // init the scan line/ram pointer to the required start point
    bDMDByte = 0;
}

// Chip type information
const char* DMD::getChipType() {
    #ifdef DMD_TARGET_ESP8266
        return "ESP8266";
    #elif defined(DMD_TARGET_ESP32S2)
        return "ESP32-S2";
    #elif defined(DMD_TARGET_ESP32C3)
        return "ESP32-C3";
    #elif defined(DMD_TARGET_ESP32)
        return "ESP32";
    #else
        return "Unknown";
    #endif
}

// BRIGHTNESS CONTROL FUNCTIONS
void DMD::setBrightness(uint8_t brightness) {
    _brightness = brightness;
}

uint8_t DMD::getBrightness() {
    return _brightness;
}

/*--------------------------------------------------------------------------------------
 Set or clear a pixel at the x and y location (0,0 is the top left corner)
--------------------------------------------------------------------------------------*/
void DMD::writePixel(unsigned int bX, unsigned int bY, byte bGraphicsMode, byte bPixel)
{
    unsigned int uiDMDRAMPointer;

    if (bX >= (DMD_PIXELS_ACROSS * DisplaysWide) || bY >= (DMD_PIXELS_DOWN * DisplaysHigh))
    {
        return;
    }
    byte panel = (bX / DMD_PIXELS_ACROSS) + (DisplaysWide * (bY / DMD_PIXELS_DOWN));
    bX = (bX % DMD_PIXELS_ACROSS) + (panel << 5);
    bY = bY % DMD_PIXELS_DOWN;
    // set pointer to DMD RAM byte to be modified
    uiDMDRAMPointer = bX / 8 + bY * (DisplaysTotal << 2);

    byte lookup = bPixelLookupTable[bX & 0x07];

    switch (bGraphicsMode)
    {
    case GRAPHICS_NORMAL:
        if (bPixel == true)
            bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup; // zero bit is pixel on
        else
            bDMDScreenRAM[uiDMDRAMPointer] |= lookup; // one bit is pixel off
        break;
    case GRAPHICS_INVERSE:
        if (bPixel == false)
            bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup; // zero bit is pixel on
        else
            bDMDScreenRAM[uiDMDRAMPointer] |= lookup; // one bit is pixel off
        break;
    case GRAPHICS_TOGGLE:
        if (bPixel == true)
        {
            if ((bDMDScreenRAM[uiDMDRAMPointer] & lookup) == 0)
                bDMDScreenRAM[uiDMDRAMPointer] |= lookup; // one bit is pixel off
            else
                bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup; // one bit is pixel off
        }
        break;
    case GRAPHICS_OR:
        // only set pixels on
        if (bPixel == true)
            bDMDScreenRAM[uiDMDRAMPointer] &= ~lookup; // zero bit is pixel on
        break;
    case GRAPHICS_NOR:
        // only clear on pixels
        if ((bPixel == true) &&
            ((bDMDScreenRAM[uiDMDRAMPointer] & lookup) == 0))
            bDMDScreenRAM[uiDMDRAMPointer] |= lookup; // one bit is pixel off
        break;
    }
}

void DMD::drawString(int bX, int bY, const char *bChars, byte length,
                     byte bGraphicsMode)
{
    if (bX >= (DMD_PIXELS_ACROSS * DisplaysWide) || bY >= DMD_PIXELS_DOWN * DisplaysHigh)
        return;
    uint8_t height = pgm_read_byte(this->Font + FONT_HEIGHT);
    if (bY + height < 0)
        return;

    int strWidth = 0;
    this->drawLine(bX - 1, bY, bX - 1, bY + height, GRAPHICS_INVERSE);

    for (int i = 0; i < length; i++)
    {
        int charWide = this->drawChar(bX + strWidth, bY, bChars[i], bGraphicsMode);
        if (charWide > 0)
        {
            strWidth += charWide;
            this->drawLine(bX + strWidth, bY, bX + strWidth, bY + height, GRAPHICS_INVERSE);
            strWidth++;
        }
        else if (charWide < 0)
        {
            return;
        }
        if ((bX + strWidth) >= DMD_PIXELS_ACROSS * DisplaysWide || bY >= DMD_PIXELS_DOWN * DisplaysHigh)
            return;
    }
}

void DMD::drawMarquee(const char *bChars, byte length, int left, int top)
{
    marqueeWidth = 0;
    for (int i = 0; i < length; i++)
    {
        marqueeText[i] = bChars[i];
        marqueeWidth += charWidth(bChars[i]) + 1;
    }
    marqueeHeight = pgm_read_byte(this->Font + FONT_HEIGHT);
    marqueeText[length] = '\0';
    marqueeOffsetY = top;
    marqueeOffsetX = left;
    marqueeLength = length;
    drawString(marqueeOffsetX, marqueeOffsetY, marqueeText, marqueeLength,
               GRAPHICS_NORMAL);
}

boolean DMD::stepMarquee(int amountX, int amountY)
{
    boolean ret = false;
    marqueeOffsetX += amountX;
    marqueeOffsetY += amountY;
    if (marqueeOffsetX < -marqueeWidth)
    {
        marqueeOffsetX = DMD_PIXELS_ACROSS * DisplaysWide;
        clearScreen(true);
        ret = true;
    }
    else if (marqueeOffsetX > DMD_PIXELS_ACROSS * DisplaysWide)
    {
        marqueeOffsetX = -marqueeWidth;
        clearScreen(true);
        ret = true;
    }

    if (marqueeOffsetY < -marqueeHeight)
    {
        marqueeOffsetY = DMD_PIXELS_DOWN * DisplaysHigh;
        clearScreen(true);
        ret = true;
    }
    else if (marqueeOffsetY > DMD_PIXELS_DOWN * DisplaysHigh)
    {
        marqueeOffsetY = -marqueeHeight;
        clearScreen(true);
        ret = true;
    }

    // Special case horizontal scrolling to improve speed
    if (amountY == 0 && amountX == -1)
    {
        // Shift entire screen one bit
        for (int i = 0; i < DMD_RAM_SIZE_BYTES * DisplaysTotal; i++)
        {
            if ((i % (DisplaysWide * 4)) == (DisplaysWide * 4) - 1)
            {
                bDMDScreenRAM[i] = (bDMDScreenRAM[i] << 1) + 1;
            }
            else
            {
                bDMDScreenRAM[i] = (bDMDScreenRAM[i] << 1) + ((bDMDScreenRAM[i + 1] & 0x80) >> 7);
            }
        }

        // Redraw last char on screen
        int strWidth = marqueeOffsetX;
        for (byte i = 0; i < marqueeLength; i++)
        {
            int wide = charWidth(marqueeText[i]);
            if (strWidth + wide >= DisplaysWide * DMD_PIXELS_ACROSS)
            {
                drawChar(strWidth, marqueeOffsetY, marqueeText[i], GRAPHICS_NORMAL);
                return ret;
            }
            strWidth += wide + 1;
        }
    }
    else if (amountY == 0 && amountX == 1)
    {
        // Shift entire screen one bit
        for (int i = (DMD_RAM_SIZE_BYTES * DisplaysTotal) - 1; i >= 0; i--)
        {
            if ((i % (DisplaysWide * 4)) == 0)
            {
                bDMDScreenRAM[i] = (bDMDScreenRAM[i] >> 1) + 128;
            }
            else
            {
                bDMDScreenRAM[i] = (bDMDScreenRAM[i] >> 1) + ((bDMDScreenRAM[i - 1] & 1) << 7);
            }
        }

        // Redraw last char on screen
        int strWidth = marqueeOffsetX;
        for (byte i = 0; i < marqueeLength; i++)
        {
            int wide = charWidth(marqueeText[i]);
            if (strWidth + wide >= 0)
            {
                drawChar(strWidth, marqueeOffsetY, marqueeText[i], GRAPHICS_NORMAL);
                return ret;
            }
            strWidth += wide + 1;
        }
    }
    else
    {
        drawString(marqueeOffsetX, marqueeOffsetY, marqueeText, marqueeLength,
                   GRAPHICS_NORMAL);
    }

    return ret;
}

/*--------------------------------------------------------------------------------------
 Clear the screen in DMD RAM
--------------------------------------------------------------------------------------*/
void DMD::clearScreen(byte bNormal)
{
    if (bNormal) // clear all pixels
        memset(bDMDScreenRAM, 0xFF, DMD_RAM_SIZE_BYTES * DisplaysTotal);
    else // set all pixels
        memset(bDMDScreenRAM, 0x00, DMD_RAM_SIZE_BYTES * DisplaysTotal);
}

/*--------------------------------------------------------------------------------------
 Draw or clear a line from x1,y1 to x2,y2
--------------------------------------------------------------------------------------*/
void DMD::drawLine(int x1, int y1, int x2, int y2, byte bGraphicsMode)
{
    int dy = y2 - y1;
    int dx = x2 - x1;
    int stepx, stepy;

    if (dy < 0)
    {
        dy = -dy;
        stepy = -1;
    }
    else
    {
        stepy = 1;
    }
    if (dx < 0)
    {
        dx = -dx;
        stepx = -1;
    }
    else
    {
        stepx = 1;
    }
    dy <<= 1; // dy is now 2*dy
    dx <<= 1; // dx is now 2*dx

    writePixel(x1, y1, bGraphicsMode, true);
    if (dx > dy)
    {
        int fraction = dy - (dx >> 1); // same as 2*dy - dx
        while (x1 != x2)
        {
            if (fraction >= 0)
            {
                y1 += stepy;
                fraction -= dx; // same as fraction -= 2*dx
            }
            x1 += stepx;
            fraction += dy; // same as fraction -= 2*dy
            writePixel(x1, y1, bGraphicsMode, true);
        }
    }
    else
    {
        int fraction = dx - (dy >> 1);
        while (y1 != y2)
        {
            if (fraction >= 0)
            {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;
            writePixel(x1, y1, bGraphicsMode, true);
        }
    }
}

/*--------------------------------------------------------------------------------------
 Draw or clear a circle of radius r at x,y centre
--------------------------------------------------------------------------------------*/
void DMD::drawCircle(int xCenter, int yCenter, int radius,
                     byte bGraphicsMode)
{
    int x = 0;
    int y = radius;
    int p = (5 - radius * 4) / 4;

    drawCircleSub(xCenter, yCenter, x, y, bGraphicsMode);
    while (x < y)
    {
        x++;
        if (p < 0)
        {
            p += 2 * x + 1;
        }
        else
        {
            y--;
            p += 2 * (x - y) + 1;
        }
        drawCircleSub(xCenter, yCenter, x, y, bGraphicsMode);
    }
}

void DMD::drawCircleSub(int cx, int cy, int x, int y, byte bGraphicsMode)
{

    if (x == 0)
    {
        writePixel(cx, cy + y, bGraphicsMode, true);
        writePixel(cx, cy - y, bGraphicsMode, true);
        writePixel(cx + y, cy, bGraphicsMode, true);
        writePixel(cx - y, cy, bGraphicsMode, true);
    }
    else if (x == y)
    {
        writePixel(cx + x, cy + y, bGraphicsMode, true);
        writePixel(cx - x, cy + y, bGraphicsMode, true);
        writePixel(cx + x, cy - y, bGraphicsMode, true);
        writePixel(cx - x, cy - y, bGraphicsMode, true);
    }
    else if (x < y)
    {
        writePixel(cx + x, cy + y, bGraphicsMode, true);
        writePixel(cx - x, cy + y, bGraphicsMode, true);
        writePixel(cx + x, cy - y, bGraphicsMode, true);
        writePixel(cx - x, cy - y, bGraphicsMode, true);
        writePixel(cx + y, cy + x, bGraphicsMode, true);
        writePixel(cx - y, cy + x, bGraphicsMode, true);
        writePixel(cx + y, cy - x, bGraphicsMode, true);
        writePixel(cx - y, cy - x, bGraphicsMode, true);
    }
}

/*--------------------------------------------------------------------------------------
 Draw or clear a box(rectangle) with a single pixel border
--------------------------------------------------------------------------------------*/
void DMD::drawBox(int x1, int y1, int x2, int y2, byte bGraphicsMode)
{
    drawLine(x1, y1, x2, y1, bGraphicsMode);
    drawLine(x2, y1, x2, y2, bGraphicsMode);
    drawLine(x2, y2, x1, y2, bGraphicsMode);
    drawLine(x1, y2, x1, y1, bGraphicsMode);
}

/*--------------------------------------------------------------------------------------
 Draw or clear a filled box(rectangle) with a single pixel border
--------------------------------------------------------------------------------------*/
void DMD::drawFilledBox(int x1, int y1, int x2, int y2,
                        byte bGraphicsMode)
{
    for (int b = x1; b <= x2; b++)
    {
        drawLine(b, y1, b, y2, bGraphicsMode);
    }
}

/*--------------------------------------------------------------------------------------
 Draw the selected test pattern
--------------------------------------------------------------------------------------*/
void DMD::drawTestPattern(byte bPattern)
{
    unsigned int ui;

    int numPixels = DisplaysTotal * DMD_PIXELS_ACROSS * DMD_PIXELS_DOWN;
    int pixelsWide = DMD_PIXELS_ACROSS * DisplaysWide;
    for (ui = 0; ui < numPixels; ui++)
    {
        switch (bPattern)
        {
        case PATTERN_ALT_0: // every alternate pixel, first pixel on
            if ((ui & pixelsWide) == 0)
                // even row
                writePixel((ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, ui & 1);
            else
                // odd row
                writePixel((ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, !(ui & 1));
            break;
        case PATTERN_ALT_1: // every alternate pixel, first pixel off
            if ((ui & pixelsWide) == 0)
                // even row
                writePixel((ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, !(ui & 1));
            else
                // odd row
                writePixel((ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, ui & 1);
            break;
        case PATTERN_STRIPE_0: // vertical stripes, first stripe on
            writePixel((ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, ui & 1);
            break;
        case PATTERN_STRIPE_1: // vertical stripes, first stripe off
            writePixel((ui & (pixelsWide - 1)), ((ui & ~(pixelsWide - 1)) / pixelsWide), GRAPHICS_NORMAL, !(ui & 1));
            break;
        }
    }
}

/*--------------------------------------------------------------------------------------
 Scan the dot matrix LED panel display
 Universal version for all platforms
--------------------------------------------------------------------------------------*/
void DMD::scanDisplayBySPI()
{
    // Skip if other SPI device is using the bus
    if (digitalRead(PIN_OTHER_SPI_nCS) == HIGH)
    {
        // SPI transfer pixels to the display hardware shift registers
        int rowsize = DisplaysTotal << 2;
        int offset = rowsize * bDMDByte;
        
        for (int i = 0; i < rowsize; i++)
        {
            #ifdef DMD_TARGET_ESP8266
                // ESP8266: Gunakan SPI settings yang kompatibel
                spi->beginTransaction(SPISettings(DMD_SPI_CLOCK, MSBFIRST, SPI_MODE0));
            #else
                // ESP32 family
                spi->beginTransaction(SPISettings(DMD_SPI_CLOCK, MSBFIRST, SPI_MODE0));
            #endif
            
            spi->transfer(bDMDScreenRAM[offset + i + row3]);
            spi->transfer(bDMDScreenRAM[offset + i + row2]);
            spi->transfer(bDMDScreenRAM[offset + i + row1]);
            spi->transfer(bDMDScreenRAM[offset + i]);
            spi->endTransaction();
        }

        oeRowsOff();
        latchShiftRegToOutput();
        
        switch (bDMDByte)
        {
        case 0: // row 1, 5, 9, 13 were clocked out
            lightRow_01_05_09_13();
            bDMDByte = 1;
            break;
        case 1: // row 2, 6, 10, 14 were clocked out
            lightRow_02_06_10_14();
            bDMDByte = 2;
            break;
        case 2: // row 3, 7, 11, 15 were clocked out
            lightRow_03_07_11_15();
            bDMDByte = 3;
            break;
        case 3: // row 4, 8, 12, 16 were clocked out
            lightRow_04_08_12_16();
            bDMDByte = 0;
            break;
        }
        
        // ==== BRIGHTNESS CONTROL (UNIVERSAL) ====
        // Adjust timing based on platform for best results
        #ifdef DMD_TARGET_ESP8266
            uint32_t rowTime = 120; // ESP8266 sedikit lebih lambat
        #else
            uint32_t rowTime = 100; // ESP32 family
        #endif
        
        if (_brightness == 255) {
            // Full brightness
            oeRowsOn();
            delayMicroseconds(rowTime);
        }
        else if (_brightness == 0) {
            // Completely off
            oeRowsOff();
            delayMicroseconds(rowTime);
        }
        else {
            // PWM brightness control
            uint32_t onTime = (rowTime * _brightness) / 255;
            uint32_t offTime = rowTime - onTime;
            
            // ON period
            if (onTime > 0) {
                oeRowsOn();
                delayMicroseconds(onTime);
            }
            
            // OFF period
            if (offTime > 0) {
                oeRowsOff();
                delayMicroseconds(offTime);
            }
        }
        
        // Ensure OE is off for next iteration
        oeRowsOff();
    }
}

void DMD::selectFont(const uint8_t *font)
{
    this->Font = font;
}

int DMD::drawChar(const int bX, const int bY, const unsigned char letter, byte bGraphicsMode)
{
    if (bX > (DMD_PIXELS_ACROSS * DisplaysWide) || bY > (DMD_PIXELS_DOWN * DisplaysHigh))
        return -1;
    unsigned char c = letter;
    uint8_t height = pgm_read_byte(this->Font + FONT_HEIGHT);
    if (c == ' ')
    {
        int charWide = charWidth(' ');
        this->drawFilledBox(bX, bY, bX + charWide, bY + height, GRAPHICS_INVERSE);
        return charWide;
    }
    uint8_t width = 0;
    uint8_t bytes = (height + 7) / 8;

    uint8_t firstChar = pgm_read_byte(this->Font + FONT_FIRST_CHAR);
    uint8_t charCount = pgm_read_byte(this->Font + FONT_CHAR_COUNT);

    uint16_t index = 0;

    if (c < firstChar || c >= (firstChar + charCount))
        return 0;
    c -= firstChar;

    if (pgm_read_byte(this->Font + FONT_LENGTH) == 0 && pgm_read_byte(this->Font + FONT_LENGTH + 1) == 0)
    {
        // zero length is flag indicating fixed width font (array does not contain width data entries)
        width = pgm_read_byte(this->Font + FONT_FIXED_WIDTH);
        index = c * bytes * width + FONT_WIDTH_TABLE;
    }
    else
    {
        // variable width font, read width data, to get the index
        for (uint8_t i = 0; i < c; i++)
        {
            index += pgm_read_byte(this->Font + FONT_WIDTH_TABLE + i);
        }
        index = index * bytes + charCount + FONT_WIDTH_TABLE;
        width = pgm_read_byte(this->Font + FONT_WIDTH_TABLE + c);
    }
    if (bX < -width || bY < -height)
        return width;

    // last but not least, draw the character
    for (uint8_t j = 0; j < width; j++)
    { // Width
        for (uint8_t i = bytes - 1; i < 254; i--)
        { // Vertical Bytes
            uint8_t data = pgm_read_byte(this->Font + index + j + (i * width));
            int offset = (i * 8);
            if ((i == bytes - 1) && bytes > 1)
            {
                offset = height - 8;
            }
            for (uint8_t k = 0; k < 8; k++)
            { // Vertical bits
                if ((offset + k >= i * 8) && (offset + k <= height))
                {
                    if (data & (1 << k))
                    {
                        writePixel(bX + j, bY + offset + k, bGraphicsMode, true);
                    }
                    else
                    {
                        writePixel(bX + j, bY + offset + k, bGraphicsMode, false);
                    }
                }
            }
        }
    }
    return width;
}

int DMD::charWidth(const unsigned char letter)
{
    return charWidthOfFont(letter, this->Font);
}

void DMD::drawContainer(DMDContainer *container)
{
    int16_t x0 = container->getX0();
    int16_t y0 = container->getY0();
    int16_t w = container->getW();
    int16_t h = container->getH();
    uint8_t* buf = container->getBufferData();

    for (uint16_t i = 0; i < w; i++)
    {
        for (uint16_t j = 0; j < h; j++)
        {
            writePixel(i + x0 - 1, j + y0, GRAPHICS_NORMAL, buf[j * w + i]);
        }
    }
}