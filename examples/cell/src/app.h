#ifndef CELL_APP_H
#define CELL_APP_H

#include "engine.h"

#include <ctime>

#include "renderer/master_renderer.h"
#include "renderer/world_renderer.h"
#include "renderer/drawable_world.h"
#include "player/player.h"

namespace cell {

    class App : public undicht::Engine {
    protected:

        Player _player;

        MasterRenderer _master_renderer;
        DrawableWorld _world;

    public:

        void init();
        void cleanUp();

        void mainLoop();
        void onWindowResize();

    };

} // namespace cell

#endif // CELL_APP_H