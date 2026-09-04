#include "HardwareProfile.h"
#include "Graphics.h"
#include "GameEngine.h"
#include "Sound.h"

void show_title(void) {
    Graphics::fill_screen(BLACK);
    Graphics::draw_string(50, 80, "PACMAN", YELLOW, BLACK, 3);
    Graphics::draw_string(30, 120, "PIC32MX795", CYAN, BLACK, 2);
    Graphics::draw_string(40, 160, "STARTING...", WHITE, BLACK, 1);
    delay_ms(900);
    Graphics::fill_screen(BLACK);
}

int main(void) {
    BL_TRIS=0; DC_TRIS=0; RST_TRIS=0;
    DC_LAT=0; RST_LAT=1; BL_LAT=0;

    spi4_init();
    init_display();
    BL_LAT=1;

    lcg_srand(_CP0_GET_COUNT());
    sound_init();

    show_title();
    sound_start();
    GameEngine::draw_maze();
    GameEngine::draw_status();
    GameEngine::game_loop();
    return 0;
}
