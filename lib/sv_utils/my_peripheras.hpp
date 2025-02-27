#ifndef MY_PERIPHERAS_HPP
#define MY_PERIPHERAS_HPP

#include <Wire.h>
#include "my_ILI9341_lcd.hpp"
#include "my_IMU.hpp"

Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);


void myTestPattern();

void setupDisplay()
{
    // setup the display
    display.begin();
    display.setRotation(DISPLAY_PORTRAIT);
    display.fillScreen(ILI9341_BLACK);

    myTestPattern();
}

void testInvertDisplay()
{
    display.invertDisplay(true);
    delay(500);
    display.invertDisplay(false);
    delay(500);
}

void myTestPattern()
{
    // draw 16x16 pixel grid
    display.fillScreen(ILI9341_BLACK);
    int rectWidth = 16;

    for (int i = 0; i < SCREEN_WIDTH; i += rectWidth)
    {
        for (int j = 0; j < SCREEN_HEIGHT; j += rectWidth)
        {
            display.drawRect(i, j, rectWidth, rectWidth, MY_DISPLAY_WHITE);
        }
    }

    delay(500);
    // Invert and restore display, pausing in-between
    testInvertDisplay();

    // draw three lines from center of the display separated by 120 degrees
    display.fillScreen(ILI9341_BLACK);
    auto halfWidth = SCREEN_WIDTH / 2;
    auto halfHeight = SCREEN_HEIGHT / 2;
    for (int i = 0; i < 3; i++)
    {
        auto x = halfWidth + halfHeight * cos(2 * PI / 3 * i);
        auto y = halfHeight + halfHeight * sin(2 * PI / 3 * i);
        display.drawLine(halfWidth, halfHeight, x, y, MY_DISPLAY_WHITE);
    }
    delay(500);
    testInvertDisplay();

    // draw concentric circles with lines cut it into 8 parts
    display.fillScreen(ILI9341_BLACK);
    for (int i = 0; i < 4; i++)
    {
        display.drawCircle(halfWidth, halfHeight, halfHeight - i * 10, MY_DISPLAY_WHITE);
    }

    for (int i = 0; i < 8; i++)
    {
        auto x = halfWidth + halfHeight * cos(2 * PI / 8 * i);
        auto y = halfHeight + halfHeight * sin(2 * PI / 8 * i);
        display.drawLine(halfWidth, halfHeight, x, y, MY_DISPLAY_WHITE);
    }
    delay(500);
    testInvertDisplay();
}

#endif