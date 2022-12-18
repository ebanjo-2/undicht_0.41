#ifndef TONK_GAME_H
#define TONK_GAME_H

#include "engine.h"
#include "renderer.h"
#include "map/tile_map.h"
#include "map/map.h"
#include "glm/glm.hpp"
#include "map/map_generator.h"
#include "map/tile_set.h"
#include "ui/dev_ui.h"

namespace tonk {

    class TonkGame : public undicht::Engine {

    protected:

        Renderer _renderer;
        TileMap _tile_map;
        TileSet _tile_set;
        Map _map;
        MapGenerator _map_generator;

        ImTextureID _imgui_tile_map_handle;

        DevUI _dev_ui;

    public:

        void init();
        void mainLoop();
        void cleanUp();
        void onWindowResize();

    protected:
        // processing user input

        glm::vec2 _map_center = glm::vec2(0.0f, 0.0f);

        void processUserInput();

    protected:
        // debug menu

        bool _map_complete = false;

        void debugMenu();
    };

} // tonk

#endif // TONK_GAME_H