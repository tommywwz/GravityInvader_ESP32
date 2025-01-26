#ifndef INVADER_HPP
#define INVADER_HPP

#include <Arduino.h>

class Invader
{
public:
    Invader();
    ~Invader();

private:
    int16_t coordx;
    int16_t coordy;
    bool isAlive;
    int16_t speed;
};

#endif // INVADER_HPP