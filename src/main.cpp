#include <Arduino.h>
#include <list>
#include <atomic>
#include "my_peripheras.hpp"
#include "init_freertos.hpp"
#include "SpaceShip.hpp"
#include "Bullet.hpp"
#include "Invader.hpp"

#define BUTTON_PIN 4

list<Bullet*> Bullets;
list<Invader*> Invaders;
SpaceShip* Player;

enum class GameState
{
    START,
    PLAYING,
    GAMEOVER
};

std::atomic<GameState> gameState;

void IRAM_ATTR buttonISR()
{
    xSemaphoreGiveFromISR(xSemaButton, NULL);
}

void error_hanlder(const char* msg)
{
    Serial.println(msg);
    while (1)
    {
        delay(1000);
    }
}

void taskRender(void *pvParameters)
{
    TickType_t xLastWakeTime = 0;
    for (;;)
    {
        auto currTick = xTaskGetTickCount();
        display.clearDisplay();

        auto diff = currTick - xLastWakeTime;
        auto refreshRate = 1000;
        if (diff)
        {
            refreshRate = 1000 / diff;
        }

        xLastWakeTime = currTick;

        // Display refresh rate
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.print(refreshRate);
        display.print(" fps");

        display.drawBitmap(Player->getCoordX(), Player->getCoordY(), Player->getBitmap(), SpaceShip_WIDTH, SpaceShip_HEIGHT, SSD1306_WHITE);

        if (xSemaphoreTake(xMutexInvaders, portMAX_DELAY) == pdTRUE)
        {
            Serial.println("Render");
            for (auto inv : Invaders)
            {
                display.drawBitmap(inv->getCoordX(), inv->getCoordY(), inv->getBitmap(), INVADER_WIDTH, INVADER_HEIGHT, SSD1306_WHITE);
            }
            xSemaphoreGive(xMutexInvaders);
        }

        if (xSemaphoreTake(xMutexBullets, portMAX_DELAY) == pdTRUE)
        {
            for (auto bullet : Bullets)
            {
                display.drawRect(bullet->getCoordX(), bullet->getCoordY(), 1, 2, SSD1306_WHITE);
            }
            xSemaphoreGive(xMutexBullets);
        }

        display.display();

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void taskGameLogic(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t xlastSpawnTime = xTaskGetTickCount();
    TickType_t xlastInvaderTime = xTaskGetTickCount();
    const TickType_t xSpawnFrequency = pdMS_TO_TICKS(1000);
    const TickType_t xInvaderDelay = pdMS_TO_TICKS(50);

    int16_t diffculity = 3;
    int16_t bulletSpeed = 1;

    for (;;)
    {
        // switch (gameState.load())
        // {
        //     case GameState::START:
            
        //     case GameState::PLAYING:
            
        //     case GameState::GAMEOVER:

        //     default:
        //         break;
        // }

        // spawn invaders
        if (xTaskGetTickCount() - xlastSpawnTime > xSpawnFrequency)
        {
            Serial.println("Spawn");
            if (xSemaphoreTake(xMutexInvaders, portMAX_DELAY) == pdTRUE)
            {
                for (int i = 0; i < diffculity; i++)
                {
                    Invaders.push_back(new Invader());
                }
                xSemaphoreGive(xMutexInvaders);
            }
            xlastSpawnTime = xTaskGetTickCount();
        }

        // update bullets position
        if (xSemaphoreTake(xMutexBullets, portMAX_DELAY) == pdTRUE)
        {
            Serial.println("Update bullets");
            for (auto it = Bullets.begin(); it != Bullets.end();)
            {
                if ((*it)->getCoordY() <= 0)
                {
                    delete *it;
                    it = Bullets.erase(it);
                }
                else
                {
                    (*it)->moveUp(bulletSpeed);
                    it++;
                }
            }
            xSemaphoreGive(xMutexBullets);
        }

        // update invaders position
        if (xTaskGetTickCount() - xlastInvaderTime > xInvaderDelay)
        {
            Serial.println("Update invaders");
            if (xSemaphoreTake(xMutexInvaders, portMAX_DELAY) == pdTRUE)
            {
                for (auto it = Invaders.begin(); it != Invaders.end();)
                {
                    if ((*it)->getCoordY() >= SCREEN_HEIGHT)
                    {
                        delete *it;
                        it = Invaders.erase(it);
                    }
                    else
                    {
                        (*it)->moveDown();
                        it++;
                    }
                }
                xlastInvaderTime = xTaskGetTickCount();
                xSemaphoreGive(xMutexInvaders);
            }
        }

        // check for collision
        if (xSemaphoreTake(xMutexInvaders, portMAX_DELAY) == pdTRUE)
        {
            Serial.println("Check collision");
            for (auto inv = Invaders.begin(); inv != Invaders.end();)
            {
                if (xSemaphoreTake(xMutexBullets, portMAX_DELAY) == pdTRUE)
                {
                    for (auto bullet = Bullets.begin(); bullet != Bullets.end();)
                    {
                        if ((*bullet)->getCoordX() >= (*inv)->getCoordX() && (*bullet)->getCoordX() <= (*inv)->getCoordX() + INVADER_WIDTH &&
                            (*bullet)->getCoordY() >= (*inv)->getCoordY() && (*bullet)->getCoordY() <= (*inv)->getCoordY() + INVADER_HEIGHT)
                        {
                            delete *bullet;
                            bullet = Bullets.erase(bullet);
                            delete *inv;
                            inv = Invaders.erase(inv);
                            break; // break the inner loop since the invader is destroyed
                        }
                        else
                        {
                            bullet++;
                        }
                    }
                    xSemaphoreGive(xMutexBullets);
                }
                inv++;
            }
            Serial.println("Check collision end");
            xSemaphoreGive(xMutexInvaders);
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(20));
    }
}

void taskIMU(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    sensors_event_t a, g, temp;
    for (;;)
    {
        auto currTick = xTaskGetTickCount();
        auto diff = currTick - xLastWakeTime;

        mpu.getEvent(&a, &g, &temp);

        // flipped x and y because of the orientation of the sensor
        double accY = -a.acceleration.x; 
        double accX = -a.acceleration.y;

        Player->moveX(accX, diff);
        Player->moveY(accY, diff);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
    }
}

void taskButton(void *pvParameters)
{
    TickType_t lastDebounceTime = 0;
    for (;;)
    {
        Serial.println("Button task");
        if (xSemaphoreTake(xSemaButton, portMAX_DELAY) == pdTRUE)
        {
            Serial.println("Button take semaphore");
            // Debounce
            if (xTaskGetTickCount() - lastDebounceTime > pdTICKS_TO_MS(200))
            {
                // Shoot
                if (xSemaphoreTake(xMutexBullets, portMAX_DELAY) == pdTRUE)
                {
                    Bullets.push_back(new Bullet(Player->getCoordX()+SpaceShip_WIDTH_HALF, Player->getCoordY()));
                    xSemaphoreGive(xMutexBullets);
                }
                lastDebounceTime = xTaskGetTickCount();
            }
        }
        
    }
}

void setup()
{
    // setup Serial
    Serial.begin(115200);

    // setup buttons
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

    setupDisplay();
    setupGyro();
    
    Player = new SpaceShip();
    gameState.store(GameState::START);

    setupFreeRTOS();    
}

void loop()
{
    // put your main code here, to run repeatedly:
}

