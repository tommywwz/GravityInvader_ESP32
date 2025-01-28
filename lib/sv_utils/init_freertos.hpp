#ifndef INIT_FREERTOS_HPP
#define INIT_FREERTOS_HPP

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// task functions
void taskRender(void *pvParameters);
void taskGameLogic(void *pvParameters);
void taskIMU(void *pvParameters);
void taskButton(void *pvParameters);

// semaphores
SemaphoreHandle_t xSemaButton;
SemaphoreHandle_t xMutexBullets;
SemaphoreHandle_t xMutexInvaders;


void setupFreeRTOS()
{
    // Initialize the semaphore and mutex
    xSemaButton = xSemaphoreCreateBinary();
    xMutexBullets = xSemaphoreCreateMutex();
    xMutexInvaders = xSemaphoreCreateMutex();

    // Create a task that will be executed
    xTaskCreatePinnedToCore(
        taskRender,   /* Task function. */
        "Render",     /* name of task. */
        10000,        /* Stack size of task */
        NULL,         /* parameter of the task */
        2,            /* priority of the task */
        NULL,         /* Task handle to keep track of created task */
        0);           /* pin task to core 0 */
    
    xTaskCreatePinnedToCore(
        taskGameLogic,   /* Task function. */
        "GameLogic",     /* name of task. */
        10000,           /* Stack size of task */
        NULL,            /* parameter of the task */
        1,               /* priority of the task */
        NULL,            /* Task handle to keep track of created task */
        1);              /* pin task to core 1 */

    xTaskCreatePinnedToCore(
        taskIMU,   /* Task function. */
        "IMU",     /* name of task. */
        10000,     /* Stack size of task */
        NULL,      /* parameter of the task */
        2,         /* priority of the task */
        NULL,      /* Task handle to keep track of created task */
        0);        /* pin task to core 0 */

    xTaskCreatePinnedToCore(
        taskButton,   /* Task function. */
        "Button",     /* name of task. */
        10000,        /* Stack size of task */
        NULL,         /* parameter of the task */
        1,            /* priority of the task */
        NULL,         /* Task handle to keep track of created task */
        1);           /* pin task to core 1 */


}

#endif // INIT_FREERTOS_HPP