#include "app.h"

namespace cell {

    void App::init() {

        undicht::Engine::init();

        _world_renderer.init(_gpu, _swap_chain, _default_render_pass);

        _player.init();
    }

    void App::cleanUp() {
        
        _player.cleanUp();
        
        _world_renderer.cleanUp();

        undicht::Engine::cleanUp();

    }

    void App::mainLoop() {

        _player.move();

        _world_renderer.loadCamera(_player);

    }

    
} // cell