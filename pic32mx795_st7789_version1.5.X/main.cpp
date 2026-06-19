#include "HardwareProfile.h"
#include "Graphics.h"
#include "GameEngine.h"
#include "Sound.h"

int main(void) {
    BL_TRIS=0; DC_TRIS=0; RST_TRIS=0;
    DC_LAT=0; RST_LAT=1; BL_LAT=0;

    spi4_init();
    init_display();
    BL_LAT = 1;

    lcg_srand(_CP0_GET_COUNT());
    sound_init();

    GameEngine::game_loop();

    return 0;
}
