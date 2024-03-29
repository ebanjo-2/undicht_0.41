#ifndef CELL_APP_H
#define CELL_APP_H

#include "engine.h"

#include <ctime>

#include "renderer/master_renderer.h"
#include "world/world_loader.h"
#include "player/player.h"
#include "user_interface/debug/debug_menu.h"
#include "world/edit/world_edit.h"

namespace cell {

    class App : public undicht::Engine {
    protected:

        Player _player;

        MasterRenderer _master_renderer;
        DebugMenu _debug_menu;

        WorldLoader _world_loader;
        DrawableWorld _world;
        WorldEdit _world_edit;

        double _last_edit = 0.0;


    public:

        void init();
        void cleanUp();

        void mainLoop();
        void onWindowResize();

    };

} // namespace cell

#endif // CELL_APP_H