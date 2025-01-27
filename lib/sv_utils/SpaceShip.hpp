#ifndef SPACESHIP_HPP
#define SPACESHIP_HPP

#include <Arduino.h>
#include <list>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "Bullet.hpp"
#include "my_oled.hpp"

using namespace std;

#define SpaceShip_WIDTH 8
#define SpaceShip_HEIGHT 8
#define SpaceShip_WIDTH_HALF SpaceShip_WIDTH / 2
#define SpaceShip_HEIGHT_HALF SpaceShip_HEIGHT / 2

class SpaceShip
{
public:
    SpaceShip();
    ~SpaceShip();

    /**
     * @brief Get the Coord X object (Thread safe)
     * 
     * @param acc_x The acceleration in the x direction
     * @param diff The time difference
     */
    void moveX(double acc_x, uint32_t diff);

    /**
     * @brief Get the Coord Y object (Thread safe)
     * 
     * @param acc_y The acceleration in the y direction
     * @param diff The time difference
     */
    void moveY(double acc_y, uint32_t diff);

    /**
     * @brief Get the Coord X object (Thread safe)
     * 
     * @return int16_t X coordinate
     */
    int16_t getCoordX();

    /**
     * @brief Get the Coord Y object (Thread safe)
     * 
     * @return int16_t Y coordinate
     */
    int16_t getCoordY();

    const uint8_t* getBitmap() const;


private:
    static const uint8_t PROGMEM spaceship_bmp[];

    int16_t coordx;
    int16_t coordy;

    double scalingFactor = 0.2;

    SemaphoreHandle_t xMutex;
};

#endif // SPACESHIP_HPP