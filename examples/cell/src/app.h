#ifndef CELL_APP_H
#define CELL_APP_H

#include "engine.h"

#include <ctime>

#include "renderer/master_renderer.h"
#include "world/world_loader.h"
#include "player/player.h"
#include "user_interface/debug/debug_menu.h"

namespace cell {

    class App : public undicht::Engine {
    protected:

        Player _player;

        MasterRenderer _master_renderer;
        DebugMenu _debug_menu;

        WorldLoader _world_loader;

        /*DrawableWorld _world;
        WorldFile _world_file;
        EnvironmentGenerator _env_gen;*/

    public:

        void init();
        void cleanUp();

        void mainLoop();
        void onWindowResize();

    };

} // namespace cell

#endif // CELL_APP_H