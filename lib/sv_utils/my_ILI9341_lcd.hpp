#ifndef MY_ILI9341_LCD_HPP
#define MY_ILI9341_LCD_HPP

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   5
#define TFT_RST  4
#define TFT_DC   2

#define DISPLAY_PORTRAIT 0
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define HALF_SCREEN_WIDTH (SCREEN_WIDTH / 2)
#define HALF_SCREEN_HEIGHT (SCREEN_HEIGHT / 2)

#define MY_DISPLAY_BLACK   ILI9341_BLACK
#define MY_DISPLAY_BLUE    ILI9341_BLUE
#define MY_DISPLAY_RED     ILI9341_RED
#define MY_DISPLAY_GREEN   ILI9341_GREEN
#define MY_DISPLAY_CYAN    ILI9341_CYAN
#define MY_DISPLAY_MAGENTA ILI9341_MAGENTA
#define MY_DISPLAY_YELLOW  ILI9341_YELLOW
#define MY_DISPLAY_WHITE   ILI9341_WHITE

class MyILI9341LCD : public Adafruit_ILI9341
{
    public:

    MyILI9341LCD(uint8_t cs, uint8_t dc, uint8_t rst);

    ~MyILI9341LCD();

    void clearDisplay();

    void myTestPattern();

};


#endif // MY_ILI9341_LCD_HPP