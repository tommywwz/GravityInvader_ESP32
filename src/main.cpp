#include <Arduino.h>
#include "my_peripheras.hpp"
#include "init_freertos.hpp"

void setup()
{
    // put your setup code here, to run once:
    setupDisplay();
    setupGyro();
    setupFreeRTOS();    
}

void loop()
{
    // put your main code here, to run repeatedly:
}

