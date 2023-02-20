#ifndef CELL_APP_H
#define CELL_APP_H

#include "engine.h"

#include <ctime>

#include "renderer/master_renderer.h"
#include "renderer/world_renderer.h"
#include "renderer/drawable_world.h"
#include "materials/material_atlas.h"
#include "player/player.h"
#include "renderer/light_buffer.h"
#include "files/world_file.h"

namespace cell {

    class App : public undicht::Engine {
    protected:

        Player _player;

        MasterRenderer _master_renderer;
        DrawableWorld _world;
        //LightBuffer _lights;
        //DirectLight _sun;
        MaterialAtlas _materials;
        WorldFile _world_file;

    public:

        void init();
        void cleanUp();

        void mainLoop();
        void onWindowResize();

    };

} // namespace cell

#endif // CELL_APP_H