#include "my_oled.hpp"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setupDisplay()
{
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    Serial.println("SSD1306 Found!");

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
    delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();


    myTestPattern(); // Draw a customized test pattern

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
    display.clearDisplay();
    int rectWidth = 16;

    for (int i = 0; i < SCREEN_WIDTH; i += rectWidth)
    {
        for (int j = 0; j < SCREEN_HEIGHT; j += rectWidth)
        {
            display.drawRect(i, j, rectWidth, rectWidth, SSD1306_WHITE);
        }
    }

    display.display();
    delay(500);
    // Invert and restore display, pausing in-between
    testInvertDisplay();

    // draw three lines from center of the display separated by 120 degrees
    display.clearDisplay();
    auto halfWidth = SCREEN_WIDTH / 2;
    auto halfHeight = SCREEN_HEIGHT / 2;
    for (int i = 0; i < 3; i++)
    {
        auto x = halfWidth + halfHeight * cos(2 * PI / 3 * i);
        auto y = halfHeight + halfHeight * sin(2 * PI / 3 * i);
        display.drawLine(halfWidth, halfHeight, x, y, SSD1306_WHITE);
    }
    display.display();
    delay(500);
    testInvertDisplay();

    // draw concentric circles with lines cut it into 8 parts
    display.clearDisplay();
    for (int i = 0; i < 4; i++)
    {
        display.drawCircle(halfWidth, halfHeight, halfHeight - i * 10, SSD1306_WHITE);
    }

    for (int i = 0; i < 8; i++)
    {
        auto x = halfWidth + halfHeight * cos(2 * PI / 8 * i);
        auto y = halfHeight + halfHeight * sin(2 * PI / 8 * i);
        display.drawLine(halfWidth, halfHeight, x, y, SSD1306_WHITE);
    }
    display.display();
    delay(500);
    testInvertDisplay();
}