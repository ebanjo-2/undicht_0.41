#ifndef CELL_APP_H
#define CELL_APP_H

#include "engine.h"
#include "renderer/world_renderer.h"
#include "player/player.h"

namespace cell {

    class App : public undicht::Engine {
    protected:

        Player _player;

        WorldRenderer _world_renderer;

    public:

        void init();
        void cleanUp();

        void mainLoop();

    };

} // namespace cell

#endif // CELL_APP_H