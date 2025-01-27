#ifndef INVADER_HPP
#define INVADER_HPP

#include <Arduino.h>
#include <random>

#include "my_oled.hpp"

#define INVADER_WIDTH 8
#define INVADER_HEIGHT 8

class Invader
{
public:
    Invader(int16_t difficulty);
    ~Invader();

    void moveDown();
    int16_t getCoordX() const;
    int16_t getCoordY() const;

    const uint8_t* getBitmap() const;

private:
    int speed;
    int16_t coordx;
    int16_t coordy;
    bool isAlive;
    
    static std::mt19937 gen;
    static std::uniform_int_distribution<> dis;

    static const uint8_t PROGMEM invader_bmp[];
};

#endif // INVADER_HPP