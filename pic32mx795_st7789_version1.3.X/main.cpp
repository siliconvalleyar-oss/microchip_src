/**
 * @file main.cpp
 * @brief Punto de entrada del juego Space Shooter v1.3.
 *        Usa ST7789_common/ como driver base.
 */

#include "HardwareProfile.h"
#include "Graphics.h"
#include "GameEngine.h"

// ==================== MAIN ====================
int main(void) {
    BL_TRIS=0; DC_TRIS=0; RST_TRIS=0;
    DC_LAT=0; RST_LAT=1;
    BL_LAT=0;

    spi4_init();
    init_display();
    BL_LAT=1;

    lcg_srand(_CP0_GET_COUNT());

    GameEngine::init();
    GameEngine::gameLoop();

    return 0;
}
