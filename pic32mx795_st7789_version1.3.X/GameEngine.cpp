/**
 * @file GameEngine.cpp
 * @brief Implementación del motor del juego.
 */

#include "GameEngine.h"
#include "PhysicsComponents.h" // (si se desea separar)
#include <cstdio>

namespace GameEngine {
    // Definición de arrays y variables
    Enemy enemies[8];
    Bullet bullets[5];
    uint8_t lives;
    uint32_t score;
    int16_t shipX, shipY;
    int8_t invincibleCounter;
    bool shipVisible;
    uint8_t lastFireState;

    // Función auxiliar para dibujar enemigos invertidos
    void drawEnemyShip(int16_t x, int16_t y, EnemyType type) {
        uint16_t body, cockpit, engine, wing, detail;
        switch(type) {
            case ENEMY_RED:    body = COLOR565(200,0,0); cockpit = COLOR565(255,100,100); engine = COLOR565(255,80,0); wing = COLOR565(150,0,0); detail = COLOR565(180,50,50); break;
            case ENEMY_GREEN:  body = COLOR565(0,180,0); cockpit = COLOR565(100,255,100); engine = COLOR565(0,200,0); wing = COLOR565(0,130,0); detail = COLOR565(80,200,80); break;
            case ENEMY_BLUE:   body = COLOR565(0,0,200); cockpit = COLOR565(100,100,255); engine = COLOR565(0,100,200); wing = COLOR565(0,0,150); detail = COLOR565(80,80,200); break;
            case ENEMY_ORANGE: body = COLOR565(255,100,0); cockpit = COLOR565(255,180,100); engine = COLOR565(255,140,0); wing = COLOR565(200,80,0); detail = COLOR565(255,150,80); break;
            default:           body = COLOR565(255,0,255); cockpit = COLOR565(255,150,255); engine = COLOR565(200,0,200); wing = COLOR565(180,0,180); detail = COLOR565(220,100,220); break;
        }
        // Dibujar invertida (flip vertical)
        Graphics::draw_ship_palette(x, y, body, cockpit, engine, wing, detail, BLACK, true);
    }

    void init(void) {
        for(int i=0; i<8; i++) enemies[i].active = false;
        for(int i=0; i<5; i++) bullets[i].active = false;
        lives = 3;
        score = 0;
        shipX = (TFT_W - SHIP_W) / 2;
        shipY = TFT_H - SHIP_H - 10;
        invincibleCounter = 0;
        shipVisible = true;
        lastFireState = 1;

        Graphics::fill_screen(BLACK);
        drawHUD();

#ifdef PRESENTACION
        Graphics::draw_string(55, 100, "SPACE", CYAN, BLACK, 3);
        Graphics::draw_string(40, 130, "SHOOTER", CYAN, BLACK, 3);
        Graphics::draw_string(30, 165, "PIC32MX795", COLOR565(100,100,255), BLACK, 2);
        delay_ms(2000);
        Graphics::fill_rect(0, 90, TFT_W, 100, BLACK);
#endif

        for(int i=0; i<3; i++) spawnEnemy();
    }

    void spawnEnemy(void) {
        for(int i=0; i<8; i++) {
            if(!enemies[i].active) {
                enemies[i].x = rand() % (TFT_W - SHIP_W);
                enemies[i].y = -SHIP_H;
                enemies[i].speed = 1 + (rand() % 3);
                enemies[i].type = (EnemyType)(rand() % NUM_TYPES);
                enemies[i].active = true;
                break;
            }
        }
    }

    void updateEnemies(void) {
        for(int i=0; i<8; i++) {
            if(enemies[i].active) {
                // Borrar completamente antes de mover (evita residuos)
                Graphics::erase_ship(enemies[i].x, enemies[i].y, BLACK);
                enemies[i].y += enemies[i].speed;
                if(enemies[i].y > TFT_H) {
                    enemies[i].active = false;
                    if(lives > 0) {
                        lives--;
                        drawHUD();
                        invincibleCounter = 60;
                    }
                } else {
                    drawEnemyShip(enemies[i].x, enemies[i].y, enemies[i].type);
                }
            }
        }
        static uint8_t spawnCounter = 0;
        if(++spawnCounter > 25) {
            spawnCounter = 0;
            if(rand() % 3 == 0) spawnEnemy();
        }
    }

    void updateBullets(void) {
        for(int i=0; i<5; i++) {
            if(bullets[i].active) {
                Graphics::draw_pixel(bullets[i].x, bullets[i].y, BLACK);
                bullets[i].y += bullets[i].speed;
                if(bullets[i].y < 0) bullets[i].active = false;
                else Graphics::draw_pixel(bullets[i].x, bullets[i].y, YELLOW);
            }
        }
    }

