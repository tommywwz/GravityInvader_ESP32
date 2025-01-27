#ifndef BUlLET_HPP
#define BUlLET_HPP

#include <Arduino.h>

class Bullet
{
public:
    Bullet(int16_t x, int16_t y);
    ~Bullet();

    void moveUp(int16_t speed);
    int16_t getCoordX();
    int16_t getCoordY();

private:
    int16_t coordx;
    int16_t coordy;
};

#endif // BUlLET_HPP