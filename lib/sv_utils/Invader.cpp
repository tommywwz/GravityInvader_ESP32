#include "Invader.hpp"

std::mt19937 Invader::gen(std::random_device{}());
std::uniform_int_distribution<> Invader::dis(20, SCREEN_WIDTH - 40);

Invader::Invader(int16_t difficulty)
{
    // randomize x position
    coordx = dis(gen);
    coordy = 0;
    isAlive = true;
    speed = 1;
}

Invader::~Invader()
{
}

void Invader::moveDown()
{
    coordy += speed;
}

int16_t Invader::getCoordX() const
{
    return coordx;
}

int16_t Invader::getCoordY() const
{
    return coordy;
}

const uint8_t* Invader::getBitmap() const
{
    return invader_bmp;
}

const uint8_t PROGMEM Invader::invader_bmp[] = {
    B00011000,
    B00111100,
    B01111110,
    B11011011,
    B11111111,
    B10100101,
    B00111100,
    B01011010};