    void updateShip(void) {
        static int16_t prevShipX = -1, prevShipY = -1;
        static uint8_t auto_counter = 0;
        static int8_t auto_dir = 1;

#ifdef USE_BUTTONS
        static uint8_t leftPressed=0, rightPressed=0;
        if(BTN_LEFT) { if(!leftPressed) { shipX -= 8; leftPressed=1; } } else leftPressed=0;
        if(BTN_RIGHT) { if(!rightPressed) { shipX += 8; rightPressed=1; } } else rightPressed=0;
#else
        auto_counter++;
        if(auto_counter >= 4) { shipX += auto_dir * 4; auto_counter = 0; }
        if(shipX <= 5) auto_dir = 1;
        if(shipX >= TFT_W - SHIP_W - 5) auto_dir = -1;
#endif

        if(shipX < 2) shipX = 2;
        if(shipX > TFT_W - SHIP_W - 2) shipX = TFT_W - SHIP_W - 2;

#ifdef USE_BUTTONS
        if(!BTN_FIRE && lastFireState) {
            for(int i=0; i<5; i++) {
                if(!bullets[i].active) {
                    bullets[i].x = shipX + SHIP_W/2;
                    bullets[i].y = shipY - 2;
                    bullets[i].speed = -5;
                    bullets[i].active = true;
                    break;
                }
            }
        }
        lastFireState = BTN_FIRE;
#else
        static uint8_t fireCounter = 0;
        if(++fireCounter >= 20) {
            fireCounter = 0;
            for(int i=0; i<5; i++) {
                if(!bullets[i].active) {
                    bullets[i].x = shipX + SHIP_W/2;
                    bullets[i].y = shipY - 2;
                    bullets[i].speed = -5;
                    bullets[i].active = true;
                    break;
                }
            }
        }
#endif

        if(invincibleCounter > 0) {
            invincibleCounter--;
            shipVisible = (invincibleCounter & 4) ? true : false;
        } else shipVisible = true;

        if(shipX != prevShipX || shipY != prevShipY || (invincibleCounter > 0 && (invincibleCounter & 4) != ((invincibleCounter+1) & 4))) {
            if(prevShipX != -1 && prevShipY != -1) Graphics::erase_ship(prevShipX, prevShipY, BLACK);
            if(shipVisible) Graphics::draw_ship(shipX, shipY, BLACK);
            prevShipX = shipX;
            prevShipY = shipY;
        }
    }

    void checkCollisions(void) {
        // Colisiones bala-enemigo
        for(int b=0; b<5; b++) {
            if(!bullets[b].active) continue;
            for(int e=0; e<8; e++) {
                if(!enemies[e].active) continue;
                if(bullets[b].x >= enemies[e].x && bullets[b].x <= enemies[e].x+SHIP_W &&
                   bullets[b].y >= enemies[e].y && bullets[b].y <= enemies[e].y+SHIP_H) {
                    bullets[b].active = false;
                    enemies[e].active = false;
                    score += 10;
                    drawHUD();
                    drawExplosion(enemies[e].x + SHIP_W/2, enemies[e].y + SHIP_H/2);
                    break;
                }
            }
        }

        // Colisiones nave-enemigo
        if(invincibleCounter == 0) {
            for(int e=0; e<8; e++) {
                if(!enemies[e].active) continue;
                if(shipX < enemies[e].x+SHIP_W && shipX+SHIP_W > enemies[e].x &&
                   shipY < enemies[e].y+SHIP_H && shipY+SHIP_H > enemies[e].y) {
                    lives--;
                    drawHUD();
                    invincibleCounter = 60;
                    enemies[e].active = false;
                    drawExplosion(shipX + SHIP_W/2, shipY + SHIP_H/2);
                    if(lives == 0) {
                        Graphics::draw_string(TFT_W/2-60, TFT_H/2-10, "GAME OVER", RED, BLACK, 2);
                        while(1);
                    }
                    break;
                }
            }
        }
    }
/*
    void drawExplosion(int16_t x, int16_t y) {
        for(int r=1; r<8; r++) {
            Graphics::draw_circle(x, y, r, COLOR565(255,200,0));
            delay_us(50000);
        }
        for(int r=1; r<8; r++) {
            Graphics::draw_circle(x, y, r, BLACK);
        }
    }*/
    
    void drawExplosion(int16_t x, int16_t y) {
    for(int r=1; r<8; r++) {
        Graphics::draw_circle(x, y, r, YELLOW);   // YELLOW es 0xFFE0
        delay_us(50000);
    }
    for(int r=1; r<8; r++) {
        Graphics::draw_circle(x, y, r, BLACK);
    }
}

    void drawHUD(void) {
        Graphics::fill_rect(0, 0, TFT_W, 14, BLACK);
        for(uint8_t i=0; i<lives && i<5; i++) {
            int16_t lx = 4 + i*14;
            Graphics::draw_pixel(lx+3, 2, RED);
            Graphics::fill_rect(lx+1, 4, 6, 2, RED);
            Graphics::fill_rect(lx+2, 6, 4, 2, RED);
            Graphics::fill_rect(lx+3, 8, 2, 1, RED);
        }
        char buf[12];
        std::sprintf(buf, "%lu", score);
        Graphics::draw_string(TFT_W/2-30, 3, buf, WHITE, BLACK, 1);
        Graphics::draw_hline(0, 13, TFT_W, GRAY);
    }

    void gameLoop(void) {
        while(1) {
            updateShip();
            updateBullets();
            updateEnemies();
            checkCollisions();
            delay_ms(16);
        }
    }
}
