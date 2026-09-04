/**
 * @file GameEngine.h
 * @brief Motor del juego: entidades, lógica, colisiones.
 */

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "HardwareProfile.h"
#include "Graphics.h"

namespace GameEngine {
    // Tipos de enemigos
    enum EnemyType {
        ENEMY_RED,
        ENEMY_GREEN,
        ENEMY_BLUE,
        ENEMY_ORANGE,
        ENEMY_PURPLE,
        NUM_TYPES
    };

    struct Enemy {
        int16_t x, y;
        int8_t speed;
        bool active;
        EnemyType type;
    };

    struct Bullet {
        int16_t x, y;
        int8_t speed;
        bool active;
    };

    extern Enemy enemies[8];
    extern Bullet bullets[5];
    extern uint8_t lives;
    extern uint32_t score;
    extern int16_t shipX, shipY;
    extern int8_t invincibleCounter;
    extern bool shipVisible;

    void init(void);
    void spawnEnemy(void);
    void updateEnemies(void);
    void updateBullets(void);
    void updateShip(void);
    void checkCollisions(void);
    void drawHUD(void);
    void drawExplosion(int16_t x, int16_t y);
    void gameLoop(void);
}

#endif // GAME_ENGINE_H
