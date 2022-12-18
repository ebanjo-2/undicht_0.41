#include "iostream"
#include "tonk_game.h"

int main() {

    tonk::TonkGame game;
    game.init();
    game.run();
    game.cleanUp();

    return 0;
}