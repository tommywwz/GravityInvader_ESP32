#include "my_ILI9341_lcd.hpp"

MyILI9341LCD::MyILI9341LCD(uint8_t cs, uint8_t dc, uint8_t rst) : Adafruit_ILI9341(cs, dc, rst)
{
    // ctor
    // draw a test pattern
}

MyILI9341LCD::~MyILI9341LCD()
{
    // dtor
}

void MyILI9341LCD::clearDisplay()
{
    fillScreen(ILI9341_BLACK);
}

