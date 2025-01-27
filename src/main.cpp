#include <Arduino.h>
#include <list>
#include <atomic>
#include <unordered_map>
#include "my_peripheras.hpp"
#include "init_freertos.hpp"
#include "SpaceShip.hpp"
#include "Bullet.hpp"
#include "Invader.hpp"

#define BUTTON_PIN 4
#define GRID_SIZE 16
#define GRID_SHIFT 4 // log2(GRID_SIZE)

#define START_BUTTON_WIDTH 40
#define START_BUTTON_HEIGHT 20
#define START_BUTTON_X HALF_SCREEN_WIDTH - START_BUTTON_WIDTH / 2
#define START_BUTTON_Y HALF_SCREEN_HEIGHT - START_BUTTON_HEIGHT / 2

#define GAMEOVER_Y HALF_SCREEN_HEIGHT - 10

enum class GameState
{
    START,
    PLAYING,
    GAMEOVER
};


unordered_map<uint32_t, list<Bullet>> BulletsMap; // the key if the y coordinate in 20 pxl steps grid
list<Invader*> Invaders;
SpaceShip* Player;
atomic<uint32_t> KillCount;
atomic<uint32_t> BestScore;
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
        display.setCursor(SCREEN_WIDTH - 25, 0);
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.print(refreshRate);
        display.print("hz");



        display.drawBitmap(Player->getCoordX(), Player->getCoordY(), Player->getBitmap(), SpaceShip_WIDTH, SpaceShip_HEIGHT, SSD1306_WHITE);

        switch (gameState.load())
        {
            case GameState::START:
            {
                // if bullet hit the start button, start the game
                display.drawRoundRect(START_BUTTON_X, START_BUTTON_Y, START_BUTTON_WIDTH, START_BUTTON_HEIGHT, 3, SSD1306_WHITE);
                display.setCursor(START_BUTTON_X + 5, START_BUTTON_Y + 5);
                display.setTextSize(1);
                display.setTextColor(SSD1306_WHITE);
                display.println("Start");

                display.setCursor(0, 0);
                display.setTextSize(1);
                display.setTextColor(SSD1306_WHITE);
                display.println("Best: ");
                display.print(BestScore.load());

                break;
            }
            
            case GameState::PLAYING:
            {
                // Display kill count
                display.setCursor(0, 0);
                display.setTextSize(1);
                display.setTextColor(SSD1306_WHITE);
                display.println("Kills: ");
                display.print(KillCount.load());

                // render invaders
                if (xSemaphoreTake(xMutexInvaders, portMAX_DELAY) == pdTRUE)
                {
                    Serial.println("Render");
                    for (auto inv : Invaders)
                    {
                        display.drawBitmap(inv->getCoordX(), inv->getCoordY(), inv->getBitmap(), INVADER_WIDTH, INVADER_HEIGHT, SSD1306_WHITE);
                    }
                    xSemaphoreGive(xMutexInvaders);
                }

                break;
            }
            
            case GameState::GAMEOVER:
            {
                display.setCursor(HALF_SCREEN_WIDTH - 53, HALF_SCREEN_HEIGHT - 10);
                display.setTextSize(2);
                display.setTextColor(SSD1306_WHITE);
                display.println("Game Over");
                break;
            }

            default:
                break;
        }

        // render bullets
        if (xSemaphoreTake(xMutexBullets, portMAX_DELAY) == pdTRUE)
        {
            for (auto it = BulletsMap.begin(); it != BulletsMap.end(); it++)
            {
                for (auto bullet = it->second.begin(); bullet != it->second.end(); bullet++)
                {
                    display.drawPixel(bullet->getCoordX(), bullet->getCoordY(), SSD1306_WHITE);
                }
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
    const TickType_t xSpawnFrequency = pdMS_TO_TICKS(1500);
    const TickType_t xInvaderDelay = pdMS_TO_TICKS(60);

    int16_t diffculity = 3;
    int16_t bulletSpeed = 1;
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> diff_dist(0, diffculity);

    for (;;)
    {
        switch (gameState.load())
        {
            case GameState::START:
            {
                // if bullet hit the start button, start the game
                for (auto it = BulletsMap.begin(); it != BulletsMap.end(); it++)
                {
                    for (auto bullet = it->second.begin(); bullet != it->second.end(); bullet++)
                    {
                        if (bullet->getCoordX() >= START_BUTTON_X && bullet->getCoordX() <= START_BUTTON_X + START_BUTTON_WIDTH &&
                            bullet->getCoordY() >= START_BUTTON_Y && bullet->getCoordY() <= START_BUTTON_Y + START_BUTTON_HEIGHT)
                        {
                            // clean up bullets
                            for (auto it = BulletsMap.begin(); it != BulletsMap.end();)
                            {
                                it = BulletsMap.erase(it);
                            }
                            gameState.store(GameState::PLAYING);
                            break;
                        }
                    }
                }
                break;
            }
            
            case GameState::PLAYING:
            {
                // spawn invaders
                if (xTaskGetTickCount() - xlastSpawnTime > xSpawnFrequency)
                {
                    Serial.println("Spawn");
                    if (xSemaphoreTake(xMutexInvaders, portMAX_DELAY) == pdTRUE)
                    {
                        for (int i = 0; i < diff_dist(gen); i++)
                        {
                            Invaders.push_back(new Invader());
                        }
                        xSemaphoreGive(xMutexInvaders);
                    }
                    xlastSpawnTime = xTaskGetTickCount();
                }

                // update invaders position
                if (xTaskGetTickCount() - xlastInvaderTime > xInvaderDelay)
                {
                    Serial.println("Update invaders");
                    if (xSemaphoreTake(xMutexInvaders, portMAX_DELAY) == pdTRUE)
                    {
                        for (auto it = Invaders.begin(); it != Invaders.end();)
                        {
                            if ((*it)->getCoordY() >= SCREEN_HEIGHT - INVADER_HEIGHT)
                            {
                                /**
                                 * GAME OVER
                                 * 
                                 * clean up the invaders and bullets
                                 * reset the kill count
                                 * update the best score
                                 * set the game state to GAMEOVER
                                 */
                                delete *it;
                                it = Invaders.erase(it);
                                // clean up invaders
                                for (auto inv = Invaders.begin(); inv != Invaders.end();)
                                {
                                    delete *inv;
                                    inv = Invaders.erase(inv);
                                }
                                // clean up bullets
                                for (auto it = BulletsMap.begin(); it != BulletsMap.end();)
                                {
                                    it = BulletsMap.erase(it);
                                }
                                // set the best score
                                if (KillCount.load() > BestScore.load())
                                {
                                    BestScore.store(KillCount.load());
                                }
                                KillCount.store(0);
                                
                                gameState.store(GameState::GAMEOVER);
                                break;
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
                            auto invX = static_cast<uint32_t>((*inv)->getCoordX());
                            auto invY = static_cast<uint32_t>((*inv)->getCoordY());
                            auto invXBeg = invX - 1 >> GRID_SHIFT;
                            auto invXEnd = (invX + INVADER_WIDTH + 1) >> GRID_SHIFT;
                            if (BulletsMap.find(invXBeg) != BulletsMap.end())
                            {
                                for (auto bullet = BulletsMap[invXBeg].begin(); bullet != BulletsMap[invXBeg].end();)
                                {
                                    if (bullet->getCoordX() >= invX && bullet->getCoordX() <= invX + INVADER_WIDTH &&
                                        bullet->getCoordY() >= invY && bullet->getCoordY() <= invY + INVADER_HEIGHT)
                                    {
                                        bullet = BulletsMap[invXBeg].erase(bullet);
                                        delete *inv;
                                        inv = Invaders.erase(inv);
                                        KillCount.fetch_add(1);
                                        break; // break the inner loop since the invader is destroyed
                                    }
                                    else
                                    {
                                        bullet++;
                                    }
                                }
                                if (BulletsMap[invXBeg].empty())
                                {
                                    BulletsMap.erase(invXBeg);
                                }
                            }
                            else if (BulletsMap.find(invXEnd) != BulletsMap.end())
                            {
                                for (auto bullet = BulletsMap[invXEnd].begin(); bullet != BulletsMap[invXEnd].end();)
                                {
                                    if (bullet->getCoordX() >= invX && bullet->getCoordX() <= invX + INVADER_WIDTH &&
                                        bullet->getCoordY() >= invY && bullet->getCoordY() <= invY + INVADER_HEIGHT)
                                    {
                                        bullet = BulletsMap[invXEnd].erase(bullet);
                                        delete *inv;
                                        inv = Invaders.erase(inv);
                                        KillCount.fetch_add(1);
                                        break; // break the inner loop since the invader is destroyed
                                    }
                                    else
                                    {
                                        bullet++;
                                    }
                                }
                                if (BulletsMap[invXEnd].empty())
                                {
                                    BulletsMap.erase(invXEnd);
                                }
                            }
                            xSemaphoreGive(xMutexBullets);
                            
                        }
                        inv++;
                    }
                    Serial.println("Check collision end");
                    xSemaphoreGive(xMutexInvaders);
                }
                break;
            }
            
            case GameState::GAMEOVER:
            {
                if (xSemaphoreTake(xMutexBullets, portMAX_DELAY) == pdTRUE)
                {
                    // if bullet hit the buttom of GAMEOVER screen, restart the game
                    for (auto it = BulletsMap.begin(); it != BulletsMap.end(); it++)
                    {
                        for (auto bullet = it->second.begin(); bullet != it->second.end(); bullet++)
                        {
                            // only check for y coordinate
                            if (bullet->getCoordY() <= GAMEOVER_Y)
                            {
                                // clean up bullets
                                for (auto it = BulletsMap.begin(); it != BulletsMap.end();)
                                {
                                    it = BulletsMap.erase(it);
                                }
                                gameState.store(GameState::START);
                                break;
                            }
                        }
                    }
                    xSemaphoreGive(xMutexBullets);
                }
                break;
            }

            default:
                break;
        }

        // update bullets position
        if (xSemaphoreTake(xMutexBullets, portMAX_DELAY) == pdTRUE)
        {
            Serial.println("Update bullets");
            for (auto it = BulletsMap.begin(); it != BulletsMap.end();)
            {
                for (auto bullet = it->second.begin(); bullet != it->second.end();)
                {
                    if (bullet->getCoordY() <= 0)
                    {
                        bullet = it->second.erase(bullet);
                    }
                    else
                    {
                        bullet->moveUp(bulletSpeed);
                        bullet++;
                    }
                }
                if (it->second.empty())
                {
                    it = BulletsMap.erase(it);
                }
                else
                {
                    it++;
                }
            }
            xSemaphoreGive(xMutexBullets);
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
            if (xTaskGetTickCount() - lastDebounceTime > pdTICKS_TO_MS(100))
            {
                // Shoot
                if (xSemaphoreTake(xMutexBullets, portMAX_DELAY) == pdTRUE)
                {
                    auto playerX = Player->getCoordX();
                    auto playerY = Player->getCoordY();
                    BulletsMap[static_cast<uint32_t>(playerX + SpaceShip_WIDTH_HALF) >> GRID_SHIFT].push_back(Bullet(playerX + SpaceShip_WIDTH_HALF, playerY));
                    // Bullets.push_back(new Bullet(Player->getCoordX()+SpaceShip_WIDTH_HALF, Player->getCoordY()));
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
    KillCount.store(0);
    BestScore.store(0);

    setupFreeRTOS();    
}

void loop()
{
    // put your main code here, to run repeatedly:
}

