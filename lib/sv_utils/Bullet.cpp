#include "Bullet.hpp"


Bullet::Bullet(int16_t x, int16_t y)
{
    coordx = x;
    coordy = y;
}

Bullet::~Bullet()
{
}

void Bullet::moveUp(int16_t speed)
{
    coordy -= speed;
}

int16_t Bullet::getCoordX()
{
    return coordx;
}

int16_t Bullet::getCoordY()
{
    return coordy;
}


