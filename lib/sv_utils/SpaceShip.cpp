#include "SpaceShip.hpp"


SpaceShip::SpaceShip()
{
    coordx = SCREEN_WIDTH / 2;
    coordy = SCREEN_HEIGHT - SpaceShip_HEIGHT - 1;
    xMutex = xSemaphoreCreateMutex();
}

SpaceShip::~SpaceShip()
{
    if (xMutex != NULL)
    {
        vSemaphoreDelete(xMutex);
    }
}

void SpaceShip::moveX(float acc_x, uint32_t diff)
{
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
    {
        coordx += acc_x * diff * scalingFactor;
        xSemaphoreGive(xMutex);
    }
}

void SpaceShip::moveY(float acc_y, uint32_t diff)
{
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
    {
        coordy += acc_y * diff * scalingFactor;

        // check if the spaceship is out of the screen
        if (coordx <= 0)
        {
            coordx = 0;
        } else if (coordx >= SCREEN_WIDTH - SpaceShip_WIDTH - 1)
        {
            coordx = SCREEN_WIDTH - SpaceShip_WIDTH - 1;
        }

        if (coordy <= HALF_SCREEN_HEIGHT)
        {
            coordy = HALF_SCREEN_HEIGHT;
        } else if (coordy >= SCREEN_HEIGHT - SpaceShip_HEIGHT - 1)
        {
            coordy = SCREEN_HEIGHT - SpaceShip_HEIGHT - 1;
        }

        xSemaphoreGive(xMutex);
    }
}

int16_t SpaceShip::getCoordX()
{
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
    {
        int16_t x = coordx;
        xSemaphoreGive(xMutex);
        return x;
    }

    return -1; // this should never happen
}

int16_t SpaceShip::getCoordY()
{
    if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
    {
        int16_t y = coordy;
        xSemaphoreGive(xMutex);
        return y;
    }

    return -1; // this should never happen
}

const uint8_t* SpaceShip::getBitmap() const
{
    return spaceship_bmp;
}

const uint8_t PROGMEM SpaceShip::spaceship_bmp[] = {
    B00011000,
    B00111100,
    B01111110,
    B11111111,
    B11111111,
    B11011011,
    B10100101,
    B00011000};
